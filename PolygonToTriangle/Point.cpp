#include "stdafx.h"
#include "Point.h"
#include <cmath>

Point* Point::set_zero()
{
	this->x = 0;
	this->y = 0;
	return this;
}

Point* Point::set(double tx, double ty)
{
	this->x = tx;
	this->y = ty;
	return this;
}

Point* Point::negate()
{
	this->x = -this->x;
	this->y = -this->y;
	return this;
}

Point* Point::add(const Point &other)
{
	this->x += other.x;
	this->y += other.y;
	return this;
}

Point* Point::sub(const Point &other)
{
	this->x -= other.x;
	this->y -= other.y;
	return this;
}

Point* Point::mul(double s)
{
	this->x *= s;
	this->y *= s;
	return this;
}

double Point::length()
{
	return sqrt(this->x*this->x + this->y*this->y);
}

double Point::normalize()
{
	double len = this->length();
	this->x /= len;
	this->y /= len;
	return len;
}

bool Point::operator==(const Point &other) const
{
	return this->x == other.x && this->y == other.y;
}

bool Point::operator<(const Point &other) const
{
	return this->y == other.y ? this->x < other.x : this->y < other.y;
}

Point negate(const Point &other)
{
	return Point(-other.x, -other.y);
}

Point add(const Point &a, const Point &b)
{
	return Point(a.x + b.x, a.y + b.y);
}

Point sub(const Point &a, const Point &b)
{
	return Point(a.x - b.x, a.y - b.y);
}

Point mul(double s, const Point &p)
{
	return Point(s * p.x, s * p.y);
}

double cross(const Point &a, const Point &b)
{
	return a.x * b.y - a.y * b.x;
}

Point cross(double a, const Point &b)
{
	return Point(-a * b.y, a * b.x);
}

Point cross(const Point &a, double b)
{
	return Point(b * a.y, -b * a.x);
}

double dot(const Point &a, const Point &b)
{
	return a.x * b.x + a.y * b.y;
}

bool cmp_point_pointer(Point *p1, Point *p2)
{
	if (*p1 < *p2) return true;
	else return false;
}