#include "stdafx.h"
#include "Triangle.h"
#include <cstddef>

Triangle::Triangle()
{
	for (int i = 0; i < 3; i++)
	{
		points[i] = NULL;
		neighbors[i] = NULL;
		constrained_edge[i] = delaunay_edge[i] = false;
	}
	interior = false;
}

Triangle::Triangle(Point &a, Point &b, Point &c)
{
	points[0] = &a;
	points[1] = &b;
	points[2] = &c;
	for (int i = 0; i < 3; i++)
	{
		neighbors[i] = NULL;
		constrained_edge[i] = delaunay_edge[i] = false;
	}
	interior = false;
}

bool Triangle::operator==(const Triangle &other) const
{
	if (interior != other.interior) return false;
	for (int i = 0; i < 3; i++)
	{
		if (points[i] != other.points[i] ||
			neighbors[i] != other.neighbors[i] ||
			constrained_edge[i] != other.constrained_edge[i] ||
			delaunay_edge[i] != other.delaunay_edge[i])
			return false;
	}
	return true;
}

Point* Triangle::get_point(unsigned int index) const
{
	return points[index];
}

Triangle* Triangle::get_neighbor(unsigned int index)
{
	return neighbors[index];
}

bool Triangle::contains_point(const Point &point)
{
	return (&point == points[0] || &point == points[1] || &point == points[2]);
}

bool Triangle::contains_edge(const Edge &edge)
{
	return contains_point(*edge.p) && contains_point(*edge.q);
}

bool Triangle::contains_points(const Point &p1, const Point &p2)
{
	return contains_point(p1) && contains_point(p2);
}

bool Triangle::is_interior()
{
	return interior;
}

Triangle* Triangle::set_interior(bool t_interior)
{
	interior = t_interior;
	return this;
}

void Triangle::mark_neighbor_pointers(const Point &p1, const Point &p2, Triangle &t)
{
	if ((&p1 == points[2] && &p2 == points[1]) || (&p1 == points[1] && &p2 == points[2])) {
		neighbors[0] = &t;
	}
	else if ((&p1 == points[0] && &p2 == points[2]) || (&p1 == points[2] && &p2 == points[0])) {
		neighbors[1] = &t;
	}
	else if ((&p1 == points[0] && &p2 == points[1]) || (&p1 == points[1] && &p2 == points[0])) {
		neighbors[2] = &t;
	}
}

void Triangle::mark_neighbor(Triangle &t)
{
	if (t.contains_points(*points[1], *points[2])) {
		neighbors[0] = &t;
		t.mark_neighbor_pointers(*points[1], *points[2], *this);
	}
	else if (t.contains_points(*points[0], *points[2])) {
		neighbors[1] = &t;
		t.mark_neighbor_pointers(*points[0], *points[2], *this);
	}
	else if (t.contains_points(*points[0], *points[1])) {
		neighbors[2] = &t;
		t.mark_neighbor_pointers(*points[0], *points[1], *this);
	}
}

void Triangle::clear_neighbors()
{
	for (int i = 0; i < 3; i++) neighbors[i] = NULL;
}

void Triangle::clear_delaunay_edges()
{
	for (int i = 0; i < 3; i++) delaunay_edge[i] = false;
}

Point* Triangle::point_cw(const Point &p) const
{
	if (&p == points[0]) {
		return points[2];
	}
	else if (&p == points[1]) {
		return points[0];
	}
	else if (&p == points[2]) {
		return points[1];
	}
	else {
		return NULL;
	}
}

Point* Triangle::point_ccw(const Point &p) const
{
	if (&p == points[0]) {
		return points[1];
	}
	else if (&p == points[1]) {
		return points[2];
	}
	else if (&p == points[2]) {
		return points[0];
	}
	else {
		return NULL;
	}
}

Triangle* Triangle::neighbor_cw(const Point &p) const
{
	if (&p == points[0]) {
		return neighbors[1];
	}
	else if (&p == points[1]) {
		return neighbors[2];
	}
	else {
		return neighbors[0];
	}
}

Triangle* Triangle::neighbor_ccw(const Point &p) const
{
	if (&p == points[0]) {
		return neighbors[2];
	}
	else if (&p == points[1]) {
		return neighbors[0];
	}
	else {
		return neighbors[1];
	}
}

bool Triangle::get_constrained_edge_cw(const Point &p) const
{
	if (&p == points[0]) {
		return constrained_edge[1];
	}
	else if (&p == points[1]) {
		return constrained_edge[2];
	}
	else {
		return constrained_edge[0];
	}
}

bool Triangle::get_constrained_edge_ccw(const Point &p) const
{
	if (&p == points[0]) {
		return constrained_edge[2];
	}
	else if (&p == points[1]) {
		return constrained_edge[0];
	}
	else {
		return constrained_edge[1];
	}
}

bool Triangle::get_constrained_edge_across(const Point &p) const
{
	if (&p == points[0]) {
		return constrained_edge[0];
	}
	else if (&p == points[1]) {
		return constrained_edge[1];
	}
	else {
		return constrained_edge[2];
	}
}

void Triangle::set_constrained_edge_cw(const Point &p, bool ce)
{
	if (&p == points[0]) {
		constrained_edge[1] = ce;
	}
	else if (&p == points[1]) {
		constrained_edge[2] = ce;
	}
	else {
		constrained_edge[0] = ce;
	}
}

void Triangle::set_constrained_edge_ccw(const Point &p, bool ce)
{
	if (&p == points[0]) {
		constrained_edge[2] = ce;
	}
	else if (&p == points[1]) {
		constrained_edge[0] = ce;
	}
	else {
		constrained_edge[1] = ce;
	}
}

bool Triangle::get_delaunay_edge_cw(const Point &p) const
{
	if (&p == points[0]) {
		return delaunay_edge[1];
	}
	else if (&p == points[1]) {
		return delaunay_edge[2];
	}
	else {
		return delaunay_edge[0];
	}
}

bool Triangle::get_delaunay_edge_ccw(const Point &p) const
{
	if (&p == points[0]) {
		return delaunay_edge[2];
	}
	else if (&p == points[1]) {
		return delaunay_edge[0];
	}
	else {
		return delaunay_edge[1];
	}
}

void Triangle::set_delaunay_edge_cw(const Point &p, bool e)
{
	if (&p == points[0]) {
		delaunay_edge[1] = e;
	}
	else if (&p == points[1]) {
		delaunay_edge[2] = e;
	}
	else {
		delaunay_edge[0] = e;
	}
}

void Triangle::set_delaunay_edge_ccw(const Point &p, bool e)
{
	if (&p == points[0]) {
		delaunay_edge[2] = e;
	}
	else if (&p == points[1]) {
		delaunay_edge[0] = e;
	}
	else {
		delaunay_edge[1] = e;
	}
}

Triangle* Triangle::neighbor_across(const Point &p) const
{
	if (&p == points[0]) {
		return neighbors[0];
	}
	else if (&p == points[1]) {
		return neighbors[1];
	}
	else {
		return neighbors[2];
	}
}

Point* Triangle::opposite_point(const Triangle &t, const Point &p) const
{
	Point *cw = t.point_cw(p);
	return point_cw(*cw);
}

void Triangle::legalize(const Point &opoint, Point &npoint)
{
	if (&opoint == points[0]) {
		points[1] = points[0];
		points[0] = points[2];
		points[2] = &npoint;
	}
	else if (&opoint == points[1]) {
		points[2] = points[1];
		points[1] = points[0];
		points[0] = &npoint;
	}
	else if (&opoint == points[2]) {
		points[0] = points[2];
		points[2] = points[1];
		points[1] = &npoint;
	}
}

unsigned int Triangle::index(const Point &p) const
{
	if (&p == points[0]) {
		return 0;
	}
	else if (&p == points[1]) {
		return 1;
	}
	else if (&p == points[2]) {
		return 2;
	}
	return -1;
}

unsigned int Triangle::edge_index(const Point &p1, const Point &p2) const
{
	if (&p1 == points[0]) {
		if (&p2 == points[1]) {
			return 2;
		}
		else if (&p2 == points[2]) {
			return 1;
		}
	}
	else if (&p1 == points[1]) {
		if (&p2 == points[2]) {
			return 0;
		}
		else if (&p2 == points[0]) {
			return 2;
		}
	}
	else if (&p1 == points[2]) {
		if (&p2 == points[0]) {
			return 1;
		}
		else if (&p2 == points[1]) {
			return 0;
		}
	}
	return -1;
}

void Triangle::mark_constrained_edge_by_index(unsigned int index)
{
	constrained_edge[index] = true;
}

void Triangle::mark_constrained_edge_by_edge(const Edge &edge)
{
	mark_constrained_edge_by_points(*edge.p, *edge.q);
}

void Triangle::mark_constrained_edge_by_points(const Point &p, const Point &q)
{
	if ((&q == points[0] && &p == points[1]) || (&q == points[1] && &p == points[0])) {
		constrained_edge[2] = true;
	}
	else if ((&q == points[0] && &p == points[2]) || (&q == points[2] && &p == points[0])) {
		constrained_edge[1] = true;
	}
	else if ((&q == points[1] && &p == points[2]) || (&q == points[2] && &p == points[1])) {
		constrained_edge[0] = true;
	}
}