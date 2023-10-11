#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <stdlib.h>
#include <vector>

#include "AABB.h"

class Obstacle {
public:
	// Constructors
	Obstacle();
	Obstacle(float x, float z, float width, float height, float length);

	// Setters
	void setObstaclePosition(float pos_x, float pos_z);

	// Getters
	float* getObstaclePosition(void);
	AABB getObstacleAABB(void);

	void updateObstaclePosition(AABB sleigh, float direction_x, float direction_z, float speed, float delta);

	float pos[2];
	float width, height, length;
	AABB aabb;
};

#endif

