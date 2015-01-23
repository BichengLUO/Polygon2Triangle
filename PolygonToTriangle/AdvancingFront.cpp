#include "stdafx.h"
#include "AdvancingFront.h"

void AdvancingFront::set_head(Node *node)
{
	head = node;
}

void AdvancingFront::set_tail(Node *node)
{
	tail = node;
}

void AdvancingFront::set_search(Node *node)
{
	search_node = node;
}

Node* AdvancingFront::search() const
{
	return search_node;
}

Node* AdvancingFront::find_search_node(double x) const
{
	return search_node;
}

Node* AdvancingFront::locate_node(double x)
{
	Node *node = search_node;

	if (x < node->value) {
		while (node = node->prev) {
			if (x >= node->value) {
				search_node = node;
				return node;
			}
		}
	}
	else {
		while (node = node->next) {
			if (x < node->value) {
				search_node = node->prev;
				return node->prev;
			}
		}
	}
	return NULL;
}

Node* AdvancingFront::locate_point(const Point &point)
{
	double px = point.x;
	Node* node = find_search_node(px);
	double nx = node->point->x;

	if (px == nx) {
		if (&point != node->point) {
			if (&point == node->prev->point) {
				node = node->prev;
			}
			else if (&point == node->next->point) {
				node = node->next;
			}
		}
	}
	else if (px < nx) {
		while (node = node->prev) {
			if (&point == node->point) {
				break;
			}
		}
	}
	else {
		while (node = node->next) {
			if (&point == node->point) {
				break;
			}
		}
	}

	if (node) {
		search_node = node;
	}
	return node;
}