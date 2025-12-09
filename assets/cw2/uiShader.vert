#version 430

layout( location = 0 ) in vec2 iPosition;
layout( location = 1 ) in vec3 iColor;

out vec3 v2fColor;

uniform mat4 projection; //screen projection

void main()
{
	// Copy input color to the output color attribute.
	v2fColor = iColor;

	//gl_Position = projection * vec4(iPosition, 0.f, 1.f);
	gl_Position = vec4(iPosition.xy, 0.f, 1.f);
}