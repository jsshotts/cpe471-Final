#version 330 core
out vec4 color;
in vec2 vertex_tex;

uniform sampler2D tex;

void main()
{
	color = texture(tex, vertex_tex);
}