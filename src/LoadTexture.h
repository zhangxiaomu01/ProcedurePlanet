#ifndef __LOADTEXTURE_H__
#define __LOADTEXTURE_H__

#include <windows.h>
#include <string>
#include "GL/glew.h"
#include "GL/gl.h"

GLuint LoadTexture(const char* fname);
GLuint LoadSkyTexture(const char* fname);


#endif