#ifndef DRAW_BEZIER_H
#define DRAW_BEZIER_H

#include "CP_Polygon.h"

void draw_bezier(CDC* pDC, CP_Loop &loop, double scale, CP_Point translation, int screenX, int screenY,
	int r, int g, int b, int size);

#endif