#version 410 core 
out vec4 color;
in vec2 fragTex;

uniform sampler2D tex;
uniform sampler2D tex2;

uniform vec3 campos;
uniform vec3 camdir;
uniform int f;

void main()
{
	vec3 flashlightMap = texture(tex2, fragTex).rgb;
	//float intensity = (flashlightMap.x + flashlightMap.y + flashlightMap.z) /3;

	vec3 texturecolor = texture(tex, fragTex,0).rgb;
	vec3 texturecolorLOWRES = texture(tex, fragTex,4).rgb;
	texturecolor.r = pow(texturecolor.r,4);
	texturecolor.g = pow(texturecolor.g,2);
	texturecolor.b = pow(texturecolor.b,1.2);
	color.rgb = texturecolor+texturecolorLOWRES;
	//color.rgb *= flashlightMap;
	color.a=1;
}