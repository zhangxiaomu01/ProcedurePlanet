#version 430
layout(location = 0) uniform mat4 P;
layout(location = 1) uniform mat4 V;
layout(location = 2) uniform mat4 M;
layout(location = 3) uniform mat4 PVM;
layout(location = 4) uniform mat4 VM;

layout(location = 8) uniform float time;
layout(location = 10) uniform float slider[6];

layout(location = 16) uniform vec3 camera_High;
layout(location = 17) uniform vec3 camera_Low;

layout(location = 20) uniform int isFract;
layout(location = 21) uniform sampler2D permTex;
layout(location = 23) uniform float landOffset;


//layout (triangles, equal_spacing, ccw) in;	//discrete LOD
//Try some of these other options
layout (triangles, fractional_odd_spacing, ccw) in;	//continuous LOD
//layout (triangles, fractional_even_spacing, ccw) in;	//continuous LOD

#define TEXEL_FULL 0.003906250
#define TEXEL_HALF 0.001953125
#define FADE(t) ((t) * (t) * (t) * ((t) * ((t) * 6.0 - 15.0) + 10.0))


in float dist_tc[];
in vec3 pos_low_tc[];
in float distance_val[];

out vec3 n_te;
out vec3 n_overall;
out vec4 posMap;
out float distance_eye;

const float PI = 3.1415926535897932384626433832795;

float heightScale = 0.035;
int   octaves = 40;
float gain = 2.1;
float lacunarity = 2.01;
float offset =  1.0;
float h = 0.9;


//float hash( const in float n ) {
//    return fract(sin(n)*43758.5453123);
//}
//float noise( const in  vec3 x ) {
//    vec3 p = floor(x);
//    vec3 f = fract(x);
//    f = f*f*(3.0-2.0*f);
//    float n = p.x + p.y*157.0 + 113.0*p.z;
//    return mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
//                   mix( hash(n+157.0), hash(n+158.0),f.x),f.y),
//               mix(mix( hash(n+113.0), hash(n+114.0),f.x),
//                   mix( hash(n+270.0), hash(n+271.0),f.x),f.y),f.z);
//}
//float terrainMapH(vec3 n)
//{
//  float h=0.0,f=200.0 + slider[0]*100;
//  while(f>2.0f)	
//  {
//	h=h*0.5+ noise(n*f);
//	f/=2.0f;
//  }
//  return 1.0-clamp(h*0.5,0.0,1.0);//-0.61;
//}

float perlin(vec3 p)
{
  vec3 p0i = TEXEL_FULL * floor(p) + TEXEL_HALF;
  vec3 p1i = p0i + TEXEL_FULL;
  vec3 p0f = fract(p);
  vec3 p1f = p0f - 1.0;
  
  vec3 u = FADE(p0f);
  
  // Could using a single RGBA texture for the four permutation table lookups increase performance?
  float perm00 = texture(permTex, vec2(p0i.y, p0i.z)).a;
  float perm10 = texture(permTex, vec2(p1i.y, p0i.z)).a;
  float perm01 = texture(permTex, vec2(p0i.y, p1i.z)).a;
  float perm11 = texture(permTex, vec2(p1i.y, p1i.z)).a;
  
  float a = dot(texture(permTex, vec2(p0i.x, perm00)).rgb * 4.0 - 1.0, vec3(p0f.x, p0f.y, p0f.z));
  float b = dot(texture(permTex, vec2(p1i.x, perm00)).rgb * 4.0 - 1.0, vec3(p1f.x, p0f.y, p0f.z));
  float c = dot(texture(permTex, vec2(p0i.x, perm10)).rgb * 4.0 - 1.0, vec3(p0f.x, p1f.y, p0f.z));
  float d = dot(texture(permTex, vec2(p1i.x, perm10)).rgb * 4.0 - 1.0, vec3(p1f.x, p1f.y, p0f.z));
  float e = dot(texture(permTex, vec2(p0i.x, perm01)).rgb * 4.0 - 1.0, vec3(p0f.x, p0f.y, p1f.z));
  float f = dot(texture(permTex, vec2(p1i.x, perm01)).rgb * 4.0 - 1.0, vec3(p1f.x, p0f.y, p1f.z));
  float g = dot(texture(permTex, vec2(p0i.x, perm11)).rgb * 4.0 - 1.0, vec3(p0f.x, p1f.y, p1f.z));
  float h = dot(texture(permTex, vec2(p1i.x, perm11)).rgb * 4.0 - 1.0, vec3(p1f.x, p1f.y, p1f.z));
  
  vec4 lerpZ = mix(vec4(a, c, b, d), vec4(e, g, f, h), u.z);
  vec2 lerpY = mix(lerpZ.xz, lerpZ.yw, u.y);
  return mix(lerpY.x, lerpY.y, u.x);
}


float ridgedMultifractal(vec3 dir)
{
  float frequency = lacunarity, signal, weight;
  
  // Get the base signal (absolute value to create the ridges; square for sharper ridges)
  signal = offset - abs(perlin(dir));
  signal *= signal;
  float result = signal;
  
  float exponentArraySum = 1.0;
  for (int i = 1; i < octaves; i++)
  {
    // This could be precalculated
    float exponentValue = pow(frequency, -h);
    exponentArraySum += exponentValue;
    frequency *= lacunarity;
    
    dir *= lacunarity;
    weight = clamp(signal * gain, 0.0, 1.0);
    
    // Get the next "octave" (only true octave if lacunarity = 2.0, right?)
    signal = offset - abs(perlin(dir));
    signal *= signal;
    signal *= weight;
    
    result += signal * exponentValue;
  }
  
  // Scale result to [0,1] (not true when offset != 1.0)
  result /= exponentArraySum;
  
  return result;
}

void main()
{
	float heigtVal = 0.0;
	const float u = gl_TessCoord.x;
	const float v = gl_TessCoord.y;
	const float w = gl_TessCoord.z;

	const vec4 p0 = gl_in[0].gl_Position;
	const vec4 p1 = gl_in[1].gl_Position;
	const vec4 p2 = gl_in[2].gl_Position;
	//Generate each points' positions
	vec4 p = u*p0 + v*p1 + w*p2;
	vec3 p_low = u*pos_low_tc[0] + u*pos_low_tc[1] + u*pos_low_tc[2];
	
	distance_eye = u*distance_val[0] + u*distance_val[1] + u*distance_val[2];
	//Generate each points' normal vectors
	vec3 nV = normalize(p.xyz);
	n_overall = nV;
	//float offset_val = 0.1f;
	//vec3 offset_x = vec3(0.1f,0.0f,0.0f);
	//vec3 offset_y = vec3(0.0f,0.1f,0.0f);
	//vec3 offset_z = vec3(0.0f,0.0f,0.1f);

	//float hL = terrainMapH(normalize(nV.xyz - offset_x));
	//float hR = terrainMapH(normalize(nV.xyz + offset_x));
	//float hU = terrainMapH(normalize(nV.xyz + offset_y));
	//float hD = terrainMapH(normalize(nV.xyz - offset_y));
	//float hF = terrainMapH(normalize(nV.xyz + offset_z));
	//float hB = terrainMapH(normalize(nV.xyz - offset_z));

	//vec3 norm_temp;
	//norm_temp.x = abs(hL-hR)*0.1f;
	//norm_temp.y = abs(hD- hU)*0.1f;
	//norm_temp.z = abs(hB- hF)*0.1f;

	//vec3 np_low = normalize(p_low);


	
	//Generate terrain offset
	//float terrainOffset = terrainMapH(nV.xyz+vec3(landOffset,landOffset*2,landOffset*3));
	vec3 inputPos = nV+vec3(slider[0],slider[1]*2,slider[2]*3);
	float terrainOffset = ridgedMultifractal(inputPos);
	float terrainOffset_01 = terrainOffset*2.0 - 1.0;
	
	vec4 nf = vec4(nV,1.0);
	//vec4 nfl = vec4(np_low, 1.0);
	
	if(isFract == 0)
	{
		nf.xyz = nV;
		//nfl.xyz = np_low;
	}
	else
	{
		heigtVal = 1.0 + terrainOffset_01*0.05*slider[5];
		nf.xyz = nV*heigtVal;
		//nfl.xyz = np_low * (1.0 - terrainOffset*0.1*slider[5]);
	}
	nf.w = 1.0f;
	//nfl.w = 1.0f;

	vec3 t1 = p_low - camera_Low;
	vec3 e = t1 - p_low;
	vec3 t2 = ((-camera_Low - e) + (p_low - (t1 - e))) + vec3(nf) - camera_High;
	vec3 highDiff = t1+t2;
	vec3 lowDiff = t2 - (highDiff - t1);
	vec4 final_Pos = vec4(highDiff+ lowDiff, 1.0);

	posMap = vec4(inputPos*heigtVal,terrainOffset_01);

	//final_Pos = normalize(final_Pos);

	//vec4 p_eye = V*M*nf;
	
	//gl_Position = P*p_eye;
	gl_Position = PVM*final_Pos;
}
