#version 430
in vec3 v2fColor;
in vec3 v2fNormal;

layout( location = 0 ) out vec3 oColor;

layout( location = 1) uniform vec3 uLightDir;
layout( location = 2) uniform vec3 uLightDiffuse;
layout( location = 3) uniform vec3 uSceneAmbient;

void main()
{
	vec3 normal = normalize(v2fNormal);
	float nDotL = max(0.f, dot(normal, uLightDir));

	//oColor = v2fColor;
	//apply simplfied blinn phong
	oColor = (uSceneAmbient + nDotL*uLightDiffuse) * v2fColor;
}

