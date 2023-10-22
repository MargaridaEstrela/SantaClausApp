#version 410

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
in vec4 normal;		//por causa do gerador de geometria
in vec4 texCoord;
in vec3 tangent, bitangent;

out Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir[9];
	vec2 tex_coord;
} DataOut;

void main () {

	vec3 n, t, b;
	vec4 pos = m_viewModel * position;
	vec3 lDir, eyeDir;
	vec3 aux;

	n = normalize(m_normal * normal.xyz);
	eyeDir =  vec3(-pos);
	lDir = vec3(directionalLight);

	if(normalMap)  {  //transform eye and light vectors by tangent basis
		t = normalize(m_normal * tangent.xyz);
		b = normalize(m_normal * bitangent.xyz);

		aux.x = dot(lDir, t);
		aux.y = dot(lDir, b);
		aux.z = dot(lDir, n);
		lDir = normalize(aux);

		aux.x = dot(eyeDir, t);
		aux.y = dot(eyeDir, b);
		aux.z = dot(eyeDir, n);
		eyeDir = normalize(aux);
	}

	DataOut.normal = n;
	DataOut.eye = eyeDir;

	DataOut.lightDir[0] = lDir;
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