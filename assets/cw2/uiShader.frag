#version 430

layout( location = 0 ) out vec3 oColor;

uniform vec3 inColour;

void main()
{
	// Copy input color to the output color attribute.
	oColor = inColour;
}