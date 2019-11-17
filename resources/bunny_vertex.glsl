#version 330 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
out vec3 vertex_pos;
out vec3 vertex_normal;
out vec2 vertex_tex;

float hash(float n) { return fract(sin(n) * 753.5453123); }
float snoise(vec3 x)
{
	vec3 p = floor(x);
	vec3 f = fract(x);
	f = f * f * (3.0 - 2.0 * f);

	float n = p.x + p.y * 157.0 + 113.0 * p.z;
	return mix(mix(mix(hash(n + 0.0), hash(n + 1.0), f.x),
		mix(hash(n + 157.0), hash(n + 158.0), f.x), f.y),
		mix(mix(hash(n + 113.0), hash(n + 114.0), f.x),
			mix(hash(n + 270.0), hash(n + 271.0), f.x), f.y), f.z);
}
//Changing octaves, frequency and presistance results in a total different landscape.
float noise(vec3 position, int octaves, float frequency, float persistence) {
	float total = 0.0;
	float maxAmplitude = 0.0;
	float amplitude = 1.0;
	for (int i = 0; i < octaves; i++) {
		total += snoise(position * frequency) * amplitude;
		frequency *= 2.0;
		maxAmplitude += amplitude;
		amplitude *= persistence;
	}
	return total / maxAmplitude;
}
float getHeight(vec3 pos)
{
	float height = noise(pos.xzy, 11, 0.015, 0.6);
	float baseheight = noise(pos.xzy, 4, 0.05, 0.4);
	//baseheight = pow(baseheight, 1)*3;
	height = baseheight*height;
	return height *= 20;
}
vec3 getNormal(vec3 pos)
{
	vec3 a,b,c;
    a =  pos;
    b =  a + vec3(1,0,0);
    c =  a + vec3(0,0,1);

    a.y += getHeight(a.xyz);
    b.y += getHeight(b.xyz);
    c.y += getHeight(c.xyz);

    vec3 ac = a-c;
	vec3 bc = b-c;

    return cross(ac.xyz,bc.xyz);
}
void main()
{
	vertex_normal = vec4(M * vec4(vertNor,0.0)).xyz;
	vec4 tpos =  M * vec4(vertPos, 1.0);
	tpos =  M * tpos;
	tpos.y += getHeight(tpos.xyz);
	
	gl_Position = P * V * tpos;
	vertex_pos = tpos.xyz;
	vertex_tex = vertTex;
	vertex_normal = getNormal(vertex_pos);
}