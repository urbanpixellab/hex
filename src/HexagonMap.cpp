//
//  HexagonMap.cpp
//
//
//  Created by enrico on 29/08/17.
//
//

#include "HexagonMap.hpp"

HexagonMap::HexagonMap()
{
    load();
    
    
#ifdef TARGET_OPENGLES
    shader.load("shadersES2/stripes");
#else
    shader.load("shadersGL3/stripes");
#endif

    //shader.load("stripes.vert","stripes.frag");
    isEditMode = false;
    receiver.setup(20000);
    nextTimeEvent = ofGetElapsedTimef() + 10;
    
    activeHexagon = 0;
    activeVertex = 0;
}

void HexagonMap::addHexagon(ofxOscMessage &message)
{
    float value = message.getArgAsFloat(0);

    // Limit to 4 Hexagons and only add when value is 1 --> OSCTouch gives two events 
    if(value == 1 && hexagons.size() < MAX_HEXAGONS){
        
        ofMesh m;
        m.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
        ofVec2f c = ofVec2f(ofGetWidth() / 2,ofGetHeight() /2);
        m.addVertex(c);
        for (int i = 0; i < 7; i++)
        {
            int rad = 100;
            int x = c.x + rad * cos((i / 6.) * TWO_PI);
            int y = c.y + rad * sin((i / 6.) * TWO_PI);
            m.addVertex(ofVec2f(x,y));
        }
        addTexCoords(m);
        hexagons.push_back(m);
        
        HexagonSetting newSetting;
        newSetting.direction = floor(ofRandom(2));
        newSetting.color = ofColor(ofRandom(128,255),ofRandom(128,255),ofRandom(128,255));
        newSetting.eventID = ofRandom(EVENTS::SIZE);
        newSetting.isMuted = false;
        hexSettings.push_back(newSetting);
    }
}

void HexagonMap::addHexagon(ofVec2f * verts,int length)
{
    ofMesh m;
    m.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
    for (int i = 0; i < length; i++)
    {
        m.addVertex(verts[i]);
    }
    addTexCoords(m);

    hexagons.push_back(m);
    HexagonSetting newSetting;
    newSetting.direction = floor(ofRandom(2));
    newSetting.color = ofColor(ofRandom(128,255),ofRandom(128,255),ofRandom(128,255));
    newSetting.eventID = ofRandom(EVENTS::SIZE);
    newSetting.isMuted = false;
    hexSettings.push_back(newSetting);
}

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
        else if (label == "/up" || label == "/down" || label == "/left" || label == "/right") movePoint(m);
        
        // SET active hexagon
        for(int i=0;i<MAX_HEXAGONS;i++){
            string messageToCheck = "/hexagon/1/"+ofToString(i+1);
            
            if (m.getAddress() == messageToCheck){
                setActiveHexagon(i);
            }
        }
        
        // SET active hexagon
        for(int i=0;i<NUM_VERTICES;i++){
            string messageToCheck = "/point/1/"+ofToString(i+1);
            
            if (m.getAddress() == messageToCheck){
                setActiveVertex(i);
            }
        }
        
        
        
        /*
         OSC Commands
         /left 1 5 25 125
         /right 1 5 25 125
         /up 1 5 25 125
         /down 1 5 25 125 
         
         /hexagon/1/1 /hexagon/1/2
         
         /point/1/1 /point/1/2
         
         /edit
         
         /save
         */




    }
    if (!isEditMode)
    {
        float now = ofGetElapsedTimef();
        if (now > nextTimeEvent)
        {
            createNewSetting();
            nextTimeEvent = now + ofRandom(MIN_TIME,MAX_TIME);
        }
    }
}

void HexagonMap::createNewSetting()
{
    // trigger an event
    for (int i = 0; i < hexSettings.size(); i++)
    {
        hexSettings[i].direction = ofRandom(2);
        hexSettings[i].color = ofColor(ofRandom(128,255),ofRandom(128,255),ofRandom(128,255));
        hexSettings[i].eventID = ofRandom(EVENTS::SIZE);
    }
}

void HexagonMap::movePoint(ofxOscMessage &m)
{
    if (!isEditMode) return;
    // get label to gfigure out which command was sent
    string label = m.getAddress();
    float amount =  m.getArgAsFloat(0);
    
    ofVec2f shift = ofVec2f(0,0);
    if(label == "/up"){
        shift.y = -amount;
    }
    else if(label == "/down"){
        shift.y = amount;
        
    }
    else if(label == "/left"){
        shift.x = -amount;
        
    }
    else if(label == "/right"){
        shift.x = amount;
        
    }
    
    // Safety
    if (activeHexagon >= hexagons.size() || activeVertex > 7) return;
    
    // IF center point is selected we move the whole hexagon
    if(activeVertex == 0){
        for (int i = 0; i < hexagons[activeHexagon].getNumVertices(); ++i)
        {
            hexagons[activeHexagon].getVertices()[i] += shift;
        }
    }
    else{
        hexagons[activeHexagon].getVertices()[activeVertex] += shift;
    }
}

void HexagonMap::setEditMode(ofxOscMessage &m)
{
    float value = m.getArgAsFloat(0);
    if(value == 1) {
    
        isEditMode = ! isEditMode;
        save();
        for (int i = 0; i < hexagons.size(); i++)
        {
            addTexCoords(hexagons[i]);
        }
    }
}

void HexagonMap::setActiveHexagon(int i){
    activeHexagon = i;
}

void HexagonMap::setActiveVertex(int i){
    activeVertex = i;
}

// Mute the hexagon
void HexagonMap::muteHexagon(ofxOscMessage &m)
{
    float value = m.getArgAsFloat(0);
    if(value == 1 && isEditMode == true) {
        hexSettings[activeHexagon].isMuted = ! hexSettings[activeHexagon].isMuted;
    }
}


void HexagonMap::draw()
{
    // Loop through the hexagons
    for (int i = 0; i < hexagons.size(); i++){
        
        // CHECK if they are not muted
        if(!hexSettings[i].isMuted){
    
            // EDIT MODE
            if (isEditMode)
            {
                // draw the active one solid
                if(i == activeHexagon){
                    ofSetLineWidth(8);
                    hexagons[i].drawFaces();
                }
                // and the others as wireframe
                else{
                    ofSetLineWidth(5);
                    hexagons[i].drawWireframe();
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

void HexagonMap::drawSingleHexagon(int & id)
{
    float scale = ofNoise(ofGetElapsedTimef() + id);
    switch (hexSettings[id].eventID)
    {
        case EVENTS::NORMAL:
            ofSetColor(255);
            shader.begin();
            shader.setUniform1f("phase", ofNoise(ofGetElapsedTimef()));
            shader.setUniform1i("direction", hexSettings[id].direction);
            hexagons[id].draw();
            shader.end();
            break;
            
        case EVENTS::FORWARD:
            ofSetColor(255);
            shader.begin();
            shader.setUniform1f("phase", ofGetElapsedTimef());
            shader.setUniform1i("direction", hexSettings[id].direction);
            hexagons[id].draw();
            shader.end();
            break;
            
        case EVENTS::SCALED:
            ofSetColor(255);
            shader.begin();
            shader.setUniform1f("phase", ofNoise(ofGetElapsedTimef()));
            shader.setUniform1i("direction", hexSettings[id].direction);
            ofPushMatrix();
            ofTranslate(hexagons[id].getCentroid());
            ofScale(scale,scale);
            ofTranslate(-hexagons[id].getCentroid());
            hexagons[id].draw();
            ofPopMatrix();
            shader.end();
            break;
            
        case EVENTS::COLORED:
            ofSetColor(hexSettings[id].color);
            hexagons[id].draw();
            break;
            
        case EVENTS::SCALED_COLORED:
            ofSetColor(hexSettings[id].color);
            ofPushMatrix();
            ofTranslate(hexagons[id].getCentroid());
            ofScale(scale,scale);
            ofTranslate(-hexagons[id].getCentroid());
            hexagons[id].draw();
            ofPopMatrix();
            break;
            
        default:
            break;
    }

}

void HexagonMap::load()
{
    ofxXmlSettings settings;
    settings.load("settings.xml");
    for (int id = 0; id < settings.getNumTags("hex"); id++)
    {
        settings.pushTag("hex",id);
        int pCount = settings.getNumTags("p");
        ofVec2f points[8];
        for (int i = 0; i < pCount; i++)
        {
            settings.pushTag("p",i);
            points[i].x = settings.getValue("px", 0);
            points[i].y = settings.getValue("py", 0);
            settings.popTag();
        }
        settings.popTag();
        addHexagon(points, 8);
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

void HexagonMap::save()
{
    // FIXME: does not work yet
    ofxXmlSettings settings;
    for (int id = 0; id < hexagons.size(); id++)
    {
        settings.addTag("hex");
        settings.pushTag("hex",id);
        
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

