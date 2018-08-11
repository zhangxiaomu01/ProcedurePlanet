#include <windows.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <GL/gl.h>
#include <GL/glext.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui_impl_glut.h"

#include <iostream>
#include <string>
#include <vector>

#include "InitShader.h"
#include "FpCamera.h"
#include "planet.h"
#include "LoadTexture.h"

#define BUFFER_OFFSET(i)  ((char*)NULL + (i))

enum PlanetTexture{
	earth,
	ice,
	rock,
	gas,
	cloud
};

//4 shaders, including tess. control and tess. evaluation
static const std::string planet_vs("shaders\\planet_vs.glsl");
static const std::string planet_tc("shaders\\planet_tc.glsl");
static const std::string planet_te("shaders\\planet_te.glsl");
static const std::string planet_fs("shaders\\planet_fs.glsl");

static const std::string atmosphere_vs("shaders\\atmosphere_vs.glsl");
static const std::string atmosphere_tc("shaders\\atmosphere_tc.glsl");
static const std::string atmosphere_te("shaders\\atmosphere_te.glsl");
static const std::string atmosphere_fs("shaders\\atmosphere_fs.glsl");

static const std::string ocean_vs("shaders\\Ocean_vs.glsl");
static const std::string ocean_tc("shaders\\Ocean_tc.glsl");
static const std::string ocean_te("shaders\\Ocean_te.glsl");
static const std::string ocean_fs("shaders\\Ocean_fs.glsl");

static const std::string t_vertex_shader("shaders\\01test_vs.glsl");
static const std::string t_fragment_shader("shaders\\01test_fs.glsl");

static const std::string earthTex("Textures\\Terrestrial1.png");
static const std::string iceTex("Textures\\Icy.png");
static const std::string rockTex("Textures\\Rock.png");
static const std::string gasTex("Textures\\Gaseous2.png");
static const std::string cloudTex("Textures\\Clouds1.png");
GLuint planetTex_id[5] = { -1,-1,-1,-1,-1 };


//const std::string checkerFile("checker.png");
//const std::string checkerFile("Test_Texture.png");
const std::string checkerFile("checker.png");
GLuint tex_id = -1;

GLuint planet_sprogram = -1;
GLuint atmosphere_sprogram = -1;
GLuint ocean_sprogram = -1;
GLuint test_shader = -1;

//VAO and VBO for the tessellated patch
//GLuint patch_vao = -1;
//GLuint patch_vbo = -1;

//Number of terrain patches
//const float patchX = 50.0f;
//const float patchY = 50.0f;
//const float patchScale = 10.0f;
//const int numPatches = int(patchX*patchY);

int win_width = 1280;
int win_height = 720;

float angle = 0;
bool isFill = false;
bool isFract = true;
bool isCalNorm = false;

int randomSeed = 0;

PLANET PlanetBody;
PLANET OceanLayer;
PLANET AtmosphereLayer;



const float aspect_ratio = float(win_width) / float(win_height);
const float fov = 3.141592f / 4.0f;
//Set up some uniform variables
const glm::mat4 P = glm::perspective(fov, aspect_ratio, 0.1f, 1000.0f);

int generateRadomSeed()
{
	int k = rand() % 100;
	//std::cout << k << std:: endl;
	return k;
}

void draw_gui()
{
   ImGui_ImplGlut_NewFrame();

   ImGui::Begin("Control Panel", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
   const int n_sliders = 6;
   static float slider[n_sliders] = { 1.0f,1.0f,1.0f,1.0f,1.0f,1.5f };
   const std::string labels[n_sliders] = { "gl_TessLevelOuter[0]","gl_TessLevelOuter[1]","gl_TessLevelOuter[2]","gl_TessLevelOuter[3]","gl_TessLevelInner[0]", "gl_TessLevelInner[1]" };
   for (int i = 0; i<n_sliders; i++)
   {
      ImGui::SliderFloat(labels[i].c_str(), &slider[i], 0, 64);
   }
   int slider_loc = glGetUniformLocation(planet_sprogram, "slider");
   glUniform1fv(slider_loc, n_sliders, slider);

   ImGui::SliderFloat("testAngle", &angle, -5, 5);

   ImGui::Checkbox("isRendering", &isFill);

   ImGui::Checkbox("isFractal", &isFract);
   ImGui::SameLine();

   ImGui::Checkbox("isCalNormal", &isCalNorm);

   ImGui::End();

   ImGui::Render();
}

// glut display callback function.
// This function gets called every time the scene gets redisplayed 
void display()
{
	int seed[2];
	seed[0] = randomSeed / 10;
	seed[1] = randomSeed - seed[0] * 10;
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   const int P_loc = 0;
   const int V_loc = 1;
   const int M_loc = 2;
   const int isFract_loc = 4;
   const int tex_loc = 5;
   const int isCalNorm_loc = 6;
   //glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
   glm::mat4 V = GetViewMatrix();
   //glm::mat4 M = glm::scale(glm::vec3(patchScale));
   glm::mat4 M = glm::rotate(angle, glm::vec3(0.0f, 1.0f, 0.0f));

   glm::vec3 camModel = glm::vec3(glm::inverse(M)*glm::vec4(GetCameraPosition(),1.0f));
   PlanetBody.planetBaseMesh(camModel, 2.0);
   if (seed[0] < 8)
   {
	   glUseProgram(planet_sprogram);
	   glUniformMatrix4fv(P_loc, 1, false, glm::value_ptr(P));
	   glUniformMatrix4fv(V_loc, 1, false, glm::value_ptr(V));
	   glUniformMatrix4fv(M_loc, 1, false, glm::value_ptr(M));

	   glActiveTexture(GL_TEXTURE0);
	   if (seed[0] < 3)
	   {
		   glBindTexture(GL_TEXTURE_2D, planetTex_id[earth]);
	   }
	   else if (seed[0] < 6 &&seed[0] >= 3)
	   {
		   glBindTexture(GL_TEXTURE_2D, planetTex_id[ice]);
	   }
	   else if (seed[0] < 8 && seed[0] >= 6)
	   {
		   glBindTexture(GL_TEXTURE_2D, planetTex_id[rock]);
	   }
	   glUniform1i(tex_loc, 0);

	   glUniform1i(isFract_loc, static_cast <int>(isFract));

	   glUniform1i(isCalNorm_loc, static_cast <int>(isCalNorm));

	   glEnable(GL_CULL_FACE);
	   glCullFace(GL_BACK);

	   if (isFill)
		   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Draw wireframe so we can see the edges of generated triangles

	   PlanetBody.drawPlanet();

   }
  
   if (seed[0] < 3)
   {
	   glUseProgram(ocean_sprogram);
	   glUniformMatrix4fv(P_loc, 1, false, glm::value_ptr(P));
	   glUniformMatrix4fv(V_loc, 1, false, glm::value_ptr(V));
	   glUniformMatrix4fv(M_loc, 1, false, glm::value_ptr(M));

	   if (isFill)
		   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Draw wireframe so we can see the edges of generated triangles

	   OceanLayer.drawPlanet();
   }
   
   if (seed[0] >= 8)
   {
	   glUseProgram(atmosphere_sprogram);
	   glUniformMatrix4fv(P_loc, 1, false, glm::value_ptr(P));
	   glUniformMatrix4fv(V_loc, 1, false, glm::value_ptr(V));
	   glUniformMatrix4fv(M_loc, 1, false, glm::value_ptr(M));

	   glActiveTexture(GL_TEXTURE0);
	  /* if (seed[1] < 3)
	   {
		   glBindTexture(GL_TEXTURE_2D, planetTex_id[cloud]);
	   }*/
	   
		glBindTexture(GL_TEXTURE_2D, planetTex_id[gas]);
	   
	   glUniform1i(tex_loc, 0);

	   if (isFill)
		   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Draw wireframe so we can see the edges of generated triangles

	   glDisable(GL_CULL_FACE);
	   //glEnable(GL_BLEND);
	  // glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
	   AtmosphereLayer.drawPlanet();
	   //glDisable(GL_BLEND);
   }
   

   glUseProgram(planet_sprogram);
   glUniform1f(7, float(seed[1]));
   //OceanLayer.drawPlanet();

   //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   //glUseProgram(test_shader);
   //glUniformMatrix4fv(P_loc, 1, false, glm::value_ptr(P));
   //glUniformMatrix4fv(V_loc, 1, false, glm::value_ptr(V));
   //glUniformMatrix4fv(M_loc, 1, false, glm::value_ptr(M));
   //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   //myTest.drawPlanet();
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   //myTest.drawPlanet();

   draw_gui();

   glutSwapBuffers();
}

void idle()
{
   glutPostRedisplay();

   const int time_ms = glutGet(GLUT_ELAPSED_TIME);
   float time_sec = 0.001f*time_ms;

   glUseProgram(atmosphere_sprogram);
   const int time_loc = 3;
   glUniform1f(time_loc, time_sec);

}

void reload_shader()
{
   //Use the version of InitShader with 4 parameters. The shader names are in the order the stage are in the pipeline:
   //Vertex shader, Tess. control, Tess. evaluation, fragment shader
   GLuint new_shader = InitShader(planet_vs.c_str(), planet_tc.c_str(), planet_te.c_str(), planet_fs.c_str());

   if (new_shader == -1) // loading failed
   {
      glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
   }
   else
   {
      glClearColor(0.35f, 0.35f, 0.35f, 0.0f);

      if (planet_sprogram != -1)
      {
         glDeleteProgram(planet_sprogram);
      }
	  planet_sprogram = new_shader;

   }
}


void reshape(int w, int h)
{
   win_width = w;
   win_height = h;
   glViewport(0, 0, w, h);
}



void printGlInfo()
{
   std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
   std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
   std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
   std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

void initOpenGl()
{
   glewInit();

   glEnable(GL_DEPTH_TEST);

   test_shader = InitShader(t_vertex_shader.c_str(), t_fragment_shader.c_str());
   ocean_sprogram = InitShader(ocean_vs.c_str(), ocean_tc.c_str(), ocean_te.c_str(), ocean_fs.c_str());
   atmosphere_sprogram = InitShader(atmosphere_vs.c_str(), atmosphere_tc.c_str(), atmosphere_te.c_str(), atmosphere_fs.c_str());

   tex_id = LoadTexture(checkerFile.c_str());
   planetTex_id[earth] = LoadTexture(earthTex.c_str());
   planetTex_id[ice] = LoadTexture(iceTex.c_str());
   planetTex_id[rock] = LoadTexture(rockTex.c_str());
   planetTex_id[gas] = LoadTexture(gasTex.c_str());
   planetTex_id[cloud] = LoadTexture(cloudTex.c_str());

   reload_shader();

   //glGenVertexArrays(1, &patch_vao);
   //glBindVertexArray(patch_vao);
   //std::vector<glm::vec3> vertices;
   ////DEMO: modify this to make a (patchX x patchY) grid of patches
   //for (float x = 0.0; x < patchX;x+=1.0f)
   //{
	  // for (float y = 0.0; y < patchY;y+=1.0f)
	  // {
		 //  vertices.push_back(glm::vec3(x+0.0f, y+0.0f, 0.0f));
		 //  vertices.push_back(glm::vec3(x+1.0f, y+0.0f, 0.0f));
		 //  vertices.push_back(glm::vec3(x+1.0f, y+1.0f, 0.0f));
		 //  vertices.push_back(glm::vec3(x+0.0f, y+1.0f, 0.0f));
	  // }
	  // 
   //}
   //

   ////create vertex buffers for vertex coords
   //glGenBuffers(1, &patch_vbo);
   //glBindBuffer(GL_ARRAY_BUFFER, patch_vbo);
   //glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
   //int pos_loc = glGetAttribLocation(shader_program, "pos_attrib");

   //if (pos_loc >= 0)
   //{
   //   glEnableVertexAttribArray(pos_loc);
   //   glVertexAttribPointer(pos_loc, 3, GL_FLOAT, false, 0, BUFFER_OFFSET(0));
   //}

   
   //3.14159265f/2.0f
   InitCamera(glm::vec3(0.0f, 0.0f, 3.5f),glm::vec3(0.0f,0.0f,0.0f));
   OceanLayer.planetBaseMesh(GetCameraPosition(),1.0f);
   AtmosphereLayer.planetBaseMesh(GetCameraPosition(), 1.0f);
}

// glut keyboard callback function.
// This function gets called when an ASCII key is pressed
void keyboard(unsigned char key, int x, int y)
{
	ImGui_ImplGlut_KeyCallback(key);
	std::cout << "key : " << key << ", x: " << x << ", y: " << y << std::endl;
	CameraKeyboard(key);
	switch (key)
	{
	case 'r':
	case 'R':
		reload_shader();
		break;
	case 'f':
		randomSeed = generateRadomSeed();
		break;
	}
}

void keyboard_up(unsigned char key, int x, int y)
{
   ImGui_ImplGlut_KeyUpCallback(key);

}

void special_up(int key, int x, int y)
{
   ImGui_ImplGlut_SpecialUpCallback(key);
}

void passive(int x, int y)
{
   ImGui_ImplGlut_PassiveMouseMotionCallback(x, y);
}

void special(int key, int x, int y)
{
   ImGui_ImplGlut_SpecialCallback(key);
   CameraSpecialKey(key);
}

void motion(int x, int y)
{
   ImGui_ImplGlut_MouseMotionCallback(x, y);
   mouseMove(x, y);
}

void mouse(int button, int state, int x, int y)
{
   ImGui_ImplGlut_MouseButtonCallback(button, state);
   mouseButton(button, state, x, y);
}

int main(int argc, char **argv)
{
   //Configure initial window state
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
   glutInitWindowPosition(5, 5);
   glutInitWindowSize(win_width, win_height);
   int win = glutCreateWindow("Final_Project");

   printGlInfo();

   //Register callback functions with glut. 
   glutDisplayFunc(display);
   glutKeyboardFunc(keyboard);
   glutIdleFunc(idle);
   glutSpecialFunc(special);
   glutKeyboardUpFunc(keyboard_up);
   glutSpecialUpFunc(special_up);
   glutMouseFunc(mouse);
   glutMotionFunc(motion);
   glutPassiveMotionFunc(motion);
   glutReshapeFunc(reshape);

   initOpenGl();
   ImGui_ImplGlut_Init(); // initialize the imgui system

                          //Enter the glut event loop.
   glutMainLoop();
   glutDestroyWindow(win);
   return 0;
}



