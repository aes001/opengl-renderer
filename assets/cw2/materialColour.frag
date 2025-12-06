#version 430
in vec3 v2fColor;
in vec3 v2fNormal;

layout( location = 0 ) out vec3 oColor;

uniform vec3 uLightDir;
uniform vec3 uLightDiffuse;
uniform vec3 uSceneAmbient;

void main()
{
	vec3 normal = normalize(v2fNormal);
	float nDotL = max(0.f, dot(normal, uLightDir));

	//apply simplfied blinn phong
	oColor = (uSceneAmbient + nDotL*uLightDiffuse) * v2fColor;
	//oColor = normal;
}
