#include <iostream>
#include <algorithm>
#include "AABB.h"

AABB::AABB() {
	this->xmin = 0.0f;
	this->xmax = 0.0f;
	this->ymin = 0.0f;
	this->ymax = 0.0f;
	this->zmin = 0.0f;
	this->zmax = 0.0f;
}

AABB::AABB(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax) {
	this->xmin = xmin;
	this->xmax = xmax;
	this->ymin = ymin;
	this->ymax = ymax;
	this->zmin = zmin;
	this->zmax = zmax;
}

void AABB::update(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax) {
	this->xmin = xmin;
	this->xmax = xmax;
	this->ymin = ymin;
	this->ymax = ymax;
	this->zmin = zmin;
	this->zmax = zmax;
}

bool AABB::intersects(AABB aabb) {

	if ((xmax >= aabb.xmin && xmin <= aabb.xmin) || (aabb.xmax >= xmin && aabb.xmin <= xmin))
		if ((ymax >= aabb.ymin && ymin <= aabb.ymin) || (aabb.ymax >= ymin && aabb.ymin <= ymin))
			if ((zmax >= aabb.zmin && zmin <= aabb.zmin) || (aabb.zmax >= zmin && aabb.zmin <= zmin))
				return true;

	return false;
}

float* AABB::getIntersectionDistance(AABB aabb) {

	float* distances = new float[2];

	// Calculate the overlap in the X-axis
	distances[0] = std::max(0.0f, std::min(xmax, aabb.xmax) - std::max(xmin, aabb.xmin));

	// Calculate the overlap in the Y-axis
	distances[1] = std::max(0.0f, std::min(ymax, aabb.ymax) - std::max(ymin, aabb.ymin));

	return distances;
}