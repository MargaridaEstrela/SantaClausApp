#version 430

uniform sampler2D texmap;
uniform sampler2D texmap1;
uniform sampler2D texmap2;
uniform sampler2D texmap3;
uniform sampler2D texmap4;
uniform sampler2D texmap5;
uniform samplerCube cubeMap;
uniform	sampler2D texUnitDiff;
uniform	sampler2D texUnitDiff1;
uniform	sampler2D texUnitSpec;
uniform	sampler2D texUnitNormalMap;

uniform	sampler2D texUnitDiff;
uniform	sampler2D texUnitDiff1;
uniform	sampler2D texUnitSpec;
uniform	sampler2D texUnitNormalMap;

uniform int texMode;

out vec4 colorOut;

uniform mat4 m_View;
uniform int reflect_perFrag; //reflect vector calculated in the frag shader

struct Materials {
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec4 emissive;
	float shininess;
	int texCount;
};

uniform Materials mat;

uniform bool fog;
uniform bool directionalLightOn;
uniform bool pointLightsOn;
uniform bool spotLightsOn;
uniform bool shadowMode;

uniform bool normalMap;  //for normal mapping
uniform bool specularMap;
uniform uint diffMapCount;

uniform vec4 spotDir;
const float spotCosCutOff = 0.93;
const float spotExp = 40.0f;

const float linear = 0.1;
const float expo = 0.1;

const float density = 0.1;
const float gradient = 1.0;

const vec4 shadowC = vec4(.6, .6, .6, 1);
const float reflect_factor = 0.9;

in Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir[9];
	vec2 tex_coord;
} DataIn;

vec4 diff, auxSpec;

void main() {
	if(shadowMode && !fog) {
		colorOut = shadowC;
		return;
	}

	colorOut = vec4(0);

	if (!directionalLightOn && !pointLightsOn && !spotLightsOn) return;

	vec4 spec = vec4(0.0);
	vec4 texel, texel1, cube_texel;

	vec3 n = normalize(DataIn.normal);
	if(normalMap)
		n = normalize(2.0 * texture(texUnitNormalMap, DataIn.tex_coord).rgb - 1.0);  //normal in tangent space
	else
		n = normalize(DataIn.normal);

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
			vec3 sd = normalize(vec3(-spotDir));
			float spotCos = dot(l, sd);
			attenuation += 0.1;
			
			if (spotCos > spotCosCutOff)  {	//inside cone?
				attenuation = 1 / pow(spotCos, spotExp);
				intensity = max(dot(n,l), 0.0) * attenuation;
				if (intensity > 0.0) {
					vec3 h = normalize(l + e);
					float intSpec = max(dot(h,n), 0.0);
					spec = mat.specular * pow(intSpec, mat.shininess) * attenuation;
				}
			}
		}

		if (mat.texCount == 0) {
			diff = mat.diffuse;
			spec = mat.specular;
		}
		else {
			if(diffMapCount == 0)
				diff = mat.diffuse;
			else if(diffMapCount == 1)
				diff = mat.diffuse * texture(texUnitDiff, DataIn.tex_coord);
			else
				diff = mat.diffuse * texture(texUnitDiff, DataIn.tex_coord) * texture(texUnitDiff1, DataIn.tex_coord);

		if(specularMap) 
			auxSpec = mat.specular * texture(texUnitSpec, DataIn.tex_coord);
		else
			auxSpec = mat.specular;
		}

		if (i == 0) intensity *= 0.5;				// directional light
		else if (i >= 7) intensity *= 0.1;			// spotlights
		else intensity *= 0.6;						// pointlights
	
		colorOut += max(intensity * mat.diffuse + spec, mat.ambient) / attenuation;

		if(mat.texCount != 0)
		{
			vec4 diff, auxSpec;

			if(diffMapCount == 0)
				diff = mat.diffuse;
			else if(diffMapCount == 1)
				diff = mat.diffuse * texture(texUnitDiff, DataIn.tex_coord);
			else
				diff = mat.diffuse * texture(texUnitDiff, DataIn.tex_coord) * texture(texUnitDiff1, DataIn.tex_coord);

			if(specularMap) 
				auxSpec = mat.specular * texture(texUnitSpec, DataIn.tex_coord);
			else
				auxSpec = mat.specular;

			if (intensity > 0.0) {
				vec3 h = normalize(l + e);
				float intSpec = max(dot(h,n), 0.0);
				spec = auxSpec * pow(intSpec, mat.shininess);
				
			}

			colorOut = vec4((max(intensity * diff, diff*0.15) + spec).rgb, 1.0);
			return;
		}
		else if (texMode == 0) // modulate diffuse color with texel color
		{
			texel = texture(texmap, DataIn.tex_coord);  // texel from snow.jpeg
			colorOut += max(intensity*texel + spec, 0.07*texel) / attenuation;
		} 
		else if (texMode == 1) // Roof
		{
			texel = texture(texmap1, DataIn.tex_coord);  // texel from roof.jpeg
			colorOut += max(intensity*texel + spec, 0.07*texel) / attenuation;
		}
		else if (texMode == 2) // sleigh
		{
			texel = texture(texmap2, DataIn.tex_coord);  // texel from lightwood.tga
			colorOut += max(intensity*texel + spec, 0.07*texel) / attenuation;
		}
		else if (texMode == 3) // snowballs
		{
			texel = texture(texmap, DataIn.tex_coord);  // texel from snow.png
			colorOut += min(intensity*texel + spec, 0.5*texel) / attenuation;
		}
		else if (texMode == 4) // trees
		{
			texel = texture(texmap3, DataIn.tex_coord);  
		
			if(texel.a == 0.0) discard;
			else
				colorOut = texel;
		}
		else if (texMode == 5) // lamps glass
		{
			texel = texture(texmap4, DataIn.tex_coord);  // texel from glass.jpeg
			colorOut += max(intensity*texel + spec, 0.3*texel) / attenuation;
			colorOut[3] = 0.7; 
		}
		else if (texMode == 6) // lamps
		{
			texel = texture(texmap5, DataIn.tex_coord);  // texel from green_metal.webp
			colorOut += max(intensity*texel + spec, 0.07*texel) / attenuation;
		}
		else // multitexturing	
		{
			texel = texture(texmap2, DataIn.tex_coord);  // texel from lighwood.tga
			texel1 = texture(texmap, DataIn.tex_coord);  // texel from snow.jpeg
			colorOut += max(intensity*texel*texel1 + spec, 0.07*texel*texel1) / attenuation;
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