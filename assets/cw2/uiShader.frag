#version 430

layout( location = 0 ) out vec4 oColor;

uniform vec4 inColour;

void main()
{
	// Copy input color to the output color attribute.
	oColor = inColour;
}