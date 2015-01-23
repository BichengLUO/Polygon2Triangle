#ifndef _EDGE
#define _EDGE

#include "Point.h"
#include <cstddef>

class Edge
{
public:
	Point *p;
	Point *q;
	Edge() : p(NULL), q(NULL) {}
	Edge(Point &p1, Point &p2);
};

class EdgeEvent
{
public:
	Edge *constrained_edge;
	bool right;
	EdgeEvent() : constrained_edge(NULL), right(false) {}
};

#endif