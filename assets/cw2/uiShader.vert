#version 430

layout( location = 0 ) in vec2 iPosition;
layout ( location = 1 ) in uint BorderFlag;

flat out uint v2fBorderFlag;

void main()
{
	v2fBorderFlag = BorderFlag;

	gl_Position = vec4(iPosition.xy, 0.f, 1.f);
}