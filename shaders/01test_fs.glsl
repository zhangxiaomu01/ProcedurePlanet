#version 430

out vec4 fragcolor;       

//in vec2 uv_coord;
//in vec3 normal_coord;    
     
void main(void)
{   

	vec4 color1 = vec4(0.9, 0.9, 1.0, 1.0);
	vec4 color2 = vec4(0.0, 0.0, 0.3, 1.0);

	fragcolor  = mix(color2, color1, 0.5f);
	//fragcolor  = vec4(uv_coord,0.0f,1.0f);
}

