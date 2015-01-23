#ifndef _SIMPLE_POLYGON
#define _SIMPLE_POLYGON

#include <cstdlib>

typedef struct {
	double x, y;
} SPoint;

typedef struct SPolygon
{
	SPolygon()
	{
		n = 0;
		V = NULL;
	}
	SPolygon(int npts)
	{
		n = npts;
		V = (SPoint*)malloc(npts * sizeof(SPoint));
	}

	~SPolygon()
	{
		free(V);
	}

public:
	int n;
	SPoint *V;
} SPolygon;


bool simple_polygon(SPolygon &Pn);
bool simple_polygons(SPolygon Pn[], int length);

#endif