#include "LoadTexture.h"
#include "FreeImage.h"
#include <iostream>


GLuint LoadTexture(const char* fname)
{
   GLuint tex_id;

   FIBITMAP* tempImg = FreeImage_Load(FreeImage_GetFileType(fname, 0), fname);
   FIBITMAP* img = FreeImage_ConvertTo32Bits(tempImg);

   FreeImage_Unload(tempImg);

   GLuint w = FreeImage_GetWidth(img);
   GLuint h = FreeImage_GetHeight(img);
   GLuint scanW = FreeImage_GetPitch(img);

   GLubyte* byteImg = new GLubyte[h*scanW];
   FreeImage_ConvertToRawBits(byteImg, img, scanW, 32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, FALSE);
   FreeImage_Unload(img);

   glGenTextures(1, &tex_id);
   glBindTexture(GL_TEXTURE_2D, tex_id);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, byteImg);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

   delete byteImg;

   return tex_id;
}
/*
	----
	| T |
-----------------
| F | L | B | R |
-----------------
	| D |
	----

	----
	| 2 |
-----------------      2
| 1 | 5 | 0 | 4 |    0   1
-----------------      3
	| 3 |
	----
Look like this. D- Bottom, B- Back
0-x_positive 4-z_positive 2-y_positive
*/
GLuint LoadSkyTexture(const char* fname)
{
	GLuint tex_id;
	GLint  faceBoundary[6][4];//Defines the cropped image boundaries
	FIBITMAP* imageContainer[6];//Store the cropped images

	glGenTextures(1, &tex_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	FIBITMAP* tempImg = FreeImage_Load(FreeImage_GetFileType(fname, 0), fname);
	FIBITMAP* img = FreeImage_ConvertTo32Bits(tempImg);
	FreeImage_Unload(tempImg);

	GLuint w = FreeImage_GetWidth(img);
	GLuint h = FreeImage_GetHeight(img);
	GLuint interval = w / 4;
	if (w/4 == h/3)
	{
		//defines the cropped image based on the image above
		faceBoundary[0][0] = 0;
		faceBoundary[0][1] = interval;
		faceBoundary[0][2] = interval;
		faceBoundary[0][3] = 2 * interval;

		faceBoundary[5][0] = interval;
		faceBoundary[5][1] = 2*interval;
		faceBoundary[5][2] = interval;
		faceBoundary[5][3] = 2 * interval;

		faceBoundary[1][0] = 2 * interval;
		faceBoundary[1][1] = 3 * interval;
		faceBoundary[1][2] = interval;
		faceBoundary[1][3] = 2 * interval;

		faceBoundary[4][0] = 3 * interval;
		faceBoundary[4][1] = 4 * interval;
		faceBoundary[4][2] = interval;
		faceBoundary[4][3] = 2 * interval;

		faceBoundary[2][0] = interval;
		faceBoundary[2][1] = 2 * interval;
		faceBoundary[2][2] = 0;
		faceBoundary[2][3] = interval;

		faceBoundary[3][0] = interval;
		faceBoundary[3][1] = 2 * interval;
		faceBoundary[3][2] = 2 * interval;
		faceBoundary[3][3] = 3 * interval;
		
	}
	else
	{
		printf("Please make sure the texture is in standard form: four patches == width, while three patches == height\n");
		return -1;
	}

	for (int i = 0; i < 6; i++)
	{
		
		imageContainer[i] = FreeImage_Copy(img, faceBoundary[i][0], faceBoundary[i][2],
			faceBoundary[i][1], faceBoundary[i][3]);
		//For the above and botom image, have to rotate 180 degrees 
		if (i == 2 || i == 3)
		{
			imageContainer[i] = FreeImage_Rotate(imageContainer[i], 180);
		}

		if (imageContainer[i] !=NULL)
		{
			GLuint scanW = FreeImage_GetPitch(imageContainer[i]);
			GLubyte* byteImg = new GLubyte[interval*scanW];
			FreeImage_ConvertToRawBits(byteImg, imageContainer[i], scanW, 32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, TRUE);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, interval, interval, 0, GL_BGRA, GL_UNSIGNED_BYTE, byteImg);
			delete byteImg;
		}
		//FreeImage_Save(FIF_PNG, imageContainer[i], "subimage.png");
		FreeImage_Unload(imageContainer[i]);
	}
	FreeImage_Unload(img);
	
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	return tex_id;
}

