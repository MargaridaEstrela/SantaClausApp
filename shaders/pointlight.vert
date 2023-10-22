#version 430

uniform mat4 m_pvm;
uniform mat4 m_viewModel;
uniform mat3 m_normal;

uniform bool normalMap;

uniform vec4 directionalLight;

uniform vec4 pointLight1;
uniform vec4 pointLight2;
uniform vec4 pointLight3;
uniform vec4 pointLight4;
uniform vec4 pointLight5;
uniform vec4 pointLight6;

uniform vec4 spotLightL;
uniform vec4 spotLightR;

in vec4 position;
in vec4 normal, tangent, bitangent;		//por causa do gerador de geometria
in vec4 texCoord;

out Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir[9];
	vec2 tex_coord;
} DataOut;

void main () {
	vec3 n, t, b;
	vec3 lightDir[9], eyeDir;
	vec3 aux;

	vec4 pos = m_viewModel * position;

	n = normalize(m_normal * normal.xyz);
	eyeDir =  vec3(-pos);

	// Bump Mapping
	if(normalMap)  {  //convert eye and light vectors to tangent space

		//Calculate components of TBN basis in eye space
		t = normalize(m_normal * tangent.xyz);  
		b = tangent.w * cross(n,t);

		aux.x = dot(lightDir[0], t);
		aux.y = dot(lightDir[0], b);
		aux.z = dot(lightDir[0], n);
		lightDir[0] = normalize(aux);

		aux.x = dot(eyeDir, t);
		aux.y = dot(eyeDir, b);
		aux.z = dot(eyeDir, n);
		eyeDir = normalize(aux);

		DataOut.normal = n;
		DataOut.lightDir[0] = lightDir[0];
		DataOut.eye = eyeDir;

	}

	else{

		DataOut.normal = normalize(m_normal * normal.xyz);
		DataOut.eye = vec3(-pos);
		DataOut.lightDir[0] = vec3(directionalLight);

	}

	DataOut.lightDir[0] = vec3(directionalLight);
	DataOut.lightDir[1] = vec3(pointLight1 - pos);
	DataOut.lightDir[2] = vec3(pointLight2 - pos);
	DataOut.lightDir[3] = vec3(pointLight3 - pos);
	DataOut.lightDir[4] = vec3(pointLight4 - pos);
	DataOut.lightDir[5] = vec3(pointLight5 - pos);
	DataOut.lightDir[6] = vec3(pointLight6 - pos);
	DataOut.lightDir[7] = vec3(spotLightL - pos);
	DataOut.lightDir[8] = vec3(spotLightR - pos);

	DataOut.tex_coord = texCoord.st;

	gl_Position = m_pvm * position;	
}