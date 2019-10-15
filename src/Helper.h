#ifndef __HELPER_H__
#define __HELPER_H__
#include <algorithm>
#include<vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifndef M_PI
#define M_PI           3.14159265358979323846  /* pi */
#endif

double clamp(double x, double upper, double lower);
void DoubletoTwoFloats(glm::dvec3 &a, glm::vec3 &high, glm::vec3 &low);

#endif
