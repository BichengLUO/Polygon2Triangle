#ifndef _NODE
#define _NODE

#include "Point.h"
#include "Triangle.h"

typedef struct _node
{
	Point *point;
	Triangle *triangle;
	_node *next;
	_node *prev;
	double value;

	_node() : point(NULL), triangle(NULL), next(NULL), prev(NULL), value(0) {}
	_node(Point &p) : point(&p), triangle(NULL), next(NULL), prev(NULL), value(p.x) {}
	_node(Point &p, Triangle &t) : point(&p), triangle(&t), next(NULL), prev(NULL), value(p.x) {}
} Node;

#endif