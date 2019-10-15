#version 430

layout(location = 0) uniform mat4 P;
layout(location = 1) uniform mat4 V;
layout(location = 2) uniform mat4 M;

layout(location = 5) uniform vec3 light;
layout(location = 10) uniform float slider[6];

float hash( const in float n ) {
    return fract(sin(n)*43758.5453123);
}
float noise( const in  vec3 x ) {
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);
    float n = p.x + p.y*157.0 + 113.0*p.z;
    return mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                   mix( hash(n+157.0), hash(n+158.0),f.x),f.y),
               mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                   mix( hash(n+270.0), hash(n+271.0),f.x),f.y),f.z);
}
float terrainMapH(vec3 n)
{
  float h=0.0,f=200.0;
  while(f>2.0f)	
  {
	h=h*0.5+ noise(n*f);
	f/=2.0f;
  }
  return 1.0-clamp(h*0.5,0.0,1.0);//-0.61;
}




vec3 terrainCalcNormalHigh( in vec3 pos, float t ) {
    vec2 e = vec2(1.0,-1.0)*t;

    return normalize( e.xyy*terrainMapH( pos + e.xyy ) + 
					  e.yyx*terrainMapH( pos + e.yyx ) + 
					  e.yxy*terrainMapH( pos + e.yxy ) + 
					  e.xxx*terrainMapH( pos + e.xxx ) );
}



out vec4 fragcolor; 

          
in float dist;
in vec3 n_overall;
in vec2 tex_coord_te;
in vec4 p_world;

vec3 light1 = vec3(slider[0],slider[1],slider[2]);

     
void main(void)
{   
	float specular_t = 0;

	vec3 n_te = terrainCalcNormalHigh(n_overall,0.001);
	
	float shiness = 10.0f;

	vec3 v_w = normalize(vec3(inverse(V) * vec4(0,0,0,1.0) - p_world));

	vec3 light_w = normalize(light);
	
	
	vec4 normal_w = normalize(transpose(inverse(M))*vec4(n_overall,0.0f));

	vec3 r = normalize(reflect(-light_w, vec3(normal_w)));
	//vec3 h = vec3((eye_light + vec4(v,0.0f))/length(eye_light + vec4(v,0.0f)));
	//vec3 h = normalize(vec3((eye_light + vec4(v,0.0f))));
	
	specular_t = pow(max(dot(r,v_w),0),shiness);

	float fresnelFac = pow(clamp(1.0-dot(vec3(normal_w), v_w),0.0,1.0), 5.0)*3.0;

	//vec3 norm = vec3(n_te.r*0.5 + 0.6);
	float diffuse_t = clamp(dot(vec3(normal_w),light_w),0.0f,1.0f);
	vec4 color1 = vec4(0.023, 0.11, 0.478, 1.0);
	vec4 color2 = vec4(0.1, 0.25, 0.478, 1.0);

	//fragcolor = color1*diffuse_t;
	fragcolor = color1*diffuse_t + specular_t*vec4(0.7f) + color2*fresnelFac*diffuse_t;
	//fragcolor = vec4(fresnelFac);
	fragcolor.w = 1.0f;
}

