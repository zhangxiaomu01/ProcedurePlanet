#version 430
layout(location = 0) uniform mat4 P;
layout(location = 1) uniform mat4 V;
layout(location = 2) uniform mat4 M;
layout(location = 3) uniform mat4 PVM;

layout(location = 8) uniform float time;
//layout(location = 4) uniform int isFract;
layout(location = 10) uniform float slider[6];

//layout (triangles, equal_spacing, ccw) in;	//discrete LOD

layout (triangles, fractional_odd_spacing, ccw) in;	//continuous LOD
//layout (triangles, fractional_even_spacing, ccw) in;	//continuous LOD
in float dist_tc[];



out vec3 n_overall;
out vec2 tex_coord_te;
out vec4 p_world;

const float PI = 3.1415926535897932384626433832795;

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



void main()
{
	const float u = gl_TessCoord.x;
	const float v = gl_TessCoord.y;
	const float w = gl_TessCoord.z;

	const vec4 p0 = gl_in[0].gl_Position;
	const vec4 p1 = gl_in[1].gl_Position;
	const vec4 p2 = gl_in[2].gl_Position;
	//Generate each points' positions
	vec4 p = u*p0 + v*p1 + w*p2;

	//Generate each points' normal vectors
	vec3 nV = normalize(p.xyz);
	n_overall = nV;
	//n_te = terrainCalcNormalHigh( nV, 0.0000001f);
	
	//Generate terrain offset
	float terrainOffset = terrainMapH(nV.xyz);

	float wave_X = sin(time+nV.x)*0.0001f;

	vec4 nf = vec4(nV,1.0f);
	nf.xyz = nV*1.01f + nV*wave_X;
	nf.w = 1.0f;


	//Genrate UV texture coordinate
	float tex_u = atan(nV.z, nV.x)/2*PI + 0.5f;
	float tex_v = nV.y* 0.5f + 0.5f;
	tex_coord_te = vec2(tex_u/6.0f,tex_v);
	
	
	p_world = M*nf;
	
	gl_Position = P*V*p_world;
	//gl_Position = PVM*nf;
}
