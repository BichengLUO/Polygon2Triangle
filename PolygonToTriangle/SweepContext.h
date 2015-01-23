#ifndef _SWEEP_CONTEXT
#define _SWEEP_CONTEXT

#include "Triangle.h"
#include "AdvancingFront.h"
#include "Basin.h"
#include <vector>

#define EPSILON 1e-12
const double kAlpha = 0.3;

typedef struct _bounding_box
{
	Point min;
	Point max;
} BoundingBox;

typedef enum _orientation
{
	CW = 1,
	CCW = -1,
	COLLINEAR = 0
} Orientation;

class SweepContext
{
public:
	Triangles triangles;
	Points points;
	std::vector<Edge*> edge_list;
	Triangles map;

	AdvancingFront front;
	Point pmin;
	Point pmax;
	Point head;
	Point tail;
	Node *af_head;
	Node *af_middle;
	Node *af_tail;
	Basin basin;
	EdgeEvent edge_event;

	SweepContext() : triangles(),
		points(), edge_list(), map(),
		front(), pmin(), pmax(),
		head(), tail(),
		af_head(NULL), af_middle(NULL), af_tail(NULL),
		basin(), edge_event(),
		new_edges(), new_nodes(), new_triangles()
	{
		init_edges(points);
	}

	SweepContext(Points tpoints) : triangles(),
		points(tpoints), edge_list(), map(),
		front(), pmin(), pmax(),
		head(), tail(),
		af_head(NULL), af_middle(NULL), af_tail(NULL),
		basin(), edge_event(),
		new_edges(), new_nodes(), new_triangles()
	{
		init_edges(points);
	}

	SweepContext* add_hole(PolyLine &polyline);
	SweepContext* add_holes(Holes &holes);
	SweepContext* add_point(Point &point);
	SweepContext* add_points(const Points &ps);

	SweepContext* triangulate();
	BoundingBox get_bounding_box();
	Triangles get_triangles();
	unsigned int point_count();
	void set_head(Point &p1);
	void set_tail(Point &p1);
	Triangles get_map();
	void init_triangulation();
	void init_edges(PolyLine &polyline);
	Point* get_point(unsigned int index);
	void add_to_map(Triangle &triangle);
	Node* locate_node(const Point &point);
	void create_advancing_front();
	void remove_node(Node *node);
	void map_triangle_to_nodes(Triangle &t);
	void remove_from_map(const Triangle &triangle);
	void mesh_clean(Triangle &triangle);

	void sweep_points();
	Node* point_event(Point &point);
	void edge_event_by_edge(Edge &edge, Node &node);
	void edge_event_by_points(const Point &ep, const Point &eq, Triangle *triangle, Point &point);
	bool is_edge_side_of_triangle(Triangle &triangle, const Point &ep, const Point &eq);
	Node* new_front_triangle(Point &point, Node &node);
	void fill(Node &node);
	void fill_advancing_front(const Node &n);
	bool is_basin_angle_right(const Node &node);
	bool legalize(Triangle &t);
	bool in_circle(const Point &pa, const Point &pb, const Point &pc, const Point &pd);
	void rotate_triangle_pair(Triangle &t, Point &p, Triangle &ot, Point &op);
	void fill_basin(const Node &node);
	void fill_basin_req(Node &node);
	bool is_shallow(const Node &node);

	void fill_edge_event(const Edge &edge, Node &node);
	void fill_right_above_edge_event(const Edge &edge, Node *node);
	void fill_right_below_edge_event(const Edge &edge, const Node *node);
	void fill_right_concave_edge_event(const Edge &edge, const Node *node);
	void fill_right_convex_edge_event(const Edge &edge, const Node *node);
	void fill_left_above_edge_event(const Edge &edge, Node *node);
	void fill_left_below_edge_event(const Edge &edge, const Node *node);
	void fill_left_convex_edge_event(const Edge &edge, const Node *node);
	void fill_left_concave_edge_event(const Edge &edge, const Node *node);

	void flip_edge_event(const Point &ep, const Point &eq, Triangle *t, Point &p);
	Triangle* next_flip_triangle(const Orientation &o, Triangle &t, Triangle &ot, const Point &p, const Point &op);
	Point* next_flip_point(const Point &ep, const Point &eq, const Triangle &ot, const Point &op);
	void flip_scan_edge_event(const Point &ep, const Point &eq, const Triangle &flip_triangle, const Triangle &t, const Point &p);
	void finalization_polygon();

	Orientation orient2d(const Point &pa, const Point &pb, const Point &pc);
	bool is_angle_obtuse(const Point &pa, const Point &pb, const Point &pc);
	bool in_scan_area(const Point &pa, const Point &pb, const Point &pc, const Point &pd);

	void release_all();

private:
	std::vector<Edge*> new_edges;
	std::vector<Node*> new_nodes;
	std::vector<Triangle*> new_triangles;
};

#endif