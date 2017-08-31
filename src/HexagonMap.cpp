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
    shader.load("stripes.vert","stripes.frag");
    isEditMode = false;
    receiver.setup(20000);
    nextTimeEvent = ofGetElapsedTimef() + 10;
}

void HexagonMap::addHexagon()
{
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
    hexSettings.push_back(newSetting);
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
        if (m.getAddress() == "/add") addHexagon();
        else if (m.getAddress() == "/map") mapPoint(m);
        else if (m.getAddress() == "/moveHexagon") moveHexagon(m);
        else if (m.getAddress() == "/movePoint") movePoint(m);
        else if (m.getAddress() == "/edit") setEditMode(m);
        else if (m.getAddress() == "/save") save();




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

void HexagonMap::mapPoint(ofxOscMessage &m)
{
    if (!isEditMode) return;
    int ID = m.getArgAsInt32(0);
    int pID = m.getArgAsInt32(1);
    ofVec2f p = ofVec2f(m.getArgAsFloat(2),m.getArgAsFloat(3));
    if (ID >= hexagons.size() || pID > 7) return;
    hexagons[ID].getVertices()[pID] = p;
}

void HexagonMap::moveHexagon(ofxOscMessage &m)
{
    //move the complete hexagon by point in x & y
    if (!isEditMode) return;
    int ID = m.getArgAsInt32(0);
    ofVec2f shift = hexagons[ID].getCentroid() - ofVec2f(m.getArgAsFloat(1),m.getArgAsFloat(2));
    for (int i = 0; i < hexagons[ID].getNumVertices(); ++i)
    {
        hexagons[ID].getVertices()[i] += shift;
    }
}

void HexagonMap::movePoint(ofxOscMessage &m)
{
    if (!isEditMode) return;
    int ID = m.getArgAsInt32(0);
    int pID = m.getArgAsInt32(1);
    ofVec2f shift = ofVec2f(m.getArgAsFloat(2),m.getArgAsFloat(3));
    if (ID >= hexagons.size() || pID > 7) return;
    hexagons[ID].getVertices()[pID] += shift;
}

void HexagonMap::setEditMode(ofxOscMessage &m)
{
    isEditMode = m.getArgAsBool(0);
    save();
    for (int i = 0; i < hexagons.size(); i++)
    {
        addTexCoords(hexagons[i]);
    }
}


void HexagonMap::draw()
{
    if (isEditMode)
    {
	ofSetLineWidth(5);
        for (int i = 0; i < hexagons.size(); i++)
        {
            hexagons[i].drawWireframe();
        }
    }
    else // draw with shader
    {
        for (int i = 0; i < hexagons.size(); i++) drawSingleHexagon(i);
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

void HexagonMap::save()
{
    ofxXmlSettings settings;
    for (int id = 0; id < hexagons.size(); id++)
    {
        settings.addTag("hex");
        settings.pushTag("hex");
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
