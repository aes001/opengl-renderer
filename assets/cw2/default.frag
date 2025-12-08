#version 430
in vec3 v2fColor;
in vec3 v2fNormal;
in vec2 v2fTexCoord;
in vec3 v2fPosition;

layout( location = 0 ) out vec3 oColor;

layout( location = 1) uniform vec3 uLightDir;
layout( location = 2) uniform vec3 uLightDiffuse;
layout( location = 3) uniform vec3 uSceneAmbient;

uniform vec3 uCamPosition;

struct PointLight {
    vec4 lPosition;
    vec4 lColour;
};

layout(std140) uniform LightBlock {
    PointLight lights[3];
};

layout( binding = 0 ) uniform sampler2D uTexture;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 view)
{
	if(light.lColour[3] == 0) //check if light off
		return vec3(0.f, 0.f, 0.f);

	vec3 LPos = vec3(light.lPosition) - fragPos;
	float distAttenuation = 10/(LPos[0]*LPos[0] + LPos[1]*LPos[1] + LPos[2]*LPos[2]); //intensity scaling factor of 10

	vec3 L = normalize(LPos);
	vec3 sum = view + L;
	vec3 H = normalize((sum)/sqrt(sum[0]*sum[0] + sum[1]*sum[1] + sum[2]*sum[2]));
	vec3 specular = distAttenuation * vec3(light.lColour) * 0.5 * pow( max(0.f, dot(H, normal)), 100); //0.5 = specular reflectiveness, 10 = shininess

	vec3 diffuse = 0.2* distAttenuation * vec3(light.lColour) * max(0.f, dot(L, normal));

	return (specular + diffuse);

}

void main()
{
	vec3 normal = normalize(v2fNormal);

	vec3 result_light;

	//diffuse light
	vec3 diffuseLight = uLightDiffuse * max(0.f, dot(normal, uLightDir));
	result_light = diffuseLight;

	vec3 V = normalize(-uCamPosition - v2fPosition);

	for(int i = 0; i<3; i++)
	{
		result_light += CalcPointLight(lights[i], normal, v2fPosition, V);
	}

	oColor = (uSceneAmbient + result_light) * texture( uTexture, v2fTexCoord ).rgb;
}

