#version 430

layout( location = 0 ) in vec2 iPosition;


void main()
{

	//gl_Position = projection * vec4(iPosition, 0.f, 1.f);
	gl_Position = vec4(iPosition.xy, 0.f, 1.f);
}