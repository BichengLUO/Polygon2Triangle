#ifndef CP_TRIANGLE_H
#define CP_TRIANGLE_H
#include <iostream>
using namespace std;
#include <vector>
#include <map>

#include "CP_Polygon.h"

class CP_Triangle
{
public:
	int m_vertices[3];
public:
	CP_Triangle() { m_vertices[0] = 0; m_vertices[1] = 0; m_vertices[2] = 0; }
};

typedef vector<CP_Triangle> VT_TriagleArray;

class CP_TriagleMesh
{
public:
	VT_TriagleArray m_triagleIDArray;
	CP_Polygon* m_polygon;
public:
	CP_TriagleMesh() : m_polygon(NULL) { }
	void mb_clear() { m_polygon = NULL; m_triagleIDArray.clear(); }
	void mb_buildTriagleMesh(CP_Polygon& pn, double tolerance);
}; 

typedef std::map<std::pair<int, int>, std::pair<double, double> > dense_offsets;
extern dense_offsets loose_region(const CP_Region &region);
extern bool tolerate_equal(CP_Point &p1, CP_Point &p2, double tolerance);
#endif