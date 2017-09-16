precision highp float; // this will make the default precision high

//we passed this in from our vert shader
varying vec2        texcoord0;

//These are variables we set in our ofApp using the ofShader API

//our texture reference
//passed in by
//shader.setUniformTexture("tex0", sourceImage.getTextureReference(), sourceImage.getTextureReference().texData.textureID);
//uniform sampler2D   tex0;

//width and height that we are working with
//passed in by
//shader.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
//uniform vec2        resolution;

//a changing value to work with
//passed in by
//shader.setUniform1f("time", ofGetElapsedTimef());
//uniform float       time;


//Each shader has one main() function you can use
//Below are a few implementations. Make sure you have all but one commented out



uniform float phase;
uniform int direction;
uniform float stripeWidth;
uniform int xandy;
uniform float tiles;

void main ()
{
  //float tiles = 5.0;
  float rep = stripeWidth / tiles;
  //float rep = 1.0 / tiles;
  vec2 tc = mod(texcoord0.xy + vec2(phase),vec2(rep));
  float trigger = rep * 2.5;
  float c = step(trigger,tc.x * tiles);
  if(xandy == 1) c *= step(trigger,tc.y * (tiles/0.5));

  if(direction == 1){ 
  	c = step(trigger,tc.y * tiles);
	if(xandy == 1) c *= step(trigger,tc.x * (tiles/0.5));
  }

  
  vec3 col = vec3(c);
  gl_FragColor = vec4(col,1.0);  
}
