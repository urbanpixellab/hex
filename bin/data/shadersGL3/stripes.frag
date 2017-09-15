#version 150

out vec4 outputColor;
in vec2 texcoord0;


uniform float phase;
uniform float stripeWidth;
uniform int direction;

void main ()
{
  float tiles = 5.0;
  float rep = stripeWidth / tiles;
  //float rep = 1.0 / tiles;
  vec2 tc = mod(texcoord0.xy + vec2(phase),vec2(rep));
  float trigger = rep * 2.5;
  float c = step(trigger,tc.x * tiles);
  if(direction == 1) c = step(trigger,tc.y * tiles);
  vec3 col = vec3(c);
  
  outputColor = vec4(col,1.0);
}
