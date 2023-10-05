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
	void setIsHit(bool isHit);

	// Getters
	float* getObstaclePosition(void);
	AABB getObstacleAABB(void);
	bool getIsHit(void);

	void updateObstaclePosition(float direction_x, float direction_z, float speed, float delta);

	float pos[2];
	float width, height, length;
	AABB aabb;
	bool isHit;
};

#endif

