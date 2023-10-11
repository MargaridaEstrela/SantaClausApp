#ifndef AABB_H
#define AABB_H

class AABB {
public:

	float xmin;
	float xmax;
	float ymin;
	float ymax;
	float zmin;
	float zmax;

	AABB();
	AABB(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax);

	void update(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax);
	bool intersects(AABB aabb);
	float* getIntersectionDistance(AABB aabb);
};

#endif
