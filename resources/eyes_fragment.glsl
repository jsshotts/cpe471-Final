#version 330 core
out vec4 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform vec3 campos;
uniform vec3 camdir;
uniform int f;

void main()
{
	//coloring
	vec3 lpMoon = vec3(0, 100, -1000);
	vec3 lightColor = vec3(1, 1, 1); //the moon
	vec3 baseColor = vec3(1, 0, 0);

	vec3 lpFl = campos;
	vec3 Flcolor = vec3(1, 1, 0.65);
	vec3 lightDir = normalize(vertex_pos - lpFl);
	float innerCutoff = cos(radians(12.5f));
	float outerCutoff = cos(radians(17.5f));
	float epsilon = innerCutoff - outerCutoff;
	float theta = dot(lightDir, normalize(-camdir));
	float intensity = clamp((theta - outerCutoff) / epsilon, 0.0, 1.0);

	vec3 n = normalize(vertex_normal);
	vec3 ld = normalize(lpMoon - vertex_pos);
	float diffuse = clamp(dot(n, ld), 0, 1);

	//specular
	vec3 cd = normalize(campos - vertex_pos);
	vec3 h = normalize(cd+ld);
	float spec = dot(n,h);
	spec = clamp(spec,0,1);
	spec = pow(spec, 10);

	//final
	//color.rgb = baseColor * (0.2 + diffuse * 0.6) + lightColor*spec*3;
	color.rgb = vec3(0.8, 0, 0);
	color.a = 1;

	//transparancy
	float len = distance(campos.xz, vertex_pos.xz);
	if (len > 30)
		color.a = 0;
	else 
		color.a = 1;
		
	if (theta > outerCutoff && f == 1)
	{
		color.rgb = Flcolor * intensity * 0.7 + vec3(0.1, 0.1, 0.1);
	}
}