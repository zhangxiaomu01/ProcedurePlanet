#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <GL/freeglut.h>
#include <iostream>

//Set initial camera position and orientation
void InitCamera(const glm::dvec3& pos, const glm::dvec3& rot);

//Pass the key from glutSpecialFunc
void CameraSpecialKey(int key);

//Pass the key from glutKeyboardFunc
void CameraKeyboard(unsigned char key);

void mouseButton(int button, int state, int x, int y);

void mouseMove(int x, int y);

//Get the camera view matrix
glm::mat4 GetViewMatrix(); 

//Get the camera position
glm::vec3 GetCameraPosition();

#endif