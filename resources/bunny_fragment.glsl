#version 330 core
out vec4 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform vec3 campos;

uniform sampler2D tex;
uniform sampler2D tex2;

void main()
{
//coloring
vec3 lp = vec3(0, 100, -1000);
vec3 lightColor = vec3(1, 1, 1); //the moon
vec3 baseColor = vec3(1, 1, 1);

//diffuse
vec3 n = normalize(vertex_normal);
vec3 ld = normalize(lp - vertex_pos);
float diffuse = clamp(dot(n, ld), 0, 1);

//specular
vec3 cd = normalize(campos - vertex_pos);
vec3 h = normalize(cd+ld);
float spec = dot(n,h);
spec = clamp(spec,0,1);
spec = pow(spec, 100);

//final
color.rgb = baseColor * (0.2 + diffuse * 0.6) + lightColor*spec*3;
color.a = 1;
color.r = 1;
}