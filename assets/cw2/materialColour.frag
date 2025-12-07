#version 430
in vec3 v2fColor;
in vec3 v2fNormal;
in vec3 v2fPosition;
in float v2fSpecRef;
in float v2fShininess;
in vec3 v2fmodelTransform;

layout( location = 0 ) out vec3 oColor;

uniform vec3 uLightDir;
uniform vec3 uLightDiffuse;
uniform vec3 uSceneAmbient;

uniform vec3 uCamPosition;
//need one per light?
uniform vec3 uLightPosition;
uniform vec3 uSpecLightColour;

void main()
{
	vec3 normal = normalize(v2fNormal);

	//diffuse light
	vec3 diffuseLight = uLightDiffuse * max(0.f, dot(normal, uLightDir));

	vec3 vertPos = v2fPosition + v2fmodelTransform;
	//specular light
	vec3 LPos = uLightPosition - vertPos;
	float distAttenuation = 10/(LPos[0]*LPos[0] + LPos[1]*LPos[1] + LPos[2]*LPos[2]);

	vec3 V = normalize(-uCamPosition - vertPos);
	vec3 L = normalize(LPos);
	vec3 sum = V + L;
	vec3 H = normalize((sum)/sqrt(sum[0]*sum[0] + sum[1]*sum[1] + sum[2]*sum[2]));
	vec3 specLight = distAttenuation * uSpecLightColour * v2fSpecRef * pow( max(0.f, dot(H, normal)), v2fShininess);


	//apply simplfied blinn phong
	oColor = (uSceneAmbient + diffuseLight + specLight) * v2fColor;
	//oColor = normalize(specLight);
}
