#version 410

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
const float spotCutOff = 0.7;
const float spotExpo = 80.0f;

const float linear = 0.1;
const float expo = 0.1;

uniform bool directionalLightOn;
uniform bool pointLightsOn;
uniform bool spotLightsOn;

in Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir[9];
} DataIn;

void main() {

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
			if (dot(sd, l) < spotCutOff) intensity = 0.0;
			else attenuation = 1 / pow(dot(sd, l), spotExpo);
		}
	
		if (intensity > 0.0) {
			vec3 h = normalize(l + e);
			float intSpec = max(dot(h,n), 0.0);
			spec = mat.specular * pow(intSpec, mat.shininess);
		}
	
		colorOut += max(intensity * mat.diffuse + spec, mat.ambient) / attenuation;
	}

}