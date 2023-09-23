#include "camera.h"

Camera::Camera() {
    setCameraPosition(0.0f, 0.0f, 0.0f);
    setCameraTarget(0.0f, 0.0f, 0.0f);
    setCameraType(0);
}

Camera::Camera(float camPos[3], float camTarget[3], int type) {
    setCameraPosition(camPos[0], camPos[1], camPos[2]);
    setCameraTarget(camTarget[0], camTarget[1], camTarget[2]);
    setCameraType(type);
}

void Camera::setCameraPosition(float pos_x, float pos_y, float pos_z) {
    this->camPos[0] = pos_x;
    this->camPos[1] = pos_y;
    this->camPos[2] = pos_z;
}

void Camera::setCameraTarget(float target_x, float target_y, float target_z) {
    this->camTarget[0] = target_x;
    this->camTarget[1] = target_y;
    this->camTarget[2] = target_z;
}

void Camera::setCameraType(int type) {
    this->type = type;
}

float* Camera::getCameraPosition(void) {
    return this->camPos;
}

float* Camera::getCameraTarget(void) {
    return this->camTarget;
}

int Camera::getCameraType(void) {
    return this->type;
}