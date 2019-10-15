#include "Helper.h"

double clamp(double x, double upper, double lower)
{
	return std::min(upper, std::max(x, lower));
}

void DoubletoTwoFloats(glm::dvec3 &a, glm::vec3 &high, glm::vec3 &low)
{
	for (int i = 0; i < a.length(); i++)
	{
		high[i] = static_cast<float> (a[i]);
		low[i] = static_cast<float> (a[i] - high[i]);
	}
}
