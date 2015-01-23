#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Avl.h"
#include "SimplePolygon.h"

enum SEG_SIDE
{
	LEFT,
	RIGHT
};


int xyorder(SPoint* p1, SPoint* p2)
{
	if (p1->x > p2->x) return 1;
	if (p1->x < p2->x) return -1;

	if (p1->y > p2->y) return 1;
	if (p1->y < p2->y) return -1;

	return 0;
}


inline double isLeft(SPoint P0, SPoint P1, SPoint P2)
{
	return (P1.x - P0.x)*(P2.y - P0.y) - (P2.x - P0.x)*(P1.y - P0.y);
}


class SLseg;

typedef struct _event Event;
struct _event {
	int edge;
	enum SEG_SIDE type;
	SPoint *eV;
	SLseg *seg;
	Event *otherEnd;
};

int E_compare(const void* v1, const void* v2)
{
	Event **pe1 = (Event**)v1;
	Event **pe2 = (Event**)v2;

	int r = xyorder((*pe1)->eV, (*pe2)->eV);
	if (r == 0)
	{
		if ((*pe1)->type == (*pe2)->type)
			return 0;
		if ((*pe1)->type == LEFT)
			return -1;
		else return 1;
	}
	else
		return r;
}


class EventQueue
{
	int ne;
	int ix;
	Event *Edata;
	Event **Eq;
public:
	EventQueue(SPolygon &P);
	EventQueue(SPolygon P[], int length);
	~EventQueue(void)
	{
		delete[] Eq;
		delete[] Edata;
	}

	Event* next();
};


EventQueue::EventQueue(SPolygon &P)
{
	ix = 0;
	ne = 2 * P.n;
	Edata = new Event[ne];
	Eq = new Event*[ne];
	for (int i = 0; i < ne; i++)
		Eq[i] = &Edata[i];

	for (int i = 0; i < P.n; i++)
	{
		Eq[2 * i]->edge = i;
		Eq[2 * i + 1]->edge = i;
		Eq[2 * i]->eV = &(P.V[i]);
		Eq[2 * i]->otherEnd = Eq[2 * i + 1];
		Eq[2 * i + 1]->otherEnd = Eq[2 * i];
		Eq[2 * i]->seg = Eq[2 * i + 1]->seg = 0;

		SPoint *pi1 = (i + 1 < P.n) ? &(P.V[i + 1]) : &(P.V[0]);
		Eq[2 * i + 1]->eV = pi1;
		if (xyorder(&P.V[i], pi1) < 0)
		{
			Eq[2 * i]->type = LEFT;
			Eq[2 * i + 1]->type = RIGHT;
		}
		else
		{
			Eq[2 * i]->type = RIGHT;
			Eq[2 * i + 1]->type = LEFT;
		}
	}

	qsort(Eq, ne, sizeof(Event*), E_compare);
}

EventQueue::EventQueue(SPolygon P[], int length)
{
	ix = 0;
	ne = 0;
	for (int i = 0; i < length; i++)
		ne += 2 * P[i].n;
	Edata = new Event[ne];
	Eq = new Event*[ne];
	for (int i = 0; i < ne; i++)
		Eq[i] = &Edata[i];
	int bas = 0;
	for (int j = 0; j < length; j++)
	{
		for (int i = 0; i < P[j].n; i++)
		{
			int i1 = 2 * (i + bas);
			int i2 = 2 * (i + bas) + 1;
			SPoint *pi1 = (i + 1 < P[j].n) ? &(P[j].V[i + 1]) : &(P[j].V[0]);

			Eq[i1]->edge = i + bas;
			Eq[i2]->edge = i + bas;

			Eq[i1]->eV = &(P[j].V[i]);
			Eq[i2]->eV = pi1;

			Eq[i1]->otherEnd = Eq[i2];
			Eq[i2]->otherEnd = Eq[i1];

			Eq[i1]->seg = 0;
			Eq[i2]->seg = 0;

			if (xyorder(&P[j].V[i], pi1) < 0)
			{
				Eq[i1]->type = LEFT;
				Eq[i2]->type = RIGHT;
			}
			else
			{
				Eq[i1]->type = RIGHT;
				Eq[i2]->type = LEFT;
			}
		}
		bas += P[j].n;
	}

	qsort(Eq, ne, sizeof(Event*), E_compare);
}

Event* EventQueue::next()
{
	if (ix >= ne)
		return NULL;
	else
		return Eq[ix++];
}


class SLseg : public Comparable<SLseg*>
{
public:
	int edge;
	SPoint lP;
	SPoint rP;
	SPoint *lPp;
	SLseg *above;
	SLseg *below;

	SLseg() : Comparable<SLseg*>(this) {}
	~SLseg() {}

	bool operator< (const SLseg &a)
	{
		if (this->lPp == a.lPp) {
			if (this->rP.y < a.rP.y)
				return true;
			else
				return false;
		}

		return isLeft(this->lP, this->rP, a.lP) > 0;
	}

	bool operator== (const SLseg &a)
	{
		return this->edge == a.edge;
	}

	cmp_t Compare(SLseg* key) const
	{
		return *key == *this ? EQ_CMP
			: (*key < *this ? MIN_CMP : MAX_CMP);
	}
};

typedef AvlNode<SLseg*> Tnode;

class SweepLine {
	int nv;
	int *nvs;
	int nvs_length;
	SPolygon *Pn;
	AvlTree<SLseg*> Tree;
public:
	SweepLine(SPolygon &P)
	{
		nv = P.n;
		Pn = &P;
		nvs_length = 0;
	}
	SweepLine(SPolygon P[], int length)
	{
		nvs = new int[length];
		int total_counts = 0;
		for (int i = 0; i < length; i++)
		{
			nvs[i] = P[i].n;
			total_counts += P[i].n;
		}
		Pn = new SPolygon(total_counts);
		nv = total_counts;
		int bas = 0;
		for (int i = 0; i < length; i++)
		{
			for (int j = 0; j < P[i].n; j++)
				Pn->V[bas + j] = P[i].V[j];
			bas += P[i].n;
		}
		nvs_length = length;
	}

	~SweepLine(void)
	{
		cleanTree(Tree.myRoot);
		if (nvs_length > 0)
		{
			delete[] nvs;
			delete Pn;
		}
	}

	void cleanTree(Tnode *p)
	{
		if (!p) return;
		delete p->Data();
		cleanTree(p->Subtree(AvlNode<SLseg*>::LEFT));
		cleanTree(p->Subtree(AvlNode<SLseg*>::RIGHT));
	}

	SLseg* add(Event*);
	SLseg* find(Event*);
	bool intersect(SLseg*, SLseg*);
	void remove(SLseg*);
};

SLseg* SweepLine::add(Event* E)
{
	SLseg *s = new SLseg;
	s->edge = E->edge;
	E->seg = s;

	SPoint* v1 = &(Pn->V[s->edge]);
	SPoint* eN;
	if (nvs_length > 0)
	{
		int start = 0;
		int end = 0;
		for (int i = 0; i < nvs_length; i++)
		{
			end += nvs[i];
			if (end > s->edge) break;
			start += nvs[i];
		}
		eN = (s->edge + 1 < end ? &(Pn->V[s->edge + 1]) : &(Pn->V[start]));
	}
	else 
		eN = (s->edge + 1 < Pn->n ? &(Pn->V[s->edge + 1]) : &(Pn->V[0]));
	SPoint* v2 = eN;
	if (xyorder(v1, v2) < 0)
	{
		s->lPp = v1;
		s->lP = *v1;
		s->rP = *v2;
	}
	else
	{
		s->rP = *v1;
		s->lP = *v2;
		s->lPp = v2;
	}
	s->above = NULL;
	s->below = NULL;

	Tnode *nd = Tree.Insert(s);
	Tnode *nx = Tree.Next(nd);
	Tnode *np = Tree.Prev(nd);

	if (nx != NULL)
	{
		s->above = (SLseg*)nx->Data();
		s->above->below = s;
	}
	if (np != NULL)
	{
		s->below = (SLseg*)np->Data();
		s->below->above = s;
	}
	return s;
}

void SweepLine::remove(SLseg* s)
{
	Tnode *nd = Tree.Search(s);
	if (nd == NULL)
		return;

	Tnode *nx = Tree.Next(nd);
	if (nx != NULL)
	{
		SLseg* sx = (SLseg*)(nx->Data());
		sx->below = s->below;
	}
	Tnode *np = Tree.Prev(nd);
	if (np != NULL)
	{
		SLseg* sp = (SLseg*)(np->Data());
		sp->above = s->above;
	}
	Tree.Delete(nd->Key());
	delete s;
}


bool SweepLine::intersect(SLseg* s1, SLseg* s2)
{
	if (s1 == NULL || s2 == NULL)
		return false;

	int e1 = s1->edge;
	int e2 = s2->edge;
	if ((e1 + 1) % nv == e2 || e1 == (e2 + 1) % nv)
	{
		if (nvs_length > 0)
		{
			int end1 = 0;
			for (int i = 0; i < nvs_length; i++)
			{
				end1 += nvs[i];
				if (end1 > e1) break;
			}
			int end2 = 0;
			for (int i = 0; i < nvs_length; i++)
			{
				end2 += nvs[i];
				if (end2 > e2) break;
			}
			if (end1 == end2) return false;
		}
		else
		{
			return false;
		}
	}

	double lsign, rsign;
	lsign = isLeft(s1->lP, s1->rP, s2->lP);
	rsign = isLeft(s1->lP, s1->rP, s2->rP);
	if (lsign * rsign > 0 || (lsign == 0 && rsign != 0) || (lsign != 0 && rsign == 0))
		return false;
	lsign = isLeft(s2->lP, s2->rP, s1->lP);
	rsign = isLeft(s2->lP, s2->rP, s1->rP);
	if (lsign * rsign > 0 || (lsign == 0 && rsign != 0) || (lsign != 0 && rsign == 0))
		return false;
	if (lsign == 0 && rsign == 0)
	{
		if (s1->lP.x != s1->rP.x)
		{
			double x1 = s1->lP.x;
			double x2 = s1->rP.x;
			double x3 = s2->lP.x;
			double x4 = s2->rP.x;
			if (x1 > x2 && ((x3 > x1 && x4 > x1) || (x3 < x2 && x4 < x2)))
				return false;
			if (x1 < x2 && ((x3 > x2 && x4 > x2) || (x3 < x1 && x4 < x1)))
				return false;
		}
		else
		{
			double y1 = s1->lP.y;
			double y2 = s1->rP.y;
			double y3 = s2->lP.y;
			double y4 = s2->rP.y;
			if (y1 > y2 && ((y3 > y1 && y4 > y1) || (y3 < y2 && y4 < y2)))
				return false;
			if (y1 < y2 && ((y3 > y2 && y4 > y2) || (y3 < y1 && y4 < y1)))
				return false;
		}
	}
	return true;
}

bool simple_polygon(SPolygon &Pn)
{
	EventQueue Eq(Pn);
	SweepLine SL(Pn);
	Event *e;
	SLseg *s;

	while (e = Eq.next())
	{
		if (e->type == LEFT)
		{
			s = SL.add(e);
			if (SL.intersect(s, s->above))
				return false;
			if (SL.intersect(s, s->below))
				return false;
		}
		else
		{
			s = e->otherEnd->seg;
			if (SL.intersect(s->above, s->below))
				return false;
			SL.remove(s);
		}
	}
	return true;
}

bool simple_polygons(SPolygon Pn[], int length)
{
	EventQueue Eq(Pn, length);
	SweepLine SL(Pn, length);
	Event *e;
	SLseg *s;

	while (e = Eq.next())
	{
		if (e->type == LEFT)
		{
			s = SL.add(e);
			if (SL.intersect(s, s->above))
				return false;
			if (SL.intersect(s, s->below))
				return false;
		}
		else
		{
			s = e->otherEnd->seg;
			if (SL.intersect(s->above, s->below))
				return false;
			SL.remove(s);
		}
	}
	return true;
}