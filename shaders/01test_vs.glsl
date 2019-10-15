#version 430 core
layout(location = 0) uniform mat4 P;
layout(location = 1) uniform mat4 V;
layout(location = 2) uniform mat4 M;


layout(location = 0) in vec3 pos_attrib;
layout(location = 1) in vec3 tex_coord;
layout(location = 2) in vec3 normal_attrib;

out vec2 uv_coord;
out vec3 normal_coord;

void main(void)
{
	//uv_coord = vec2(tex_coord);
	//normal_coord = normal_attrib;
	gl_Position = P*V*M*vec4(pos_attrib, 1.0);
}