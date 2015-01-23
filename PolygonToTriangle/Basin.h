#ifndef _BASIN
#define _BASIN

#include "Node.h"

class Basin
{
public:
	Node *left_node;
	Node *bottom_node;
	Node *right_node;
	double width;
	bool left_highest;
	
	Basin() : left_node(NULL), bottom_node(NULL), right_node(NULL), width(0), left_highest(false) {}
	void clear();
};

#endif