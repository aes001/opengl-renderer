#version 430

layout( location = 0 ) in vec3 iPosition;
layout( location = 1 ) in vec3 iColor;
layout( location = 2 ) in vec3 iNormal;
layout( location = 3 ) in float iSpecRef;
layout( location = 4 ) in float iShininess;

uniform mat4 uProjCameraWorld[2];
uniform vec3 uModelTransform[2];


out vec3 v2fColor; // v2f = vertex to fragment
out vec3 v2fNormal;
out vec3 v2fPosition;
out float v2fSpecRef;
out float v2fShininess;
out vec3 v2fmodelTransform;


void main()
{
	v2fColor = iColor;

	v2fNormal = normalize(iNormal);

	v2fPosition = iPosition;
	v2fSpecRef = iSpecRef;
	v2fShininess = iShininess;
	v2fmodelTransform = uModelTransform[gl_InstanceID];

	gl_Position = uProjCameraWorld[gl_InstanceID] * vec4( iPosition.xyz, 1.0 );
}