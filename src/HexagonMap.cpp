//
//  HexagonMap.cpp
//
//
//  Created by enrico on 29/08/17.
//
//

// TODO rotate hexagons
// TDOD add visuals/scenes
// TODO add fuckups

#include "HexagonMap.hpp"

HexagonMap::HexagonMap()
{
    fuckUpCounter = 0;
    nextFuckUpAt = ofRandom(MIN_FUCK_UP,MAX_FUCK_UP);
    
#ifdef TARGET_OPENGLES
    shader.load("shadersES2/stripes");
#else
    shader.load("shadersGL3/stripes");
#endif
    
    loadScenes();
    load();

    // FOR projector
    projector.setup();
    string myProjectorIP = "192.168.20.10";
    
    //Plug an ethernet into your projector and check it settings to find this IP
    projector.setProjectorIP(myProjectorIP);
    
    //You can set a password on your projector to access PJ Link
    projector.setProjectorPassword("abcde");
    projector.setProjectorType(PJLINK_MODE);

    
    
    isEditMode = false;
    inPreviewMode = false;
    showTest = false;
    receiver.setup(20000);
    nextTimeEvent = ofGetElapsedTimef() + 10;
    
    activeHexagon = 0;
    activeVertex = 0;
    
    createNewSetting();
}

// Add a Hexagon from an OSC message
void HexagonMap::addHexagon(ofxOscMessage &message)
{
    float value = message.getArgAsFloat(0);

    // Limit to 4 Hexagons and only add when value is 1 --> OSCTouch gives two events 
    if(value == 1 && hexagons.size() < MAX_HEXAGONS){
        
        ofMesh m;
        m.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
        ofVec3f c = ofVec3f(ofGetWidth() / 2,ofGetHeight() /2,0);
        m.addVertex(c);
        m.addIndex(0);
        for (int i = 0; i < 7; i++)
        {
            int rad = 100;
            int x = c.x + rad * cos((i / 6.) * TWO_PI);
            int y = c.y + rad * sin((i / 6.) * TWO_PI);
            m.addVertex(ofVec3f(x,y,0));
            m.addIndex(i+1);
        }
        addTexCoords(m);
        hexagons.push_back(m);
        
        HexagonSetting newSetting;
        newSetting.direction = floor(ofRandom(2));
        newSetting.color = ofColor(ofRandom(128,255),ofRandom(128,255),ofRandom(128,255));
        newSetting.drawingID = ofRandom(DRAWING::SIZE);
        newSetting.isMuted = false;
        hexSettings.push_back(newSetting);
    }
}

// Add a Hexagon from an xmlsettings
void HexagonMap::addHexagon(ofVec3f * verts,int length)
{
    ofMesh m;
    m.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
    for (int i = 0; i < length; i++)
    {
        m.addVertex(verts[i]);
        m.addIndex(i);

    }
    addTexCoords(m);

    hexagons.push_back(m);
    
    HexagonSetting newSetting;
    newSetting.direction = 0;
    newSetting.color = ofColor(255,255,255);
    newSetting.drawingID = 0;
    newSetting.xandy = 0;
    newSetting.tiles = 5.0;
    newSetting.isMuted = false;
    hexSettings.push_back(newSetting);
}

// add texture coordinates
void HexagonMap::addTexCoords(ofMesh &m)
{
    // make a bounding box and calculate the texcoords
    m.clearTexCoords();
    float minX = m.getCentroid().x;
    float maxX = m.getCentroid().x;
    float minY = m.getCentroid().y;
    float maxY = m.getCentroid().y;
    for (int i = 0; i < m.getNumVertices(); i++)
    {
        ofVec2f p = m.getVertices()[i];
        if (p.x < minX) minX = p.x;
        if (p.x > maxX) maxX = p.x;
        if (p.y < minY) minY = p.y;
        if (p.y > maxY) maxY = p.y;
    }
    for (int i = 0; i < m.getNumVertices(); i++)
    {
        ofVec2f p = m.getVertices()[i];
        float tx = ofMap(p.x, minX, maxX, 0, 1);
        float ty = ofMap(p.y, minY, maxY, 0, 1);
        m.addTexCoord(ofVec2f(tx,ty));
    }
}

// proces OSC messages
void HexagonMap::update()
{
    while (receiver.hasWaitingMessages())
    {
        ofxOscMessage m;
        receiver.getNextMessage(m);
        string label = m.getAddress();
        
        cout << "OSC MESSAGE RECIEVED: " << label << endl;
        
        if (m.getAddress() == "/mute") muteHexagon(m);
        else if (m.getAddress() == "/edit") setEditMode(m);
        else if (m.getAddress() == "/save") save(m);
        else if (m.getAddress() == "/revert") revert(m);
        else if (m.getAddress() == "/scene") receiveSceneOSC(m);
        else if (m.getAddress() == "/preview") loadPreviewSceneOSC(m);
        else if (m.getAddress() == "/test") showTheTest(m);
        else if (label == "/up" || label == "/down" || label == "/left" || label == "/right") movePoint(m);
        else if (label == "/poweron"){
            float value = m.getArgAsFloat(0);
            if(value == 1) {
                projector.On();
                cout << projector.getProjectorStatus() << endl;
            }
        }
        else if (label == "/poweroff"){
            float value = m.getArgAsFloat(0);
            if(value == 1) {
                projector.Off();
                cout << projector.getProjectorStatus() << endl;
            }
        }
        
        // SET active hexagon
        for(int i=0;i<MAX_HEXAGONS;i++){
            string messageToCheck = "/hexagon/1/"+ofToString(i+1);
            if (m.getAddress() == messageToCheck) setActiveHexagon(i);
        }
        
        // SET active hexagon point
        // the 2 offset is because we also use scale/move as index
        for(int i=0;i<NUM_VERTICES+2;i++){
            string messageToCheck = "/point/1/"+ofToString(i+1);
            if (m.getAddress() == messageToCheck) setActiveVertex(i);
        }
      }
    // if not in editmode check if we go to th enext setting
    if (!isEditMode)
    {
        float now = ofGetElapsedTimef();
        if (now > nextTimeEvent)
        {
            createNewSetting();
            nextTimeEvent = now + ofRandom(globalSpeedLow,globalSpeedHigh);
        }
    }
}

// Change Hexagon visual settings
void HexagonMap::createNewSetting()
{
    // select a scene and set the hexagons, also count the number of settings then mak one crazy after 10-20, by now only the speed, but could be also something else
    
    // no scenes? do something random
    if (scenes.size() == 0) //random
    {
        for (int i = 0; i < hexSettings.size(); i++)
        {
            hexSettings[i].direction = ofRandom(2);
            hexSettings[i].color = ofColor(ofRandom(128,255),ofRandom(128,255),ofRandom(128,255));
            hexSettings[i].drawingID = ofRandom(DRAWING::SIZE);
        }
        return;
    }
    fuckUpCounter++;
    int fucked = -1;
    if (fuckUpCounter == nextFuckUpAt)
    {
        //time to change one for one drawing period
        fucked = ofRandom(hexagons.size());
        nextFuckUpAt = ofRandom(MIN_FUCK_UP,MAX_FUCK_UP);
        fuckUpCounter = 0;
    }
    
    actualScene = ofRandom(scenes.size());
    ofLogVerbose("New scene is: "+ofToString(actualScene));
    setScene(actualScene, fucked);
}

/* 
 There are three different possibilities for movePoint
 - scale the form (0 point or a in OSC GUI)
 - move the whole shape (1 point or b in OSC GUI)
 - move point of shape ( 2,3,4,5,6,7,8 or c,d,e,f,g,h,i in OSC GUI)

*/
void HexagonMap::movePoint(ofxOscMessage &m)
{
    float value = m.getArgAsFloat(0);
    
    if (!isEditMode || value == 0) return;
    
        // get label to gfigure out which command was sent
        string label = m.getAddress();
        float amount = m.getArgAsFloat(0);
        
        ofVec3f shift = ofVec3f(0,0,0);
        if(label == "/up")          shift.y = -amount;
        else if(label == "/down")   shift.y =  amount;
        else if(label == "/left")   shift.x = -amount;
        else if(label == "/right")  shift.x =  amount;
        
        ofLogVerbose("MOVE vertex: "+ofToString(activeVertex));
        
        // Safety
        if (activeHexagon >= hexagons.size() || activeVertex > 9) return;
        
        // SCALE IF a is selected from gui we scale the whole hexagon
        if(activeVertex == 0){
            ofLogVerbose("We should Scale the whole hexagon( "+ofToString(hexagons[activeHexagon].getNumVertices())+" )");
            
            int scaleAmount = 0;
            if(shift.x == 0)        scaleAmount = int(shift.y);
            else if(shift.y == 0)   scaleAmount = int(shift.x);
        
            ofVec2f centerPoint = hexagons[activeHexagon].getVertices()[0];
            for (int i = 1; i < hexagons[activeHexagon].getNumVertices(); ++i)
            {
                ofVec2f point = hexagons[activeHexagon].getVertices()[i];
                int dist = int(centerPoint.distance(point));
                int rad = ofClamp(dist + scaleAmount,20,ofGetWidth());
                
                int x = centerPoint.x + rad * cos((i / 6.) * TWO_PI);
                int y = centerPoint.y + rad * sin((i / 6.) * TWO_PI);
                
                hexagons[activeHexagon].getVertices()[i].x = x;
                hexagons[activeHexagon].getVertices()[i].y = y;
            }
        }
        // MOVE ALL IF b is selected from gui we move the whole hexagon
        else if(activeVertex == 1){
            ofLogVerbose("We should Move the whole hexagon");
            for (int i = 0; i < hexagons[activeHexagon].getNumVertices(); ++i)
            {
                hexagons[activeHexagon].getVertices()[i] += shift;
            }
        }
        // MOVE POINT ELSE we move point of hexagon
        // since we use 0 ad 1 for move and scale we need to do -2 to get the correct vertex.
        // vertex 2 is Midle point
        // vertex 3 should move with -- vertex 9
        else{
            ofLogVerbose("We should Move");
            if(activeVertex == 3){ // move two points so the shape stays closed
                ofLogVerbose("We should Move 2 points");
                hexagons[activeHexagon].getVertices()[1] += shift;
                hexagons[activeHexagon].getVertices()[7] += shift;
            }else{
                hexagons[activeHexagon].getVertices()[activeVertex-2] += shift;
            }
            
        }
}

// TOGGLE edit mode
void HexagonMap::setEditMode(ofxOscMessage &m)
{
    float value = m.getArgAsFloat(0);
    if(value == 1) {
    
        isEditMode = ! isEditMode;
        save();
        for (int i = 0; i < hexagons.size(); i++)
        {
            addTexCoords(hexagons[i]);
            // save current center points for aal hexagons voor revert
            hexCenters[i] = hexagons[i].getVertices()[0];
        }
    }
}

// Set the active HEXAGON (the one to edit)
void HexagonMap::setActiveHexagon(int i){
    activeHexagon = i;
}

// Set the active POINT of Hexagon (the one to edit)
void HexagonMap::setActiveVertex(int i){
    activeVertex = i;
    ofLogVerbose("Set active vertex: "+ofToString(i));
}

// set the scene
void HexagonMap::setScene(int actualScene, int fucked)
{
    //cout << "setScene! " << endl;
    for (int i = 0; i < hexSettings.size(); i++)
    {
        hexSettings[i].direction = scenes[actualScene].direction;
        hexSettings[i].color = scenes[actualScene].color;
        hexSettings[i].drawingID = scenes[actualScene].drawingID;
        hexSettings[i].stripeWidth = scenes[actualScene].stripeWidth;
        hexSettings[i].xandy = scenes[actualScene].xandy;
        hexSettings[i].tiles = scenes[actualScene].tiles;
        //cout << "setScene " << i << " : stripeWidth: " << hexSettings[i].stripeWidth << endl;
        hexSettings[i].speed = scenes[actualScene].speed;
        if(fucked == i) hexSettings[i].speed = scenes[actualScene].speed * -1;
    }
}

// receive scene to show from OSC
void HexagonMap::receiveSceneOSC(ofxOscMessage &m)
{
    int scene = round(m.getArgAsFloat(0));
    if(scene != actualScene){
        ofLogVerbose("Scene recieved: "+ofToString(scene));
        setScene(scene, -1);
    }
}

// load preview scene on recieve message from OSC
void HexagonMap::loadPreviewSceneOSC(ofxOscMessage &m)
{
    int scene = round(m.getArgAsFloat(0));
    if(scene != actualScene){
        ofLogVerbose("Load previewScene");
        loadPreviewScene();
    }
    
}


// Mute the hexagon (Make in invidible)
void HexagonMap::muteHexagon(ofxOscMessage &m)
{
    float value = m.getArgAsFloat(0);
    if(value == 1 && isEditMode == true) {
        hexSettings[activeHexagon].isMuted = ! hexSettings[activeHexagon].isMuted;
    }
}

void HexagonMap:: showTheTest(ofxOscMessage &m){
    float value = m.getArgAsFloat(0);
    if(value == 1) {
        
        showTest = ! showTest;
        cout << "show test: " << showTest << endl;
    }
}

// Revert the edited hexagon
void HexagonMap::revert(ofxOscMessage &m)
{
    float value = m.getArgAsFloat(0);
    
    if(value == 1 && isEditMode == true) {
        
        ofVec3f centerPoint = hexCenters[activeHexagon];
        hexagons[activeHexagon].getVertices()[0] = centerPoint;
        for (int i = 1; i < hexagons[activeHexagon].getNumVertices(); ++i)
        {
            ofVec2f point = hexagons[activeHexagon].getVertices()[i];
            int rad = 100;
            
            int x = centerPoint.x + rad * cos((i / 6.) * TWO_PI);
            int y = centerPoint.y + rad * sin((i / 6.) * TWO_PI);
            
            hexagons[activeHexagon].getVertices()[i].x = x;
            hexagons[activeHexagon].getVertices()[i].y = y;
        }
    }
}

// DRAW the hexagon
void HexagonMap::draw()
{
    // Loop through the hexagons
    if(!showTest){
        ofSetBackgroundColor(0);
        for (int i = 0; i < hexagons.size(); i++){
            
            // CHECK if they are not muted
            if(!hexSettings[i].isMuted){
        
                // EDIT MODE
                if (isEditMode)
                {
                    // draw the active one solid (with a cross for the center)
                    if(i == activeHexagon){
                        ofSetColor(255);

                        hexagons[i].draw();
                        
                        // calculate the average distance of points from the centre
                        int dist = 0;
                        ofVec2f centerPoint = hexagons[i].getVertices()[0];
                        for(int j = 1; j < hexagons[i].getNumVertices(); j++){
                            ofVec2f point = hexagons[i].getVertices()[j];
                            dist += int(centerPoint.distance(point)/1.8);
                        }
                        dist = dist / (hexagons[i].getNumVertices()-1);

                        ofSetLineWidth(8);
                        ofSetColor(255,0,0);
                        ofDrawLine(centerPoint.x-dist,centerPoint.y,centerPoint.x+dist,centerPoint.y);
                        ofDrawLine(centerPoint.x,centerPoint.y-dist,centerPoint.x,centerPoint.y+dist);
                    }
                    // and the others with shader
                    else{
                        ofSetColor(255,255,255);
                        drawSingleHexagon(i);
                    }
                }
                // REGULAR DRAW MODE
                else // draw with shader
                {
                    drawSingleHexagon(i);
                }
            }
        }
    }
    else{
        // Draw white square
        ofSetBackgroundColor(255);
    }
    
    
}

// DRAW a single HEXAGON with shader
void HexagonMap::drawSingleHexagon(int & id)
{
    float scale = ofNoise(ofGetElapsedTimef() + id);
    
    //ofColor a = ofColor(hexSettings[id].color.r-20,hexSettings[id].color.g-20,hexSettings[id].color.b-20);
    ofColor b = ofColor(0);
    ofColor color = b.lerp(hexSettings[id].color, ofNoise(ofGetElapsedTimef()));
    
    
    switch (hexSettings[id].drawingID)
    {
        // stripes back&forth
        case DRAWING::NORMAL:
            ofSetColor(hexSettings[id].color);
            shader.begin();
            shader.setUniform1f("phase", ofNoise(ofGetElapsedTimef() * hexSettings[id].speed));
            shader.setUniform1i("direction", hexSettings[id].direction);
            shader.setUniform1f("stripeWidth", hexSettings[id].stripeWidth);
            shader.setUniform1i("xandy", hexSettings[id].xandy);
            shader.setUniform1f("tiles", hexSettings[id].tiles);
            hexagons[id].draw();
            shader.end();
            break;
            
        // stripes and moving
        case DRAWING::FORWARD:
            ofSetColor(hexSettings[id].color);
            shader.begin();
            shader.setUniform1f("phase", ofGetElapsedTimef() * hexSettings[id].speed);
            shader.setUniform1i("direction", hexSettings[id].direction);
            shader.setUniform1f("stripeWidth", hexSettings[id].stripeWidth);
            shader.setUniform1i("xandy", hexSettings[id].xandy);
            shader.setUniform1f("tiles", hexSettings[id].tiles);
            hexagons[id].draw();
            shader.end();
            break;
            
        // stripes & scaling
        case DRAWING::SCALED:
            ofSetColor(hexSettings[id].color);
            shader.begin();
            shader.setUniform1f("phase", ofNoise(ofGetElapsedTimef()));
            shader.setUniform1i("direction", hexSettings[id].direction);
            shader.setUniform1f("stripeWidth", hexSettings[id].stripeWidth);
            shader.setUniform1i("xandy", hexSettings[id].xandy);
            shader.setUniform1f("tiles", hexSettings[id].tiles);
            ofPushMatrix();
            ofTranslate(hexagons[id].getCentroid());
            ofScale(scale,scale);
            ofTranslate(-hexagons[id].getCentroid());
            hexagons[id].draw();
            ofPopMatrix();
            shader.end();
            break;
            
        // solid
        case DRAWING::COLORED:
      
            ofSetColor(color); // changes with lerp
            hexagons[id].draw();
            break;
            
        // solid & scaling
        case DRAWING::SCALED_COLORED:
            ofSetColor(hexSettings[id].color);
            ofPushMatrix();
            ofTranslate(hexagons[id].getCentroid());
            ofScale(scale,scale);
            ofTranslate(-hexagons[id].getCentroid());
            hexagons[id].draw();
            ofPopMatrix();
            break;
        case DRAWING::ROTATE:
            ofSetColor(hexSettings[id].color);
            ofPushMatrix();
            ofRotate(ofNoise(ofGetElapsedTimef()));
            shader.begin();
            shader.setUniform1f("phase", ofNoise(ofGetElapsedTimef() * hexSettings[id].speed));
            shader.setUniform1i("direction", hexSettings[id].direction);
            shader.setUniform1f("stripeWidth", hexSettings[id].stripeWidth);
            hexagons[id].draw();
            shader.end();
            ofPopMatrix();
            break;

            
        default:
            break;
    }

}

// LOAD hexagon settings from xmlsettings
void HexagonMap::load()
{
    ofxXmlSettings settings;
    settings.load("settings.xml");
    for (int id = 0; id < settings.getNumTags("hex"); id++)
    {
        settings.pushTag("hex",id);
        bool muted = settings.getValue("isMuted", false);
        int pCount = settings.getNumTags("p");
        ofVec3f points[8];
        for (int i = 0; i < pCount; i++)
        {
            settings.pushTag("p",i);
            points[i].x = settings.getValue("px", 0);
            points[i].y = settings.getValue("py", 0);
            settings.popTag();
        }
        settings.popTag();
        addHexagon(points, 8);
        hexSettings.back().isMuted = muted;
        // get center of hex and push back in the array
        hexCenters.push_back(points[0]);
    }
}

// LOAD SCENE from xmlsettings
void HexagonMap::loadScenes()
{
    ofxXmlSettings settings;
    settings.load("scenes-raspberry.xml");
    globalSpeedLow = settings.getValue("gloabalSpeedLow", MIN_TIME);
    globalSpeedHigh = settings.getValue("gloabalSpeedHigh", MAX_TIME);
    for (int i = 0; i < settings.getNumTags("scene"); i++)
    {
        Scene newScene;
        settings.pushTag("scene",i);
        newScene.drawingID = settings.getValue("drawingID", 0);
        newScene.stripeWidth = settings.getValue("stripeWidth", 0.5); // percent 0-1
        newScene.direction = settings.getValue("direction", 1);
        newScene.xandy = settings.getValue("xandy", 1);
        newScene.tiles = settings.getValue("tiles", 1);
        int r = settings.getValue("colorR", 255);
        int g = settings.getValue("colorG", 255);
        int b = settings.getValue("colorB", 255);
        newScene.speed = settings.getValue("speed", 0.5);
        newScene.color = ofColor(r,g,b);
        settings.popTag();
        scenes.push_back(newScene);
    }
    actualScene = ofRandom(scenes.size());
    cout << "actualScene : " << actualScene << endl;
    setScene(actualScene, -1);
    
    //  createNewSetting();
}

// load preview scen to try out visual settings
void HexagonMap::loadPreviewScene()
{
    ofLogVerbose("Loading Preview Scene");
    
    ofxXmlSettings settings;
    settings.load("previewScene.xml");
    Scene newScene;
    
    // get settings form preview scene
    for (int i = 0; i < settings.getNumTags("scene"); i++)
    {
        settings.pushTag("scene",i);
        newScene.drawingID = settings.getValue("drawingID", 0);
        newScene.stripeWidth = settings.getValue("stripeWidth", 0.5); // percent 0-1
        newScene.direction = settings.getValue("direction", 1);
        newScene.xandy = settings.getValue("xandy", 1);
        newScene.tiles = settings.getValue("tiles", 1);
        int r = settings.getValue("colorR", 255);
        int g = settings.getValue("colorG", 255);
        int b = settings.getValue("colorB", 255);
        newScene.speed = settings.getValue("speed", 0.5);
        newScene.color = ofColor(r,g,b);
        settings.popTag();
    }
    
    cout << newScene.drawingID << " : "  << newScene.stripeWidth << " : "  << newScene.speed << " : "  << newScene.color << endl;
    
    // set settings from preview scene
    for (int i = 0; i < hexSettings.size(); i++)
    {
        //cout << "hexsettings: " << i << endl;
        hexSettings[i].direction = newScene.direction;
        hexSettings[i].color = newScene.color;
        hexSettings[i].drawingID = newScene.drawingID;
        hexSettings[i].stripeWidth = newScene.stripeWidth;
        hexSettings[i].speed = newScene.speed;
        hexSettings[i].xandy = newScene.xandy;
        hexSettings[i].tiles = newScene.tiles;
    }
}



// Save function wrapper to be able to check for Touch OSC '1' value
void HexagonMap::save(ofxOscMessage &m)
{
    float value = m.getArgAsFloat(0);
    if(value == 1) {
        save();
    }
}

// save hexagon settings to xml
void HexagonMap::save()
{
    ofxXmlSettings settings;
    for (int id = 0; id < hexagons.size(); id++)
    {
        settings.addTag("hex");
        settings.pushTag("hex",id);
        settings.addValue("isMuted", hexSettings[id].isMuted);
        for (int i = 0; i < hexagons[id].getNumVertices(); i++)
        {
            settings.addTag("p");
            settings.pushTag("p",i);
            settings.addValue("px",hexagons[id].getVertices()[i].x);
            settings.addValue("py",hexagons[id].getVertices()[i].y);
            settings.popTag();
        }
        
        settings.popTag();
    }
    settings.save("settings.xml");
}

// no (yet) used
void HexagonMap::togglePreviewMode(){
    inPreviewMode = ! inPreviewMode;
}



