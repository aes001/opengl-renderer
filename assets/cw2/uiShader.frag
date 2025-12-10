#version 430

flat in uint v2fBorderFlag;

layout( location = 0 ) out vec4 oColour;

uniform vec4 inColour;

void main()
{
	vec4 colour = inColour;

	//check if fragment is on border
	if(v2fBorderFlag == 1)
	{
		colour.w = 1.f;
	}

	oColour = colour;
}