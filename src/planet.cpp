#include"planet.h"

PLANET::PLANET() :m_radius(1.0), m_position(glm::dvec3(0.0)),m_vertex_count(0)
{ 
	m_attrib_loc.pos_loc = 0;
	m_attrib_loc.low_loc = 1;


	m_buffer_obj.vao = -1;
	m_buffer_obj.vbo = -1;
	m_buffer_obj.ibo = -1;
}

void PLANET::drawRecursive(glm::dvec3 p1, glm::dvec3 p2, glm::dvec3 p3, const glm::dvec3 &center, const glm::dvec3 &eyePos, double k, double sLevel)
{
	double ratio_size = sLevel*k; // default : 1
	double minsize = 0.1;    // default : 0.01
	glm::dvec3 edge_center[3] = { (p1 + p2) / 2.0, (p2 + p3) / 2.0, (p3 + p1) / 2.0 };
	bool edge_test[3]; double angle[3];
	glm::vec3 high, low;


	for (int i = 0; i<3; i++)
	{
		//calculate the distance between camera position and midpoints 
		glm::dvec3 d = -center + edge_center[i];
		edge_test[i] = glm::length(d) > ratio_size;
		double dot = glm::dot(glm::normalize(edge_center[i]), glm::normalize(d));
		angle[i] = acos(clamp(dot, 1, -1));

	}

	// culling
	/*if (std::max(angle[0], std::max(angle[1], angle[2])) < M_PI / 2 -0.4)
		return;*/

	// draw
	if ((edge_test[0] && edge_test[1] && edge_test[2]) || sLevel < minsize)
	{

		DoubletoTwoFloats(p1, high, low);
		m_vertices.push_back(high);
		m_vertices.push_back(low);

		DoubletoTwoFloats(p2, high, low);
		m_vertices.push_back(high);
		m_vertices.push_back(low);

		DoubletoTwoFloats(p3, high, low);
		m_vertices.push_back(high);
		m_vertices.push_back(low);

		return;
	}

	// Recurse
	glm::vec3 p[6] = { p1, p2, p3, edge_center[0], edge_center[1], edge_center[2] };
	int idx[12] = { 0, 3, 5,    5, 3, 4,    3, 1, 4,    5, 4, 2 };
	bool valid[4] = { 1, 1, 1, 1 };

	if (edge_test[0]) { p[3] = p1; valid[0] = 0; } // skip triangle 0 ?
												   //std::cout << edge_test[0] << std::endl;
	if (edge_test[1]) { p[4] = p2; valid[2] = 0; } // skip triangle 2 ?
	if (edge_test[2]) { p[5] = p3; valid[3] = 0; } // skip triangle 3 ?

	for (int i = 0; i < 4; ++i)
	{
		if (valid[i])
		{
			int i1 = idx[3 * i + 0], i2 = idx[3 * i + 1], i3 = idx[3 * i + 2];
			drawRecursive(glm::normalize(p[i1]), glm::normalize(p[i2]), glm::normalize(p[i3]), center, eyePos,k, sLevel / 2.0);

		}

	}
}

void PLANET::planetBaseMesh(const glm::dvec3 &center, const glm::dvec3 &eyePos, double detailLevel)
{
	//double X = 0.525731112119133606;
	//double Z = 0.850650808352039932;
	double t = (1.0 + sqrt(5.0)) / 2.0;
	//static std::vector<glm::vec3> vertices;
		
	std::vector<glm::dvec3> p{
		{ -1, t, 0 },{ 1, t, 0 },{ -1, -t, 0 },{ 1, -t, 0 },
		{ 0, -1, t },{ 0, 1, t },{ 0, -1, -t },{ 0, 1, -t },
		{ t, 0, -1 },{ t, 0, 1 },{ -t, 0, -1 },{ -t, 0, 1 }
	};

	std::vector<GLuint> idx{
		0, 11, 5, 0, 5, 1, 0, 1, 7, 0, 7, 10, 0, 10, 11,
		1, 5, 9, 5, 11, 4, 11, 10, 2, 10, 7, 6, 10, 7, 6, 7, 1, 8,
		3, 9, 4, 3, 4, 2, 3, 2, 6, 3, 6, 8, 3, 8, 9,
		4, 9, 5, 2, 4, 11, 6, 2, 10, 8, 6, 7, 9, 8, 1
	};

	

	/*for (GLuint i = 0; i < idx.size() / 3; ++i)
	{

		vertices.push_back(p[idx[i * 3 + 0]]);
		vertices.push_back(glm::vec3(0.0f, 0.0f,0.0f));
		vertices.push_back(p[idx[i * 3 + 0]]);

		vertices.push_back(p[idx[i * 3 + 1]]);
		vertices.push_back(glm::vec3(0.5f, sqrt(3.0f) / 2.0f, 0.0f));
		vertices.push_back(p[idx[i * 3 + 1]]);

		vertices.push_back(p[idx[i * 3 + 2]]);
		vertices.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
		vertices.push_back(p[idx[i * 3 + 2]]);

	}*/

	m_vertices.clear();

	for (GLuint i = 0; i < idx.size() / 3; ++i)
	{

		glm::dvec3 p3[3] = { glm::normalize(p[idx[i * 3 + 0]]) ,	// triangle point 1
							glm::normalize(p[idx[i * 3 + 1]]) ,	// triangle point 2
							glm::normalize(p[idx[i * 3 + 2]]) };	// triangle point 3

		drawRecursive(p3[0], p3[1], p3[2], center, eyePos, detailLevel,1.0);

	}

	m_vertex_count = m_vertices.size();

	const int stride = 6 * sizeof(float);
	const int numOfAttr = m_vertex_count*3;
	//std::cout << m_vertex_count/3.0f << std::endl;
	
	glGenVertexArrays(1, &m_buffer_obj.vao);
	glBindVertexArray(m_buffer_obj.vao);

	glGenBuffers(1, &m_buffer_obj.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_buffer_obj.vbo);
	glBufferData(GL_ARRAY_BUFFER, numOfAttr * sizeof(float), m_vertices.data(),GL_STATIC_DRAW);
	glEnableVertexAttribArray(m_attrib_loc.pos_loc);
	glVertexAttribPointer(m_attrib_loc.pos_loc,3,GL_FLOAT,GL_FALSE,stride, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(m_attrib_loc.low_loc);
	glVertexAttribPointer(m_attrib_loc.low_loc, 3, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(3*sizeof(float)));

	glBindVertexArray(0);

}

void PLANET::drawPlanet()
{
	glBindVertexArray(m_buffer_obj.vao);

	//glDrawArrays(GL_TRIANGLES, 0,  60);
	glPatchParameteri(GL_PATCH_VERTICES, 3); //number of input verts to the tess. control shader per patch.

	glDrawArrays(GL_PATCHES, 0, m_vertex_count);
	glBindVertexArray(0);
	
}

void PLANET::CreatePlanetFBO()
{
	const int ImageSize(1024);

	m_buffer_obj.fbo = -1;
	m_buffer_obj.renderBuffer = -1;
	m_tex_ID.positionMap_ID = -1;
	m_tex_ID.normal_ID = -1;

	glGenTextures(1, &m_tex_ID.positionMap_ID);
	glBindTexture(GL_TEXTURE_2D, m_tex_ID.positionMap_ID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ImageSize, ImageSize, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &m_tex_ID.normal_ID);
	glBindTexture(GL_TEXTURE_2D, m_tex_ID.normal_ID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ImageSize, ImageSize, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffers(1, &m_buffer_obj.renderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_buffer_obj.renderBuffer);
	glRenderbufferStorage(m_buffer_obj.renderBuffer, GL_DEPTH_COMPONENT, ImageSize, ImageSize);

	glGenFramebuffers(1, &m_buffer_obj.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_buffer_obj.fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tex_ID.positionMap_ID, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_tex_ID.normal_ID, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_buffer_obj.renderBuffer);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

tex_ID PLANET::GetTex_ID()
{
	return m_tex_ID;
}

buffer_obj PLANET::GetBuffer_ID()
{
	return m_buffer_obj;
}

PLANET::~PLANET()
{

}