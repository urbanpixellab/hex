#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    ofHideCursor();
    //ofSetFullscreen(true);
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

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
    // Toggle previewmode (not yet implemented)
    if(key == 'p'){
        Hexagon.togglePreviewMode();
    }
    
    // load preview scene
    if(key == 'l'){
        Hexagon.loadPreviewScene();
    }
    
}
