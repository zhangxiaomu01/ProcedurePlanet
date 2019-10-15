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
#include "CreateSkybox.h"
#include "PerlinNoise.h"

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

static const std::string cosmos_Cube_vs("shaders\\Skybox_vs.glsl");
static const std::string cosmos_Cube_fs("shaders\\Skybox_fs.glsl");


static const std::string iceTex("Textures\\Icy.png");
static const std::string rockTex("Textures\\Rock.png");
static const std::string gasTex("Textures\\Gaseous2.png");
static const std::string cloudTex("Textures\\Clouds1.png");
static const std::string cosmosBoxTex("Textures\\CosmosTextures.png");
static const std::string snowTex("Textures\\SnowTex.jpg");
static const std::string grassTex("Textures\\Grass_Tex_01.jpg");
static const std::string earthTex("Textures\\ground_text.jpg");
GLuint planetTex_id[5] = { -1,-1,-1,-1,-1 };
GLuint cosmosTexture_ID = -1;
GLuint PerlinTex_ID = -1;
GLuint earth_TexLayer[] = { -1,-1,-1 };


//const std::string checkerFile("checker.png");
//const std::string checkerFile("Test_Texture.png");
const std::string checkerFile("checker.png");
GLuint tex_id = -1;

GLuint planet_sprogram = -1;
GLuint atmosphere_sprogram = -1;
GLuint ocean_sprogram = -1;
GLuint cosmos_SkyProgram = -1;
GLuint test_shader = -1;



int win_width = 1920;
int win_height = 1080;

float angle = -0.8;
bool isFill = false;
bool isFract = true;
bool isCalNorm = false;

int randomSeed = 0;

GLuint cosmos_vao = -1;

PLANET PlanetBody;
PLANET OceanLayer;
PLANET AtmosphereLayer;


const double aspect_ratio = double(win_width) / double(win_height);
const double fov = 3.141592 / 4.0;
//Set up some uniform variables
const glm::dmat4 P = glm::perspective(fov, aspect_ratio, 0.001, 30.0);

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
   static float slider[n_sliders] = { 1.0f,1.0f,1.0f,1.0f,0.97f,1.0f };
   const std::string labels[n_sliders] = { "Terrain FactorX","Terrain FactorY","Terrain FactorZ","ColorBlending","AtmosHeight", "HeightCon" };
   for (int i = 0; i<n_sliders; i++)
   {
      ImGui::SliderFloat(labels[i].c_str(), &slider[i], -2, 3);
   }
   int slider_loc = glGetUniformLocation(planet_sprogram, "slider");
   glUseProgram(ocean_sprogram);
   glUniform1fv(slider_loc, n_sliders, slider);

   glUseProgram(planet_sprogram);
  
   glUniform1fv(slider_loc, n_sliders, slider);

   glUseProgram(atmosphere_sprogram);
   glUniform1fv(slider_loc, n_sliders, slider);


   ImGui::SliderFloat("testAngle", &angle, -5, 5);

   ImGui::Checkbox("isWireFrame", &isFill);

   ImGui::Checkbox("isFractal", &isFract);
   ImGui::SameLine();

   ImGui::Checkbox("isCalNormal", &isCalNorm);

   ImGui::Image((void*)PlanetBody.GetTex_ID().positionMap_ID, ImVec2(128.0f, 128.0f), ImVec2(0.0, 1.0), ImVec2(1.0, 0.0));

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
    const int PVM_loc = 3;

	const int light_loc = 5;

	const int camHigh_loc = 16;
	const int camLow_loc = 17;
    const int isFract_loc = 20;
    const int isCalNorm_loc = 22;

   //glm::dmat4 V = glm::lookAt(glm::dvec3(0.0, 0.0, 2.5), glm::dvec3(0.0, 0.0, 0.0), glm::dvec3(0.0, 1.0, 0.0));
    glm::dmat4 V = GetViewMatrix();
   //glm::mat4 M = glm::scale(glm::vec3(patchScale));
    //glm::dmat4 M = glm::rotate(double(angle), glm::dvec3(0.0, 1.0, 0.0));
	glm::dmat4 M = glm::dmat4(1.0);

	glm::dvec3 camPos = GetCameraPosition();
    glm::dvec3 camModel = glm::dvec3(glm::inverse(M)*glm::dvec4(camPos,1.0f));
	glm::vec3 camHigh, camLow;
	DoubletoTwoFloats(camPos, camHigh, camLow);
   
    PlanetBody.planetBaseMesh(camModel, camPos, 4.0);

    glm::dmat4 modelView = V*M;
    modelView[3] = glm::dvec4(0.0, 0.0, 0.0, 1.0);
	glm::dmat4 PVM_Mat = P*modelView;

	glm::vec3 light_Dir = glm::vec3(glm::rotate(double(angle), glm::dvec3(0.0, 1.0, 0.0)) * glm::vec4(0.0, 1.0, 1.0,0.0));
	//glm::vec3 light_Dir =  glm::vec4(0.0, 1.0, 1.0, 0.0);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glm::dmat4 PVM_Cosmos = V*glm::scale(glm::dvec3(10.0));
	PVM_Cosmos[3] = glm::dvec4(0.0, 0.0, 0.0, 1.0);
	PVM_Cosmos = P*PVM_Cosmos;

	glUseProgram(cosmos_SkyProgram);
	glUniformMatrix4fv(PVM_loc, 1, false, glm::value_ptr(glm::mat4(PVM_Cosmos)));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cosmosTexture_ID);
	GLuint cosmosTexture_loc = glGetUniformLocation(cosmos_SkyProgram,"cubemap");
	if (cosmosTexture_loc != -1)
	{
		glUniform1i(cosmosTexture_loc, 0);
	}
	glFrontFace(GL_CW);
	DrawSkybox(cosmos_vao);
	glFrontFace(GL_CCW);
	


	glUseProgram(planet_sprogram);
	glUniformMatrix4fv(P_loc, 1, false, glm::value_ptr(glm::mat4(P)));
	glUniformMatrix4fv(V_loc, 1, false, glm::value_ptr(glm::mat4(V)));
	glUniformMatrix4fv(M_loc, 1, false, glm::value_ptr(glm::mat4(M)));
	glUniformMatrix4fv(PVM_loc, 1, false, glm::value_ptr(glm::mat4(PVM_Mat)));

	const int tex_loc = 21;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, PerlinTex_ID);
	glUniform1i(tex_loc, 0);
	//else if (seed[0] < 6 &&seed[0] >= 3)
	//{
	//	glBindTexture(GL_TEXTURE_2D, planetTex_id[ice]);
	//}
	//else if (seed[0] < 8 && seed[0] >= 6)
	//{
	//	glBindTexture(GL_TEXTURE_2D, planetTex_id[rock]);
	//}
	
	
	const int earth_loc = 24;
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, earth_TexLayer[0]);
	glUniform1i(earth_loc,1);

	const int snow_loc = 25;
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, earth_TexLayer[1]);
	glUniform1i(snow_loc, 2);

	const int grass_loc = 26;
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, earth_TexLayer[2]);
	glUniform1i(grass_loc, 3);

	glUniform1i(isFract_loc, static_cast <int>(isFract));

	glUniform1i(isCalNorm_loc, static_cast <int>(isCalNorm));

	glUniform3f(camHigh_loc, camHigh[0], camHigh[1], camHigh[2]);
	glUniform3f(camLow_loc, camLow[0], camLow[1], camLow[2]);
	glUniform3f(light_loc, light_Dir[0], light_Dir[1], light_Dir[2]);

	if (isFill)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Draw wireframe so we can see the edges of generated triangles
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	PlanetBody.drawPlanet();

	


	//Draw Ocean
	glUseProgram(ocean_sprogram);
	glUniformMatrix4fv(P_loc, 1, false, glm::value_ptr(glm::mat4(P)));
	glUniformMatrix4fv(V_loc, 1, false, glm::value_ptr(glm::mat4(V)));
	glUniformMatrix4fv(M_loc, 1, false, glm::value_ptr(glm::mat4(M)));
	glUniformMatrix4fv(PVM_loc, 1, false, glm::value_ptr(glm::mat4(PVM_Mat)));
	glUniform3f(light_loc, light_Dir[0], light_Dir[1], light_Dir[2]);


	if (isFill)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Draw wireframe so we can see the edges of generated triangles
	if (!isFill&&!isCalNorm)
		OceanLayer.drawPlanet();

	//Draw Atmosphere
	int pass_loc = 6;
    glUseProgram(atmosphere_sprogram);
    glUniformMatrix4fv(P_loc, 1, false, glm::value_ptr(glm::mat4(P)));
    glUniformMatrix4fv(V_loc, 1, false, glm::value_ptr(glm::mat4(V)));
    glUniformMatrix4fv(M_loc, 1, false, glm::value_ptr(glm::mat4(M)));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, planetTex_id[cloud]);
	glUniform1i(tex_loc, 0);
	glEnable(GL_BLEND);
	//glDisable(GL_CULL_FACE);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glUniform1i(pass_loc, 0);
	if (!isFill && !isCalNorm)
		AtmosphereLayer.drawPlanet();
	

	glCullFace(GL_FRONT);
	glUniform1i(pass_loc, 1);
	if (!isFill && !isCalNorm)
		AtmosphereLayer.drawPlanet();

	glDisable(GL_BLEND);
  // if (seed[0] >= 8)
  // {
	 //  glUseProgram(atmosphere_sprogram);
	 //  glUniformMatrix4fv(P_loc, 1, false, glm::value_ptr(P));
	 //  glUniformMatrix4fv(V_loc, 1, false, glm::value_ptr(V));
	 //  glUniformMatrix4fv(M_loc, 1, false, glm::value_ptr(M));

	 //  glActiveTexture(GL_TEXTURE0);
	 // /* if (seed[1] < 3)
	 //  {
		//   glBindTexture(GL_TEXTURE_2D, planetTex_id[cloud]);
	 //  }*/
	 //  
		//glBindTexture(GL_TEXTURE_2D, planetTex_id[gas]);
	 //  
	 //  glUniform1i(tex_loc, 0);

	 //  if (isFill)
		//   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Draw wireframe so we can see the edges of generated triangles

	 //  glDisable(GL_CULL_FACE);

	 //  AtmosphereLayer.drawPlanet();

  // }
   

  // glUseProgram(planet_sprogram);
  // glUniform1f(7, float(seed[1]));
   //OceanLayer.drawPlanet();

   //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   //glUseProgram(test_shader);
   //glUniformMatrix4fv(P_loc, 1, false, glm::value_ptr(P));
   //glUniformMatrix4fv(V_loc, 1, false, glm::value_ptr(V));
   //glUniformMatrix4fv(M_loc, 1, false, glm::value_ptr(M));
   //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   //myTest.drawPlanet();
  // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   //myTest.drawPlanet();
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   draw_gui();

   glutSwapBuffers();
}

void idle()
{
   glutPostRedisplay();

   const int time_ms = glutGet(GLUT_ELAPSED_TIME);
   float time_sec = 0.001f*time_ms;
   const int time_loc = 8;
   glUseProgram(atmosphere_sprogram);
   glUniform1f(time_loc, time_sec);

   glUseProgram(ocean_sprogram);
   glUniform1f(time_loc, time_sec);

}

void reload_shader()
{
   //Use the version of InitShader with 4 parameters. The shader names are in the order the stage are in the pipeline:
   //Vertex shader, Tess. control, Tess. evaluation, fragment shader
   GLuint new_shader = InitShader(planet_vs.c_str(), planet_tc.c_str(), planet_te.c_str(), planet_fs.c_str());
   GLuint new_shader01 = InitShader(ocean_vs.c_str(), ocean_tc.c_str(), ocean_te.c_str(), ocean_fs.c_str());
   GLuint new_shader02 = InitShader(atmosphere_vs.c_str(), atmosphere_tc.c_str(), atmosphere_te.c_str(), atmosphere_fs.c_str());

   if (new_shader == -1 && new_shader01 == -1) // loading failed
   {
      glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
   }
   else
   {
      glClearColor(0.35f, 0.35f, 0.35f, 0.0f);

      if (planet_sprogram != -1)
      {
         glDeleteProgram(planet_sprogram);
		 glDeleteProgram(ocean_sprogram);
		 glDeleteProgram(atmosphere_sprogram);
      }
	  planet_sprogram = new_shader;
	  ocean_sprogram = new_shader01;
	  atmosphere_sprogram = new_shader02;
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
   //ocean_sprogram = 
   atmosphere_sprogram = InitShader(atmosphere_vs.c_str(), atmosphere_tc.c_str(), atmosphere_te.c_str(), atmosphere_fs.c_str());
   cosmos_SkyProgram = InitShader(cosmos_Cube_vs.c_str(), cosmos_Cube_fs.c_str());

   tex_id = LoadTexture(checkerFile.c_str());
   planetTex_id[earth] = LoadTexture(earthTex.c_str());
   planetTex_id[ice] = LoadTexture(iceTex.c_str());
   planetTex_id[rock] = LoadTexture(rockTex.c_str());
   planetTex_id[gas] = LoadTexture(gasTex.c_str());
   planetTex_id[cloud] = LoadTexture(cloudTex.c_str());
   cosmosTexture_ID = LoadSkyTexture(cosmosBoxTex.c_str());

   earth_TexLayer[0] = LoadTexture(earthTex.c_str());
   earth_TexLayer[1] = LoadTexture(snowTex.c_str());
   earth_TexLayer[2] = LoadTexture(grassTex.c_str());

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

   glm::dvec3 eyePos = GetCameraPosition();
   //3.14159265f/2.0f
   InitCamera(glm::dvec3(0.0, 0.0, 3.5),glm::dvec3(0.0,0.0,0.0));
   OceanLayer.planetBaseMesh(GetCameraPosition(), eyePos, 1.0f);
   AtmosphereLayer.planetBaseMesh(GetCameraPosition(), eyePos, 1.0f);

   cosmos_vao = createSkyBox();

   PerlinTex_ID = PerlinNoise <double>::GetPermutationTexture();

   PlanetBody.CreatePlanetFBO();
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



