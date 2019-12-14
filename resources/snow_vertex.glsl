#version 330 core
layout(location = 0) in vec3 vertPos;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform float time;
out vec3 vertex_pos;
void main()
{
	float angl = time;
	mat4 Ry = mat4(
		vec4(cos(angl), sin(angl), 0, 0),
		vec4(0, 1, 0, 0),
		vec4(-sin(angl), 0,  cos(angl), 0),
		vec4(0, 0, 0, 1)
	);
	vec4 tpos =  M * Ry * vec4(vertPos, 1.0);
	gl_Position = P * V * tpos;
	vertex_pos = tpos.xyz;
}