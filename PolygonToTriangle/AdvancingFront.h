#ifndef _ADVANCING_FRONT
#define _ADVANCING_FRONT

#include "Node.h"

class AdvancingFront
{
public:
	Node *head;
	Node *tail;
	Node *search_node;

	AdvancingFront() : head(NULL), tail(NULL), search_node(NULL) {}
	AdvancingFront(Node *thead, Node *ttail) : head(thead), tail(ttail), search_node(thead) {}

	void set_head(Node *node);
	void set_tail(Node *node);
	void set_search(Node *node);

	Node* search() const;
	Node* find_search_node(double x) const;
	Node* locate_node(double x);
	Node* locate_point(const Point &point);
};

#endif