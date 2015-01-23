#ifndef _TRIANGLE
#define _TRIANGLE

#include "Point.h"
#include "Edge.h"

class Triangle
{
public:
	Point* points[3];
	Triangle* neighbors[3];
	bool interior;
	bool constrained_edge[3];
	bool delaunay_edge[3];

	Triangle();
	Triangle(Point &a, Point &b, Point &c);
	bool operator==(const Triangle &other) const;

	Point* get_point(unsigned int index) const;
	Triangle* get_neighbor(unsigned int index);

	bool contains_point(const Point &point);
	bool contains_edge(const Edge &edge);
	bool contains_points(const Point &p1, const Point &p2);

	bool is_interior();
	Triangle* set_interior(bool t_interior);
	void mark_neighbor_pointers(const Point &p1, const Point &p2, Triangle &t);
	void mark_neighbor(Triangle &t);
	void clear_neighbors();
	void clear_delaunay_edges();

	Point* point_cw(const Point &p) const;
	Point* point_ccw(const Point &p) const;

	Triangle* neighbor_cw(const Point &p) const;
	Triangle* neighbor_ccw(const Point &p) const;

	bool get_constrained_edge_cw(const Point &p) const;
	bool get_constrained_edge_ccw(const Point &p) const;
	bool get_constrained_edge_across(const Point &p) const;

	void set_constrained_edge_cw(const Point &p, bool ce);
	void set_constrained_edge_ccw(const Point &p, bool ce);

	bool get_delaunay_edge_cw(const Point &p) const;
	bool get_delaunay_edge_ccw(const Point &p) const;

	void set_delaunay_edge_cw(const Point &p, bool e);
	void set_delaunay_edge_ccw(const Point &p, bool e);

	Triangle* neighbor_across(const Point &p) const;
	Point* opposite_point(const Triangle &t, const Point &p) const;
	void legalize(const Point &opoint, Point &npoint);
	unsigned int index(const Point &p) const;
	unsigned int edge_index(const Point &p1, const Point &p2) const;

	void mark_constrained_edge_by_index(unsigned int index);
	void mark_constrained_edge_by_edge(const Edge &edge);
	void mark_constrained_edge_by_points(const Point &p1, const Point &p2);
};

typedef std::vector<Triangle*> Triangles;

#endif