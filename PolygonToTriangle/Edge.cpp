#include "stdafx.h"
#include "Edge.h"

Edge::Edge(Point &p1, Point &p2)
{
	p = &p1;
	q = &p2;

	if (p1.y > p2.y) {
		q = &p1;
		p = &p2;
	}
	else if (p1.y == p2.y) {
		if (p1.x > p2.x) {
			q = &p1;
			p = &p2;
		}
	}

	q->p2t_edge_list.push_back(this);
}