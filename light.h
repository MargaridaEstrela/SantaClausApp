#ifndef LIGHT_H
#define LIGHT_H

class Light {
public:
	Light();
	Light(float posX, float posY, float posZ, float pointLight);
	void setPosition(float posX, float posY, float posZ, float pointLight);
	void setEye(float posX, float posY, float posZ, float pointLight);
	float* getPosition();
	float* getEye();
	void changeMode();
	bool getMode();

public:
	float pos[4];
	float eye[4];
	bool mode; // 0 = deactivated, 1 = activated
};

#endif