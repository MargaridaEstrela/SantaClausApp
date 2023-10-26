//
// AVT: Phong Shading and Text rendered with FreeType library
// The text rendering was based on https://learnopengl.com/In-Practice/Text-Rendering
// This demo was built for learning purposes only.
// Some code could be severely optimised, but I tried to
// keep as simple and clear as possible.
//
// The code comes with no warranties, use it at your own risk.
// You may use it, or parts of it, wherever you want.
// 
// Author: João Madeiras Pereira
//

#include <math.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <chrono>

// include GLEW to access OpenGL 3.3 functions
#include <GL/glew.h>


// GLUT is the toolkit to interface with the OS
#include <GL/freeglut.h>

#include <IL/il.h>

// assimp include files. These three are usually needed.
#include "assimp/Importer.hpp"	//OO version Header!
#include "assimp/scene.h"

// Use Very Simple Libs
#include "VSShaderlib.h"
#include "AVTmathLib.h"
#include "VertexAttrDef.h"
#include "geometry.h"
#include "Texture_Loader.h"

#include "avtFreeType.h"

#include "camera.h"
#include "light.h"
#include "snowball.h"
#include "obstacle.h"
#include "AABB.h"

#include "meshFromAssimp.h"
#include "l3dBillboard.h"

#include "flare.h"

using namespace std;

#define CAPTION "SantaClaus App"

#define MAX_PARTICLES  6000
#define frand()			((float)rand()/RAND_MAX)

// Created an instance of the Importer class in the meshFromAssimp.cpp file
Assimp::Importer importer;
// the global Assimp scene object
const aiScene* scene;
char model_dir[50];  //initialized by the user input at the console
// scale factor for the Assimp model to fit in the window
float scaleFactor;

int fireworks = 0;

typedef struct {
	float	life;
	float	fade;
	float	r, g, b;    // color
	GLfloat x, y, z;    // position
	GLfloat vx, vy, vz; // speed 
	GLfloat ax, ay, az; // acceleration
} Particle;

Particle particle[MAX_PARTICLES];
int dead_num_particles = 0;

int WindowHandle = 0;
int WinX = 1024, WinY = 768;

inline double clamp(const double x, const double min, const double max) {
	return (x < min ? min : (x > max ? max : x));
}

inline int clampi(const int x, const int min, const int max) {
	return (x < min ? min : (x > max ? max : x));
}

unsigned int FrameCount = 0;
unsigned int startTime = glutGet(GLUT_ELAPSED_TIME);

//keyboard inputs
bool keyStates[256];

//game hub
bool paused = false;
int score = 0;
int final_score = 0;
int lives = 5;
int status = 0;				// 0:run; 1:paused; 2:game over

//shaders
VSShaderLib shader;			//geometry
VSShaderLib shaderText;		//render bitmap text

bool normalMapKey = TRUE; // by default if there is a normal map then bump effect is implemented. press key "b" to enable/disable normal mapping 
int bumpmap = 0;

//File with the font
const string font_name = "fonts/arial.ttf";

//Meshes
//vector<struct MyMesh> myMeshes;
vector<struct MyMesh> housesMeshes;
vector<struct MyMesh> treesMeshes;
vector<struct MyMesh> sleighMesh;
vector<struct MyMesh> pawnsMeshes;
vector<struct MyMesh> lampsMeshes;
MyMesh terrainMesh;
vector<struct MyMesh> snowballMeshes;
MyMesh stencilMesh;
vector<struct Snowball> snowballs;
vector<struct MyMesh> fireworkMeshes;
MyMesh flareMesh;
MyMesh skyboxMesh;
MyMesh environmentMesh;
vector<struct MyMesh> spiderMesh;

//Flare effect
FLARE_DEF AVTflare;
float lightScreenPos[3];  //Position of the light in Window Coordinates

//External array storage defined in AVTmathLib.cpp

/// The storage for matrices
extern float mMatrix[COUNT_MATRICES][16];
extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];

/// The normal matrix
extern float mNormal3x3[9];

GLint pvm_uniformId;
GLint vm_uniformId;
GLint normal_uniformId;
GLint model_uniformId;
GLint view_uniformId;
GLint directional_uniformId;
GLint pointLight1_uniformId, pointLight2_uniformId, pointLight3_uniformId, pointLight4_uniformId, pointLight5_uniformId, pointLight6_uniformId;
GLint spotLightL_uniformId, spotLightR_uniformId;
GLint fogOnId, directionalLightOnId, pointLightsOnId, spotLightsOnId;
GLint spotDir_uniformId;

GLint tex_loc, tex_loc1, tex_loc2, tex_loc3, tex_loc4, tex_loc5, tex_loc6, tex_loc7, tex_cube_loc, tex_normalMap_loc, tex_environmentMap_loc;
GLint texMode_uniformId;
GLuint TextureArray[9];
GLuint* SpiderArray;

GLint normalMap_loc;
GLint specularMap_loc;
GLint diffMapCount_loc;
GLint reflect_perFragment_uniformId;

GLint shadowMode_uniformId;
GLuint FlareTextureArray[5];

// Snowballs
int snowball_num = 50;

// Counters
int lamps_num = 6;
int houses_num = 4;
int trees_num = 36;

// Cameras
Camera cams[3];
float camera_dist = 6.5f, camera_height = 5.0f;
float camX, camY, camZ;
int activeCam = 0;

// Mouse Tracking Variables
int startX, startY, tracking = 0;

// Camera Spherical Coordinates
float alpha = 0.0f, beta = 90.0f;
float r = 10.0f;

// Frame counting and FPS computation
long myTime, timebase = 0, frame = 0;
char s[32];

//Lights
bool directionalLightOn = true;
bool pointLightsOn = false;
bool spotLightsOn = false;
bool flareEffect = false;
bool reflect_perFragment = true;
bool fog = false;
const int n_pointLights = 6;
const int n_spotlights = 2;
Light directionalLight;
Light pointLight[n_pointLights];
Light spotlight[n_spotlights];
float spotDir[4];
float lightPos[4] = { 10.0f, 6.0f, 2.0f, 1.0f }; //position of point light in World coordinates

// Sleigh
float sleigh_length = 3.0f, sleigh_width = 2.0f, sleigh_height = 2.0f;
float init_x = 10.f, init_y = 0.0f, init_z = 10.0f;
float sleigh_x = init_x, sleigh_y = init_y, sleigh_z = init_z;
float sleigh_angle_v = 0.0f, sleigh_angle_h = 0.0f;
float sleigh_speed = 0.0f, max_speed = 10.0f;
float delta_t = 0.05, delta_v = 3.0f, delta_h = 3.0f, delta_s = 0.01f;
float sleigh_direction_x = 0.0f, sleigh_direction_y = 0.0f, sleigh_direction_z = 0.0f;
AABB sleigh_aabb = AABB();

// Obstacles
float house_height = 4.0f, house_width = 6.0f;
vector<struct Obstacle> houses;
float tree_height = 4.0f, tree_width = 1.8f;
vector<struct Obstacle> trees;
float lamp_height = 2.5f, lamp_width = 0.5f;
vector<struct Obstacle> lamps;
bool collision = false;
bool isHit = false;
int keyUp = 0;

void updateParticles()
{
	int i;
	float h;

	/* Método de Euler de integração de eq. diferenciais ordinárias
	h representa o step de tempo; dv/dt = a; dx/dt = v; e conhecem-se os valores iniciais de x e v */

	//h = 0.125f;
	h = 0.033;
	if (fireworks) {

		for (i = 0; i < MAX_PARTICLES; i++)
		{
			particle[i].x += (h * particle[i].vx);
			particle[i].y += (h * particle[i].vy);
			particle[i].z += (h * particle[i].vz);
			particle[i].vx += (h * particle[i].ax);
			particle[i].vy += (h * particle[i].ay);
			particle[i].vz += (h * particle[i].az);
			particle[i].life -= particle[i].fade;
		}
	}
}

void initParticles(void)
{
	GLfloat v, theta, phi;
	int i;

	for (i = 0; i < MAX_PARTICLES; i++)
	{
		v = 0.8 * frand() + 0.2;
		phi = frand() * 3.14;
		theta = 2.0 * frand() * 3.14;

		if (i / (MAX_PARTICLES / 4) == 0) {
			particle[i].x = 40.0f;
			particle[i].z = 0.0f;
		}
		else if (i / (MAX_PARTICLES / 4) == 1) {
			particle[i].x = -40.0f;
			particle[i].z = 0.0f;
		}
		else if (i / (MAX_PARTICLES / 4) == 2) {
			particle[i].x = 0.0f;
			particle[i].z = 40.0f;
		}
		else if (i / (MAX_PARTICLES / 4) == 3) {
			particle[i].x = 0.0f;
			particle[i].z = -40.0f;
		}

		particle[i].y = 0.0f;
		particle[i].vx = v * cos(theta) * sin(phi);
		particle[i].vy = v * cos(phi);
		particle[i].vz = v * sin(theta) * sin(phi);
		particle[i].ax = 0.0f; /* simular um pouco de vento */
		particle[i].ay = 0.5f; /* simular a aceleração da gravidade */
		particle[i].az = 0.0f;

		/* tom amarelado que vai ser multiplicado pela textura que varia entre branco e preto */
		particle[i].r = 0.882f;
		particle[i].g = 0.552f;
		particle[i].b = 0.211f;

		particle[i].life = 1.0f;		/* vida inicial */
		particle[i].fade = 0.003f;	    /* step de decréscimo da vida para cada iteração */
	}
}

void updateSleighAABB(float x, float y, float z) {
	float x_min = 50, x_max = -50, y_min = 50, y_max = -50, z_min = 50, z_max = -50;
	float sleigh_vertices[8][3] = {
		{ -sleigh_width / 2, -sleigh_height / 2, -sleigh_length / 2 },
		{ -sleigh_width / 2, -sleigh_height / 2, sleigh_length / 2 },
		{ -sleigh_width / 2, sleigh_height / 2, -sleigh_length / 2 },
		{ -sleigh_width / 2, sleigh_height / 2, sleigh_length / 2 },
		{ sleigh_width / 2, -sleigh_height / 2, -sleigh_length / 2 },
		{ sleigh_width / 2, -sleigh_height / 2, sleigh_length / 2 },
		{ sleigh_width / 2, sleigh_height / 2, -sleigh_length / 2 },
		{ sleigh_width / 2, sleigh_height / 2, sleigh_length / 2 }
	};

	// Apply transformations to sleigh vertices
	for (int i = 0; i < 8; i++) {
		// Rotate around the vertical axis (sleigh_angle_h)
		float temp_x = sleigh_vertices[i][0] * cos(sleigh_angle_h * (3.14f / 180)) -
			sleigh_vertices[i][2] * sin(sleigh_angle_h * (3.14f / 180));
		float temp_z = sleigh_vertices[i][0] * sin(sleigh_angle_h * (3.14f / 180)) +
			sleigh_vertices[i][2] * cos(sleigh_angle_h * (3.14f / 180));

		sleigh_vertices[i][0] = temp_x;
		sleigh_vertices[i][2] = temp_z;

		// Rotate around the horizontal axis (sleigh_angle_v)
		float temp_y = sleigh_vertices[i][1] * cos(sleigh_angle_v * (3.14f / 180)) -
			sleigh_vertices[i][2] * sin(sleigh_angle_v * (3.14f / 180));
		temp_z = sleigh_vertices[i][1] * sin(sleigh_angle_v * (3.14f / 180)) +
			sleigh_vertices[i][2] * cos(sleigh_angle_v * (3.14f / 180));

		sleigh_vertices[i][1] = temp_y;
		sleigh_vertices[i][2] = temp_z;

		// Translate to the center of the sleigh
		sleigh_vertices[i][0] += x;
		sleigh_vertices[i][1] += y;
		sleigh_vertices[i][2] += z;

		// Update AABB
		if (sleigh_vertices[i][0] < x_min) x_min = sleigh_vertices[i][0];
		if (sleigh_vertices[i][0] > x_max) x_max = sleigh_vertices[i][0];
		if (sleigh_vertices[i][1] < y_min) y_min = sleigh_vertices[i][1];
		if (sleigh_vertices[i][1] > y_max) y_max = sleigh_vertices[i][1];
		if (sleigh_vertices[i][2] < z_min) z_min = sleigh_vertices[i][2];
		if (sleigh_vertices[i][2] > z_max) z_max = sleigh_vertices[i][2];
	}

	sleigh_aabb.update(x_min, x_max, y_min, y_max, z_min, z_max);
}

// ------------------------------------------------------------
//
// Restart Sleigh
//

void restartSleigh(void) {

	sleigh_speed = 0.0f;
	sleigh_x = init_x;
	sleigh_y = init_y;
	sleigh_z = init_z;
	sleigh_angle_v = 0.0f;
	sleigh_angle_h = 0.0f;
	sleigh_direction_x = 0.0f;
	sleigh_direction_y = 0.0f;
	sleigh_direction_z = 0.0f;
}

// ------------------------------------------------------------
//
// Restart Game
//

void restartGame(void) {

	// restart objects for initial position
	for (int i = 0; i < houses_num; i++) {
		houses[i].restartObject();
	}

	for (int i = 0; i < trees_num; i++) {
		trees[i].restartObject();
	}

	for (int i = 0; i < lamps_num; i++) {
		lamps[i].restartObject();
	}

	for (int i = 0; i < snowball_num; i++) {
		snowballs[i].restart();
	}

	restartSleigh();

	final_score = score;
	score = 0;
	lives = 5;
	status = 0;
	paused = false;
	startTime = glutGet(GLUT_ELAPSED_TIME);

}

// ------------------------------------------------------------
//
// Check Collisions
//

bool checkCollisions(float x, float y, float z) {
	//update sleigh aabb
	updateSleighAABB(x, y, z);

	//check collision with trees
	for (int i = 0; i < trees.size(); ++i) {
		if (trees[i].getObstacleAABB().intersects(sleigh_aabb)) {
			trees[i].updateObstaclePosition(sleigh_aabb, sin(sleigh_angle_h * 3.14f / 180), cos(sleigh_angle_h * 3.14f / 180), sleigh_speed, delta_t);
			return true;
		}
	}

	//check collision with houses
	for (int i = 0; i < houses.size(); ++i) {
		if (houses[i].getObstacleAABB().intersects(sleigh_aabb)) {
			houses[i].updateObstaclePosition(sleigh_aabb, sin(sleigh_angle_h * 3.14f / 180), cos(sleigh_angle_h * 3.14f / 180), sleigh_speed, delta_t);
			return true;
		}
	}

	//check collision with lamps
	for (int i = 0; i < lamps.size(); ++i) {
		if (lamps[i].getObstacleAABB().intersects(sleigh_aabb)) {
			lamps[i].updateObstaclePosition(sleigh_aabb, sin(sleigh_angle_h * 3.14f / 180), cos(sleigh_angle_h * 3.14f / 180), sleigh_speed, delta_t);
			return true;
		}
	}

	return false;
}

void refresh(int value)
{
	glutPostRedisplay();
	glutTimerFunc(1000 / 60, refresh, 0);
}

void timer(int value)
{
	std::ostringstream oss;
	oss << CAPTION << ": " << FrameCount << " FPS @ (" << WinX << "x" << WinY << ")";
	std::string s = oss.str();
	glutSetWindow(WindowHandle);
	glutSetWindowTitle(s.c_str());
	FrameCount = 0;

	if (paused || status == 2) {
		glutTimerFunc(0, timer, 0);
		return;
	}

	//compute motion
	sleigh_direction_x = sin(sleigh_angle_h * 3.14f / 180);
	sleigh_direction_y = -sin(sleigh_angle_v * 3.14 / 180);
	sleigh_direction_z = cos(sleigh_angle_h * 3.14f / 180);

	float new_x, new_y, new_z;
	new_x = sleigh_x - sleigh_direction_x * sleigh_speed * delta_t;
	new_y = sleigh_y - sleigh_direction_y * sleigh_speed * delta_t;
	new_z = sleigh_z - sleigh_direction_z * sleigh_speed * delta_t;

	if (new_y < sleigh_height / 2)
		new_y = sleigh_height / 2;

	collision = checkCollisions(new_x, new_y, new_z);

	for (int i = 2; i < snowball_num; ++i) {
		snowballs[i].updateSnowballPosition(delta_t);
		if (snowballs[i].speed > 0) {
			snowballs[i].speed += 0.001f;
		}
		else {
			snowballs[i].speed -= 0.001f;
		}

		if (snowballs[i].getSnowballPosition()[0] > 50.0f ||
			snowballs[i].getSnowballPosition()[0] < -50.0f ||
			snowballs[i].getSnowballPosition()[1] > 50.0f ||
			snowballs[i].getSnowballPosition()[1] < -50.0f) {
			snowballs[i].generateRandomParameters(50.0f);
		}

		//check collision with snowballs
		if (snowballs[i].getSnowballAABB().intersects(sleigh_aabb)) {
			collision = true;
			lives--;
			snowballs[i].generateRandomParameters(50.0f);
			restartSleigh();
		}

		if (lives == 0) {
			final_score = score;
			score = 0;
			status = 2; // game over
			break;
		}
	}

	if (!collision) {
		sleigh_x = new_x;
		sleigh_y = new_y;
		sleigh_z = new_z;
	}
	else {
		sleigh_speed = 0.0f;
	}

	if (tracking == 0) {
		// set follow sleigh camera position
		float cam2_x = sleigh_x + sleigh_direction_x * camera_dist;
		float cam2_y = sleigh_y + sleigh_direction_y * camera_dist + camera_height;
		float cam2_z = sleigh_z + sleigh_direction_z * camera_dist;

		cams[2].setCameraPosition(cam2_x, cam2_y, cam2_z);
		cams[2].setCameraTarget(sleigh_x, sleigh_y, sleigh_z);
	}

	if (score % 1000 == 0 && score > 0) {
		fireworks = 1;
		initParticles();
	}

	score++;
	glutTimerFunc(1000 / 60, timer, 0);
}

// ------------------------------------------------------------
//
// Reshape Callback Function
//

void changeSize(int w, int h) {

	float ratio;
	// Prevent a divide by zero, when window is too short
	if (h == 0)
		h = 1;
	// set the viewport to be the entire window
	glViewport(0, 0, w, h);
	// set the projection matrix
	ratio = (1.0f * w) / h;

	WinX = w;
	WinY = h;

	loadIdentity(PROJECTION);
	perspective(53.13f, ratio, 0.1f, 1000.0f);
}

// ------------------------------------------------------------
//
// Lights stuff
//

void setPointLights() {

	pointLight[0] = Light(lamps[0].pos[0], 4.25f, lamps[0].pos[1], 1.0f);
	pointLight[1] = Light(lamps[1].pos[0], 4.25f, lamps[1].pos[1], 1.0f);
	pointLight[2] = Light(lamps[2].pos[0], 4.25f, lamps[2].pos[1], 1.0f);
	pointLight[3] = Light(lamps[3].pos[0], 4.25f, lamps[3].pos[1], 1.0f);
	pointLight[4] = Light(lamps[4].pos[0], 4.25f, lamps[4].pos[1], 1.0f);
	pointLight[5] = Light(lamps[5].pos[0], 4.25f, lamps[5].pos[1], 1.0f);
}

void setSpotLights() {

	float spot0_x = sin((sleigh_angle_h + 25.0) * 3.14f / 180);
	float spot_y = -sin(sleigh_angle_v * 3.14 / 180);
	float spot0_z = cos((sleigh_angle_h + 900.0) * 3.14f / 180);

	float spot1_x = sin((sleigh_angle_h - 25.0) * 3.14f / 180);
	float spot1_z = cos((sleigh_angle_h - 900.0) * 3.14f / 180);

	spotlight[0] = Light(sleigh_x + spot0_x, sleigh_y + spot_y, sleigh_z + spot0_z, 1.0f);
	spotlight[1] = Light(sleigh_x + spot1_x, sleigh_y + spot_y, sleigh_z + spot1_z, 1.0f);

	spotDir[0] = -sleigh_direction_x;
	spotDir[1] = -sleigh_direction_y;
	spotDir[2] = -sleigh_direction_z;
	spotDir[3] = 0.0f;
}

void changeDirectionalLightMode() {
	directionalLight.changeMode();
}

void changePointLightsMode() {
	for (int i = 0; i < n_pointLights; i++) {
		pointLight[i].changeMode();
	}
}

void changeSpotlightsMode() {
	for (int i = 0; i < n_spotlights; i++) {
		spotlight[i].changeMode();
	}
}

void mirrorLights() {
	directionalLight.pos[1] *= -1.0f;

	for (int i = 0; i < n_pointLights; i++) {
		pointLight[i].pos[1] *= -1.0f;
	}

	for (int i = 0; i < n_spotlights; i++) {
		spotlight[i].pos[1] *= -1.0f;
	}
}

void loadLights() {
	float res[4];
	for (int i = 0; i < n_pointLights; i++) {
		multMatrixPoint(VIEW, pointLight[i].getPosition(), res);   // position definided em World Coord so is converted to eye space
		pointLight[i].setEye(res[0], res[1], res[2], res[3]);
	}

	glUniform4fv(pointLight1_uniformId, 1, pointLight[0].getEye());
	glUniform4fv(pointLight2_uniformId, 1, pointLight[1].getEye());
	glUniform4fv(pointLight3_uniformId, 1, pointLight[2].getEye());
	glUniform4fv(pointLight4_uniformId, 1, pointLight[3].getEye());
	glUniform4fv(pointLight5_uniformId, 1, pointLight[4].getEye());
	glUniform4fv(pointLight6_uniformId, 1, pointLight[5].getEye());

	// Set spotlights
	float model[4];
	for (int i = 0; i < n_spotlights; i++) {
		multMatrixPoint(MODEL, spotlight[i].getPosition(), model);
		multMatrixPoint(VIEW, spotlight[i].getPosition(), res);
		spotlight[i].setEye(res[0], res[1], res[2], res[3]);
	}

	// Set pointLights
	multMatrixPoint(VIEW, spotDir, res);

	glUniform4fv(spotLightL_uniformId, 1, spotlight[0].getEye());
	glUniform4fv(spotLightR_uniformId, 1, spotlight[1].getEye());
	glUniform4fv(spotDir_uniformId, 1, res);

	// Set global light
	multMatrixPoint(VIEW, directionalLight.getPosition(), res);
	glUniform4fv(directional_uniformId, 1, res);
}

// ------------------------------------------------------------
//
// Stencil stuff
//

void createFloorStencil(GLenum func) {
	GLint loc;

	glUseProgram(shader.getProgramIndex());

	glStencilFunc(GL_NEVER, 0x0, 0x1);
	glStencilOp(func, GL_KEEP, GL_KEEP);

	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, terrainMesh.mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, terrainMesh.mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, terrainMesh.mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, terrainMesh.mat.shininess);

	pushMatrix(MODEL);

	rotate(MODEL, 90, -1, 0, 0);

	glUniform1i(texMode_uniformId, 0);
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);


	// Render mesh
	glBindVertexArray(terrainMesh.vao);

	glDrawElements(terrainMesh.type, terrainMesh.numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	popMatrix(MODEL);
}

void createStencil(int h, int w, GLint ref) {
	/* create a diamond shaped stencil area */
	loadIdentity(PROJECTION);
	if (w <= h)
		ortho(-2.0, 2.0, -2.0 * (GLfloat)h / (GLfloat)w,
			2.0 * (GLfloat)h / (GLfloat)w, -10, 10);
	else
		ortho(-2.0 * (GLfloat)w / (GLfloat)h,
			2.0 * (GLfloat)w / (GLfloat)h, -2.0, 2.0, -10, 10);

	// load identity matrices for Model-View
	loadIdentity(VIEW);
	loadIdentity(MODEL);

	glUseProgram(shader.getProgramIndex());

	//nao vai ser preciso enviar o material pois o cubo nao e desenhado
	translate(MODEL, -0.5f, 2.0f, -0.5f);
	scale(MODEL, 1.0f, 0.5f, 1.0f);

	// send matrices to OGL
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	//glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

	glClear(GL_STENCIL_BUFFER_BIT);

	glStencilFunc(GL_NEVER, 0x0, 0x1);
	glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);

	glBindVertexArray(stencilMesh.vao);
	glDrawElements(stencilMesh.type, stencilMesh.numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

// ------------------------------------------------------------
//
// Render stufff
//

// Recursive render of the Assimp Scene Graph

void aiRecursive_render(const aiNode* nd, vector<struct MyMesh>& myMeshes, GLuint*& textureIds)
{
	GLint loc;

	// Get node transformation matrix
	aiMatrix4x4 m = nd->mTransformation;
	// OpenGL matrices are column major
	m.Transpose();

	// save model matrix and apply node transformation
	pushMatrix(MODEL);

	float aux[16];
	memcpy(aux, &m, sizeof(float) * 16);
	multMatrix(MODEL, aux);


	// draw all meshes assigned to this node
	for (unsigned int n = 0; n < nd->mNumMeshes; ++n) {

		// send the material
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, myMeshes[nd->mMeshes[n]].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, myMeshes[nd->mMeshes[n]].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, myMeshes[nd->mMeshes[n]].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.emissive");
		glUniform4fv(loc, 1, myMeshes[nd->mMeshes[n]].mat.emissive);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, myMeshes[nd->mMeshes[n]].mat.shininess);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.texCount");
		glUniform1i(loc, myMeshes[nd->mMeshes[n]].mat.texCount);

		unsigned int  diffMapCount = 0;  //read 2 diffuse textures

		//devido ao fragment shader suporta 2 texturas difusas simultaneas, 1 especular e 1 normal map

		glUniform1i(normalMap_loc, false);   //GLSL normalMap variable initialized to 0
		glUniform1i(specularMap_loc, false);
		glUniform1ui(diffMapCount_loc, 0);

		if (myMeshes[nd->mMeshes[n]].mat.texCount != 0)
			for (unsigned int i = 0; i < myMeshes[nd->mMeshes[n]].mat.texCount; ++i) {

				//Activate a TU with a Texture Object
				GLuint TU = myMeshes[nd->mMeshes[n]].texUnits[i];
				glActiveTexture(GL_TEXTURE0 + TU);
				glBindTexture(GL_TEXTURE_2D, SpiderArray[TU]);

				if (myMeshes[nd->mMeshes[n]].texTypes[i] == DIFFUSE) {
					if (diffMapCount == 0) {
						diffMapCount++;
						loc = glGetUniformLocation(shader.getProgramIndex(), "texUnitDiff");
						glUniform1i(loc, TU);
						glUniform1ui(diffMapCount_loc, diffMapCount);
					}
					else if (diffMapCount == 1) {
						diffMapCount++;
						loc = glGetUniformLocation(shader.getProgramIndex(), "texUnitDiff1");
						glUniform1i(loc, TU);
						glUniform1ui(diffMapCount_loc, diffMapCount);
					}
					else printf("Only supports a Material with a maximum of 2 diffuse textures\n");
				}
				else if (myMeshes[nd->mMeshes[n]].texTypes[i] == SPECULAR) {
					loc = glGetUniformLocation(shader.getProgramIndex(), "texUnitSpec");
					glUniform1i(loc, TU);
					glUniform1i(specularMap_loc, true);
				}
				else if (myMeshes[nd->mMeshes[n]].texTypes[i] == NORMALS) { //Normal map
					loc = glGetUniformLocation(shader.getProgramIndex(), "texUnitNormalMap");
					if (normalMapKey)
						glUniform1i(normalMap_loc, normalMapKey);
					glUniform1i(loc, TU);

				}
				else printf("Texture Map not supported\n");
			}

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// bind VAO
		glBindVertexArray(myMeshes[nd->mMeshes[n]].vao);

		if (!shader.isProgramValid()) {
			printf("Program Not Valid!\n");
			exit(1);
		}
		// draw
		glDrawElements(myMeshes[nd->mMeshes[n]].type, myMeshes[nd->mMeshes[n]].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	// draw all children
	for (unsigned int n = 0; n < nd->mNumChildren; ++n) {
		aiRecursive_render(nd->mChildren[n], myMeshes, SpiderArray);
	}
	popMatrix(MODEL);
}

void render_flare(FLARE_DEF* flare, int lx, int ly, int* m_viewport) {  //lx, ly represent the projected position of light on viewport

	int     dx, dy;          // Screen coordinates of "destination"
	int     px, py;          // Screen coordinates of flare element
	int		cx, cy;
	float    maxflaredist, flaredist, flaremaxsize, flarescale, scaleDistance;
	int     width, height, alpha;    // Piece parameters;
	int     i;
	float	diffuse[4];

	GLint loc;

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int screenMaxCoordX = m_viewport[0] + m_viewport[2] - 1;
	int screenMaxCoordY = m_viewport[1] + m_viewport[3] - 1;

	//viewport center
	cx = m_viewport[0] + (int)(0.5f * (float)m_viewport[2]) - 1;
	cy = m_viewport[1] + (int)(0.5f * (float)m_viewport[3]) - 1;

	// Compute how far off-center the flare source is.
	maxflaredist = sqrt(cx * cx + cy * cy);
	flaredist = sqrt((lx - cx) * (lx - cx) + (ly - cy) * (ly - cy));
	scaleDistance = (maxflaredist - flaredist) / maxflaredist;
	flaremaxsize = (int)(m_viewport[2] * flare->fMaxSize);
	flarescale = (int)(m_viewport[2] * flare->fScale);

	// Destination is opposite side of centre from source
	dx = clampi(cx + (cx - lx), m_viewport[0], screenMaxCoordX);
	dy = clampi(cy + (cy - ly), m_viewport[1], screenMaxCoordY);

	// Render each element. To be used Texture Unit 0

	glUniform1i(texMode_uniformId, 9); // draw modulated textured particles 
	glUniform1i(tex_loc, 0);  //use TU 0

	for (i = 0; i < flare->nPieces; ++i)
	{
		// Position is interpolated along line between start and destination.
		px = (int)((1.0f - flare->element[i].fDistance) * lx + flare->element[i].fDistance * dx);
		py = (int)((1.0f - flare->element[i].fDistance) * ly + flare->element[i].fDistance * dy);
		px = clampi(px, m_viewport[0], screenMaxCoordX);
		py = clampi(py, m_viewport[1], screenMaxCoordY);

		// Piece size are 0 to 1; flare size is proportion of screen width; scale by flaredist/maxflaredist.
		width = (int)(scaleDistance * flarescale * flare->element[i].fSize);

		// Width gets clamped, to allows the off-axis flaresto keep a good size without letting the elements get big when centered.
		if (width > flaremaxsize)  width = flaremaxsize;

		height = (int)((float)m_viewport[3] / (float)m_viewport[2] * (float)width);
		memcpy(diffuse, flare->element[i].matDiffuse, 4 * sizeof(float));
		diffuse[3] *= scaleDistance;   //scale the alpha channel
		if (width > 1)
		{
			// send the material - diffuse color modulated with texture
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
			glUniform4fv(loc, 1, diffuse);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, FlareTextureArray[flare->element[i].textureId]);
			pushMatrix(MODEL);
			translate(MODEL, (float)(px - width * 0.0f), (float)(py - height * 0.0f), 0.0f);
			scale(MODEL, (float)width, (float)height, 1);
			computeDerivedMatrix(PROJ_VIEW_MODEL);
			glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
			glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
			computeNormalMatrix3x3();
			glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

			glBindVertexArray(flareMesh.vao);
			glDrawElements(flareMesh.type, flareMesh.numIndexes, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			popMatrix(MODEL);
		}
	}
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

void renderTerrain(bool blend) {
	GLint loc;

	if (!blend) glDisable(GL_BLEND);
	if (activeCam == 2 && blend) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	// send the material
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, terrainMesh.mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, terrainMesh.mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, terrainMesh.mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, terrainMesh.mat.shininess);
	pushMatrix(MODEL);

	translate(MODEL, 2.0f, 0.0f, 2.0f);
	rotate(MODEL, -90.0f, 1.0f, 0.0f, 0.0f);

	// send matrices to OGL
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

	// Render mesh
	glUniform1i(texMode_uniformId, 0);
	glBindVertexArray(terrainMesh.vao);

	glDrawElements(terrainMesh.type, terrainMesh.numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	popMatrix(MODEL);

	glDisable(GL_BLEND);
}

void renderHouses(void) {

	GLint loc;
	int houseId = 0;

	for (int i = 0; i < 8; ++i) {

		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, housesMeshes[houseId].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, housesMeshes[houseId].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, housesMeshes[houseId].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, housesMeshes[houseId].mat.shininess);

		pushMatrix(MODEL);

		// set position and scale
		if (i < 4) {
			float* pos = houses[i].getObstaclePosition();
			translate(MODEL, pos[0], 0.0f, pos[1]);
			scale(MODEL, 3.0f, 3.0f, 3.0f);
		}
		else {
			float* pos = houses[i - 4].getObstaclePosition();
			translate(MODEL, pos[0] + 1.5f, 3.0f, pos[1] - 0.2 * -8.0f);
			rotate(MODEL, -45.0f, 0.0f, 1.0f, 0.0f);
		}

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		if (i >= 4) glUniform1i(texMode_uniformId, 1);
		else  glUniform1i(texMode_uniformId, 8);
		glBindVertexArray(housesMeshes[houseId].vao);

		glDrawElements(housesMeshes[houseId].type, housesMeshes[houseId].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);
		houseId++;
	}
}

void renderSleigh(void) {

	GLint loc;
	int sleighId = 0;

	for (int i = 0; i < 5; ++i) {

		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, sleighMesh[sleighId].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, sleighMesh[sleighId].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, sleighMesh[sleighId].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, sleighMesh[sleighId].mat.shininess);

		pushMatrix(MODEL);

		translate(MODEL, sleigh_x, sleigh_y, sleigh_z);
		rotate(MODEL, sleigh_angle_h, 0.0f, 1.0f, 0.0f);
		rotate(MODEL, sleigh_angle_v, 1.0f, 0.0f, 0.0f);

		if (i == 0) {
			// set position, rotation and scale for the main part
			rotate(MODEL, -90.0f, 1.0f, 0.0f, 0.0f);
		}
		else {
			if (i % 2) {
				translate(MODEL, 1.0f, -0.7f, pow(-1.0f, i / 2) * 0.6f);
			}
			else {
				translate(MODEL, -1.0f, -0.7f, -(pow(-1.0f, i / 2) * 0.6f));
			}
			rotate(MODEL, 180.0f, 1.0f, 1.0f, 0.0f);
		}

		if (i == 0) {
			scale(MODEL, 2.0f, 2.0f, 2.0f);
		}
		else {
			scale(MODEL, 1.0f, 0.1f, 1.0f);
		}

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glUniform1i(texMode_uniformId, 2);
		glBindVertexArray(sleighMesh[sleighId].vao);

		glDrawElements(sleighMesh[sleighId].type, sleighMesh[sleighId].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);
		sleighId++;
	}
}

void renderSnowballs(void) {
	GLint loc;

	for (int i = 0; i < snowball_num; ++i) {

		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, snowballMeshes[i].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, snowballMeshes[i].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, snowballMeshes[i].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, snowballMeshes[i].mat.shininess);

		pushMatrix(MODEL);

		translate(MODEL, snowballs[i].pos[0], 0.3f, snowballs[i].pos[1]);
		rotate(MODEL, atan2(snowballs[i].direction[1], snowballs[i].direction[0]) * 180.0f / 3.14f, 0.0f, 1.0f, 0.0f);

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		if (!bumpmap) {
			glUniform1i(texMode_uniformId, 10);
		}
		else {
			glUniform1i(texMode_uniformId, 11);
		}

		glBindVertexArray(snowballMeshes[i].vao);

		glDrawElements(snowballMeshes[i].type, snowballMeshes[i].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);
	}

	// Disable blending after rendering
	glDisable(GL_BLEND);
}

void renderLamps(void) {
	GLint loc;

	for (int i = 0; i < lamps_num * 2; ++i) {

		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, lampsMeshes[i].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, lampsMeshes[i].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, lampsMeshes[i].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, lampsMeshes[i].mat.shininess);
		pushMatrix(MODEL);

		// set position and scale
		if (i < lamps_num) {
			glDisable(GL_BLEND);
			float* pos = lamps[i].getObstaclePosition();
			translate(MODEL, pos[0], 1.0f, pos[1]);
		}
		else {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			float* pos = lamps[i - lamps_num].getObstaclePosition();
			translate(MODEL, pos[0], 2.25f, pos[1]);
		}

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		if (i < lamps_num) {
			glUniform1i(texMode_uniformId, 6);
		}
		else {
			glUniform1i(texMode_uniformId, 5);

		}
		glBindVertexArray(lampsMeshes[i].vao);

		glDrawElements(lampsMeshes[i].type, lampsMeshes[i].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);
	}

	glDisable(GL_BLEND);
}

void renderBillboards(void) {
	GLint loc;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUniform1i(texMode_uniformId, 4); // draw textured quads
	float pos[3];
	int type = 0;

	for (int i = 0; i < trees.size(); i++) {
		pos[0] = trees[i].pos[0]; pos[1] = 0.0; pos[2] = trees[i].pos[1];

		pushMatrix(MODEL);
		translate(MODEL, pos[0], 0.0, pos[2]);


		l3dBillboardCylindricalBegin(cams[activeCam].camPos, pos);

		//diffuse and ambient color are not used in the tree quads
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, treesMeshes[0].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, treesMeshes[0].mat.shininess);

		pushMatrix(MODEL);
		translate(MODEL, 0.0, 3.0, 0.0f);

		// send matrices to OGL
		if (type == 0 || type == 1) {     //Cheating matrix reset billboard techniques
			computeDerivedMatrix(VIEW_MODEL);

			BillboardCheatCylindricalBegin();

			computeDerivedMatrix_PVM(); // calculate PROJ_VIEW_MODEL
		}
		else computeDerivedMatrix(PROJ_VIEW_MODEL);

		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);
		glBindVertexArray(treesMeshes[0].vao);
		glDrawElements(treesMeshes[0].type, treesMeshes[0].numIndexes, GL_UNSIGNED_INT, 0);
		popMatrix(MODEL);
		popMatrix(MODEL);
	}
}

void renderFireworks(void)
{
	GLint loc;
	if (fireworks) {

		float particle_color[4];

		updateParticles();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDepthMask(GL_FALSE);  //Depth Buffer Read Only

		glUniform1i(texMode_uniformId, 7); // draw modulated textured particles 

		for (int i = 0; i < MAX_PARTICLES; i++)
		{
			if (particle[i].life > 0.0f)
			{
				particle_color[0] = particle[i].r;
				particle_color[1] = particle[i].g;
				particle_color[2] = particle[i].b;
				particle_color[3] = particle[i].life;

				// send the material - diffuse color modulated with texture
				loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
				glUniform4fv(loc, 1, particle_color);

				pushMatrix(MODEL);
				translate(MODEL, particle[i].x, particle[i].y, particle[i].z);

				if (i / (MAX_PARTICLES / 4) == 0) {
					rotate(MODEL, -90, 0, 1, 0);
				}
				else if (i / (MAX_PARTICLES / 4) == 1) {
					rotate(MODEL, 90, 0, 1, 0);
				}
				else if (i / (MAX_PARTICLES / 4) == 2) {
					rotate(MODEL, 180, 0, 1, 0);
				}
				else if (i / (MAX_PARTICLES / 4) == 3) {
					;
				}


				// send matrices to OGL
				computeDerivedMatrix(PROJ_VIEW_MODEL);
				glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
				glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
				computeNormalMatrix3x3();
				glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

				glBindVertexArray(fireworkMeshes[0].vao);
				glDrawElements(fireworkMeshes[0].type, fireworkMeshes[0].numIndexes, GL_UNSIGNED_INT, 0);
				popMatrix(MODEL);


			}
			else dead_num_particles++;
		}

		glDepthMask(GL_TRUE); //make depth buffer again writeable

		if (dead_num_particles == MAX_PARTICLES) {
			fireworks = 0;
			dead_num_particles = 0;
			printf("All particles dead\n");
		}

	}
}

void renderSkyBox(void) {
	glUniform1i(texMode_uniformId, 12);

	//it won't write anything to the zbuffer; all subsequently drawn scenery to be in front of the sky box. 
	glDepthMask(GL_FALSE);
	glFrontFace(GL_CW); // set clockwise vertex order to mean the front

	pushMatrix(MODEL);
	pushMatrix(VIEW);  //se quiser anular a translação

	//  Fica mais realista se não anular a translação da câmara 
	// Cancel the translation movement of the camera - de acordo com o tutorial do Antons
	mMatrix[VIEW][12] = 0.0f;
	mMatrix[VIEW][13] = 0.0f;
	mMatrix[VIEW][14] = 0.0f;

	scale(MODEL, 100.0f, 100.0f, 100.0f);
	translate(MODEL, -0.5f, -0.5f, -0.5f);

	// send matrices to OGL
	glUniformMatrix4fv(model_uniformId, 1, GL_FALSE, mMatrix[MODEL]); //Transformação de modelação do cubo unitário para o "Big Cube"
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);

	glBindVertexArray(skyboxMesh.vao);
	glDrawElements(skyboxMesh.type, skyboxMesh.numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	popMatrix(MODEL);
	popMatrix(VIEW);

	glFrontFace(GL_CCW); // restore counter clockwise vertex order to mean the front
	glDepthMask(GL_TRUE);
}

void renderEnvironmentCube(void) {

	GLint loc;

	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, environmentMesh.mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, environmentMesh.mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, environmentMesh.mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, environmentMesh.mat.shininess);
	pushMatrix(MODEL);
	translate(MODEL, 18.0f, 1.0f, -6.0f);
	scale(MODEL, 2.0f, 2.0f, 2.0f);

	// send matrices to OGL
	glUniformMatrix4fv(view_uniformId, 1, GL_FALSE, mMatrix[VIEW]);
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

	glUniform1i(texMode_uniformId, 13); //  Environmental cube mapping
	if (!reflect_perFragment)
		glUniform1i(reflect_perFragment_uniformId, 0); //reflected vector calculated in the vertex shader
	else
		glUniform1i(reflect_perFragment_uniformId, 1); //reflected vector calculated in the fragment shader

	glBindVertexArray(environmentMesh.vao);
	glDrawElements(environmentMesh.type, environmentMesh.numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	popMatrix(MODEL);
}

void renderRearView(void) {

	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	pushMatrix(VIEW);
	pushMatrix(MODEL);

	createStencil(WinX, WinY, 0x0);

	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	// load identity matrices
	loadIdentity(VIEW);
	loadIdentity(MODEL);

	// apply the appropriate camera projection
	pushMatrix(PROJECTION);
	loadIdentity(PROJECTION);

	// cameras
	float ratio = (1.0f * WinX) / WinY;

	// set additional camera 3 - rear view
	float cam_x = sleigh_x;
	float cam_y = sleigh_y + 1.0f;
	float cam_z = sleigh_z;

	float target_x = sleigh_x + sleigh_direction_x * 2;
	float target_y = sleigh_y + sleigh_direction_y * 2;
	float target_z = sleigh_z + sleigh_direction_z * 2;

	float pos[3] = { cam_x, cam_y, cam_z };
	float target[3] = { target_x, target_y, target_z };

	perspective(53.13f, ratio, 0.1f, 1000.0f);
	lookAt(pos[0], pos[1], pos[2], target[0], target[1], target[2], 0, 1, 0);

	// use our shader
	glUseProgram(shader.getProgramIndex());
	glUniform1i(fogOnId, fog);
	glUniform1i(directionalLightOnId, directionalLightOn);
	glUniform1i(pointLightsOnId, pointLightsOn);
	glUniform1i(spotLightsOnId, spotLightsOn);

	// Set pointLights
	float res[4];
	for (int i = 0; i < n_pointLights; i++) {
		multMatrixPoint(VIEW, pointLight[i].getPosition(), res);
		pointLight[i].setEye(res[0], res[1], res[2], res[3]);
	}

	glUniform4fv(pointLight1_uniformId, 1, pointLight[0].getEye());
	glUniform4fv(pointLight2_uniformId, 1, pointLight[1].getEye());
	glUniform4fv(pointLight3_uniformId, 1, pointLight[2].getEye());
	glUniform4fv(pointLight4_uniformId, 1, pointLight[3].getEye());
	glUniform4fv(pointLight5_uniformId, 1, pointLight[4].getEye());
	glUniform4fv(pointLight6_uniformId, 1, pointLight[5].getEye());

	// Set spotlights
	float model[4];
	setSpotLights();
	for (int i = 0; i < n_spotlights; i++) {
		multMatrixPoint(MODEL, spotlight[i].getPosition(), model);
		multMatrixPoint(VIEW, spotlight[i].getPosition(), res);
		spotlight[i].setEye(res[0], res[1], res[2], res[3]);
	}

	// Set pointLights
	setPointLights();

	multMatrixPoint(VIEW, spotDir, res);

	glUniform4fv(spotLightL_uniformId, 1, spotlight[0].getEye());
	glUniform4fv(spotLightR_uniformId, 1, spotlight[1].getEye());
	glUniform4fv(spotDir_uniformId, 1, res);

	// Set global light
	multMatrixPoint(VIEW, directionalLight.getPosition(), res);
	glUniform4fv(directional_uniformId, 1, res);

	// Associate Texture Units to Texture Objects
	// snow.png loaded in TU0; roof.png loaded in TU1; lightwood.tga in TU2, leaf.jpeg in TU3
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, TextureArray[0]);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, TextureArray[1]);

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, TextureArray[2]);

	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, TextureArray[3]);

	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, TextureArray[4]);

	glActiveTexture(GL_TEXTURE9 + 1);
	glBindTexture(GL_TEXTURE_2D, TextureArray[5]);

	glActiveTexture(GL_TEXTURE9 + 2);
	glBindTexture(GL_TEXTURE_2D, TextureArray[6]);

	glActiveTexture(GL_TEXTURE9 + 3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, TextureArray[7]);

	glUniform1i(tex_loc, 5);
	glUniform1i(tex_loc1, 6);
	glUniform1i(tex_loc2, 7);
	glUniform1i(tex_loc3, 8);
	glUniform1i(tex_loc4, 9);
	glUniform1i(tex_loc5, 10);
	glUniform1i(tex_loc6, 11);

	glStencilFunc(GL_EQUAL, 0x0, 0x1);

	// Render objects
	renderSkyBox();
	renderTerrain(false);
	renderHouses();
	renderBillboards();
	renderSleigh();
	renderSnowballs();
	renderLamps();

	//viewer at origin looking down at  negative z direction
	pushMatrix(MODEL);
	loadIdentity(MODEL);
	pushMatrix(PROJECTION);
	loadIdentity(PROJECTION);
	pushMatrix(VIEW);

	popMatrix(PROJECTION);
	popMatrix(VIEW);
	popMatrix(MODEL);

}

void renderScene(void) {

	createStencil(WinX, WinY, 0x0);

	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	FrameCount++;
	glClear(GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	pushMatrix(VIEW);
	pushMatrix(MODEL);

	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	// load identity matrices
	loadIdentity(VIEW);
	loadIdentity(MODEL);

	// apply the appropriate camera projection
	pushMatrix(PROJECTION);
	loadIdentity(PROJECTION);

	// cameras
	float ratio = (1.0f * WinX) / WinY;

	// Cameras
	if (cams[activeCam].getCameraType() == 1) {
		// top ortho
		ortho(-18.0f * ratio, 18.0f * ratio, -18.0f * ratio, 18.0f * ratio, -1, 1000);
	}
	else {
		perspective(53.13f, ratio, 0.1f, 1000.0f);
	}

	// set the camera using a function similar to gluLookAt
	lookAt(cams[activeCam].camPos[0], cams[activeCam].camPos[1], cams[activeCam].camPos[2],
		cams[activeCam].camTarget[0], cams[activeCam].camTarget[1], cams[activeCam].camTarget[2],
		0, 1, 0);


	// use our shader
	glUseProgram(shader.getProgramIndex());
	glUniform1i(fogOnId, fog);
	glUniform1i(directionalLightOnId, directionalLightOn);
	glUniform1i(pointLightsOnId, pointLightsOn);
	glUniform1i(spotLightsOnId, spotLightsOn);

	setSpotLights();
	setPointLights();
	loadLights();

	// Associate Texture Units to Texture Objects
	// snow.png loaded in TU0; roof.png loaded in TU1; lightwood.tga in TU2, tree.png in TU3
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, TextureArray[0]);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, TextureArray[1]);

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, TextureArray[2]);

	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, TextureArray[3]);

	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, TextureArray[4]);

	glActiveTexture(GL_TEXTURE9 + 1);
	glBindTexture(GL_TEXTURE_2D, TextureArray[5]);

	glActiveTexture(GL_TEXTURE9 + 2);
	glBindTexture(GL_TEXTURE_2D, TextureArray[6]);

	glActiveTexture(GL_TEXTURE9 + 3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, TextureArray[7]);

	glUniform1i(tex_loc, 5);
	glUniform1i(tex_loc1, 6);
	glUniform1i(tex_loc2, 7);
	glUniform1i(tex_loc3, 8);
	glUniform1i(tex_loc4, 9);
	glUniform1i(tex_loc5, 10);
	glUniform1i(tex_loc6, 11);
	glUniform1i(tex_cube_loc, 12);

	float mat[16];
	GLfloat floor[4] = { 0, 1, 0, 0 };

	glEnable(GL_DEPTH_TEST);

	//
	// SHADOWS
	//
	glClear(GL_STENCIL_BUFFER_BIT);
	createFloorStencil(GL_INCR);
	glStencilFunc(GL_EQUAL, 0x2, 0x1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	mirrorLights();
	loadLights();

	loadIdentity(MODEL);
	pushMatrix(MODEL);
	scale(MODEL, 1.0f, -1.0f, 1.0f);
	glCullFace(GL_FRONT);

	renderHouses();
	renderBillboards();
	//renderSleigh();
	renderSnowballs();
	renderLamps();
	renderFireworks();

	translate(MODEL, sleigh_x, sleigh_y, sleigh_z);
	rotate(MODEL, sleigh_angle_h, 0.0f, 1.0f, 0.0f);
	rotate(MODEL, sleigh_angle_v, 1.0f, 0.0f, 0.0f);

	scale(MODEL, scaleFactor * 6.0f, scaleFactor * 6.0f, scaleFactor * 6.0f);
	rotate(MODEL, 90, 0, 1, 0);

	aiRecursive_render(scene->mRootNode, spiderMesh, SpiderArray);

	glCullFace(GL_BACK);
	popMatrix(MODEL);

	mirrorLights();

	glClear(GL_STENCIL_BUFFER_BIT);
	glStencilFunc(GL_EQUAL, 0x1, 0x1);

	renderTerrain(true);

	glUniform1i(shadowMode_uniformId, 1);

	shadow_matrix(mat, floor, directionalLight.pos);
	glDisable(GL_DEPTH_TEST); //To force the shadow geometry to be rendered even if behind the floor

	//Dark the color stored in color buffer
	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_COLOR, GL_ZERO);

	loadIdentity(MODEL);
	pushMatrix(MODEL);
	multMatrix(MODEL, mat);

	renderHouses();
	//renderBillboards();
	//renderSleigh();
	renderSnowballs();
	renderLamps();
	//renderFireworks();

	translate(MODEL, sleigh_x, sleigh_y, sleigh_z);
	rotate(MODEL, sleigh_angle_h, 0.0f, 1.0f, 0.0f);
	rotate(MODEL, sleigh_angle_v, 1.0f, 0.0f, 0.0f);

	scale(MODEL, scaleFactor * 6.0f, scaleFactor * 6.0f, scaleFactor * 6.0f);
	rotate(MODEL, 90, 0, 1, 0);

	aiRecursive_render(scene->mRootNode, spiderMesh, SpiderArray);

	popMatrix(MODEL);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	//render the geometry
	glUniform1i(shadowMode_uniformId, 0);

	loadLights();
	loadIdentity(MODEL);

	// Render objects
	renderSkyBox();
	renderEnvironmentCube();
	renderHouses();
	renderBillboards();
	//renderSleigh();
	renderSnowballs();
	renderLamps();
	renderFireworks();

	translate(MODEL, sleigh_x, sleigh_y, sleigh_z);
	rotate(MODEL, sleigh_angle_h, 0.0f, 1.0f, 0.0f);
	rotate(MODEL, sleigh_angle_v, 1.0f, 0.0f, 0.0f);

	scale(MODEL, scaleFactor * 6.0f, scaleFactor * 6.0f, scaleFactor * 6.0f);
	rotate(MODEL, 90, 0, 1, 0);

	aiRecursive_render(scene->mRootNode, spiderMesh, SpiderArray);

	if (activeCam == 2) { // follow camera
		glEnable(GL_STENCIL_TEST);
		renderRearView();
	}

	glStencilFunc(GL_EQUAL, 0x1, 0x1);
	//glDisable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	if (flareEffect && !spotLightsOn) {

		int flarePos[2];
		int m_viewport[4];
		glGetIntegerv(GL_VIEWPORT, m_viewport);

		pushMatrix(MODEL);
		loadIdentity(MODEL);
		computeDerivedMatrix(PROJ_VIEW_MODEL);  //pvm to be applied to lightPost. pvm is used in project function

		if (!project(lightPos, lightScreenPos, m_viewport))
			printf("Error in getting projected light in screen\n");  //Calculate the window Coordinates of the light position: the projected position of light on viewport
		flarePos[0] = clampi((int)lightScreenPos[0], m_viewport[0], m_viewport[0] + m_viewport[2] - 1);
		flarePos[1] = clampi((int)lightScreenPos[1], m_viewport[1], m_viewport[1] + m_viewport[3] - 1);
		popMatrix(MODEL);

		//viewer looking down at  negative z direction
		pushMatrix(PROJECTION);
		loadIdentity(PROJECTION);
		pushMatrix(VIEW);
		loadIdentity(VIEW);
		ortho(m_viewport[0], m_viewport[0] + m_viewport[2] - 1, m_viewport[1], m_viewport[1] + m_viewport[3] - 1, -1, 1);

		render_flare(&AVTflare, flarePos[0], flarePos[1], m_viewport);

		popMatrix(PROJECTION);
		popMatrix(VIEW);
	}

	/*
	// TEXT
	*/

	//Render text (bitmap fonts) in screen coordinates. So use ortoghonal projection with viewport coordinates.
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);

	//viewer at origin looking down at  negative z direction
	pushMatrix(MODEL);
	loadIdentity(MODEL);
	pushMatrix(PROJECTION);
	loadIdentity(PROJECTION);
	pushMatrix(VIEW);
	loadIdentity(VIEW);

	ortho(m_viewport[0], m_viewport[0] + m_viewport[2] - 1, m_viewport[1], m_viewport[1] + m_viewport[3] - 1, -1, 1);

	if (paused) RenderText(shaderText, "Paused", WinX / 2 - 85, WinY / 2, 1.0f, 1.0f, 1.0f, 1.0f);
	else if (status == 2) {
		RenderText(shaderText, "Game Over", WinX / 2 - 115, WinY / 2 + 25, 1.0f, 1.0f, 0.0f, 0.0f);
		string finalScore = "Final Score: " + to_string(final_score);
		RenderText(shaderText, finalScore, WinX / 2 - 90, WinY / 2 - 25, 0.5f, 1.0f, 1.0f, 1.0f);
		RenderText(shaderText, "Press [R] to restart", WinX / 2 - 85, 50, 0.4f, 1.0f, 1.0f, 1.0f);
	}

	if (status != 2) {
		string scoreNow = "Score: " + to_string(score);
		RenderText(shaderText, scoreNow, 20, WinY - 40, 0.5f, 1.0f, 1.0f, 1.0f);
		string livesNow = "Lives: " + to_string(lives);
		RenderText(shaderText, livesNow, WinX - 100, WinY - 40, 0.5f, 1.0f, 1.0f, 1.0f);
	}

	popMatrix(PROJECTION);
	popMatrix(VIEW);
	popMatrix(MODEL);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glutSwapBuffers();
}

// ------------------------------------------------------------
//
// Events from the Keyboard
//

void processKeys(void)
{
	if (collision && keyStates['o']) keyStates['o'] = !keyStates['o'];

	if (keyStates['o']) {											// key 'o' pressed
		sleigh_speed += delta_s;

		if (sleigh_speed > 0) sleigh_speed -= delta_s * delta_t;
		if (sleigh_speed > max_speed) sleigh_speed = max_speed;

		// key 'a' pressed
		if (keyStates['a']) {
			sleigh_angle_h += delta_h;

			if (keyStates['w']) sleigh_angle_v += delta_v;			// key 'w' pressed
			else if (keyStates['s']) sleigh_angle_v -= delta_v;		// key 's' pressed
		}

		// key 'd' pressed
		else if (keyStates['d']) {
			sleigh_angle_h -= delta_h;

			if (keyStates['w']) sleigh_angle_v += delta_v;			// key 'w' pressed
			else if (keyStates['s']) sleigh_angle_v -= delta_v;		// key 's' pressed
		}

		// key 'w' pressed
		else if (keyStates['w']) {
			sleigh_angle_v += delta_v;

			if (keyStates['a']) sleigh_angle_h += delta_h;			// key 'a' pressed
			else if (keyStates['d']) sleigh_angle_h -= delta_h;;	// key 'd' pressed
		}

		// key 's' pressed
		else if (keyStates['s']) {
			sleigh_angle_v -= delta_v;

			if (keyStates['a']) sleigh_angle_h += delta_h;			// key 'a' pressed
			else if (keyStates['d']) sleigh_angle_h -= delta_h;;	// key 'd' pressed
		}
	}
	else if (sleigh_speed == 0 || !keyStates['o']) {
		sleigh_speed -= delta_s;

		if (sleigh_speed > 0) sleigh_speed -= delta_s * delta_t;
		if (sleigh_speed < 0) sleigh_speed = 0.0f;

		// key 'a' pressed
		if (keyStates['a']) {
			sleigh_angle_h += delta_h;

			if (keyStates['w']) sleigh_angle_v += delta_v;			// key 'w' pressed
			else if (keyStates['s']) sleigh_angle_v -= delta_v;		// key 's' pressed
		}

		// key 'd' pressed
		else if (keyStates['d']) {
			sleigh_angle_h -= delta_h;

			if (keyStates['w']) sleigh_angle_v += delta_v;			// key 'w' pressed
			else if (keyStates['s']) sleigh_angle_v -= delta_v;		// key 's' pressed
		}

		// key 'w' pressed
		else if (keyStates['w']) {
			sleigh_angle_v += delta_v;

			if (keyStates['a']) sleigh_angle_h += delta_h;			// key 'a' pressed
			else if (keyStates['d']) sleigh_angle_h -= delta_h;;	// key 'd' pressed
		}

		// key 's' pressed
		else if (keyStates['s']) {
			sleigh_angle_v -= delta_v;

			if (keyStates['a']) sleigh_angle_h += delta_h;			// key 'a' pressed
			else if (keyStates['d']) sleigh_angle_h -= delta_h;;	// key 'd' pressed
		}
	}
}

void processKeysDown(unsigned char key, int xx, int yy)
{
	switch (key) {
	case 27:
		glutLeaveMainLoop();
		break;

	case 'c':
		//printf("Camera Spherical Coordinates (%f, %f, %f)\n", alpha, beta, r);
		pointLightsOn = !pointLightsOn;
		printf("PointLights %s\n", pointLightsOn ? "ON" : "OFF");
		changePointLightsMode();
		break;

	case 'm':
		glEnable(GL_MULTISAMPLE);
		break;

	case 'h':
		spotLightsOn = !spotLightsOn;
		printf("Spotlights %s\n", spotLightsOn ? "ON" : "OFF");
		changeSpotlightsMode();
		break;

	case 'n':
		directionalLightOn = !directionalLightOn;
		printf("DirectionalLight %s\n", directionalLightOn ? "ON" : "OFF");
		changeDirectionalLightMode();
		break;

	case 'f':
		fog = !fog;
		break;

	case 'l':
		if (spotLightsOn) flareEffect = false;
		else flareEffect = !flareEffect;
		break;

	case 'b':
		bumpmap = !bumpmap;
		break;

	case 'p':
		paused = !paused;
		status = !status;
		if (!paused) startTime = glutGet(GLUT_ELAPSED_TIME);
		break;

	case 'r':
		if (status == 2) restartGame();
		break;

	case '1':
		activeCam = 0;
		break;

	case '2':
		activeCam = 1;
		break;

	case '3':
		activeCam = 2;
		break;

	default:
		keyStates[key] = true;
		break;
	}

	processKeys();
}

void processKeysUp(unsigned char key, int xx, int yy)
{
	keyStates[key] = false;     // Set the state of the current key to not pressed
}

// ------------------------------------------------------------
//
// Mouse Events
//

void processMouseButtons(int button, int state, int xx, int yy)
{
	// start tracking the mouse
	if (state == GLUT_DOWN) {
		startX = xx;
		startY = yy;
		if (button == GLUT_LEFT_BUTTON)
			tracking = 1;
		else if (button == GLUT_RIGHT_BUTTON)
			tracking = 2;
	}

	//stop tracking the mouse
	else if (state == GLUT_UP) {
		if (tracking == 1) {
			alpha -= (xx - startX);
			beta += (yy - startY);
		}
		else if (tracking == 2) {
			r += (yy - startY) * 0.01f;
			if (r < 0.1f)
				r = 0.1f;
		}
		tracking = 0;
	}
}

// Track mouse motion while buttons are pressed

void processMouseMotion(int xx, int yy)
{

	int deltaX, deltaY;
	float alphaAux, betaAux;
	float rAux;

	deltaX = -xx + startX;
	deltaY = yy - startY;

	// left mouse button: move camera
	if (tracking == 1) {

		alphaAux = alpha + deltaX;
		betaAux = beta + deltaY;

		if (betaAux > 85.0f)
			betaAux = 85.0f;
		else if (betaAux < -85.0f)
			betaAux = -85.0f;
		rAux = r;
	}
	// right mouse button: zoom
	else if (tracking == 2) {

		alphaAux = alpha;
		betaAux = beta;
		rAux = r + (deltaY * 0.01f);
		if (rAux < 0.1f)
			rAux = 0.1f;
	}

	if (activeCam == 2 && tracking == 1) {
		camX = rAux * sin(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
		camZ = rAux * cos(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
		camY = rAux * sin(betaAux * 3.14f / 180.0f);

		cams[activeCam].setCameraPosition(camX, camY, camZ);
	}

	//  uncomment this if not using an idle or refresh func
	//	glutPostRedisplay();
}


void mouseWheel(int wheel, int direction, int x, int y) {

	r += direction * 0.1f;
	if (r < 0.1f)
		r = 0.1f;

	if (activeCam == 2 && tracking == 1) {
		camX = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
		camZ = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
		camY = r * sin(beta * 3.14f / 180.0f);

		cams[activeCam].setCameraPosition(camX, camY, camZ);
	}

	//  uncomment this if not using an idle or refresh func
	//	glutPostRedisplay();
}

// --------------------------------------------------------
//
// Shader Stuff
//


GLuint setupShaders() {

	// Shader for models
	shader.init();
	shader.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/pointlight.vert");
	shader.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/pointlight.frag");

	// set semantics for the shader variables
	glBindFragDataLocation(shader.getProgramIndex(), 0, "colorOut");
	glBindAttribLocation(shader.getProgramIndex(), VERTEX_COORD_ATTRIB, "position");
	glBindAttribLocation(shader.getProgramIndex(), NORMAL_ATTRIB, "normal");
	glBindAttribLocation(shader.getProgramIndex(), TEXTURE_COORD_ATTRIB, "texCoord");
	glBindAttribLocation(shader.getProgramIndex(), TANGENT_ATTRIB, "tangent");
	//glBindAttribLocation(shader.getProgramIndex(), BITANGENT_ATTRIB, "bitangent");

	glLinkProgram(shader.getProgramIndex());
	printf("InfoLog for Model Rendering Shader\n%s\n\n", shaderText.getAllInfoLogs().c_str());

	/*if (!shader.isProgramValid()) {
		printf("GLSL Model Program Not Valid!\n");
		exit(1);
	}*/

	pvm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_pvm");
	vm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_viewModel");
	normal_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_normal");
	model_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_Model");
	view_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_View");

	// textures
	texMode_uniformId = glGetUniformLocation(shader.getProgramIndex(), "texMode");
	tex_loc = glGetUniformLocation(shader.getProgramIndex(), "texmap");
	tex_loc1 = glGetUniformLocation(shader.getProgramIndex(), "texmap1");
	tex_loc2 = glGetUniformLocation(shader.getProgramIndex(), "texmap2");
	tex_loc3 = glGetUniformLocation(shader.getProgramIndex(), "texmap3");
	tex_loc4 = glGetUniformLocation(shader.getProgramIndex(), "texmap4");
	tex_loc5 = glGetUniformLocation(shader.getProgramIndex(), "texmap5");
	tex_loc6 = glGetUniformLocation(shader.getProgramIndex(), "texmap6");
	tex_cube_loc = glGetUniformLocation(shader.getProgramIndex(), "cubeMap");
	reflect_perFragment_uniformId = glGetUniformLocation(shader.getProgramIndex(), "reflect_perFrag"); //reflection vector calculated in the frag shader
	//tex_normalMap_loc = glGetUniformLocation(shader.getProgramIndex(), "normalMap");

	normalMap_loc = glGetUniformLocation(shader.getProgramIndex(), "normalMap");
	specularMap_loc = glGetUniformLocation(shader.getProgramIndex(), "specularMap");
	diffMapCount_loc = glGetUniformLocation(shader.getProgramIndex(), "diffMapCount");

	// shadow
	shadowMode_uniformId = glGetUniformLocation(shader.getProgramIndex(), "shadowMode");

	// toggle lights
	fogOnId = glGetUniformLocation(shader.getProgramIndex(), "fog");
	directionalLightOnId = glGetUniformLocation(shader.getProgramIndex(), "directionalLightOn");
	pointLightsOnId = glGetUniformLocation(shader.getProgramIndex(), "pointLightsOn");
	spotLightsOnId = glGetUniformLocation(shader.getProgramIndex(), "spotLightsOn");

	// lights
	directional_uniformId = glGetUniformLocation(shader.getProgramIndex(), "directionalLight");

	pointLight1_uniformId = glGetUniformLocation(shader.getProgramIndex(), "pointLight1");
	pointLight2_uniformId = glGetUniformLocation(shader.getProgramIndex(), "pointLight2");
	pointLight3_uniformId = glGetUniformLocation(shader.getProgramIndex(), "pointLight3");
	pointLight4_uniformId = glGetUniformLocation(shader.getProgramIndex(), "pointLight4");
	pointLight5_uniformId = glGetUniformLocation(shader.getProgramIndex(), "pointLight5");
	pointLight6_uniformId = glGetUniformLocation(shader.getProgramIndex(), "pointLight6");

	spotLightL_uniformId = glGetUniformLocation(shader.getProgramIndex(), "spotLightL");
	spotLightR_uniformId = glGetUniformLocation(shader.getProgramIndex(), "spotLightR");

	spotDir_uniformId = glGetUniformLocation(shader.getProgramIndex(), "spotDir");

	printf("InfoLog for Per Fragment Phong Lightning Shader\n%s\n\n", shader.getAllInfoLogs().c_str());

	// Shader for bitmap Text
	shaderText.init();
	shaderText.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/text.vert");
	shaderText.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/text.frag");

	glLinkProgram(shaderText.getProgramIndex());
	printf("InfoLog for Text Rendering Shader\n%s\n\n", shaderText.getAllInfoLogs().c_str());

	if (!shaderText.isProgramValid()) {
		printf("GLSL Text Program Not Valid!\n");
		exit(1);
	}

	return(shader.isProgramLinked() && shaderText.isProgramLinked());
}

// ------------------------------------------------------------
//
// Model loading and OpenGL setup
//

void init()
{
	MyMesh amesh;

	/* Initialization of DevIL */
	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION)
	{
		printf("wrong DevIL version \n");
		exit(0);
	}
	ilInit();

	/// Initialization of freetype library with font_name file
	freeType_init(font_name);

	// Initialize lights
	directionalLight = Light(0.0f, 5.0f, 10.0f, 0.0f);

	camX = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camZ = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camY = r * sin(beta * 3.14f / 180.0f);

	// set additional camera 1
	cams[0].setCameraPosition(0.0f, 45.0f, 0.0f); // top perspective
	cams[0].setCameraTarget(0.0f, 0.0f, -7.0f);
	cams[0].setCameraType(0);

	// set additional camera 2
	cams[1].setCameraPosition(0.0f, 100.0f, 0.0f); // top ortho
	cams[1].setCameraTarget(0.0f, 0.0f, -10.0f);
	cams[1].setCameraType(1);

	// set additional camera 3
	float cam2_x = -sleigh_direction_x * camera_dist;
	float cam2_y = (-sleigh_direction_y * camera_dist) + camera_height;
	float cam2_z = -sleigh_direction_z * camera_dist;

	cams[2].setCameraPosition(cam2_x, cam2_y, cam2_z);
	cams[2].setCameraTarget(sleigh_x, sleigh_y + 10.0f, sleigh_z);
	cams[2].setCameraType(0);

	glGenTextures(8, TextureArray);
	Texture2D_Loader(TextureArray, "snow.jpeg", 0); // for terrain
	Texture2D_Loader(TextureArray, "roof.jpeg", 1); // for roof
	Texture2D_Loader(TextureArray, "lightwood.tga", 2); // for sleigh
	Texture2D_Loader(TextureArray, "tree.png", 3); // for trees
	Texture2D_Loader(TextureArray, "glass.jpeg", 4); // for lamps
	Texture2D_Loader(TextureArray, "green_metal.jpeg", 5); // for lamps
	Texture2D_Loader(TextureArray, "particle.tga", 6); // for fireworks

	//Sky Box Texture Object
	const char* filenames[] = { "posx.jpg", "negx.jpg", "posy.jpg", "negy.jpg", "posz.jpg", "negz.jpg" };
	TextureCubeMap_Loader(TextureArray, filenames, 7);
	std::cout << "loaded" << std::endl;

	//Flare elements textures
	glGenTextures(5, FlareTextureArray);
	Texture2D_Loader(FlareTextureArray, "crcl.tga", 0);
	Texture2D_Loader(FlareTextureArray, "flar.tga", 1);
	Texture2D_Loader(FlareTextureArray, "hxgn.tga", 2);
	Texture2D_Loader(FlareTextureArray, "ring.tga", 3);
	Texture2D_Loader(FlareTextureArray, "sun.tga", 4);


	float amb[] = { 0.2f, 0.15f, 0.1f, 1.0f };
	float diff[] = { 0.8f, 0.6f, 0.4f, 1.0f };
	float spec[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 100.0f;
	int texcount = 0;

	//
	// TERRAIN
	//

	//create geometry and VAO of the terrain
	terrainMesh = createQuad(100.0f, 100.0f);
	memcpy(terrainMesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(terrainMesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(terrainMesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(terrainMesh.mat.emissive, emissive, 4 * sizeof(float));
	terrainMesh.mat.shininess = shininess;
	terrainMesh.mat.texCount = texcount;

	//
	// SLEIGH
	//

	for (int i = 0; i < 5; i++) {
		// create geometry and VAO of the sleigh
		amesh = createCylinder(1.5f, 0.5f, 20);
		memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		sleighMesh.push_back(amesh);
	}

	//
	// HOUSES
	//

	for (int i = 0; i < 4; i++) {
		// create geometry and VAO of the cube for each house
		amesh = createCube();
		memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		housesMeshes.push_back(amesh);
	}

	for (int i = 0; i < 4; i++) {
		// create geometry and VAO of the cone for each house's roof
		amesh = createCone(1.0f, 2.4f, 4);
		memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		housesMeshes.push_back(amesh);
	}

	//
	// TREES
	//
	float tree_spec[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	float tree_shininess = 10.0f;

	amesh = createQuad(6, 6);
	memcpy(amesh.mat.specular, tree_spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = tree_shininess;
	amesh.mat.texCount = texcount;
	treesMeshes.push_back(amesh);

	//
	// SNOWBALL
	//

	for (int i = 0; i < snowball_num; i++) {
		// create geometry and VAO of the sphere for each snowball
		amesh = createSphere(0.3f, 20);
		memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = 1.0f;
		amesh.mat.texCount = texcount;
		snowballMeshes.push_back(amesh);

		Snowball s = Snowball(50.0f);
		snowballs.push_back(s);
	}

	//
	// PAWNS
	//

	float t_amb[] = { 0.2f, 0.15f, 0.1f, 0.3f }; // Set the alpha value to control transparency
	float t_diff[] = { 1.0f, 0.8f, 0.0f, 0.3f };

	for (int i = 0; i < 6; i++) {
		// create geometry and VAO of the pawn
		amesh = createPawn();
		memcpy(amesh.mat.ambient, t_amb, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, t_diff, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = 50.0f;
		amesh.mat.texCount = texcount;
		pawnsMeshes.push_back(amesh);
	}

	//
	// LAMPS
	//

	for (int i = 0; i < lamps_num; i++) {
		// create geometry and VAO of the lamps'cylinder
		amesh = createCylinder(2.0f, 0.06f, 20);
		memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		lampsMeshes.push_back(amesh);
	}

	for (int i = lamps_num; i < lamps_num * 2; i++) {
		// create geometry and VAO of the lamps' sphere
		amesh = createSphere(0.25f, 20);
		memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = 50.0f;
		amesh.mat.texCount = texcount;
		lampsMeshes.push_back(amesh);
	}

	// Fireworks
	amesh = createQuad(2, 2);
	amesh.mat.texCount = texcount;
	fireworkMeshes.push_back(amesh);

	//
	// CUBE
	//
	amesh = createCube();
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	stencilMesh = amesh;

	//
	// FLARE
	//

	flareMesh = createQuad(1, 1);

	//Load flare from file
	loadFlareFile(&AVTflare, "flare.txt");

	//
	// SkyBox
	//
	amesh = createCube();
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	skyboxMesh = amesh;

	//
	// ENVIRONMENT CUBE MAPPING
	// 
	amesh = createCube();
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	environmentMesh = amesh;

	std::string filepath = "sleigh/sleigh.obj";

	//import 3D file into Assimp scene graph
	if (!Import3DFromFile(filepath, importer, scene, scaleFactor))
		return;

	//creation of Mymesh array with VAO Geometry and Material and array of Texture Objs for the model input by the user
	spiderMesh = createMeshFromAssimp(scene, SpiderArray);


	// initialize obstacles
	for (int i = 0; i < 4; i++) {
		Obstacle house = Obstacle(0.0f, i * -8.0f, house_width, house_height, house_width);
		houses.push_back(house);
	}

	for (int i = -3; i < 3; i++) {
		for (int j = -3; j < 3; j++) {
			Obstacle tree = Obstacle(5 + i * 15.0, 5 + j * 15.0, tree_width, tree_height, tree_width);
			trees.push_back(tree);
		}
	}

	for (int i = 0; i < lamps_num; i++) {
		Obstacle lamp = Obstacle(20.0f, 10.0f - i * 10.0f, lamp_width, lamp_height, lamp_width);
		lamps.push_back(lamp);
	}

	// some GL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearStencil(1);
}

// ------------------------------------------------------------
//
// Main function
//

int main(int argc, char** argv) {

	//  GLUT initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);

	glutInitContextVersion(4, 1);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(WinX, WinY);
	WindowHandle = glutCreateWindow(CAPTION);


	//  Callback Registration
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);

	glutTimerFunc(0, timer, 0);
	glutTimerFunc(0, refresh, 0);    //use it to to get 60 FPS whatever

	// Mouse and Keyboard Callbacks
	glutKeyboardUpFunc(processKeysUp);
	glutKeyboardFunc(processKeysDown);
	glutMouseFunc(processMouseButtons);
	glutMotionFunc(processMouseMotion);
	glutMouseWheelFunc(mouseWheel);

	//	return from main loop
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	//	Init GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version: %s\n", glGetString(GL_VERSION));
	printf("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	std::cout << "status " << status << std::endl;
	if (!setupShaders())
		return(1);

	init();

	//  GLUT main loop
	glutMainLoop();

	return(0);
}

unsigned int getTextureId(char* name) {
	int i;

	for (i = 0; i < NTEXTURES; ++i)
	{
		if (strncmp(name, flareTextureNames[i], strlen(name)) == 0)
			return i;
	}
	return -1;
}

void    loadFlareFile(FLARE_DEF* flare, char* filename)
{
	int     n = 0;
	FILE* f;
	char    buf[256];
	int fields;

	memset(flare, 0, sizeof(FLARE_DEF));

	f = fopen(filename, "r");
	if (f)
	{
		fgets(buf, sizeof(buf), f);
		sscanf(buf, "%f %f", &flare->fScale, &flare->fMaxSize);

		while (!feof(f))
		{
			char            name[8] = { '\0', };
			double          dDist = 0.0, dSize = 0.0;
			float			color[4];
			int				id;

			fgets(buf, sizeof(buf), f);
			fields = sscanf(buf, "%4s %lf %lf ( %f %f %f %f )", name, &dDist, &dSize, &color[3], &color[0], &color[1], &color[2]);
			if (fields == 7)
			{
				for (int i = 0; i < 4; ++i) color[i] = clamp(color[i] / 255.0f, 0.0f, 1.0f);
				id = getTextureId(name);
				if (id < 0) printf("Texture name not recognized\n");
				else
					flare->element[n].textureId = id;
				flare->element[n].fDistance = (float)dDist;
				flare->element[n].fSize = (float)dSize;
				memcpy(flare->element[n].matDiffuse, color, 4 * sizeof(float));
				++n;
			}
		}

		flare->nPieces = n;
		fclose(f);
	}
	else printf("Flare file opening error\n");
}