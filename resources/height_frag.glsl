#version 330 core
out vec4 color;
in vec3 vertex_pos;
in vec2 vertex_tex;
in vec3 vertex_normal;

uniform sampler2D tex;

uniform vec3 camoff;
uniform vec3 campos;

void main()
{
vec2 texcoords=vertex_tex;
float t=1./100.;
texcoords -= vec2(camoff.x,camoff.z)*t;
color.rgb = texture(tex, texcoords*50).rgb * normalize(-vertex_normal);
color.a=1;

float len = length(vertex_pos.xz+campos.xz);
len-=41;
len/=8.;
len=clamp(len,0,1);
color.a=1-len;

//coloring
vec3 lp = vec3(0, 100, -1000);
vec3 lightColor = vec3(1, 1, 1); //the moon
vec3 baseColor = texture(tex, texcoords*50).rgb * normalize(-vertex_normal);

//diffuse
vec3 n = normalize(vertex_normal);
vec3 ld = normalize(lp - vertex_pos);
float diffuse = clamp(dot(n, ld), 0, 1);

//specular
vec3 cd = normalize(campos - vertex_pos);
vec3 h = normalize(cd+ld);
float spec = dot(n,h);
spec = clamp(spec,0,1);
spec = pow(spec, 500);

//final
color.rgb = baseColor * (0.2 + diffuse * 0.6) + lightColor*spec*0.5;
color.a = 1;
}