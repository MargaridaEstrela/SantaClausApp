#version 410

uniform sampler2D texmap;
uniform sampler2D texmap1;
uniform sampler2D texmap2;
uniform sampler2D texmap3;

uniform int texMode;

out vec4 colorOut;

struct Materials {
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec4 emissive;
	float shininess;
	int texCount;
};

uniform Materials mat;

uniform vec4 spotDir;
const float spotCutOff = 0.95;
const float spotExpo = 20.0f;

const float linear = 0.1;
const float expo = 0.1;
const float density = 0.1;
const float gradient = 1.0;

uniform bool fog;
uniform bool directionalLightOn;
uniform bool pointLightsOn;
uniform bool spotLightsOn;

in Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir[9];
	vec2 tex_coord;
} DataIn;

void main() {

	vec4 texel, texel1;

	colorOut = vec4(0);

	if (!directionalLightOn && !pointLightsOn && !spotLightsOn) return;

	vec4 spec = vec4(0.0);

	vec3 n = normalize(DataIn.normal);
	vec3 e = normalize(DataIn.eye);

	for (int i = 0; i <9; i++) {

		if (i == 0 && !directionalLightOn) continue;
		if (i >= 1 && i < 7 && !pointLightsOn) continue;
		if (i >= 7 && !spotLightsOn) break;

		float attenuation = 1.0;
		vec3 l = normalize(DataIn.lightDir[i]);

		if (i != 0) {
			float dl = length(DataIn.lightDir[i]); // distance to light
			attenuation += linear * dl + expo * dl * dl;
		}
		
		float intensity = max(dot(n,l), 0.0);

		

		if (i >= 7) {
			attenuation += 0.1;
			vec3 sd = normalize(vec3(-spotDir));
			float spotCos = dot(l, sd);
			if (spotCos < spotCutOff) intensity = 0.0;
			else attenuation = 1 / pow(spotCos, spotExpo);

			if (intensity > 0.0) {
				vec3 h = normalize(l + e);
				float intSpec = max(dot(h,n), 0.0);
				spec = mat.specular * pow(intSpec, mat.shininess);
			}

			intensity = intensity * 0.1;
		}

		if (i >= 1 && i < 7) intensity = intensity * 0.5;
		else intensity = intensity * 0.3;
	
		colorOut += max(intensity * mat.diffuse + spec, mat.ambient) / attenuation;

		if (texMode == 0) // modulate diffuse color with texel color
		{
			texel = texture(texmap, DataIn.tex_coord);  // texel from snow.jpeg
			colorOut += max(intensity*texel + spec, 0.2*texel) / attenuation;
		} 
		else if (texMode == 1) // Roof
		{
			texel = texture(texmap1, DataIn.tex_coord);  // texel from roof.jpeg
			colorOut += max(intensity*texel + spec, 0.07*texel) / attenuation;
		}
		else if (texMode == 2) // sleigh
		{
			texel = texture(texmap2, DataIn.tex_coord);  // texel from snow.png
			colorOut += max(intensity*texel + spec, 0.07*texel) / attenuation;
		}
		else if (texMode == 3) // snowballs
		{
			texel = texture(texmap, DataIn.tex_coord);  // texel from snow.png
			colorOut += min(intensity*texel + spec, texel);
		}
		else // multitexturing	
		{
			texel = texture(texmap2, DataIn.tex_coord);  // texel from lighwood.tga
			texel1 = texture(texmap, DataIn.tex_coord);  // texel from snow.jpeg
			colorOut += max(intensity*texel + intensity*texel1 + spec, 0.07*texel*texel1) / attenuation;
		}

	}

	if (fog) {
		float distance = length(DataIn.eye) - 1;
		distance = max(0, distance);
		float visibility = exp(-pow(distance * density, gradient));
		visibility = clamp(visibility, 0.0, 1.0);
		colorOut = mix(vec4(0.5, 0.5, 0.5, 1), colorOut, visibility);
	}

}