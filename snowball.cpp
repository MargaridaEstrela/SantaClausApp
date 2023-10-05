#include "snowball.h"

Snowball::Snowball(float radius) {
    generateRandomParameters(radius);

    //generate random speed with minimim -0.5 and maximum 0.5f
    float speed = (rand() % 100) / 100.0f - 0.5f;

    setSnowballSpeed(speed);
    this->aabb = AABB(this->pos[0] - 0.15f, this->pos[0] + 0.15f, 0.0f, 0.3f, this->pos[1] - 0.15f, this->pos[1] + 0.15f);
}

Snowball::Snowball(float initPos[2], float speed, float dir[2]) {
    setSnowballPosition(initPos[0], initPos[1]);
    setSnowballDirection(dir[0], dir[1]);
    setSnowballSpeed(speed);
    this->aabb = AABB(this->pos[0] - 0.15f, this->pos[0] + 0.15f, 0.0f, 0.3f, this->pos[1] - 0.15f, this->pos[1] + 0.15f);
}

void Snowball::setSnowballPosition(float pos_x, float pos_z) {
    this->pos[0] = pos_x;
    this->pos[1] = pos_z;
}

void Snowball::setSnowballSpeed(float speed) {
	this->speed = speed;
}

void Snowball::setSnowballDirection(float dir_x, float dir_z) {
	this->direction[0] = dir_x;
	this->direction[1] = dir_z;
}

float* Snowball::getSnowballPosition(void) {
    return this->pos;
}

float Snowball::getSnowballSpeed(void) {
	return this->speed;
}

float* Snowball::getSnowballDirection(void) {
	return this->direction;
}

AABB Snowball::getSnowballAABB(void) {
	return this->aabb;
}

float* Snowball::updateSnowballPosition(float delta) {
    this->pos[0] += this->speed * this->direction[0] * delta;
    this->pos[1] += this->speed * this->direction[1] * delta;
    this->aabb.update(this->pos[0] - 0.15f, this->pos[0] + 0.15f, 0.0f, 0.3f, this->pos[1] - 0.15f, this->pos[1] + 0.15f);

    return this->pos;
}

void Snowball::generateRandomParameters(float radius) {
    //generate random position on a radius
    float angle = (rand() % 360) * 3.14f / 180.0f;
    float initPos[2] = { radius * cos(angle), radius * sin(angle) };

    //generate random direction
    float dir[2] = { (rand() % 100) / 100.0f, (rand() % 100) / 100.0f };

    setSnowballPosition(initPos[0], initPos[1]);
    setSnowballDirection(dir[0], dir[1]);
}