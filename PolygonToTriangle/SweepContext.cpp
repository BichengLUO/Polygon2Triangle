#include "stdafx.h"
#include "SweepContext.h"
#include <algorithm>

SweepContext* SweepContext::add_hole(PolyLine &polyline)
{
	init_edges(polyline);
	points.insert(points.end(), polyline.begin(), polyline.end());
	return this;
}

SweepContext* SweepContext::add_holes(Holes &holes)
{
	for (int i = 0; i < holes.size(); i++)
		init_edges(holes[i]);
	for (int i = 0; i < holes.size(); i++)
		points.insert(points.end(), holes[i].begin(), holes[i].end());
	return this;
}

SweepContext* SweepContext::add_point(Point &point)
{
	points.push_back(&point);
	return this;
}

SweepContext* SweepContext::add_points(const Points &ps)
{
	points.insert(points.end(), ps.begin(), ps.end());
	return this;
}

SweepContext* SweepContext::triangulate()
{
	init_triangulation();
	create_advancing_front();
	sweep_points();
	finalization_polygon();
	return this;
}

BoundingBox SweepContext::get_bounding_box()
{
	BoundingBox bb = {pmin, pmax};
	return bb;
}

Triangles SweepContext::get_triangles()
{
	return triangles;
}

unsigned int SweepContext::point_count()
{
	return points.size();
}

void SweepContext::set_head(Point &p1)
{
	head = p1;
}

void SweepContext::set_tail(Point &p1)
{
	tail = p1;
}

Triangles SweepContext::get_map()
{
	return map;
}

void SweepContext::init_triangulation()
{
	double xmax = points[0]->x;
	double xmin = points[0]->x;
	double ymax = points[0]->y;
	double ymin = points[0]->y;

	for (int i = 1; i < points.size(); i++) {
		Point *p = points[i];
		(p->x > xmax) && (xmax = p->x);
		(p->x < xmin) && (xmin = p->x);
		(p->y > ymax) && (ymax = p->y);
		(p->y < ymin) && (ymin = p->y);
	}
	pmin = Point(xmin, ymin);
	pmax = Point(xmax, ymax);

	double dx = kAlpha * (xmax - xmin);
	double dy = kAlpha * (ymax - ymin);
	head = Point(xmax + dx, ymin - dy);
	tail = Point(xmin - dx, ymin - dy);

	std::sort(points.begin(), points.end(), cmp_point_pointer);
}

void SweepContext::init_edges(PolyLine &polyline)
{
	unsigned int len = polyline.size();
	for (int i = 0; i < len; i++)
	{
		Edge *new_edge = new Edge(*polyline[i], *polyline[(i + 1) % len]);
		new_edges.push_back(new_edge);

		edge_list.push_back(new_edge);
	}
}

Point* SweepContext::get_point(unsigned int index)
{
	return points[index];
}

void SweepContext::add_to_map(Triangle &triangle)
{
	map.push_back(&triangle);
}

Node* SweepContext::locate_node(const Point &point)
{
	return front.locate_node(point.x);
}

void SweepContext::create_advancing_front()
{
	Triangle *triangle = new Triangle(*points[0], tail, head);
	new_triangles.push_back(triangle);

	map.push_back(triangle);

	Node *thead = new Node(*triangle->get_point(1), *triangle);
	Node *tmiddle = new Node(*triangle->get_point(0), *triangle);
	Node *ttail = new Node(*triangle->get_point(2));
	new_nodes.push_back(thead);
	new_nodes.push_back(tmiddle);
	new_nodes.push_back(ttail);

	front = AdvancingFront(thead, ttail);

	thead->next = tmiddle;
	tmiddle->next = ttail;
	tmiddle->prev = thead;
	ttail->prev = tmiddle;
}

void SweepContext::remove_node(Node *node)
{

}

void SweepContext::map_triangle_to_nodes(Triangle &t)
{
	for (int i = 0; i < 3; i++) {
		if (t.get_neighbor(i) == NULL) {
			Node *n = front.locate_point(*t.point_cw(*t.get_point(i)));
			if (n != NULL) {
				n->triangle = &t;
			}
		}
	}
}

void SweepContext::remove_from_map(const Triangle &triangle)
{
	for (int i = 0; i < map.size(); i++) {
		if (map[i] == &triangle) {
			delete map[i];
			map.erase(map.begin() + i);
			break;
		}
	}
}

void SweepContext::mesh_clean(Triangle &triangle)
{
	Triangles ttriangles;
	ttriangles.push_back(&triangle);
	while (ttriangles.size() > 0) {
		Triangle *t = ttriangles.back();
		ttriangles.pop_back();
		if (!t->is_interior()) {
			t->set_interior(true);
			triangles.push_back(t);
			for (int i = 0; i < 3; i++) {
				if (!t->constrained_edge[i]) {
					ttriangles.push_back(t->get_neighbor(i));
				}
			}
		}
	}
}

void SweepContext::sweep_points()
{
	for (int i = 1; i < point_count(); i++) {
		Point *point = get_point(i);
		Node *node = point_event(*point);
		std::vector<Edge*> *edges = &point->p2t_edge_list;
		for (int j = 0; edges->size() > 0 && j < edges->size(); j++) {
			edge_event_by_edge(*(*edges)[j], *node);
		}
	}
}

Node* SweepContext::point_event(Point &point)
{
	Node *node = locate_node(point);
	Node *new_node = new_front_triangle(point, *node);

	if (point.x <= node->point->x + EPSILON) {
		fill(*node);
	}

	fill_advancing_front(*new_node);
	return new_node;
}

void SweepContext::edge_event_by_edge(Edge &edge, Node &node)
{
	edge_event.constrained_edge = &edge;
	edge_event.right = edge.p->x > edge.q->x;

	if (is_edge_side_of_triangle(*node.triangle, *edge.p, *edge.q)) {
		return;
	}

	fill_edge_event(edge, node);
	edge_event_by_points(*edge.p, *edge.q, node.triangle, *edge.q);
}

void SweepContext::edge_event_by_points(const Point &ep, const Point &eq, Triangle *triangle, Point &point)
{
	if (is_edge_side_of_triangle(*triangle, ep, eq)) {
		return;
	}

	Point *p1 = triangle->point_ccw(point);
	Orientation o1 = orient2d(eq, *p1, ep);
	if (o1 == COLLINEAR) {
		return;
	}

	Point *p2 = triangle->point_cw(point);
	Orientation o2 = orient2d(eq, *p2, ep);
	if (o2 == COLLINEAR) {
		return;
	}

	if (o1 == o2) {
		if (o1 == CW) {
			triangle = triangle->neighbor_ccw(point);
		}
		else {
			triangle = triangle->neighbor_cw(point);
		}
		edge_event_by_points(ep, eq, triangle, point);
	}
	else {
		flip_edge_event(ep, eq, triangle, point);
	}
}

bool SweepContext::is_edge_side_of_triangle(Triangle &triangle, const Point &ep, const Point &eq)
{
	unsigned int index = triangle.edge_index(ep, eq);
	if (index != -1) {
		triangle.mark_constrained_edge_by_index(index);
		Triangle *t = triangle.get_neighbor(index);
		if (t != NULL) {
			t->mark_constrained_edge_by_points(ep, eq);
		}
		return true;
	}
	return false;
}

Node* SweepContext::new_front_triangle(Point &point, Node &node)
{
	Triangle *triangle = new Triangle(point, *node.point, *node.next->point);
	new_triangles.push_back(triangle);

	triangle->mark_neighbor(*node.triangle);
	add_to_map(*triangle);

	Node *new_node = new Node(point);
	new_nodes.push_back(new_node);

	new_node->next = node.next;
	new_node->prev = &node;
	node.next->prev = new_node;
	node.next = new_node;

	if (!legalize(*triangle)) {
		map_triangle_to_nodes(*triangle);
	}

	return new_node;
}

void SweepContext::fill(Node &node)
{
	Triangle *triangle = new Triangle(*node.prev->point, *node.point, *node.next->point);
	new_triangles.push_back(triangle);

	triangle->mark_neighbor(*node.prev->triangle);
	triangle->mark_neighbor(*node.triangle);

	add_to_map(*triangle);

	node.prev->next = node.next;
	node.next->prev = node.prev;

	if (!legalize(*triangle)) {
		map_triangle_to_nodes(*triangle);
	}
}

void SweepContext::fill_advancing_front(const Node &n)
{
	Node *node = n.next;
	while (node->next != NULL) {
		if (is_angle_obtuse(*node->point, *node->next->point, *node->prev->point)) {
			break;
		}
		fill(*node);
		node = node->next;
	}

	node = n.prev;
	while (node->prev != NULL) {
		if (is_angle_obtuse(*node->point, *node->next->point, *node->prev->point)) {
			break;
		}
		fill(*node);
		node = node->prev;
	}

	if (n.next != NULL && n.next->next != NULL) {
		if (is_basin_angle_right(n)) {
			fill_basin(n);
		}
	}
}

bool SweepContext::is_basin_angle_right(const Node &node)
{
	double ax = node.point->x - node.next->next->point->x;
	double ay = node.point->y - node.next->next->point->y;
	return (ax >= 0 || abs(ax) < ay);
}

bool SweepContext::legalize(Triangle &t)
{
	for (int i = 0; i < 3; ++i) {
		if (t.delaunay_edge[i]) {
			continue;
		}
		Triangle *ot = t.get_neighbor(i);
		if (ot != NULL) {
			Point *p = t.get_point(i);
			Point *op = ot->opposite_point(t, *p);
			unsigned int oi = ot->index(*op);

			if (ot->constrained_edge[oi] || ot->delaunay_edge[oi]) {
				t.constrained_edge[i] = ot->constrained_edge[oi];
				continue;
			}

			bool inside = in_circle(*p, *t.point_ccw(*p), *t.point_cw(*p), *op);
			if (inside) {
				t.delaunay_edge[i] = true;
				ot->delaunay_edge[oi] = true;

				rotate_triangle_pair(t, *p, *ot, *op);

				bool not_legalized = !legalize(t);
				if (not_legalized) {
					map_triangle_to_nodes(t);
				}

				not_legalized = !legalize(*ot);
				if (not_legalized) {
					map_triangle_to_nodes(*ot);
				}
				t.delaunay_edge[i] = false;
				ot->delaunay_edge[oi] = false;

				return true;
			}
		}
	}
	return false;
}

bool SweepContext::in_circle(const Point &pa, const Point &pb, const Point &pc, const Point &pd)
{
	double adx = pa.x - pd.x;
	double ady = pa.y - pd.y;
	double bdx = pb.x - pd.x;
	double bdy = pb.y - pd.y;

	double adxbdy = adx * bdy;
	double bdxady = bdx * ady;
	double oabd = adxbdy - bdxady;
	if (oabd <= 0) {
		return false;
	}

	double cdx = pc.x - pd.x;
	double cdy = pc.y - pd.y;

	double cdxady = cdx * ady;
	double adxcdy = adx * cdy;
	double ocad = cdxady - adxcdy;
	if (ocad <= 0) {
		return false;
	}

	double bdxcdy = bdx * cdy;
	double cdxbdy = cdx * bdy;

	double alift = adx * adx + ady * ady;
	double blift = bdx * bdx + bdy * bdy;
	double clift = cdx * cdx + cdy * cdy;

	double det = alift * (bdxcdy - cdxbdy) + blift * ocad + clift * oabd;
	return det > 0;
}

void SweepContext::rotate_triangle_pair(Triangle &t, Point &p, Triangle &ot, Point &op)
{
	Triangle *n1, *n2, *n3, *n4;
	n1 = t.neighbor_ccw(p);
	n2 = t.neighbor_cw(p);
	n3 = ot.neighbor_ccw(op);
	n4 = ot.neighbor_cw(op);

	bool ce1, ce2, ce3, ce4;
	ce1 = t.get_constrained_edge_ccw(p);
	ce2 = t.get_constrained_edge_cw(p);
	ce3 = ot.get_constrained_edge_ccw(op);
	ce4 = ot.get_constrained_edge_cw(op);

	bool de1, de2, de3, de4;
	de1 = t.get_delaunay_edge_ccw(p);
	de2 = t.get_delaunay_edge_cw(p);
	de3 = ot.get_delaunay_edge_ccw(op);
	de4 = ot.get_delaunay_edge_cw(op);

	t.legalize(p, op);
	ot.legalize(op, p);

	ot.set_delaunay_edge_ccw(p, de1);
	t.set_delaunay_edge_cw(p, de2);
	t.set_delaunay_edge_ccw(op, de3);
	ot.set_delaunay_edge_cw(op, de4);

	ot.set_constrained_edge_ccw(p, ce1);
	t.set_constrained_edge_cw(p, ce2);
	t.set_constrained_edge_ccw(op, ce3);
	ot.set_constrained_edge_cw(op, ce4);

	t.clear_neighbors();
	ot.clear_neighbors();
	if (n1) {
		ot.mark_neighbor(*n1);
	}
	if (n2) {
		t.mark_neighbor(*n2);
	}
	if (n3) {
		t.mark_neighbor(*n3);
	}
	if (n4) {
		ot.mark_neighbor(*n4);
	}
	t.mark_neighbor(ot);
}

void SweepContext::fill_basin(const Node &node)
{
	if (orient2d(*node.point, *node.next->point, *node.next->next->point) == CCW) {
		basin.left_node = node.next->next;
	}
	else {
		basin.left_node = node.next;
	}

	basin.bottom_node = basin.left_node;
	while (basin.bottom_node->next != NULL && basin.bottom_node->point->y >= basin.bottom_node->next->point->y) {
		basin.bottom_node = basin.bottom_node->next;
	}
	if (basin.bottom_node == basin.left_node) {
		return;
	}

	basin.right_node = basin.bottom_node;
	while (basin.right_node->next != NULL && basin.right_node->point->y < basin.right_node->next->point->y) {
		basin.right_node = basin.right_node->next;
	}
	if (basin.right_node == basin.bottom_node) {
		return;
	}

	basin.width = basin.right_node->point->x - basin.left_node->point->x;
	basin.left_highest = basin.left_node->point->y > basin.right_node->point->y;

	fill_basin_req(*basin.bottom_node);
}

void SweepContext::fill_basin_req(Node &node)
{
	if (is_shallow(node)) {
		return;
	}

	fill(node);

	Orientation o;
	if (node.prev == basin.left_node && node.next == basin.right_node) {
		return;
	}
	else if (node.prev == basin.left_node) {
		o = orient2d(*node.point, *node.next->point, *node.next->next->point);
		if (o == CW) {
			return;
		}
		node = *node.next;
	}
	else if (node.next == basin.right_node) {
		o = orient2d(*node.point, *node.prev->point, *node.prev->prev->point);
		if (o == CCW) {
			return;
		}
		node = *node.prev;
	}
	else {
		if (node.prev->point->y < node.next->point->y) {
			node = *node.prev;
		}
		else {
			node = *node.next;
		}
	}

	fill_basin_req(node);
}

bool SweepContext::is_shallow(const Node &node)
{
	double height;
	if (basin.left_highest) {
		height = basin.left_node->point->y - node.point->y;
	}
	else {
		height = basin.right_node->point->y - node.point->y;
	}

	if (basin.width > height) {
		return true;
	}
	return false;
}

void SweepContext::fill_edge_event(const Edge &edge, Node &node)
{
	if (edge_event.right) {
		fill_right_above_edge_event(edge, &node);
	}
	else {
		fill_left_above_edge_event(edge, &node);
	}
}

void SweepContext::fill_right_above_edge_event(const Edge &edge, Node *node)
{
	while (node->next->point->x < edge.p->x) {
		if (orient2d(*edge.q, *node->next->point, *edge.p) == CCW) {
			fill_right_below_edge_event(edge, node);
		}
		else {
			node = node->next;
		}
	}
}

void SweepContext::fill_right_below_edge_event(const Edge &edge, const Node *node)
{
	if (node->point->x < edge.p->x) {
		if (orient2d(*node->point, *node->next->point, *node->next->next->point) == CCW) {
			fill_right_concave_edge_event(edge, node);
		}
		else {
			fill_right_convex_edge_event(edge, node);
			fill_right_below_edge_event(edge, node);
		}
	}
}

void SweepContext::fill_right_concave_edge_event(const Edge &edge, const Node *node)
{
	fill(*node->next);
	if (node->next->point != edge.p) {
		if (orient2d(*edge.q, *node->next->point, *edge.p) == CCW) {
			if (orient2d(*node->point, *node->next->point, *node->next->next->point) == CCW) {
				fill_right_concave_edge_event(edge, node);
			}
			else {
			}
		}
	}
}

void SweepContext::fill_right_convex_edge_event(const Edge &edge, const Node *node)
{
	if (orient2d(*node->next->point, *node->next->next->point, *node->next->next->next->point) == CCW) {
		fill_right_concave_edge_event(edge, node->next);
	}
	else {
		if (orient2d(*edge.q, *node->next->next->point, *edge.p) == CCW) {
			fill_right_convex_edge_event(edge, node->next);
		}
		else {
		}
	}
}

void SweepContext::fill_left_above_edge_event(const Edge &edge, Node *node)
{
	while (node->prev->point->x > edge.p->x) {
		if (orient2d(*edge.q, *node->prev->point, *edge.p) == CW) {
			fill_left_below_edge_event(edge, node);
		}
		else {
			node = node->prev;
		}
	}
}

void SweepContext::fill_left_below_edge_event(const Edge &edge, const Node *node)
{
	if (node->point->x > edge.p->x) {
		if (orient2d(*node->point, *node->prev->point, *node->prev->prev->point) == CW) {
			fill_left_concave_edge_event(edge, node);
		}
		else {
			fill_left_convex_edge_event(edge, node);
			fill_left_below_edge_event(edge, node);
		}
	}
}

void SweepContext::fill_left_convex_edge_event(const Edge &edge, const Node *node)
{
	if (orient2d(*node->prev->point, *node->prev->prev->point, *node->prev->prev->prev->point) == CW) {
		fill_left_concave_edge_event(edge, node->prev);
	}
	else {
		if (orient2d(*edge.q, *node->prev->prev->point, *edge.p) == CW) {
			fill_left_convex_edge_event(edge, node->prev);
		}
		else {
		}
	}
}

void SweepContext::fill_left_concave_edge_event(const Edge &edge, const Node *node)
{
	fill(*node->prev);
	if (node->prev->point != edge.p) {
		if (orient2d(*edge.q, *node->prev->point, *edge.p) == CW) {
			if (orient2d(*node->point, *node->prev->point, *node->prev->prev->point) == CW) {
				fill_left_concave_edge_event(edge, node);
			}
			else {
			}
		}
	}
}

void SweepContext::flip_edge_event(const Point &ep, const Point &eq, Triangle *t, Point &p)
{
	Triangle *ot = t->neighbor_across(p);

	Point *op = ot->opposite_point(*t, p);

	if (t->get_constrained_edge_across(p)) {
		unsigned int index = t->index(p);
		return;
	}

	if (in_scan_area(p, *t->point_ccw(p), *t->point_cw(p), *op)) {

		rotate_triangle_pair(*t, p, *ot, *op);
		map_triangle_to_nodes(*t);
		map_triangle_to_nodes(*ot);

		if (p == eq && *op == ep) {
			if (eq == *edge_event.constrained_edge->q && ep == *edge_event.constrained_edge->p) {
				t->mark_constrained_edge_by_points(ep, eq);
				ot->mark_constrained_edge_by_points(ep, eq);
				legalize(*t);
				legalize(*ot);
			}
			else {
				
			}
		}
		else {
			Orientation o = orient2d(eq, *op, ep);
			t = next_flip_triangle(o, *t, *ot, p, *op);
			flip_edge_event(ep, eq, t, p);
		}
	}
	else {
		Point *newP = next_flip_point(ep, eq, *ot, *op);
		flip_scan_edge_event(ep, eq, *t, *ot, *newP);
		edge_event_by_points(ep, eq, t, p);
	}
}

Triangle* SweepContext::next_flip_triangle(const Orientation &o, Triangle &t, Triangle &ot, const Point &p, const Point &op)
{
	unsigned int edge_index;
	if (o == CCW) {
		edge_index = ot.edge_index(p, op);
		ot.delaunay_edge[edge_index] = true;
		legalize(ot);
		ot.clear_delaunay_edges();
		return &t;
	}

	edge_index = t.edge_index(p, op);

	t.delaunay_edge[edge_index] = true;
	legalize(t);
	t.clear_delaunay_edges();
	return &ot;
}

Point* SweepContext::next_flip_point(const Point &ep, const Point &eq, const Triangle &ot, const Point &op)
{
	Orientation o2d = orient2d(eq, op, ep);
	if (o2d == CW) {
		return ot.point_ccw(op);
	}
	else if (o2d == CCW) {
		return ot.point_cw(op);
	}
}

void SweepContext::flip_scan_edge_event(const Point &ep, const Point &eq, const Triangle &flip_triangle, const Triangle &t, const Point &p)
{
	Triangle *ot = t.neighbor_across(p);

	Point *op = ot->opposite_point(t, p);

	if (in_scan_area(eq, *flip_triangle.point_ccw(eq), *flip_triangle.point_cw(eq), *op)) {
		flip_edge_event(eq, *op, ot, *op);
	}
	else {
		Point *newP = next_flip_point(ep, eq, *ot, *op);
		flip_scan_edge_event(ep, eq, flip_triangle, *ot, *newP);
	}
}

void SweepContext::finalization_polygon()
{
	Triangle *t = front.head->next->triangle;
	Point *p = front.head->next->point;
	while (!t->get_constrained_edge_cw(*p)) {
		t = t->neighbor_ccw(*p);
	}

	mesh_clean(*t);
}

Orientation SweepContext::orient2d(const Point &pa, const Point &pb, const Point &pc)
{
	double detleft = (pa.x - pc.x) * (pb.y - pc.y);
	double detright = (pa.y - pc.y) * (pb.x - pc.x);
	double val = detleft - detright;
	if (val > -EPSILON && val < EPSILON) {
		return COLLINEAR;
	}
	else if (val > 0) {
		return CCW;
	}
	else {
		return CW;
	}
}

bool SweepContext::is_angle_obtuse(const Point &pa, const Point &pb, const Point &pc)
{
	double ax = pb.x - pa.x;
	double ay = pb.y - pa.y;
	double bx = pc.x - pa.x;
	double by = pc.y - pa.y;
	return (ax * bx + ay * by) < 0;
}

bool SweepContext::in_scan_area(const Point &pa, const Point &pb, const Point &pc, const Point &pd)
{
	double oadb = (pa.x - pb.x) * (pd.y - pb.y) - (pd.x - pb.x) * (pa.y - pb.y);
	if (oadb >= -EPSILON) {
		return false;
	}

	double oadc = (pa.x - pc.x) * (pd.y - pc.y) - (pd.x - pc.x) * (pa.y - pc.y);
	if (oadc <= EPSILON) {
		return false;
	}
	return true;
}

void SweepContext::release_all()
{
	for (int i = 0; i < new_edges.size(); i++)
		delete new_edges[i];
	for (int i = 0; i < new_nodes.size(); i++)
		delete new_nodes[i];
	for (int i = 0; i < new_triangles.size(); i++)
		delete new_triangles[i];
}