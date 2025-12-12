#version 430

in vec4 v2fColor;
in vec2 v2fTexCoord;

layout( location = 0 ) out vec4 oColor;

layout( binding = 0 ) uniform sampler2D uTexture;


void main()
{
	float alpha = texture( uTexture, v2fTexCoord ).r;

	oColor = vec4( v2fColor.rgb, v2fColor.a * alpha);
}

