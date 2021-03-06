#include "stdafx.h"
#include <set>
#include "CP_Polygon.h"
#include "SimplePolygon.h"

#define AWAY_ANGLE 1
#define EMBRACE_ANGLE 2
#define CROSS_ANGLE 3

bool CP_Loop::check()
{
	int size = m_pointIDArray.size();
	if (size == 1 || size == 2)
		return false;
	SPolygon polygon(size);
	std::set<std::pair<double, double> > rec;
	for (int i = 0; i < size; i++)
	{
		CP_Point &point = m_polygon->m_pointArray[m_pointIDArray[i]];
		polygon.V[i].x = point.m_x;
		polygon.V[i].y = point.m_y;

		if (rec.find(std::make_pair(point.m_x, point.m_y)) == rec.end())
			rec.insert(std::make_pair(point.m_x, point.m_y));
		else
			return false;
	}
	return simple_polygon(polygon);
}

bool CP_Region::check()
{
	int size = m_loopArray.size();
	SPolygon *polygons = new SPolygon[size];
	for (int i = 0; i < size; i++)
	{
		polygons[i].n = m_loopArray[i].m_pointIDArray.size();
		polygons[i].V = (SPoint*)malloc(polygons[i].n * sizeof(SPoint));
		for (int j = 0; j < polygons[i].n; j++)
		{
			CP_Point &point = m_polygon->m_pointArray[m_loopArray[i].m_pointIDArray[j]];
			polygons[i].V[j].x = point.m_x;
			polygons[i].V[j].y = point.m_y;
		}
	}
	bool result = simple_polygons(polygons, size);
	delete[] polygons;
	if (!result) return false;
	for (int i = 0; i < size; i++)
	{
		if (!m_loopArray[i].check())
			return false;
		if (i != 0)
		{
			CP_Loop &loop = m_loopArray[i];
			if (loop_overlap_edge(loop, m_loopArray[0]))
				return false;
			if (!loop_in_loop(loop, m_loopArray[0]))
				return false;
		}
	}
	for (int i = 1; i < size; i++)
	{
		CP_Loop &loop1 = m_loopArray[i];
		for (int j = i + 1; j <size; j++)
		{
			CP_Loop &loop2 = m_loopArray[j];
			if (loop_overlap_edge(loop1, loop2))
				return false;
			if (!loop_away_loop(loop1, loop2))
				return false;
		}
	}
	return true;
}

bool CP_Polygon::check()
{
	for (int i = 0; i < m_regionArray.size(); i++)
	{
		if (!m_regionArray[i].check())
			return false;
		for (int j = i + 1; j < m_regionArray.size(); j++)
		{
			CP_Loop &li = m_regionArray[i].m_loopArray[0];
			CP_Loop &lj = m_regionArray[j].m_loopArray[0];
			if (loop_overlap_edge(li, lj))
				return false;
			if (!loop_away_loop(li, lj))
			{
				bool i_in_j = false;
				bool j_in_i = false;
				for (int k = 1; k < m_regionArray[j].m_loopArray.size(); k++)
				{
					CP_Loop &in_lj = m_regionArray[j].m_loopArray[k];
					if (loop_overlap_edge(li, in_lj))
						return false;
					if (loop_in_loop(li, in_lj))
					{
						i_in_j = true;
						break;
					}
				}
				for (int k = 1; k < m_regionArray[i].m_loopArray.size(); k++)
				{
					CP_Loop &in_li = m_regionArray[i].m_loopArray[k];
					if (loop_overlap_edge(in_li, lj))
						return false;
					if (loop_in_loop(lj, in_li))
					{
						j_in_i = true;
						break;
					}
				}
				if (!(i_in_j || j_in_i))
					return false;
			}
		}
	}
	return true;
}

bool loop_away_loop(CP_Loop &p1, CP_Loop &p2)
{
	int count1 = p1.m_pointIDArray.size();
	for (int i = 0; i < count1; i++)
	{
		if (point_in_loop(p2, i, AWAY_ANGLE, false, false, p1))
			return false;
	}
	int count2 = p2.m_pointIDArray.size();
	for (int i = 0; i < count2; i++)
	{
		if (point_in_loop(p1, i, AWAY_ANGLE, false, false, p2))
			return false;
	}
	return true;
}

bool loop_in_loop(CP_Loop &in, CP_Loop &out)
{
	int count = in.m_pointIDArray.size();
	for (int i = 0; i < count; i++)
	{
		if (!point_in_loop(out, i, EMBRACE_ANGLE, true, true, in))
			return false;
	}
	count = out.m_pointIDArray.size();
	for (int i = 0; i < count; i++)
	{
		if (point_in_loop(in, i, EMBRACE_ANGLE, false, false, out))
			return false;
	}
	return true;
}

bool loop_overlap_edge(CP_Loop &in, CP_Loop &out)
{
	int count1 = in.m_pointIDArray.size();
	int count2 = out.m_pointIDArray.size();
	for (int i = 0; i < count1; i++)
	{
		CP_Point &p1 = in.m_polygon->m_pointArray[in.m_pointIDArray[i]];
		for (int j = 0; j < count2; j++)
		{
			CP_Point &p2 = out.m_polygon->m_pointArray[out.m_pointIDArray[j]];
			if ((p1.m_x == p2.m_x && p1.m_y == p2.m_y))
			{
				CP_Point &fp1 = in.m_polygon->m_pointArray[in.m_pointIDArray[(i + 1)%count1]];
				CP_Point &fp2 = out.m_polygon->m_pointArray[out.m_pointIDArray[(j + 1)%count2]];

				CP_Point &bp1 = in.m_polygon->m_pointArray[in.m_pointIDArray[(i - 1 + count1)%count1]];
				CP_Point &bp2 = out.m_polygon->m_pointArray[out.m_pointIDArray[(j - 1 + count2)%count2]];

				if ((fp1.m_x == fp2.m_x && fp1.m_y == fp2.m_y) ||
					(bp1.m_x == bp2.m_x && bp1.m_y == bp2.m_y) ||
					(fp1.m_x == bp2.m_x && fp1.m_y == bp2.m_y) ||
					(bp1.m_x == fp2.m_x && bp1.m_y == fp2.m_y))
					return true;
			}
		}
	}
	return false;
}

bool point_in_loop(CP_Loop &loop, int p, int point_overlap_type, bool point_overlap, bool edge_overlap, CP_Loop &p_loop)
{
	int count = loop.m_pointIDArray.size();
	CP_Point &point = p_loop.m_polygon->m_pointArray[p_loop.m_pointIDArray[p]];
	double x = point.m_x;
	double y = point.m_y;
	int i, j = count - 1;
	bool oddNodes = false;
	for (i = 0; i < count; i++)
	{
		CP_Point &pi = loop.m_polygon->m_pointArray[loop.m_pointIDArray[i]];
		CP_Point &pj = loop.m_polygon->m_pointArray[loop.m_pointIDArray[j]];
		int pr = 0;
		if (point.m_x == pi.m_x && point.m_y == pi.m_y)
			pr = loops_overlap_at_point(loop, p_loop, i, p);
		if (point.m_x == pj.m_x && point.m_y == pj.m_y)
			pr = loops_overlap_at_point(loop, p_loop, j, p);
		if (pr == point_overlap_type)
			return point_overlap;
		else if (pr != 0)
			return !point_overlap;
		double cp = (point.m_x - pi.m_x)*(pj.m_y - pi.m_y) - (point.m_y - pi.m_y)*(pj.m_x - pi.m_x);
		if (cp == 0 &&
			((point.m_x >= pi.m_x && point.m_x <= pj.m_x) || (point.m_x >= pj.m_x && point.m_x <= pi.m_x)) &&
			((point.m_y >= pi.m_y && point.m_y <= pj.m_y) || (point.m_y >= pj.m_y && point.m_y <= pi.m_y)))
			return !edge_overlap;
		if ((pi.m_y < y && pj.m_y >= y || pj.m_y < y && pi.m_y >= y)
			&& (pi.m_x <= x || pj.m_x <= x))
		{
			if (pi.m_x + (y - pi.m_y) / (pj.m_y - pi.m_y) * (pj.m_x - pi.m_x) < x)
			{
				oddNodes = !oddNodes;
			}
		}
		j = i;
	}

	return oddNodes;
}

int loops_overlap_at_point(CP_Loop &loop1, CP_Loop &loop2, int i1, int i2)
{
	bool clockawise1 = is_loop_clockawise(loop1);
	bool clockawise2 = is_loop_clockawise(loop2);

	int count1 = loop1.m_pointIDArray.size();
	int count2 = loop2.m_pointIDArray.size();

	CP_Point p1 = loop1.m_polygon->m_pointArray[loop1.m_pointIDArray[i1]];
	CP_Point p2 = loop2.m_polygon->m_pointArray[loop2.m_pointIDArray[i2]];

	CP_Point &fp1 = loop1.m_polygon->m_pointArray[loop1.m_pointIDArray[(i1 + 1) % count1]];
	CP_Point &fp2 = loop2.m_polygon->m_pointArray[loop2.m_pointIDArray[(i2 + 1) % count2]];

	CP_Point &bp1 = loop1.m_polygon->m_pointArray[loop1.m_pointIDArray[(i1 - 1 + count1) % count1]];
	CP_Point &bp2 = loop2.m_polygon->m_pointArray[loop2.m_pointIDArray[(i2 - 1 + count2) % count2]];

	double v1x = bp1.m_x - p1.m_x;
	double v1y = bp1.m_y - p1.m_y;

	double v2x = fp1.m_x - p1.m_x;
	double v2y = fp1.m_y - p1.m_y;

	double v3x = bp2.m_x - p2.m_x;
	double v3y = bp2.m_y - p2.m_y;

	double v4x = fp2.m_x - p2.m_x;
	double v4y = fp2.m_y - p2.m_y;

	bool can1 = false;
	bool can2 = false;
	if (clockawise1)
	{
		if (is_vector_in_range(v3x, v3y, v2x, v2y, v1x, v1y) &&
			is_vector_in_range(v4x, v4y, v2x, v2y, v1x, v1y))
			can1 = true;
	}
	else
	{
		if (is_vector_in_range(v3x, v3y, v1x, v1y, v2x, v2y) &&
			is_vector_in_range(v4x, v4y, v1x, v1y, v2x, v2y))
			can1 = true;
	}
	if (clockawise2)
	{
		if (is_vector_in_range(v1x, v1y, v4x, v4y, v3x, v3y) &&
			is_vector_in_range(v2x, v2y, v4x, v4y, v3x, v3y))
			can2 = true;
	}
	else
	{
		if (is_vector_in_range(v1x, v1y, v3x, v3y, v4x, v4y) &&
			is_vector_in_range(v2x, v2y, v3x, v3y, v4x, v4y))
			can2 = true;
	}
	if (can1 && can2)
		return AWAY_ANGLE;
	else if (can1 || can2)
		return EMBRACE_ANGLE;
	else
		return CROSS_ANGLE;
}

bool is_loop_clockawise(CP_Loop &loop)
{
	int count = loop.m_pointIDArray.size();
	int tallest_id = 0;
	for (int i = 0; i < count; i++)
	{
		CP_Point &point = loop.m_polygon->m_pointArray[loop.m_pointIDArray[i]];
		if (point.m_y > loop.m_polygon->m_pointArray[loop.m_pointIDArray[tallest_id]].m_y)
			tallest_id = i;
	}
	CP_Point &forward = loop.m_polygon->m_pointArray[loop.m_pointIDArray[(tallest_id + 1) % count]];
	CP_Point &point = loop.m_polygon->m_pointArray[loop.m_pointIDArray[tallest_id]];
	CP_Point &back = loop.m_polygon->m_pointArray[loop.m_pointIDArray[(tallest_id - 1 + count) % count]];
	double v1x = back.m_x - point.m_x;
	double v1y = back.m_y - point.m_y;

	double v2x = forward.m_x - point.m_x;
	double v2y = forward.m_y - point.m_y;

	double cp = v1x * v2y - v1y * v2x;
	return cp > 0;
}

bool is_vector_in_range(double vx, double vy, double from_x, double from_y, double to_x, double to_y)
{
	double vf_vt = from_x * to_y - to_x * from_y;
	double vf_v = from_x * vy - vx * from_y;
	double v_vt = vx * to_y - to_x * vy;
	if (vf_vt > 0 && vf_v > 0 && v_vt > 0)
		return true;
	if (vf_vt < 0 && (vf_v > 0 || v_vt > 0))
		return true;
	return false;
}

void gb_distanceMinPointLoop(double&d, int& idRegion, int& idLoop,
	CP_Point& pt, CP_Polygon& pn)
{
	d = 0.0;
	idRegion = -1;
	idLoop = -1;
	int nr = pn.m_regionArray.size();
	int i, j, k, nl, nv, v1, v2;
	double dt;
	for (i = 0; i<nr; i++)
	{
		nl = pn.m_regionArray[i].m_loopArray.size();
		for (j = 0; j<nl; j++)
		{
			nv = pn.m_regionArray[i].m_loopArray[j].m_pointIDArray.size();
			for (k = 0; k<nv; k++)
			{
				v1 = pn.m_regionArray[i].m_loopArray[j].m_pointIDArray[k];
				if (k == nv - 1)
					v2 = pn.m_regionArray[i].m_loopArray[j].m_pointIDArray[0];
				else v2 = pn.m_regionArray[i].m_loopArray[j].m_pointIDArray[k + 1];
				dt = gb_distancePointSegment(pt, pn.m_pointArray[v1], pn.m_pointArray[v2]);
				if ((idLoop == -1) || (d>dt))
				{
					d = dt;
					idRegion = i;
					idLoop = j;
				}
			}
		}
	}
}

void gb_distanceMinPointPolygon(double&d, int& id, CP_Point& pt, CP_Polygon& pn)
{
	d = 0.0;
	id = -1;
	int n = pn.m_pointArray.size();
	if (n <= 0)
		return;
	d = gb_distancePointPoint(pt, pn.m_pointArray[0]);
	id = 0;
	int i;
	double dt;
	for (i = 1; i<n; i++)
	{
		dt = gb_distancePointPoint(pt, pn.m_pointArray[i]);
		if (dt<d)
		{
			d = dt;
			id = i;
		}
	}
}

double gb_distancePointPoint(CP_Point& p1, CP_Point& p2)
{
	double dx = p1.m_x - p2.m_x;
	double dy = p1.m_y - p2.m_y;
	double d2 = dx*dx + dy*dy;
	double d = sqrt(d2);
	return d;
}

double gb_distancePointSegment(CP_Point& pt, CP_Point& p1, CP_Point& p2)
{
	double dx0 = p2.m_x - p1.m_x;
	double dy0 = p2.m_y - p1.m_y;
	double dx1 = pt.m_x - p1.m_x;
	double dy1 = pt.m_y - p1.m_y;
	double dx2 = pt.m_x - p2.m_x;
	double dy2 = pt.m_y - p2.m_y;
	double d1 = dx1*dx1 + dy1*dy1;
	double d2 = dx2*dx2 + dy2*dy2;
	double d01 = dx1*dx0 + dy1*dy0;
	double d02 = -dx2*dx0 - dy2*dy0;
	double d, d0;
	if ((d01>0) && (d02>0))
	{
		d0 = dx0*dx0 + dy0*dy0;
		d = d01*d01 / d0;
		d = d1 - d;
		d = sqrt(d);
		return d;
	}
	if (d1>d2)
		d = d2;
	else d = d1;
	d = sqrt(d);
	return d;
}

void gb_getIntArrayPointInPolygon(VT_IntArray& vi, CP_Polygon& pn, CP_Point& p, double eT)
{
	int i, n;
	double d;
	n = pn.m_pointArray.size();
	for (i = 0; i<n; i++)
	{
		d = gb_distancePointPoint(p, pn.m_pointArray[i]);
		if (d <= eT)
		{
			vi[i] = i;
		}
	}
}

bool gb_findPointInLoop(CP_Polygon& pn, int& idRegion, int& idLoop, int& idPointInLoop, int pointInPolygon)
{
	idRegion = 0;
	idLoop = 0;
	idPointInLoop = 0;
	int nr, nL, nv;
	int i, j, k;
	nr = pn.m_regionArray.size();
	for (i = 0; i<nr; i++)
	{
		nL = pn.m_regionArray[i].m_loopArray.size();
		for (j = 0; j<nL; j++)
		{
			nv = pn.m_regionArray[i].m_loopArray[j].m_pointIDArray.size();
			for (k = 0; k<nv; k++)
			{
				if (pn.m_regionArray[i].m_loopArray[j].m_pointIDArray[k] == pointInPolygon)
				{
					idRegion = i;
					idLoop = j;
					idPointInLoop = k;
					return true;
				}
			}
		}
	}
	return false;
}


void gb_insertPointInPolygon(CP_Polygon& pn, int& idRegion, int& idLoop, int& idPointInLoop, CP_Point& newPoint)
{
	int nv = pn.m_pointArray.size();
	pn.m_pointArray.push_back(newPoint);
	pn.m_regionArray[idRegion].m_loopArray[idLoop].m_pointIDArray.insert(
		pn.m_regionArray[idRegion].m_loopArray[idLoop].m_pointIDArray.begin() + idPointInLoop + 1,
		nv);
}

void gb_intArrayInit(VT_IntArray& vi, int data)
{
	int n = vi.size();
	int i;
	for (i = 0; i<n; i++)
		vi[i] = data;
}

void gb_intArrayInitLoop(VT_IntArray& vi, CP_Polygon& pn, int idRegion, int idLoop, double eT)
{
	int i, v;
	int n = pn.m_pointArray.size();
	vi.resize(n);
	//Yong's Bug
	gb_intArrayInit(vi, -1);
	n = pn.m_regionArray[idRegion].m_loopArray[idLoop].m_pointIDArray.size();
	for (i = 0; i<n; i++)
	{
		v = pn.m_regionArray[idRegion].m_loopArray[idLoop].m_pointIDArray[i];
		vi[v] = v;
	}
	gb_intArrayInitPointSame(vi, pn, eT);
}

void gb_intArrayInitPoint(VT_IntArray& vi, CP_Polygon& pn, int v, double eT)
{
	int n = pn.m_pointArray.size();
	if (n <= 0)
	{
		vi.clear();
		return;
	}
	vi.resize(n);
	int i;
	double d;
	for (i = 0; i<n; i++)
	{
		if (i == v)
			vi[i] = i;
		else
		{
			d = gb_distancePointPoint(pn.m_pointArray[i], pn.m_pointArray[v]);
			if (d <= eT)
				vi[i] = i;
			else vi[i] = -1;
		}
	}
}

void gb_intArrayInitPointSame(VT_IntArray& vi, CP_Polygon& pn, double eT)
{
	int i, j, n;
	double d;
	n = vi.size();
	if (n <= 0)
		return;
	for (i = 0; i<n; i++)
	{
		if (vi[i] >= 0)
		{
			for (j = 0; j<n; j++)
			{
				if (vi[j]<0)
				{
					d = gb_distancePointPoint(pn.m_pointArray[i], pn.m_pointArray[j]);
					if (d <= eT)
						vi[j] = j;
				}
			}
		}
	}
}

void gb_intArrayInitPolygon(VT_IntArray& vi, CP_Polygon& pn)
{
	int i;
	int n = pn.m_pointArray.size();
	vi.resize(n);
	for (i = 0; i<n; i++)
		vi[i] = i;
}

void gb_intArrayInitPolygonSamePoint(VT_IntArray& vr, CP_Polygon& pr, VT_IntArray& vs, CP_Polygon& ps, double eT)
{
	int i, j;
	int n0, n1;
	double da;
	n1 = pr.m_pointArray.size();
	if (n1 <= 0)
	{
		vr.clear();
		return;
	}
	vr.resize(n1);
	gb_intArrayInit(vr, -1);
	n0 = ps.m_pointArray.size();
	for (i = 0; i<n0; i++)
	{
		if (vs[i]<0)
			continue;
		for (j = 0; j<n1; j++)
		{
			if (vr[j]<0)
			{
				da = gb_distancePointPoint(ps.m_pointArray[i], pr.m_pointArray[j]);
				if (da <= eT)
					vr[j] = j;
			}
		}
	}
}

void gb_intArrayInitRegion(VT_IntArray& vi, CP_Polygon& pn, int idRegion, double eT)
{
	int i, j, nr, v;
	int n = pn.m_pointArray.size();
	vi.resize(n);
	nr = pn.m_regionArray[idRegion].m_loopArray.size();
	for (i = 0; i<nr; i++)
	{
		n = pn.m_regionArray[idRegion].m_loopArray[i].m_pointIDArray.size();
		for (j = 0; j<n; j++)
		{
			v = pn.m_regionArray[idRegion].m_loopArray[i].m_pointIDArray[j];
			vi[v] = v;
		}
	}
	gb_intArrayInitPointSame(vi, pn, eT);
}

void gb_moveLoop(CP_Polygon& pn, int idRegion, int idLoop, double vx, double vy)
{
	int nr, nL, nv;
	int i, id;
	nr = pn.m_regionArray.size();
	if ((idRegion<0) || (idRegion >= nr))
		return;
	nL = pn.m_regionArray[idRegion].m_loopArray.size();
	if ((idLoop<0) || (idLoop >= nL))
		return;
	nv = pn.m_regionArray[idRegion].m_loopArray[idLoop].m_pointIDArray.size();
	for (i = 0; i<nv; i++)
	{
		id = pn.m_regionArray[idRegion].m_loopArray[idLoop].m_pointIDArray[i];
		pn.m_pointArray[id].m_x += vx;
		pn.m_pointArray[id].m_y += vy;
	}
}

void gb_movePoint(CP_Polygon& pn, int id, double vx, double vy)
{
	int n = pn.m_pointArray.size();
	if ((id<0) || (id >= n))
		return;
	pn.m_pointArray[id].m_x += vx;
	pn.m_pointArray[id].m_y += vy;
}

void gb_movePointIntArray(CP_Polygon& pn, VT_IntArray& vi, double vx, double vy)
{
	int n = vi.size();
	int i;
	for (i = 0; i<n; i++)
		gb_movePoint(pn, vi[i], vx, vy);
}

void gb_movePolygon(CP_Polygon& pn, double vx, double vy)
{
	int n = pn.m_pointArray.size();
	int i;
	for (i = 0; i<n; i++)
	{
		pn.m_pointArray[i].m_x += vx;
		pn.m_pointArray[i].m_y += vy;
	}
}

void gb_moveRegion(CP_Polygon& pn, int idRegion, double vx, double vy)
{
	int nr, nL, nv;
	int i, j, k, id;
	nr = pn.m_regionArray.size();
	if ((idRegion<0) || (idRegion >= nr))
		return;
	i = idRegion;
	nL = pn.m_regionArray[i].m_loopArray.size();
	for (j = 0; j<nL; j++)
	{
		nv = pn.m_regionArray[i].m_loopArray[j].m_pointIDArray.size();
		for (k = 0; k<nv; k++)
		{
			id = pn.m_regionArray[i].m_loopArray[j].m_pointIDArray[k];
			pn.m_pointArray[id].m_x += vx;
			pn.m_pointArray[id].m_y += vy;
		}
	}
}


void gb_pointConvertFromGlobalToScreen(CP_Point& result, CP_Point pointGlobal, double scale, CP_Point translation, int screenX, int screenY)
{
	result.m_x = (pointGlobal.m_x - translation.m_x)*scale;
	result.m_y = (pointGlobal.m_y - translation.m_y)*scale;
	result.m_x += (screenX / 2);
	result.m_y = screenY / 2 - result.m_y;
}

void gb_pointConvertFromScreenToGlobal(CP_Point& result, CP_Point pointScreen, double scale, CP_Point translation, int screenX, int screenY)
{
	result.m_x = pointScreen.m_x - screenX / 2;
	result.m_y = screenY / 2 - pointScreen.m_y;
	result.m_x = result.m_x / scale + translation.m_x;
	result.m_y = result.m_y / scale + translation.m_y;
}

bool gb_polygonNewInLoopRegular(CP_Polygon& p, int idRegion, int n, double r, double cx, double cy)
{
	if (n<3)
		return false;
	int nr = p.m_regionArray.size();
	if ((idRegion<0) || (idRegion >= nr))
		return false;
	int nL = p.m_regionArray[idRegion].m_loopArray.size();
	if (nL <= 0)
		return false;
	p.m_regionArray[idRegion].m_loopArray.resize(nL + 1);
	int s = p.m_pointArray.size();
	int t = s + n;
	int i, k;
	p.m_pointArray.resize(t);
	double da = DOUBLE_PI / n;
	double d = 0.0;
	for (i = s; i<t; i++, d += da)
	{
		p.m_pointArray[i].m_x = cx + r*cos(d);
		p.m_pointArray[i].m_y = cy + r*sin(d);
	}
	p.m_regionArray[idRegion].m_loopArray[nL].m_polygon = &p;
	p.m_regionArray[idRegion].m_loopArray[nL].m_regionIDinPolygon = idRegion;
	p.m_regionArray[idRegion].m_loopArray[nL].m_loopIDinRegion = nL;
	p.m_regionArray[idRegion].m_loopArray[nL].m_pointIDArray.resize(n);
	for (i = 0, k = t - 1; i<n; i++, k--)
	{
		p.m_regionArray[idRegion].m_loopArray[nL].m_pointIDArray[i] = k;
	}
	return true;
}

void gb_polygonNewOutLoopRegular(CP_Polygon& p, int n, double r, double cx, double cy)
{
	if (n<3)
		return;
	int s = p.m_pointArray.size();
	int t = s + n;
	int i, k;
	p.m_pointArray.resize(t);
	double da = DOUBLE_PI / n;
	double d = 0.0;
	for (i = s; i<t; i++, d += da)
	{
		p.m_pointArray[i].m_x = cx + r*cos(d);
		p.m_pointArray[i].m_y = cy + r*sin(d);
	}
	int rs = p.m_regionArray.size();
	p.m_regionArray.resize(rs + 1);
	p.m_regionArray[rs].m_polygon = &p;
	p.m_regionArray[rs].m_regionIDinPolygon = rs;
	p.m_regionArray[rs].m_loopArray.resize(1);
	p.m_regionArray[rs].m_loopArray[0].m_polygon = &p;
	p.m_regionArray[rs].m_loopArray[0].m_regionIDinPolygon = rs;
	p.m_regionArray[rs].m_loopArray[0].m_loopIDinRegion = 0;
	p.m_regionArray[rs].m_loopArray[0].m_pointIDArray.resize(n);
	for (i = 0, k = s; i<n; i++, k++)
	{
		p.m_regionArray[rs].m_loopArray[0].m_pointIDArray[i] = k;
	}
}

bool gb_removeLoop(CP_Polygon& pn, int idRegion, int idLoop)
{
	int nL, nLv, iLv, v;
	nL = pn.m_regionArray[idRegion].m_loopArray.size();
	if ((idLoop == 0) || (nL<2))
		return(gb_removeRegion(pn, idRegion));
	nLv = pn.m_regionArray[idRegion].m_loopArray[idLoop].m_pointIDArray.size();
	for (iLv = 0; iLv<nLv; iLv++)
	{
		v = pn.m_regionArray[idRegion].m_loopArray[idLoop].m_pointIDArray[iLv];
		pn.m_pointArray.erase(pn.m_pointArray.begin() + v);
		gb_subtractOneAboveID(pn, v);
	}
	pn.m_regionArray[idRegion].m_loopArray.erase(
		pn.m_regionArray[idRegion].m_loopArray.begin() + idLoop);
	return true;
}

bool gb_removePoint(CP_Polygon& pn, int id)
{
	int ir, iL, iLv, nLv;
	bool rf = gb_findPointInLoop(pn, ir, iL, iLv, id);
	if (!rf)
		return false;
	nLv = pn.m_regionArray[ir].m_loopArray[iL].m_pointIDArray.size();
	if (nLv<4)
		return (gb_removeLoop(pn, ir, iL));
	pn.m_regionArray[ir].m_loopArray[iL].m_pointIDArray.erase(
		pn.m_regionArray[ir].m_loopArray[iL].m_pointIDArray.begin() + iLv);
	pn.m_pointArray.erase(pn.m_pointArray.begin() + id);
	gb_subtractOneAboveID(pn, id);
	return true;
}

bool gb_removeRegion(CP_Polygon& pn, int idRegion)
{
	int nr, nL, nLv, iL, iLv, v;
	nr = pn.m_regionArray.size();
	if (nr<2)
	{
		pn.mb_clear();
		return true;
	}
	nL = pn.m_regionArray[idRegion].m_loopArray.size();
	for (iL = 0; iL<nL; iL++)
	{
		nLv = pn.m_regionArray[idRegion].m_loopArray[iL].m_pointIDArray.size();
		for (iLv = 0; iLv<nLv; iLv++)
		{
			v = pn.m_regionArray[idRegion].m_loopArray[iL].m_pointIDArray[iLv];
			pn.m_pointArray.erase(pn.m_pointArray.begin() + v);
			gb_subtractOneAboveID(pn, v);
		}
	}
	pn.m_regionArray.erase(pn.m_regionArray.begin() + idRegion);
	return true;
}

void gb_subtractOneAboveID(CP_Polygon& pn, int id)
{
	int nr = pn.m_regionArray.size();
	int nL, nLv, ir, iL, iLv;
	for (ir = 0; ir<nr; ir++)
	{
		nL = pn.m_regionArray[ir].m_loopArray.size();
		for (iL = 0; iL<nL; iL++)
		{
			nLv = pn.m_regionArray[ir].m_loopArray[iL].m_pointIDArray.size();
			for (iLv = 0; iLv<nLv; iLv++)
			{
				if (pn.m_regionArray[ir].m_loopArray[iL].m_pointIDArray[iLv] >= id)
					pn.m_regionArray[ir].m_loopArray[iL].m_pointIDArray[iLv]--;
			}
		}
	}
}

void cvt_polygon_tolerance(const CP_Polygon &pn, CP_Polygon &cvt)
{
	cvt = pn;
	for (int i = 0; i < cvt.m_regionArray.size(); i++)
	{
		CP_Region &region = cvt.m_regionArray[i];
		region.m_polygon = &cvt;
		for (int j = 0; j < region.m_loopArray.size(); j++)
		{
			CP_Loop &loop = region.m_loopArray[j];
			loop.m_polygon = &cvt;
		}
	}

}