#include "Vector.h"

Vec2::Vec2() {
	this->x = 0.0f;
	this->y = 0.0f;
}
Vec2::Vec2(double val) {
	this->x = val;
	this->y = val;
}
Vec2::Vec2(double x, double y) {
	this->x = x;
	this->y = y;
}

Vec2& Vec2::Normalize() {
	double l = sqrt(powf(x, 2) + pow(y, 2)); // Same as Length() function, but faster due to no more function call
	this->x = x / l;
	this->y = y / l;
	return *this;
}
double Vec2::Length() {
	return sqrt(powf(x, 2.0f) + powf(y, 2.0f));
}
double Vec2::Dot(Vec2 other) {
	return this->x * other.x + this->y * other.y;
}

Vec2 Vec2::operator*(Vec2 other) {
	return Vec2(x * other.x, y * other.y);
}
Vec2 Vec2::operator*(double val) {
	return Vec2(x * val, y * val);
}
Vec2& Vec2::operator*=(double val) {
	this->x *= val;
	this->y *= val;
	return *this;
}
Vec2 Vec2::operator+(Vec2 other) {
	return Vec2(x + other.x, y + other.y);
}
Vec2& Vec2::operator+=(Vec2 other) {
	this->x += other.x;
	this->y += other.y;
	return *this;
}
Vec2 Vec2::operator-(Vec2 other) {
	return Vec2(x - other.x, y - other.y);
}
Vec2& Vec2::operator-=(Vec2 other) {
	this->x -= other.x;
	this->y -= other.y;
	return *this;
}
Vec2 Vec2::operator/(Vec2 other) {
	return Vec2(this->x / other.x, this->y / other.y);
}
Vec2 Vec2::operator/(double a) {
	return Vec2(this->x / a, this->y / a);
}