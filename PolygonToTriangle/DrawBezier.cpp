#include "stdafx.h"
#include "DrawBezier.h"

void draw_bezier(CDC* pDC, CP_Loop &loop, double scale, CP_Point translation, int screenX, int screenY,
	int r, int g, int b, int size)
{
	int segments = 100;
	int n = loop.m_pointIDArray.size();
	CP_Point *point_curve = new CP_Point[segments + 1];
	CP_Point *points = new CP_Point[n];
	for (int i = 0; i <= segments; i++)
	{
		for (int i = 0; i < n; i++)
			points[i] = loop.m_polygon->m_pointArray[loop.m_pointIDArray[i]];
		double t = i / (double)segments;
		for (int j = 1; j <= n - 1; j++)
		{
			for (int k = 0; k < n - j; k++)
			{
				CP_Point r;
				r.m_x = (1 - t) * points[k].m_x + t * points[k + 1].m_x;
				r.m_y = (1 - t) * points[k].m_y + t * points[k + 1].m_y;
				points[k] = r;
			}
		}
		point_curve[i] = points[0];
	}
	delete[] points;
	CPen pen(0, size, RGB(r, g, b));
	CPen * penOld = (CPen *)pDC->SelectObject(&pen);
	CP_Point &pg = point_curve[0];
	CP_Point ps;
	gb_pointConvertFromGlobalToScreen(ps, pg, scale, translation, screenX, screenY);
	pDC->MoveTo((int)(ps.m_x + 0.5), (int)(ps.m_y + 0.5));
	for (int i = 1; i <= segments; i++)
	{
		pg = point_curve[i];
		gb_pointConvertFromGlobalToScreen(ps, pg, scale, translation, screenX, screenY);
		pDC->LineTo((int)(ps.m_x + 0.5), (int)(ps.m_y + 0.5));
	}
	pDC->SelectObject(penOld);
	delete[] point_curve;
}