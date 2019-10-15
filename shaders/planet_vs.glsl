#version 430 core
//layout(location = 3) uniform mat4 PVM;


layout(location = 0) in vec3 pos_attrib;
layout(location = 1) in vec3 low_attrib;

out vec3 pos_low;

void main(void)
{
	gl_Position = vec4(pos_attrib, 1.0);
	pos_low = low_attrib;
}