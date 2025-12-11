#version 430

layout( location = 0 ) in vec3 iPosition;
layout( location = 1 ) in vec2 iTexCoord;

layout(location = 0) uniform mat4 uProjCameraWorld;
layout(location = 1) uniform vec4 uColour;

uniform vec3 uOffset;

out vec2 v2fTexCoord;
out vec4 v2fColour;

void main()
{
	v2fTexCoord = iTexCoord;
	v2fColour = uColour;

	float scale = 0.05;
	gl_Position = uProjCameraWorld * (vec4((iPosition.xyz + uOffset.xyz) * scale, 1.f));
}