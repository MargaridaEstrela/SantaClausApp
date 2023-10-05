#include "obstacle.h"

// Constructors
Obstacle::Obstacle() {
	setObstaclePosition(0.0f, 0.0f);
	this->width = 0.0f;
	this->height = 0.0f;
	this->length = 0.0f;
	this->aabb = AABB();
	this->isHit = false;
}

Obstacle::Obstacle(float x, float z, float width, float height, float length) {
	setObstaclePosition(x, z);
	this->width = width;
	this->height = height;
	this->length = length;
	this->aabb = AABB(this->pos[0], this->pos[0] + width / 2, 0.0f, height, this->pos[1], this->pos[1] + length / 2);
	this->isHit = false;
}

// Setters
void Obstacle::setObstaclePosition(float pos_x, float pos_z) {
	this->pos[0] = pos_x;
	this->pos[1] = pos_z;
}

void Obstacle::setIsHit(bool isHit) {
	this->isHit = isHit;
}

// Getters
float* Obstacle::getObstaclePosition(void) {
	return this->pos;
}

AABB Obstacle::getObstacleAABB(void) {
	return this->aabb;
}

bool Obstacle::getIsHit(void) {
	return this->isHit;
}

void Obstacle::updateObstaclePosition(float direction_x, float direction_z, float speed, float delta) {
	this->pos[0] -= speed * direction_x * delta;
	this->pos[1] -= speed * direction_z * delta;
	this->aabb = AABB(this->pos[0], this->pos[0] + width / 2, 0.0f, height, this->pos[1], this->pos[1] + length / 2);
}