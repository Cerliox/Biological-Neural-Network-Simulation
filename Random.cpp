#include "Random.h"

float Random::RandomFloat(float min, float max)
{
	float r = (float)rand() / (float)RAND_MAX;
	return min + r * (max - min);
}
double Random::RandomDouble(double min, double max) {
	double r = (double)rand() / (double)RAND_MAX;
	return min + r * (max - min);
}
int Random::RandomInt(int min, int max)
{
	return min + rand() % ((max) - min);
}