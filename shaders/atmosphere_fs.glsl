#version 430

layout(location = 0) uniform mat4 P;
layout(location = 1) uniform mat4 V;
layout(location = 2) uniform mat4 M;

layout(location = 4) uniform int isFract;
layout(location = 5) uniform sampler2D cloud_tex;
layout(location = 6) uniform int pass;

out vec4 fragcolor; 

          
in float dist;
in vec3 n_te;
in vec3 n_overall;
in vec4 p_eye;

vec3 light = vec3(0.0,1.0,1.0);
vec3 v = vec3(0,0,0);
     
void main(void)
{   
	float shiness = 2.0f;

	vec4 eye_light =transpose(inverse (V))*vec4(light,0.0f);
	vec4 eye_normal = normalize(transpose(inverse(V))*vec4(n_te,0.0f));
	if(pass == 1)
		eye_normal = -eye_normal;
	vec3 v_eye = normalize(v-vec3(p_eye));

	vec3 h = vec3((eye_light + vec4(v,0.0f))/length(eye_light + vec4(v,0.0f)));
	
	float specular_t = pow(max(dot(h,vec3(eye_normal)),0),shiness);

	float fresnelFac = pow(clamp(1.0 - dot( vec3(eye_normal), v_eye),0.0,1.0), 5.0);

	vec3 norm = vec3(n_te.r*0.5 + 0.6);
	float diffuse_t = clamp(dot(n_overall,light),0.0f,1.0f);
	vec4 color1 = vec4(0.023, 0.11, 0.478, 1.0); 
	vec4 color2 = vec4(1.0);
	vec4 colorF = mix(color1, color2, fresnelFac);

	vec4 FinalColor = colorF * fresnelFac * diffuse_t*1.8f;
	//fragcolor = texture(cloud_tex,tex_coord_te)*diffuse_t;
	//fragcolor = texture(cloud_tex,tex_coord_te)*diffuse_t*1.5f;
	fragcolor = vec4(FinalColor);
	//fragcolor.w = 1.0f;
}

