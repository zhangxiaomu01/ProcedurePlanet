#version 430

layout(location = 0) uniform mat4 P;
layout(location = 1) uniform mat4 V;
layout(location = 2) uniform mat4 M;
//layout(location = 10) uniform float slider[6];

layout (vertices = 3) out;  //number of output verts of the tess. control shader



out float dist_tc[];

vec3 eye = vec3(0.0f);

float dist_point_to_line_segment(vec3 p, vec3 l0, vec3 l1)
{
	//vec3 v = l1 - l0;
	//vec3 w = p - l0;
	//float b = clamp(dot(v,w)/dot(v,v),0,1);
	
	//return distance(p,l0+b*v);
	vec3 mid = (l0 + l1)/2.0f;
	return distance(p,mid);

}

float inner_tess_level(float d)
{
	
	//return exp(-0.1*d*slider[0])*slider[1];
	//float innerLevel = 96.0f*slider[1]-d*30.0f*slider[0];
	//if(innerLevel <=10.0f)
	//	innerLevel = 10.0f;
	float innerLevel = 10.0f;
	//innerLevel = (1.0f - (d-1.0f)/2.0f)*64.0f;
	innerLevel = exp(-(d))*64.0f;

	if(innerLevel<=10.0f)
		innerLevel = 10.0f;

	return innerLevel;
}

float outer_tess_level(vec3 p)
{
	vec3 sphereCenter = vec3(V*M*vec4(0.0f,0.0f,0.0f,1.0f));
	float d = distance(p, sphereCenter);

	//return exp(-0.1*d*slider[2])*slider[3];
	//float outerLevel = 96.0f*slider[2]-(d-1.0f)*10.0f*slider[3];
	float outerLevel = exp(-(d-1.0f))*64.0f;
	if(outerLevel <= 10.0f)
		outerLevel = 10.0f;
	return outerLevel;
}

void main()
{
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

	
	dist_tc[gl_InvocationID] = outer_tess_level(eye);

	

	vec3 p0 = vec3(V*M*normalize(gl_in[0].gl_Position));
	vec3 p1 = vec3(V*M*normalize(gl_in[1].gl_Position));
	vec3 p2 = vec3(V*M*normalize(gl_in[2].gl_Position));

	float d0 = dist_point_to_line_segment(eye,p0,p2);
	float d1 = dist_point_to_line_segment(eye,p0,p1);
	float d2 = dist_point_to_line_segment(eye,p1,p2);

	//gl_TessLevelOuter[0] = outer_tess_level(d0);
	//gl_TessLevelOuter[1] = outer_tess_level(d1);
	//gl_TessLevelOuter[2] = outer_tess_level(d2);
	//gl_TessLevelInner[0] = (outer_tess_level(d0) + outer_tess_level(d1) + outer_tess_level(d2))/3.0;
	
	
	float innerlevel = (inner_tess_level(d0) + inner_tess_level(d1) + inner_tess_level(d2))/3.0;
	gl_TessLevelOuter[0] = outer_tess_level(eye);
	gl_TessLevelOuter[1] = outer_tess_level(eye);
	gl_TessLevelOuter[2] = outer_tess_level(eye);
	gl_TessLevelInner[0] = innerlevel;

	//float innerlevel = slider[1];
	//gl_TessLevelOuter[0] = slider[0];
	//gl_TessLevelOuter[1] = slider[0];
	//gl_TessLevelOuter[2] = slider[0];
	//gl_TessLevelInner[0] = innerlevel;
}
