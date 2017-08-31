//
//  HexagonMap.hpp
//
//
//  Created by enrico on 29/08/17.
//
//

#ifndef HexagonMap_hpp
#define HexagonMap_hpp

#include <stdio.h>
#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxXmlSettings.h"

#define MIN_TIME 10
#define MAX_TIME 20

class HexagonMap
{
    
public:
    enum EVENTS
    {
        NORMAL = 0,
        FORWARD,
        SCALED,
        COLORED,
        SCALED_COLORED,
        SIZE
    };
    
    struct HexagonSetting
    {
        int     direction; // the stripe direction
        ofColor color;
        int     eventID;
    };
    
    
    HexagonMap();
    ~HexagonMap(){};
    
    void addHexagon();
    void addHexagon(ofVec2f * verts,int length);
    void addTexCoords(ofMesh &m);
    void mapPoint(ofxOscMessage &m);
    void moveHexagon(ofxOscMessage &m);
    void movePoint(ofxOscMessage &m);
    void setEditMode(ofxOscMessage &m);
    void update();
    void createNewSetting();
    void draw();
    void drawSingleHexagon(int &id);
    void drawScaled();
    void drawNormal();
    
    void load();
    void save();
    
    
private:
    
    ofxOscReceiver              receiver;
    vector<ofMesh>              hexagons;
    vector<HexagonSetting>      hexSettings;
    
    ofShader                    shader;
    bool                        isEditMode;
    
    float                       nextTimeEvent;
    ofColor                     rndColor;
};

#endif /* HexagonMap_hpp */
