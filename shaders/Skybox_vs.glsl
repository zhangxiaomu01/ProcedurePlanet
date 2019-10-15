#version 430            
layout (location = 3) uniform mat4 PVM;

in vec3 pos_attrib;

  
out vec3 pos;

void main(void)
{
   gl_Position = PVM * vec4(pos_attrib, 1.0);
   pos = pos_attrib;

}
