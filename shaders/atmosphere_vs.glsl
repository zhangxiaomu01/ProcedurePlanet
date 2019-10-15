#version 420 core

layout(location = 0) in vec3 pos_attrib;
layout(location = 1) in vec3 low_attrib;



void main(void)
{

	gl_Position = vec4(pos_attrib, 1.0);
}