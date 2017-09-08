#include "ofMain.h"
#include "ofApp.h"

int main( ){
  ofSetupOpenGL(800,600,OF_WINDOW);
    //ofGLWindowSettings settings;
    //settings.setGLVersion(3,2);
    //ofCreateWindow(settings);
  ofRunApp(new ofApp());
}
