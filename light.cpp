#include "light.h"

Light::Light() {}

Light::Light(float posX, float posY, float posZ, float pointLight) {
	setPosition(posX, posY, posZ, pointLight);
	mode = true;
}

void Light::setPosition(float posX, float posY, float posZ, float pointLight) {
	this->pos[0] = posX;
	this->pos[1] = posY;
	this->pos[2] = posZ;
	this->pos[3] = pointLight;
}

float* Light::getPosition() {
	return this->pos;
}

void Light::changeMode() {
	this->mode = !this->mode;
}

bool Light::getMode() {
	return this->mode;
}