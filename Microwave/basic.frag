#version 330 core

in vec4 chCol; 
out vec4 outCol; 


uniform float uAlpha;

void main() 
{

	outCol = vec4(chCol.rgb, chCol.a + uAlpha);
}