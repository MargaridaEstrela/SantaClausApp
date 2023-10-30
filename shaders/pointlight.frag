#version 410

uniform sampler2D texmap;
uniform sampler2D texmap1;
uniform sampler2D texmap2;
uniform sampler2D texmap3;
uniform sampler2D texmap4;
uniform sampler2D texmap5;
uniform sampler2D texmap6;
uniform samplerCube cubeMap;
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

const vec4 shadowC = vec4(0.2, 0.2, 0.2, 0.2);
const float reflect_factor = 0.9;

in Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir[9];
	vec2 tex_coord;
	vec3 skyboxTexCoord;
	vec3 reflected;
} DataIn;

void main() {
	colorOut = vec4(0);

	if(shadowMode && !fog) {
		colorOut = shadowC;
		return;
	}

	if (!directionalLightOn && !pointLightsOn && !spotLightsOn) return;

	vec4 spec = vec4(0.0);
	vec4 texel, texel1, cube_texel;

	vec3 n = normalize(DataIn.normal);
	if(normalMap || texMode == 11)
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

		if (i == 0) intensity *= 0.5;				// directional light
		else if (i >= 7) intensity *= 0.1;			// spotlights
		else intensity *= 0.6;						// pointlights
	
		colorOut += max(intensity * mat.diffuse + spec, mat.ambient) / attenuation;
		
		if (texMode == 0) // modulate diffuse color with texel color
		{
			texel = texture(texmap, DataIn.tex_coord);  // texel from lighwood.tga
			texel1 = texture(texmap, DataIn.tex_coord);  // texel from snow.jpeg
			colorOut += max(intensity*texel + intensity*texel1 + spec, 0.07*texel*texel1) / attenuation;
			colorOut[3] = 0.8;
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
				colorOut += min(intensity*texel + spec, 0.8*texel) / attenuation;
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
		else if (texMode == 7) // fireworks
		{
			texel = texture(texmap6, DataIn.tex_coord);
			colorOut = 0.7 * texel;
		}	
		else if (texMode == 8) // multitexturing	
		{
			texel = texture(texmap2, DataIn.tex_coord);  // texel from lighwood.tga
			texel1 = texture(texmap, DataIn.tex_coord);  // texel from snow.jpeg
			colorOut += max(intensity*texel*texel1 + spec, 0.07*texel*texel1) / attenuation;
		} 
		else if (texMode == 9) // Flare
		{
			texel = texture(texmap, DataIn.tex_coord);  //texel from element flare texture
			if((texel.a == 0.0)  || (mat.diffuse.a == 0.0) ) discard;
			else
				colorOut = mat.diffuse * texel;
		} 
		else if (texMode == 10 || texMode == 11) // texMode==11 normal comes from normalMap, if ==10 means regular normal vector 
		{
			texel = texture(texmap, DataIn.tex_coord);  // texel from snow.png
			colorOut += max(intensity*texel + spec, 0.5*texel) / attenuation;
		}
		else if (texMode == 12) 
		{
			colorOut = texture(cubeMap, DataIn.skyboxTexCoord);
		}
		else if (texMode == 13)
		{
			if(reflect_perFrag == 1) {  //reflected vector calculated here
				vec3 reflected1 = vec3 (transpose(m_View) * vec4 (vec3(reflect(-e, n)), 0.0)); //reflection vector in world coord
				reflected1.x= -reflected1.x;   
				cube_texel = texture(cubeMap, reflected1);
			}
			else
				cube_texel = texture(cubeMap, DataIn.reflected); //use interpolated reflected vector calculated in vertex shader
	
			texel = texture(texmap4, DataIn.tex_coord);  // texel from lighwood.tga
			vec4 aux_color = mix(texel, cube_texel, reflect_factor);
			colorOut += max(intensity*aux_color + spec, 0.07*aux_color); 
		}
	}

	if (fog) {
		float distance = length(DataIn.eye) - 1;
		distance = max(0, distance);
		float visibility = exp(-pow(distance * density, gradient));
		visibility = clamp(visibility, 0.0, 1.0);

		if (shadowMode)
			colorOut = mix(vec4(0.3, 0.3, 0.3, 1), shadowC, visibility);
		else
			colorOut = mix(vec4(0.5, 0.5, 0.5, 1), colorOut, visibility);
	}

}