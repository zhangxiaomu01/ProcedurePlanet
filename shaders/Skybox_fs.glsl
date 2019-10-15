#version 430

uniform samplerCube cubemap;
out vec4 fragcolor;

       
//in vec2 tex_coord;
in vec3 pos;   
      
void main(void)
{   
	fragcolor = texture(cubemap, pos);
}