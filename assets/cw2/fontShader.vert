#version 430

layout( location = 0 ) in vec2 iPosition;
layout( location = 1 ) in vec4 iColor;
layout( location = 2 ) in vec2 iTexCoord;

out vec4 v2fColor;
out vec2 v2fTexCoord;

uniform vec2 uViewPortDimensions;

void main()
{
	v2fColor = iColor;

	v2fTexCoord = iTexCoord;

	float posNormalizedX = ((iPosition.x / uViewPortDimensions.x) * 2) - 1;
	float posNormalizedY = 1 - (iPosition.y / uViewPortDimensions.y) * 2;

	gl_Position = vec4(posNormalizedX, posNormalizedY, 0.f, 1.f);
}