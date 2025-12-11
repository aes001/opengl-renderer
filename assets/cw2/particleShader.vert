#version 430

layout( location = 0 ) in vec3 iPosition;

layout(location = 0) uniform mat4 uProjCameraWorld;
uniform vec3 uOffset;

void main()
{
	float scale = 0.05;
	gl_Position = uProjCameraWorld * (vec4((iPosition.xyz + uOffset.xyz) * scale, 1.f));
}