#ifndef CAMERA_H
#define CAMERA_H

#include <stdlib.h>
#include <vector>

class Camera {
public:
	// Constructors
	Camera();
	Camera(float camPos[3], float camTarget[3], int type);

	// Setters
	void setCameraPosition(float pos_x, float pos_y, float pos_z);
	void setCameraTarget(float target_x, float target_y, float target_z);
	void setCameraType(int type);

	// Getters
	float* getCameraPosition(void);
	float* getCameraTarget(void);
	int getCameraType(void);

	float camPos[3];
	float camTarget[3];
	int type;
};

#endif
