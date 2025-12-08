#version 430
in vec3 v2fColor;
in vec3 v2fNormal;
in vec3 v2fPosition;
in vec3 v2fSpecRef;
in float v2fShininess;
in vec3 v2fmodelTransform;

layout( location = 0 ) out vec3 oColor;

uniform vec3 uLightDir;
uniform vec3 uLightDiffuse;
uniform vec3 uSceneAmbient;

uniform vec3 uCamPosition;
//need one per light?
//uniform vec3 uLightPosition;
//uniform vec3 uSpecLightColour;

struct PointLight {
    vec4 lPosition;
    vec4 lColour;
};

layout(std140) uniform LightBlock {
    PointLight lights[3];
};

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 view)
{
	vec3 LPos = vec3(light.lPosition) - fragPos;
	float distAttenuation = 10/(LPos[0]*LPos[0] + LPos[1]*LPos[1] + LPos[2]*LPos[2]); //intensity scaling factor of 10

	vec3 L = normalize(LPos);
	vec3 sum = view + L;
	vec3 H = normalize((sum)/sqrt(sum[0]*sum[0] + sum[1]*sum[1] + sum[2]*sum[2]));
	return distAttenuation * vec3(light.lColour) * v2fSpecRef * pow( max(0.f, dot(H, normal)), v2fShininess);


}

void main()
{
	vec3 normal = normalize(v2fNormal);

	vec3 result_light;

	//diffuse light
	vec3 diffuseLight = uLightDiffuse * max(0.f, dot(normal, uLightDir));
	result_light = diffuseLight;

	vec3 fragPos = v2fPosition + v2fmodelTransform;
	vec3 V = normalize(-uCamPosition - fragPos);

	for(int i = 0; i<3; i++)
	{
		result_light += CalcPointLight(lights[i], normal, fragPos, V);
	}

	//apply simplfied blinn phong
	oColor = (uSceneAmbient + result_light) * v2fColor;
	//oColor = normalize(specLight);
}
