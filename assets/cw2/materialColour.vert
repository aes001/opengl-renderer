#version 430

layout( location = 0 ) in vec3 iPosition;
layout( location = 1 ) in vec3 iColor;
layout( location = 2 ) in vec3 iNormal;

uniform mat4 uProjCameraWorld[2];


out vec3 v2fColor; // v2f = vertex to fragment
out vec3 v2fNormal;


void main()
{
	v2fColor = iColor;

	v2fNormal = normalize(iNormal);

	gl_Position = uProjCameraWorld[gl_InstanceID] * vec4( iPosition.xyz, 1.0 );
}