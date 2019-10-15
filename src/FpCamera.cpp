#include "FpCamera.h"

const double camMouseRotateSpeed(0.001);
static double cameraMoveSpeed = 0.005;
static double cameraTurnSpeed = 0.05;
const double cameraSpeedStep = 0.001;

static glm::dvec3 CamPos(0.0);
static glm::dvec3 CamRot(0.0);
static glm::dmat4 C(1.0);

static double camAngleX = 0.0;
static double camAngleY = 0.0;
static int xMouseOrigin = -1;
static int yMouseOrigin = -1;
static double deltaAngleX = 0.0;
static double deltaAngleY = 0.0;

double lx = 0.0, ly = 0.0, lz = 0.0;

glm::mat4 GetViewMatrix()
{
	return glm::inverse(C);
}

glm::vec3 GetCameraPosition()
{
   return CamPos;
}

void UpdateC()
{
	C = glm::translate(CamPos) *
		glm::rotate(CamRot.z, glm::dvec3(0.0, 0.0, 1.0)) *
		glm::rotate(CamRot.y, glm::dvec3(0.0, 1.0, 0.0)) *
		glm::rotate(CamRot.x, glm::dvec3(1.0, 0.0, 0.0));
	//C = glm::lookAt(CamPos, glm::dvec3(lz, ly, 1),glm::dvec3(0,1,0));
}

void InitCamera(const glm::dvec3& pos, const glm::dvec3& rot)
{
   CamPos = pos;
   CamRot = rot;
   UpdateC();
} 

void CameraSpecialKey(int key)
{
   if (key == GLUT_KEY_UP) { CamPos += glm::dvec3(C * glm::dvec4(0.0, 0.0, -cameraMoveSpeed, 0.0)); }
   if (key == GLUT_KEY_DOWN) { CamPos += glm::dvec3(C * glm::dvec4(0.0, 0.0, +cameraMoveSpeed, 0.0)); }

   if (key == GLUT_KEY_RIGHT) { CamRot.z += cameraTurnSpeed; }
   if (key == GLUT_KEY_LEFT) { CamRot.z -= cameraTurnSpeed; }

   UpdateC();
}

void CameraKeyboard(unsigned char key)
{
   if (key == 'q') { CamRot.z += cameraTurnSpeed; }
   if (key == 'e') { CamRot.z -= cameraTurnSpeed; }

   if (key == 'w') { CamPos += glm::dvec3(C * glm::dvec4(0.0, 0.0, -cameraMoveSpeed, 0.0)); }
   if (key == 's') { CamPos += glm::dvec3(C * glm::dvec4(0.0, 0.0, +cameraMoveSpeed, 0.0)); }

   if (key == 'a') { CamPos += glm::dvec3(C * glm::dvec4(-cameraMoveSpeed, 0.0, 0.0, 0.0)); }
   if (key == 'd') { CamPos += glm::dvec3(C * glm::dvec4(cameraMoveSpeed, 0.0, 0.0, 0.0)); }

   if (key == 'z') { CamPos += glm::dvec3(C * glm::dvec4(0.0, +cameraMoveSpeed*0.5, 0.0, 0.0)); }
   if (key == 'x') { CamPos += glm::dvec3(C * glm::dvec4(0.0, -cameraMoveSpeed*0.5, 0.0, 0.0)); }

   if (key == 'v') { InitCamera(glm::dvec3(0.0, 0.0, 3.5), glm::dvec3(0.0, 0.0, 0.0)); }

   if (key == '+') { cameraMoveSpeed += cameraSpeedStep; }
   if (key == '-') 
   { if(cameraMoveSpeed>0.0)
		cameraMoveSpeed -= cameraSpeedStep;
   }
   

   UpdateC();
}
 
void mouseButton(int button, int state, int x, int y)
{
	if (button == GLUT_RIGHT_BUTTON)
	{
		if (state == GLUT_UP)
		{
			camAngleX += deltaAngleX;
			camAngleY += deltaAngleY;
			xMouseOrigin = -1;
			yMouseOrigin = -1;
		}
		else
		{
			xMouseOrigin = x;
			yMouseOrigin = y;
		}
	}
}

void mouseMove(int x, int y)
{
	if (xMouseOrigin > 0 || yMouseOrigin > 0)
	{
		deltaAngleX = (x - xMouseOrigin)*camMouseRotateSpeed;
		deltaAngleY = (y - yMouseOrigin)*camMouseRotateSpeed;
		
		lx = sin(deltaAngleX);
		lz = -cos(camAngleX + deltaAngleX);

		ly = -sin(camAngleY + deltaAngleY);


		CamRot.y -= deltaAngleX*0.05;
		CamRot.x -= deltaAngleY*0.05;
		UpdateC();
	}
}