#include "stdafx.h"
#include "CP_Triangle.h"
#include "SweepContext.h"
#include <map>

void CP_TriagleMesh::mb_buildTriagleMesh(CP_Polygon& pn, double tolerance)
{
	m_polygon = &pn;
	for (int i = 0; i < m_polygon->m_regionArray.size(); i++)
	{
		std::map<Point*, int> point_to_id;
		Points contour_points;
		Holes holes;

		CP_Region &region = m_polygon->m_regionArray[i];
		dense_offsets offsets = loose_region(region);
		CP_Loop &loop0 = region.m_loopArray[0];
		for (int k = 0; k < loop0.m_pointIDArray.size(); k++)
		{
			CP_Point &point0 = m_polygon->m_pointArray[loop0.m_pointIDArray[k]];
			double vx = 0;
			double vy = 0;
			if (offsets.find(std::make_pair(0, k)) != offsets.end())
			{
				vx = offsets[std::make_pair(0, k)].first;
				vy = offsets[std::make_pair(0, k)].second;
			}
			Point *npc = new Point(point0.m_x + vx, point0.m_y + vy);
			contour_points.push_back(npc);
			point_to_id[npc] = loop0.m_pointIDArray[k];
		}
		for (int j = 1; j < region.m_loopArray.size(); j++)
		{
			Hole hole;
			CP_Loop &loop = region.m_loopArray[j];
			for (int k = loop.m_pointIDArray.size()-1; k >= 0; k--)
			{
				CP_Point &point = m_polygon->m_pointArray[loop.m_pointIDArray[k]];
				double vx = 0;
				double vy = 0;
				if (offsets.find(std::make_pair(j, k)) != offsets.end())
				{
					vx = offsets[std::make_pair(j, k)].first;
					vy = offsets[std::make_pair(j, k)].second;
				}
				Point *np = new Point(point.m_x + vx, point.m_y + vy);
				hole.push_back(np);
				point_to_id[np] = loop.m_pointIDArray[k];
			}
			holes.push_back(hole);
		}

		SweepContext sweep(contour_points);
		sweep.add_holes(holes);
		sweep.triangulate();
		Triangles triangles = sweep.get_triangles();
		for (int i = 0; i < triangles.size(); i++)
		{
			Point *p1 = triangles[i]->points[0];
			Point *p2 = triangles[i]->points[1];
			Point *p3 = triangles[i]->points[2];
			int id1 = point_to_id[p1];
			int id2 = point_to_id[p2];
			int id3 = point_to_id[p3];
			CP_Point &cp1 = m_polygon->m_pointArray[id1];
			CP_Point &cp2 = m_polygon->m_pointArray[id2];
			CP_Point &cp3 = m_polygon->m_pointArray[id3];
			if (!tolerate_equal(cp1, cp2, tolerance) &&
				!tolerate_equal(cp2, cp3, tolerance) &&
				!tolerate_equal(cp1, cp3, tolerance))
			{
				CP_Triangle tr;
				tr.m_vertices[0] = id1;
				tr.m_vertices[1] = id2;
				tr.m_vertices[2] = id3;
				m_triagleIDArray.push_back(tr);
			}
		}
		sweep.release_all();
		std::map<Point*, int>::iterator it;
		for (it = point_to_id.begin(); it != point_to_id.end(); it++)
			delete it->first;
	}
}

bool tolerate_equal(CP_Point &p1, CP_Point &p2, double tolerance)
{
	double len_squared = (p1.m_x - p2.m_x)*(p1.m_x - p2.m_x) + (p1.m_y - p2.m_y)*(p1.m_y - p2.m_y);
	if (len_squared < tolerance * tolerance)
		return true;
	else
		return false;
}

dense_offsets loose_region(const CP_Region &region)
{
	dense_offsets offsets;
	std::map<std::pair<double, double>, std::pair<int, int> > rec;
	std::vector<std::pair<int, int> > dense_points;
	bool *clockawise = new bool[region.m_loopArray.size()];
	for (int i = 0; i < region.m_loopArray.size(); i++)
	{
		const CP_Loop &loop = region.m_loopArray[i];
		int tallest_id = 0;
		int count = loop.m_pointIDArray.size();

		for (int j = 0; j < count; j++)
		{
			CP_Point &point = region.m_polygon->m_pointArray[loop.m_pointIDArray[j]];
			if (rec.find(std::make_pair(point.m_x, point.m_y)) == rec.end())
				rec[std::make_pair(point.m_x, point.m_y)] = std::make_pair(i, j);
			else
				dense_points.push_back(std::make_pair(i, j));
			if (point.m_y > region.m_polygon->m_pointArray[loop.m_pointIDArray[tallest_id]].m_y)
				tallest_id = j;
		}

		CP_Point &forward = region.m_polygon->m_pointArray[loop.m_pointIDArray[(tallest_id + 1) % count]];
		CP_Point &point = loop.m_polygon->m_pointArray[loop.m_pointIDArray[tallest_id]];
		CP_Point &back = region.m_polygon->m_pointArray[loop.m_pointIDArray[(tallest_id - 1 + count) % count]];
		double v1x = back.m_x - point.m_x;
		double v1y = back.m_y - point.m_y;

		double v2x = forward.m_x - point.m_x;
		double v2y = forward.m_y - point.m_y;
		double cp = v1x * v2y - v1y * v2x;

		clockawise[i] = cp > 0;
	}
	for (int i = 0; i < dense_points.size(); i++)
	{
		int loop_id = dense_points[i].first;
		int vertex_id = dense_points[i].second;
		const CP_Loop &loop = region.m_loopArray[loop_id];
		int count = loop.m_pointIDArray.size();

		CP_Point &point = region.m_polygon->m_pointArray[loop.m_pointIDArray[vertex_id]];
		CP_Point &forward = region.m_polygon->m_pointArray[loop.m_pointIDArray[(vertex_id + 1)%count]];
		CP_Point &back = region.m_polygon->m_pointArray[loop.m_pointIDArray[(vertex_id - 1 + count)%count]];

		double v1x = point.m_x - back.m_x;
		double v1y = point.m_y - back.m_y;
		double v2x = forward.m_x - point.m_x;
		double v2y = forward.m_y - point.m_y;

		double dpx = v2x - v1x;
		double dpy = v2y - v1y;

		double l = sqrt(dpx*dpx + dpy*dpy);
		dpx = dpx / l * 0.01;
		dpy = dpy / l * 0.01;

		double cp = v1x * v2y - v2x * v1y;

		if (cp != 0 && clockawise[loop_id] ^ cp < 0)
		{
			dpx = -dpx;
			dpy = -dpy;
		}

		offsets[dense_points[i]] = std::make_pair(dpx, dpy);
	}
	delete[] clockawise;
	return offsets;
}