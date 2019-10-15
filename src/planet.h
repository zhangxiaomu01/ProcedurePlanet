#ifndef __PLANET_H__
#define __PLANET_H__

#include<windows.h>
#include<iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include<GL/GL.h>
#include <GL/glext.h>

#include<vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Helper.h"

#define BUFFER_OFFSET(i)  ((char*)NULL + (i))

struct attrib_loc {
	GLuint pos_loc;
	GLuint low_loc;
	//GLuint tex_coord_loc;
	//GLuint normal_loc;
};

struct buffer_obj {
	GLuint vao;
	GLuint vbo;
	GLuint ibo;
	GLuint fbo;
	GLuint renderBuffer;
};

struct tex_ID {
	GLuint positionMap_ID;
	GLuint normal_ID;
	GLuint colorMap_ID;
};

class PLANET {
private:
	double m_radius;

	int m_vertex_count; 
	std::vector<glm::vec3> m_vertices;

	glm::dvec3 m_position;
	attrib_loc m_attrib_loc;
	buffer_obj m_buffer_obj;
	tex_ID m_tex_ID;

public:
	PLANET();
	void planetBaseMesh(const glm::dvec3 &center, const glm::dvec3 &eyePos, double detailLevel);
	void drawRecursive(glm::dvec3 p1, glm::dvec3 p2, glm::dvec3 p3, const glm::dvec3 &center, const glm::dvec3 &eyePos,
						double k = 1.0, double sLevel = 1.0);
	void drawPlanet();
	
	tex_ID GetTex_ID();
	buffer_obj GetBuffer_ID();

	void CreatePlanetFBO();

	~PLANET();

};


#endif
