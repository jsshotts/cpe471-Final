#version 330 core
out vec4 color;
in vec3 vertex_pos;

uniform vec3 campos;
uniform vec3 camdir;
uniform int f;
void main()
{
	color.r = color.g = color.b = 1;
	//coloring
	vec3 lpMoon = vec3(0, 100, -1000);
	vec3 lightColor = vec3(1, 1, 1); //the moon
	vec3 baseColor = vec3(1, 1, 1);
	vec3 lpFl = campos;
	vec3 Flcolor = vec3(1, 1, 0.65);
	vec3 lightDir = normalize(vertex_pos - lpFl);
	float innerCutoff = cos(radians(12.5f));
	float outerCutoff = cos(radians(17.5f));
	float epsilon = innerCutoff - outerCutoff;
	float theta = dot(lightDir, normalize(-camdir));
	float intensity = clamp((theta - outerCutoff) / epsilon, 0.0, 1.0);
	//intensity = pow(intensity, 2);
	//intensity = 1.0f;

	vec3 n = normalize(camdir);
	//diffuse
		vec3 ld = normalize(lpMoon - vertex_pos);
		float diffuse = clamp(dot(n, ld), 0, 1);

		//specular
		vec3 cd = normalize(campos - vertex_pos);
		vec3 h = normalize(cd+ld);
		float spec = dot(n,h);
		spec = clamp(spec,0,1);
		spec = pow(spec, 10);

		//final
		color.rgb = baseColor * (0.05 + diffuse * 0.3) + lightColor*spec*0.1;
		color.a = 1;
	if (theta > outerCutoff && f == 1)
	{
//		vec3 ld = normalize(lpFl - vertex_pos);
//		float diffuse = clamp(dot(n, ld), 0, 1);
//		diffuse *= intensity;
//
//		vec3 cd = normalize(campos - vertex_pos);
//		vec3 h = normalize(cd+ld);
//		float spec = dot(n,h);
//		spec = clamp(spec,0,1);
//		spec = pow(spec, 10);
//		spec *= intensity;
//
//		color.rgb = Flcolor * baseColor * diffuse * 0.7;// + Flcolor * spec * 0.3;
//		//color.rgb = vec3(intensity, intensity, intensity);1/length(campos - vertex_pos)*20 * 
//		color.a = 1;

		vec3 ld2 = normalize(lpFl - vertex_pos);
		float diffuse2 = clamp(dot(n, ld2), 0, 1);
		diffuse2 *= intensity;
//
//		vec3 cd = normalize(campos - vertex_pos);
//		vec3 h = normalize(cd+ld);
//		float spec = dot(n,h);
//		spec = clamp(spec,0,1);
//		spec = pow(spec, 10);
//		spec *= intensity;

		color.rgb += Flcolor * diffuse2 * 0.3;// + Flcolor * spec * 0.3;
		//color.rgb = vec3(intensity, intensity, intensity);1/length(campos - vertex_pos)*20 * 
		color.a = 1;
		color.rgb = vec3(1, 1, 1);
	}
	color.rgb = vec3(1, 1, 1);
//	float len = length(-campos.xz + vertex_pos.xz);
//	len-=41;
//	len/=8.;
//	len=clamp(len,0,1);
//	color.a=1-len;
}