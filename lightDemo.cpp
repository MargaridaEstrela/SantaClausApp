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
#include <string>

// include GLEW to access OpenGL 3.3 functions
#include <GL/glew.h>


// GLUT is the toolkit to interface with the OS
#include <GL/freeglut.h>

#include <IL/il.h>


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

using namespace std;

#define CAPTION "SantaClaus App"
int WindowHandle = 0;
int WinX = 1024, WinY = 768;

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

//File with the font
const string font_name = "fonts/arial.ttf";

//Meshes
vector<struct MyMesh> myMeshes;
vector<struct MyMesh> housesMeshes;
vector<struct MyMesh> treesMeshes;
vector<struct MyMesh> sleighMesh;
vector<struct MyMesh> pawnsMeshes;
vector<struct MyMesh> lampsMeshes;
MyMesh terrainMesh;
vector<struct MyMesh> snowballMeshes;
vector<struct Snowball> snowballs;

//External array storage defined in AVTmathLib.cpp

/// The storage for matrices
extern float mMatrix[COUNT_MATRICES][16];
extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];

/// The normal matrix
extern float mNormal3x3[9];

GLint pvm_uniformId;
GLint vm_uniformId;
GLint normal_uniformId;
GLint directional_uniformId;
GLint pointLight1_uniformId, pointLight2_uniformId, pointLight3_uniformId, pointLight4_uniformId, pointLight5_uniformId, pointLight6_uniformId;
GLint spotLightL_uniformId, spotLightR_uniformId;
GLint fogOnId, directionalLightOnId, pointLightsOnId, spotLightsOnId;
GLint spotDir_uniformId;

GLint tex_loc, tex_loc1, tex_loc2, tex_loc3, tex_loc4, tex_loc5;
GLint texMode_uniformId;
GLuint TextureArray[6];

// Snowballs
int snowball_num = 50;

// Counters
int lamps_num = 6;
int houses_num = 4;
int trees_num = 5;

// Cameras
Camera cams[3];
float camera_dist = 5.0f, camera_height = 2.0f;
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
bool fog = false;
const int n_pointLights = 6;
const int n_spotlights = 2;
Light directionalLight;
Light pointLight[n_pointLights];
Light spotlight[n_spotlights];
float spotDir[4];

// Sleigh
float sleigh_length = 3.0f, sleigh_width = 2.0f, sleigh_height = 2.0f;
float init_x = 10.0f, init_y = 5.0f, init_z = 10.0f;
float sleigh_x = init_x, sleigh_y = init_y, sleigh_z = init_z;
float sleigh_angle_v = 0.0f, sleigh_angle_h = 0.0f;
float sleigh_speed = 0.0f, max_speed = 10.0f;
float delta_t = 0.05, delta_v = 3.0f, delta_h = 3.0f, delta_s = 0.01f;
float sleigh_direction_x = 0.0f, sleigh_direction_y = 0.0f, sleigh_direction_z = 0.0f;
AABB sleigh_aabb = AABB();

// Obstacles
float house_height = 4.0f, house_width = 6.0f;
vector<struct Obstacle> houses;
float tree_height = 3.0f, tree_width = 1.8f;
vector<struct Obstacle> trees;
float lamp_height = 4.5f, lamp_width = 0.5f;
vector<struct Obstacle> lamps;
bool collision = false;
int keyUp = 0;

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

	if (paused) {
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
			restartSleigh();
			collision = true;
			lives--;
		}

		if (lives == 0) {
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

	score++;
	glutTimerFunc(1 / delta_t, timer, 0);
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
	float spot0_z = cos((sleigh_angle_h + 150.0) * 3.14f / 180);

	float spot1_x = sin((sleigh_angle_h - 25.0) * 3.14f / 180);
	float spot1_z = cos((sleigh_angle_h - 150.0) * 3.14f / 180);

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

// ------------------------------------------------------------
//
// Render stufff
//

void renderTerrain(void) {
	GLint loc;

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
		else  glUniform1i(texMode_uniformId, 7);
		glBindVertexArray(housesMeshes[houseId].vao);

		glDrawElements(housesMeshes[houseId].type, housesMeshes[houseId].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);
		houseId++;
	}
}

void renderTrees(void) {

	GLint loc;
	int treeId = 0;

	// Enable blending for transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (int i = 0; i < 5; ++i) {

		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, treesMeshes[treeId].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, treesMeshes[treeId].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, treesMeshes[treeId].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, treesMeshes[treeId].mat.shininess);
		pushMatrix(MODEL);

		// set position and scale
		float* pos = trees[i].getObstaclePosition();
		translate(MODEL, pos[0], 0.0f, pos[1]);

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glUniform1i(texMode_uniformId, 4);
		glBindVertexArray(treesMeshes[treeId].vao);

		glDrawElements(treesMeshes[treeId].type, treesMeshes[treeId].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);
		treeId++;
	}

	// Disable blending after rendering
	glDisable(GL_BLEND);
}

void renderSleigh(void) {

	GLint loc;
	int sleighId = 0;

	// Enable blending for transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

	// Disable blending after rendering
	glDisable(GL_BLEND);
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
		glUniform1i(texMode_uniformId, 3);
		glBindVertexArray(snowballMeshes[i].vao);

		glDrawElements(snowballMeshes[i].type, snowballMeshes[i].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);
	}
}

void renderPawns(void) {
	GLint loc;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (int i = 0; i < 6; ++i) {

		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, pawnsMeshes[i].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, pawnsMeshes[i].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, pawnsMeshes[i].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, pawnsMeshes[i].mat.shininess);
		pushMatrix(MODEL);

		translate(MODEL, 20.0f, 0.0f, -10.0f * (i - 1));

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glUniform1i(texMode_uniformId, 2);
		glBindVertexArray(pawnsMeshes[i].vao);

		glDrawElements(pawnsMeshes[i].type, pawnsMeshes[i].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);
	}

	glDisable(GL_BLEND);
}

void renderLamps(void) {
	GLint loc;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
			float* pos = lamps[i].getObstaclePosition();
			translate(MODEL, pos[0], 0.0f, pos[1]);
		}
		else {
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

void renderScene(void) {

	FrameCount++;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	pushMatrix(VIEW);
	pushMatrix(MODEL);

	// load identity matrices
	loadIdentity(VIEW);
	loadIdentity(MODEL);

	// apply the appropriate camera projection
	pushMatrix(PROJECTION);
	loadIdentity(PROJECTION);

	float ratio = (1.0f * WinX) / WinY;

	if (cams[activeCam].getCameraType() == 1) {
		// top ortho
		ortho(-18.0f * ratio, 18.0f * ratio, -18.0f * ratio, 18.0f * ratio, -1, 1000);
	}
	else {
		perspective(53.13f, ratio, 0.1f, 1000.0f);
	}

	// set the camera using a function similar to gluLookAt
	//lookAt(camX, camY, camZ, 0, 0, 0, 0, 1, 0);
	lookAt(cams[activeCam].camPos[0], cams[activeCam].camPos[1], cams[activeCam].camPos[2],
		cams[activeCam].camTarget[0], cams[activeCam].camTarget[1], cams[activeCam].camTarget[2],
		0, 1, 0);

	// use our shader
	glUseProgram(shader.getProgramIndex());
	glUniform1i(fogOnId, fog);
	glUniform1i(directionalLightOnId, directionalLightOn);
	glUniform1i(pointLightsOnId, pointLightsOn);
	glUniform1i(spotLightsOnId, spotLightsOn);

	// Set pointLights
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
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureArray[0]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, TextureArray[1]);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, TextureArray[2]);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, TextureArray[3]);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, TextureArray[4]);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, TextureArray[5]);

	glUniform1i(tex_loc, 0);
	glUniform1i(tex_loc1, 1);
	glUniform1i(tex_loc2, 2);
	glUniform1i(tex_loc3, 3);
	glUniform1i(tex_loc4, 4);
	glUniform1i(tex_loc5, 5);

	// Render objects
	renderTerrain();
	renderHouses();
	renderTrees();
	renderSleigh();
	renderSnowballs();
	renderLamps();

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

	//std::cout << "status " << status << std::endl;

	if (paused) RenderText(shaderText, "Paused", WinX / 2 - 85, WinY / 2, 1.0f, 1.0f, 1.0f, 1.0f);
	else if (status == 2) {
		RenderText(shaderText, "Game Over", WinX / 2 - 125, WinY / 2, 1.0f, 1.0f, 0.0f, 0.0f);
		string final_score = "Final Score: " + to_string(score);
		RenderText(shaderText, final_score, WinX / 2 - 85, WinY / 2 - 50, 0.5f, 1.0f, 1.0f, 1.0f);
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

	case 'p':
		paused = !paused;
		status = !status;
		if (!paused) startTime = glutGet(GLUT_ELAPSED_TIME);
		std::cout << "status " << status << std::endl;
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

	glLinkProgram(shader.getProgramIndex());
	printf("InfoLog for Model Rendering Shader\n%s\n\n", shaderText.getAllInfoLogs().c_str());

	/*if (!shader.isProgramValid()) {
		printf("GLSL Model Program Not Valid!\n");
		exit(1);
	}*/

	pvm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_pvm");
	vm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_viewModel");
	normal_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_normal");

	// textures
	texMode_uniformId = glGetUniformLocation(shader.getProgramIndex(), "texMode");
	tex_loc = glGetUniformLocation(shader.getProgramIndex(), "texmap");
	tex_loc1 = glGetUniformLocation(shader.getProgramIndex(), "texmap1");
	tex_loc2 = glGetUniformLocation(shader.getProgramIndex(), "texmap2");
	tex_loc3 = glGetUniformLocation(shader.getProgramIndex(), "texmap3");
	tex_loc4 = glGetUniformLocation(shader.getProgramIndex(), "texmap4");
	tex_loc5 = glGetUniformLocation(shader.getProgramIndex(), "texmap5");

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
	cams[0].setCameraPosition(camX, 60.0f, camZ); // top perspective
	cams[0].setCameraTarget(0.0f, 0.0f, -10.0f);
	cams[0].setCameraType(0);

	// set additional camera 2
	cams[1].setCameraPosition(0.1f, 100.0f, 0.0f); // top ortho
	cams[1].setCameraTarget(0.0f, 0.0f, -10.0f);
	cams[1].setCameraType(1);

	// set additional camera 3
	float cam2_x = -sleigh_direction_x * camera_dist;
	float cam2_y = (-sleigh_direction_y * camera_dist) + camera_height;
	float cam2_z = -sleigh_direction_z * camera_dist;

	cams[2].setCameraPosition(cam2_x, cam2_y, cam2_z);
	cams[2].setCameraTarget(sleigh_x, sleigh_y, sleigh_z);
	cams[2].setCameraType(0);

	glGenTextures(6, TextureArray);
	Texture2D_Loader(TextureArray, "snow.jpeg", 0); // for terrain
	Texture2D_Loader(TextureArray, "roof.jpeg", 1); // for roof
	Texture2D_Loader(TextureArray, "lightwood.tga", 2); // for sleigh
	Texture2D_Loader(TextureArray, "leaf.jpeg", 3); // for trees
	Texture2D_Loader(TextureArray, "glass.jpeg", 4); // for lamps
	Texture2D_Loader(TextureArray, "green_metal.jpeg", 5); // for lamps

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

	for (int i = 0; i < 5; i++) {
		// create geometry and VAO of the cone for each tree
		amesh = createCone(3.0f, 0.9f, 20);
		memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		treesMeshes.push_back(amesh);
	}

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
		amesh = createCylinder(4.0f, 0.06f, 20);
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

	// initialize obstacles
	for (int i = 0; i < 4; i++) {
		Obstacle house = Obstacle(0.0f, i * -8.0f, house_width, house_height, house_width);
		houses.push_back(house);
	}

	for (int i = 0; i < 5; i++) {
		Obstacle tree = Obstacle(1.5f, (i - 0.68f) * -8.0f, tree_width, tree_height, tree_width);
		trees.push_back(tree);
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
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

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