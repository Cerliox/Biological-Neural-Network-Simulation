#pragma once
#include <math.h>

struct Vec2 {
public:
	double x, y;

	Vec2();
	Vec2(double);
	Vec2(double, double);

	Vec2& Normalize();
	double Length();

	double Dot(Vec2);

	Vec2 operator*(Vec2);
	Vec2 operator*(double);
	Vec2& operator*=(double);

	Vec2 operator+(Vec2);
	Vec2& operator+=(Vec2);

	Vec2 operator-(Vec2);
	Vec2& operator-=(Vec2);

	Vec2 operator/(Vec2);
	Vec2 operator/(double);
};
double fminmax(double, double, double);