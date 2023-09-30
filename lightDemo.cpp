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

#include "avtFreeType.h"

#include "camera.h"
#include "light.h"
#include "snowball.h"

using namespace std;

#define CAPTION "AVT Demo: Phong Shading and Text rendered with FreeType"
int WindowHandle = 0;
int WinX = 1024, WinY = 768;

unsigned int FrameCount = 0;

//shaders
VSShaderLib shader;  //geometry
VSShaderLib shaderText;  //render bitmap text

//File with the font
const string font_name = "fonts/arial.ttf";

//Meshes
vector<struct MyMesh> myMeshes;
vector<struct MyMesh> housesMeshes;
vector<struct MyMesh> treesMeshes;
vector<struct MyMesh> sleighMesh;
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
GLint tex_loc, tex_loc1, tex_loc2;
GLint pointLight1_uniformId, pointLight2_uniformId, pointLight3_uniformId, pointLight4_uniformId, pointLight5_uniformId, pointLight6_uniformId;
GLint spotLightL_uniformId, spotLightR_uniformId;
GLint directionalLightOnId, pointLightsOnId, spotLightsOnId;
GLint spotDir_uniformId;

// Snowballs
int snowball_num = 30;

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
const int n_pointLights = 6;
const int n_spotlights = 2;
Light directionalLight;
Light pointLight[n_pointLights];
Light spotlight[n_spotlights];
float spotDir[4];

// Sleigh coordinates
float sleigh_x = 15.0f, sleigh_y = 5.0f, sleigh_z = 10.0f;
float sleigh_angle_v = 0.0f, sleigh_angle_h = 0.0f;
float sleigh_speed = 0.0f, max_speed = 2.0f;
float delta_t = 0.05, delta_v = 3.0f, delta_h = 3.0f, delta_s = 0.01f;
float sleigh_direction_x = 0.0f, sleigh_direction_y = 0.0f, sleigh_direction_z = 0.0f;

void timer(int value)
{
	std::ostringstream oss;
	oss << CAPTION << ": " << FrameCount << " FPS @ (" << WinX << "x" << WinY << ")";
	std::string s = oss.str();
	glutSetWindow(WindowHandle);
	glutSetWindowTitle(s.c_str());
	FrameCount = 0;

	//compute motion
	sleigh_direction_x = sin(sleigh_angle_h * 3.14f / 180);
	sleigh_direction_y = - sin(sleigh_angle_v * 3.14 / 180);
	sleigh_direction_z = cos(sleigh_angle_h * 3.14f / 180);

	sleigh_x -= sleigh_direction_x * sleigh_speed * delta_t;
	sleigh_y -= sleigh_direction_y * sleigh_speed * delta_t;
	sleigh_z -= sleigh_direction_z * sleigh_speed * delta_t;

	// set follow sleigh camera position
	float cam2_x = sleigh_x + sleigh_direction_x * camera_dist;
	float cam2_y = sleigh_y + sleigh_direction_y * camera_dist + camera_height;
	float cam2_z = sleigh_z + sleigh_direction_z * camera_dist;

	cams[2].setCameraPosition(cam2_x, cam2_y, cam2_z);
	cams[2].setCameraTarget(sleigh_x, sleigh_y, sleigh_z);
	cams[2].setCameraType(2);

	for (int i = 0; i < snowball_num; ++i) {
		snowballs[i].updateSnowballPosition(delta_t);
		if (snowballs[i].speed > 0) {
			snowballs[i].speed += 0.01f;
		}
		else {
			snowballs[i].speed -= 0.005f;
		}

		if (snowballs[i].getSnowballPosition()[0] > 50.0f ||
			snowballs[i].getSnowballPosition()[0] < -50.0f ||
			snowballs[i].getSnowballPosition()[1] > 50.0f ||
			snowballs[i].getSnowballPosition()[1] < -50.0f) {
			snowballs[i].generateRandomParameters(50.0f);
		}
	}

	glutTimerFunc(1 / delta_t, timer, 0);
}

void refresh(int value)
{
	glutPostRedisplay();
	glutTimerFunc(1000 / 60, refresh, 0);
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
	pointLight[0] = Light(30.0f, 10.0f, -50.0f, 1.0f);
	pointLight[1] = Light(30.0f, 10.0f, -40.0f, 1.0f);
	pointLight[2] = Light(30.0f, 10.0f, -30.0f, 1.0f);
	pointLight[3] = Light(30.0f, 10.0f, -20.0f, 1.0f);
	pointLight[4] = Light(30.0f, 10.0f, -10.0f, 1.0f); 
	pointLight[5] = Light(30.0f, 10.0f, 0.0f, 1.0f);
}

void setSpotLights() {
	spotlight[0] = Light(sleigh_x - 1.0f, sleigh_y + 0.5f, sleigh_z - 1.5f, 1.0f);
	spotlight[1] = Light(sleigh_x + 1.0f, sleigh_y + 0.5f, sleigh_z - 1.5f, 1.0f);

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
	glBindVertexArray(terrainMesh.vao);

	glDrawElements(terrainMesh.type, terrainMesh.numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	popMatrix(MODEL);
}

void renderHouses(void) {

	GLint loc;
	int houseId = 0;

	for (int i = 0; i < 4; ++i) {

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
		translate(MODEL, 0.0f, 0.0f, i * -8.0f);
		scale(MODEL, 3.0f, 3.0f, 3.0f);

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
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
		translate(MODEL, 1.5f, 0.0f, (i - 0.68f) * -8.0f);
		scale(MODEL, 3.0f, 3.0f, 3.0f);

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(treesMeshes[treeId].vao);

		glDrawElements(treesMeshes[treeId].type, treesMeshes[treeId].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);
		treeId++;
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
				translate(MODEL, 1.0f, - 0.7f,  pow(-1.0f, i / 2) * 0.6f);
			}
			else {
				translate(MODEL, - 1.0f, - 0.7f, - (pow(-1.0f, i / 2) * 0.6f));
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
		// rotate in the direction of the movement
		rotate(MODEL, atan2(snowballs[i].direction[1], snowballs[i].direction[0]) * 180.0f / 3.14f, 0.0f, 1.0f, 0.0f);

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(snowballMeshes[i].vao);

		glDrawElements(snowballMeshes[i].type, snowballMeshes[i].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);
	}
}

void renderScene(void) {

	FrameCount++;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);

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
		multMatrixPoint(VIEW, spotlight[i].getPosition(), res);
		spotlight[i].setEye(res[0], res[1], res[2], res[3]);
	}

	multMatrixPoint(VIEW, spotDir, res);

	glUniform4fv(spotLightL_uniformId, 1, spotlight[0].getEye());
	glUniform4fv(spotLightR_uniformId, 1, spotlight[1].getEye());
	glUniform4fv(spotDir_uniformId, 1, res);

	// Set global light
	multMatrixPoint(VIEW, directionalLight.getPosition(), res);
	glUniform4fv(directional_uniformId, 1, res);


	// Render objects
	renderTerrain();
	renderHouses();
	renderTrees();
	renderSleigh();
	renderSnowballs();

	pushMatrix(VIEW);
	loadIdentity(VIEW);

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

void processKeys(unsigned char key, int xx, int yy)
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

		case 'a': 
			sleigh_angle_h += delta_h; 
			break;

		case 'd': 
			sleigh_angle_h -= delta_h; 
			break;

		case 'w':
			sleigh_angle_v += delta_v; 
			break;

		case 's': 
			sleigh_angle_v -= delta_v; 
			break;

		case 'o': 
			sleigh_speed += delta_s;

			if (sleigh_speed > 0) {
				sleigh_speed -= delta_s * delta_t;
			} 
			if (sleigh_speed > max_speed) {
				sleigh_speed = max_speed;
			}
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
	}

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

	if (activeCam == 2) {
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

	if (activeCam == 2) {
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
	//glBindAttribLocation(shader.getProgramIndex(), TEXTURE_COORD_ATTRIB, "texCoord");

	glLinkProgram(shader.getProgramIndex());
	printf("InfoLog for Model Rendering Shader\n%s\n\n", shaderText.getAllInfoLogs().c_str());

	if (!shader.isProgramValid()) {
		printf("GLSL Model Program Not Valid!\n");
		exit(1);
	}

	pvm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_pvm");
	vm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_viewModel");
	normal_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_normal");
	tex_loc = glGetUniformLocation(shader.getProgramIndex(), "texmap");
	tex_loc1 = glGetUniformLocation(shader.getProgramIndex(), "texmap1");
	tex_loc2 = glGetUniformLocation(shader.getProgramIndex(), "texmap2");

	directionalLightOnId = glGetUniformLocation(shader.getProgramIndex(), "directionalLightOn");
	pointLightsOnId = glGetUniformLocation(shader.getProgramIndex(), "pointLightsOn");
	spotLightsOnId = glGetUniformLocation(shader.getProgramIndex(), "spotLightsOn");

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
	directionalLight = Light(0.0f, 25.0f, 10.0f, 0.0f);
	setPointLights();

	camX = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camZ = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camY = r * sin(beta * 3.14f / 180.0f);

	// set additional camera 1
	cams[0].setCameraPosition(camX, 60.0f, camZ); // top perspective
	cams[0].setCameraTarget(0.0f, 0.0f, -10.0f);
	cams[0].setCameraType(0);

	// set additional camera 2
	cams[1].setCameraPosition(0.1f, 50.0f, 0.0f); // top ortho
	cams[1].setCameraTarget(0.0f, 0.0f, -10.0f);
	cams[1].setCameraType(1);

	// set additional camera 3
	float cam2_x = - sleigh_direction_x * camera_dist;
	float cam2_y = (-sleigh_direction_y * camera_dist) + camera_height;
	float cam2_z = -sleigh_direction_z * camera_dist;

	cams[2].setCameraPosition(cam2_x, cam2_y, cam2_z);
	cams[2].setCameraTarget(sleigh_x, sleigh_y, sleigh_z);
	cams[2].setCameraType(0);

	float amb[] = { 0.2f, 0.15f, 0.1f, 1.0f };
	float diff[] = { 0.8f, 0.6f, 0.4f, 1.0f };
	float spec[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 100.0f;
	int texcount = 0;

	// create geometry and VAO of the terrain
	terrainMesh = createQuad(100.0f, 100.0f);
	memcpy(terrainMesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(terrainMesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(terrainMesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(terrainMesh.mat.emissive, emissive, 4 * sizeof(float));
	terrainMesh.mat.shininess = shininess;
	terrainMesh.mat.texCount = texcount;


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

	/*

	// create geometry and VAO of the sphere
	amesh = createSphere(1.0f, 20);
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);

	float amb1[]= {0.3f, 0.0f, 0.0f, 1.0f};
	float diff1[] = {0.8f, 0.1f, 0.1f, 1.0f};
	float spec1[] = {0.9f, 0.9f, 0.9f, 1.0f};
	shininess=500.0;

	// create geometry and VAO of the cylinder
	amesh = createCylinder(1.5f, 0.5f, 20);
	memcpy(amesh.mat.ambient, amb1, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec1, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);

	// create geometry and VAO of the cone
	amesh = createCone(1.5f, 0.5f, 20);
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh); */

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

	for (int i = 0; i < 5; i++) {
		// create geometry and VAO of the cone for each tree
		amesh = createCone(1.0f, 0.3f, 20);
		memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		treesMeshes.push_back(amesh);
	}

	for (int i = 0; i < snowball_num; i++) {
		// create geometry and VAO of the sphere for each snowball
		amesh = createSphere(0.3f, 20);
		memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		snowballMeshes.push_back(amesh);

		Snowball s = Snowball(50.0f);
		snowballs.push_back(s);
	}

	// some GL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
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
	//glutIdleFunc(renderScene);  // Use it for maximum performance
	glutTimerFunc(0, refresh, 0);    //use it to to get 60 FPS whatever

	//	Mouse and Keyboard Callbacks
	glutKeyboardFunc(processKeys);
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

	if (!setupShaders())
		return(1);

	init();

	//  GLUT main loop
	glutMainLoop();

	return(0);
}