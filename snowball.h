#ifndef SNOWBALL_H
#define SNOWBALL_H

#include <stdlib.h>
#include <vector>

#include "AABB.h"

class Snowball {
public:
	// Constructors
	Snowball(float radius);
	Snowball(float initPos[2], float speed, float dir[2]);

	// Setters
	void setSnowballPosition(float pos_x, float pos_z);
	void setSnowballSpeed(float speed);
	void setSnowballDirection(float dir_x, float dir_z);

	// Getters
	float* getSnowballPosition(void);
	float getSnowballSpeed(void);
	float* getSnowballDirection(void);
	AABB getSnowballAABB(void);

	float* updateSnowballPosition(float delta);
	void generateRandomParameters(float radius);

	float pos[2];
	float speed;
	float direction[2];
	AABB aabb;
};

#endif
