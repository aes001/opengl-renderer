#version 430
#include "../../main/Lights.hpp"

in vec3 v2fColor;
in vec3 v2fNormal;
in vec2 v2fTexCoord;

layout( location = 0 ) out vec3 oColor;

layout( location = 1) uniform vec3 uLightDir;
layout( location = 2) uniform vec3 uLightDiffuse;
layout( location = 3) uniform vec3 uSceneAmbient;
layout( location = 4) uniform vec3 uLightSpecular;//unused
layout( location = 4) uniform DirLight dirLight;

layout( binding = 0 ) uniform sampler2D uTexture;

//direct light calculation
vec3 calculateDirectLight(vec3 direction, vec3 l_diffuse, vec3 l_ambient, vec3 l_specular, vec3 surface_normal)
{
	float nDotL = max(0.f, dot(surface_normal, uLightDir));

	vec3 diffuse = nDotL * l_diffuse;
	vec3 direct = l_ambient + diffuse;
	return direct;
}

void main()
{
	vec3 normal = normalize(v2fNormal);
	vec3 direct_light = calculateDirectLight(uLightDir, uLightDiffuse, uSceneAmbient, uLightSpecular, normal);


	oColor = direct_light * texture( uTexture, v2fTexCoord ).rgb;
}

