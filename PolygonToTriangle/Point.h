#ifndef _POINT
#define _POINT

#include <vector>

class Edge;

class Point
{
public:
	double x;
	double y;
	std::vector<Edge*> p2t_edge_list;
	
	Point() : x(0), y(0), p2t_edge_list() {}
	Point(double tx, double ty) : x(tx), y(ty), p2t_edge_list() {}

	Point* set_zero();
	Point* set(double tx, double ty);
	Point* negate();
	Point* add(const Point &other);
	Point* sub(const Point &other);
	Point* mul(double s);
	double length();
	double normalize();
	bool operator==(const Point &other) const;
	bool operator<(const Point &other) const;
};

Point negate(const Point &other);
Point add(const Point &a, const Point &b);
Point sub(const Point &a, const Point &b);
Point mul(double s, const Point &p);
double cross(const Point &a, const Point &b);
Point cross(double a, const Point &b);
Point cross(const Point &a, double b);
double dot(const Point &a, const Point &b);
bool cmp_point_pointer(Point *p1, Point *p2);

typedef std::vector<Point*> PolyLine;
typedef PolyLine Points;
typedef PolyLine Hole;
typedef std::vector<PolyLine> Holes;
#endif