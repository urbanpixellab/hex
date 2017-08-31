#pragma once

#include "ofMain.h"
#include "HexagonMap.hpp"

class ofApp : public ofBaseApp{

public:
    void setup();
    void update();
    void draw();

private:
    HexagonMap          Hexagon;
};
