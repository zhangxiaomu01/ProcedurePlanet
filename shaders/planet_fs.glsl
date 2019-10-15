#version 430
layout(location = 0) uniform mat4 P;
layout(location = 1) uniform mat4 V;
layout(location = 2) uniform mat4 M;

layout(location = 5) uniform vec3 light;

layout(location = 10) uniform float slider[6];

layout(location = 16) uniform vec3 camera_High;
layout(location = 17) uniform vec3 camera_Low;

layout(location = 20) uniform int isFract;
layout(location = 21) uniform sampler2D permTex;
layout(location = 22) uniform int isCalNorm;
layout(location = 24) uniform sampler2D earthTex;
layout(location = 25) uniform sampler2D snowTex;
layout(location = 26) uniform sampler2D grassTex;

#define TEXEL_FULL 0.003906250
#define TEXEL_HALF 0.001953125
#define FADE(t) ((t) * (t) * (t) * ((t) * ((t) * 6.0 - 15.0) + 10.0))

const float PI = 3.1415926535897932384626433832795;

float heightScale = 0.035;
int   octaves = 20;
float gain = 2.1;
float lacunarity = 2.01;
float offset =  1.0;
float h = 0.9;

out vec4 fragcolor; 

          
in float dist;
in vec3 n_te;
in vec3 n_overall;
in vec4 posMap;
in float distance_eye;
//vec3 light = vec3(0.0,1.0,1.0);

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
//  float h=0.0,f=200.0;
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


vec3 terrainCalcNormalHigh( in vec3 pos, float t ) {
    vec3 e = vec3(1.0,-1.0,0.0)*t;
	
	//vec3 posXMY0 = normalize(pos + e.xyy);
 //   vec3 posXPY0 = normalize(pos + e.yyx);
 //   vec3 posX0YM = normalize(pos + e.yxy);
 //   vec3 posX0YP = normalize(pos + e.xxx);
	
	//vec3 edgeXM = posXMY0 - pos;
 //   vec3 edgeXP = posXPY0 - pos;
 //   vec3 edgeYM = posX0YM - pos;
 //   vec3 edgeYP = posX0YP - pos;

	//vec3 normalM = cross(edgeXM, edgeYM);
 //   vec3 normalP = cross(edgeXP, edgeYP);

	//vec3 normal = normalize(normalM + normalP);
	////normal = vec3(0.5) + 0.5*normal;
	//return normal;

	vec3 normal = normalize(  e.xyy*ridgedMultifractal( pos + e.xyy ) + 
					   e.yyx*ridgedMultifractal( pos + e.yyx ) + 
					   e.yxy*ridgedMultifractal( pos + e.yxy ) + 
					   e.xxx*ridgedMultifractal( pos + e.xxx ) );
	
	normal = vec3(0.7) + 0.3*normal;
    return normal;
}    

//vec3 NormalGenerator()
//{
//	vec3 posX0Y0 = texture2D(positionmap, gl_TexCoord[0].st).rgb;
//    vec3 posXMY0 = texture2D(positionmap, gl_TexCoord[0].st - vec2(texelSize, 0.0)).rgb;
//    vec3 posXPY0 = texture2D(positionmap, gl_TexCoord[0].st + vec2(texelSize, 0.0)).rgb;
//    vec3 posX0YM = texture2D(positionmap, gl_TexCoord[0].st - vec2(0.0, texelSize)).rgb;
//    vec3 posX0YP = texture2D(positionmap, gl_TexCoord[0].st + vec2(0.0, texelSize)).rgb;
  
//    // Edges connecting the samples
//    vec3 edgeXM = posXMY0 - posX0Y0;
//    vec3 edgeXP = posXPY0 - posX0Y0;
//    vec3 edgeYM = posX0YM - posX0Y0;
//    vec3 edgeYP = posX0YP - posX0Y0;
  
//    // Using only one of these normals is faster but not as accurate
//    vec3 normalM = cross(edgeXM, edgeYM);
//    vec3 normalP = cross(edgeXP, edgeYP);
   
//    // Normalize the sum of both normals (averaging happens automatically)
//    vec3 normal = normalize(normalM + normalP);		
//}



void main(void)
{   

	float tex_u = atan(n_overall.z, n_overall.x)/2*PI + 0.5f;
	float tex_v = n_overall.y* 0.5f + 0.5f;
	vec2 tex_coord = vec2(tex_u,tex_v);

	vec4 planet_Center = M* vec4(0.0,0.0,0.0,1.0);
	float distance_eye_center = pow((distance(vec3(planet_Center),camera_High)- 1.0f)/4.0f,2.0f);
	distance_eye_center = clamp(distance_eye_center,0.005,1.0);
	vec3 norm = terrainCalcNormalHigh(n_overall, 0.01*distance_eye_center);
	//norm = vec3(transpose(inverse(M))*vec4(norm,0.0));
	norm.yz = norm.zy;
	norm.xz = -norm.xz;
	float diffuse_t = clamp(dot(n_overall,light),0.0f,1.0f);
	float diffuse_Detail =  clamp(dot(norm,light),0.2f,1.0f);

	vec4 color1 = vec4(0.5, 0.5, 0.5, 1.0);
	vec4 color2 = vec4(0.0, 0.0, 0.3, 1.0);

	float reverse_distance = clamp(1-distance_eye_center, 0.1, 1.0)*9;
	vec4 earthLayer = texture(earthTex,tex_coord*3*reverse_distance);
	vec4 snowLayer = texture(snowTex,tex_coord*2*reverse_distance);
	vec4 grassLayer = texture(grassTex,tex_coord*15*reverse_distance);
	float blendingFac = clamp(posMap.w,0.0,1.0);
	blendingFac = smoothstep( 0.2, 0.9,blendingFac);

	vec4 colorLayer= mix(earthLayer, snowLayer, pow(blendingFac,1.2*slider[3])); 
	colorLayer = mix(grassLayer, colorLayer,pow(blendingFac,slider[3]/3.33));

	if(isFract == 0)
	{
		fragcolor = texture(snowTex,tex_coord);
	}
	else
	{
		//fragcolor = vec4(0.1f,0.1f,0.1f,1.0f)+texture(checker_tex,tex_coord_te)*diffuse_t;
		if(isCalNorm == 0)
			fragcolor = colorLayer*diffuse_t*diffuse_Detail+colorLayer*vec4(0.1f,0.1f,0.1f,1.0f);
			//*diffuse_t*diffuse_Detail
		else
			//fragcolor = vec4(1.0f)*diffuse_t*clamp(norm.r,0.1f,0.9f);
			fragcolor = vec4(vec3(pow(blendingFac,1.2)),1.0);
			//fragcolor = vec4(posMap);
			// *clamp(norm.r,0.1f,0.9f)
	}
	fragcolor.w = 1.0f;
}

