#version 330 core
out vec4 color;
in vec3 vertex_pos;
in vec2 vertex_tex;
in vec3 vertex_normal;

uniform sampler2D tex;
uniform sampler2D tex2;

uniform vec3 camoff;
uniform vec3 campos;
uniform vec3 camdir;
uniform int f;

void main()
{
	vec2 texcoords=vertex_tex;
	float t=1./100.;
	texcoords += vec2(camoff.x,camoff.z)*t;
	color.rgb = texture(tex, texcoords*50).rgb * normalize(vertex_normal);

	//coloring
	vec3 lpMoon = vec3(0, 100, -1000);
	vec3 lightColor = vec3(1, 1, 1); //the moon
	vec3 baseColor = texture(tex, texcoords*50).rgb * normalize(vertex_normal);

	vec3 lpFl = campos;
	vec3 Flcolor = vec3(1, 1, 0.65);
	vec3 lightDir = normalize(vertex_pos - lpFl);
	float innerCutoff = cos(radians(12.5f));
	float outerCutoff = cos(radians(17.5f));
	float epsilon = innerCutoff - outerCutoff;
	float theta = dot(lightDir, normalize(-camdir));
	float intensity = clamp((theta - outerCutoff) / epsilon, 0.0, 1.0);

	//diffuse
	vec3 n = normalize(vertex_normal);
	vec3 ld = normalize(lpMoon - vertex_pos);
	float diffuse = clamp(dot(n, ld), 0, 1);

	//specular
	vec3 cd = normalize(campos - vertex_pos);
	vec3 h = normalize(cd+ld);
	float spec = dot(n,h);
	spec = clamp(spec,0,1);
	spec = pow(spec, 500);

	//final
	color.rgb = baseColor * (0.05 + diffuse * 0.3) + lightColor*spec*0.05;
	float len = length(campos.xz - vertex_pos.xz);
	len-=41;
	len/=8.;
	len=clamp(len,0,1);
	color.a=1-len;

	//flashlight
	if (theta > outerCutoff && f == 1)
	{
		vec3 ld2 = normalize(lpFl - vertex_pos);
		float diffuse2 = clamp(dot(n, ld2), 0, 1);
		diffuse2 *= intensity;

		color.rgb += Flcolor * diffuse2 * 0.7 + baseColor * intensity * 0.2;
	}	
}