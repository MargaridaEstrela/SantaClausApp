#include "obstacle.h"

// Constructors
Obstacle::Obstacle() {
	setObstaclePosition(0.0f, 0.0f);
	setInitialPosition(0.0f, 0.0f);
	this->width = 0.0f;
	this->height = 0.0f;
	this->length = 0.0f;
	this->aabb = AABB();
}

Obstacle::Obstacle(float x, float z, float width, float height, float length) {
	setObstaclePosition(x, z);
	setInitialPosition(x, z);
	this->width = width;
	this->height = height;
	this->length = length;
	this->aabb = AABB(this->pos[0], this->pos[0] + width / 2, 0.0f, height, this->pos[1], this->pos[1] + length / 2);
}

// Setters
void Obstacle::setObstaclePosition(float pos_x, float pos_z) {
	this->pos[0] = pos_x;
	this->pos[1] = pos_z;
}

void Obstacle::setInitialPosition(float pos_x, float pos_z) {
	this->initialPos[0] = pos_x;
	this->initialPos[1] = pos_z;
}

// Getters
float* Obstacle::getObstaclePosition(void) {
	return this->pos;
}

float* Obstacle::getInitialPosition(void) {
	return this->initialPos;
}

AABB Obstacle::getObstacleAABB(void) {
	return this->aabb;
}

void Obstacle::restartObject(void) {
	float* pos = this->getInitialPosition();
	this->setObstaclePosition(pos[0], pos[1]);
}

void Obstacle::updateObstaclePosition(AABB sleigh, float direction_x, float direction_z, float speed, float delta) {
	float* dist_inter = this->aabb.getIntersectionDistance(sleigh);
	this->pos[0] -= dist_inter[0] -speed * direction_x * delta;
	this->pos[1] -= dist_inter[1] -speed * direction_z * delta;
	this->aabb = AABB(this->pos[0], this->pos[0] + width / 2, 0.0f, height, this->pos[1], this->pos[1] + length / 2);
}