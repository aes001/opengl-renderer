#version 430

in vec2 v2fTexCoord;
in vec4 v2fColour;

layout( location = 0 ) out vec4 oColour;

layout( binding = 0 ) uniform sampler2D uTexture;

void main()
{
	oColour = (texture( uTexture, v2fTexCoord) * v2fColour);
}