#version 330 core 

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inTex; //Koordinate texture, propustamo ih u FS kao boje
out vec2 chTex;

uniform float uScale;

void main()
{	vec2 scaledPos;
	if(inPos.y == -1){
		if(inPos.x < 0.0)
		 scaledPos = vec2(
			inPos.x - uScale, 
			inPos.y
		);
		else 
		 scaledPos = vec2(
			inPos.x + uScale, 
			inPos.y
		);
	}else{
		if(inPos.x < 0.0)
			 scaledPos = vec2(
				inPos.x - uScale, 
				inPos.y + uScale
			);
		else
			 scaledPos = vec2(
				inPos.x + uScale, 
				inPos.y + uScale
			);
	}

	if(uScale > 0.0)
		gl_Position = vec4(scaledPos, 0.0, 1.0);
	else
		gl_Position = vec4(inPos.x, inPos.y, 0.0, 1.0);
	chTex = inTex;
}