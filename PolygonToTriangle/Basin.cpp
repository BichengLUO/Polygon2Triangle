#include "stdafx.h"
#include "Basin.h"

void Basin::clear()
{
	left_node = NULL;
	bottom_node = NULL;
	right_node = NULL;
	width = 0.0;
	left_highest = false;
}