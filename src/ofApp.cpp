#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    ofHideCursor();
    ofSetFullscreen(true);
    ofSetBackgroundColor(0);
}

//--------------------------------------------------------------
void ofApp::update()
{
    Hexagon.update();
}

//--------------------------------------------------------------
void ofApp::draw()
{
    Hexagon.draw();
}
