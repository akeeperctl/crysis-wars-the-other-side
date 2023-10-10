#ifndef _TRIANGULATOR_H_
#define _TRIANGULATOR_H_
#include <cmath>
#include <vector>
#include <list>


typedef struct Vtx
{
	float            x{}, y{}, z{};
	void*            pForeign{};
	std::vector<int> m_lstTris; // triangles that contain this point
	bool             bImportant{};

	bool operator==(const Vtx& other) const
	{
		const float dist2 = ((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y) + (z - other.z) * (z - other.z));
		if (dist2 < 0.7)
			return true;

		if ((std::fabs(x - other.x) < 0.0001) && (std::fabs(y - other.y) < 0.0001))
			return true;

		return false;
	}
} Vtx;


typedef struct Tri
{
	int   v[3]{};
	Vtx   center;
	float radius{};

	void* outsider;

	Tri()
	{
		outsider = nullptr;
	}
} Tri;

#if defined(__MWERKS__) || defined(LINUX)
struct POINT {int x,y; };
#endif

typedef struct tagMyPoint : public POINT
{
	struct tagMyPoint* next;

	bool operator==(const tagMyPoint& other) const
	{
		return ((this->x == other.x) && (this->y == other.y));
	}
} MYPOINT;

typedef std::list<Tri*> TARRAY;


typedef std::vector<Vtx> VARRAY;

extern int g_lajno;

class CTriangulator
{
	VARRAY             m_vProcessed;
	TARRAY             m_vTriangles;
	std::list<MYPOINT> m_lstUnique;

public:
	void CalcCircle(const Vtx& v1, const Vtx& v2, const Vtx& v3, Tri* pTri) const;

	CTriangulator();
	~CTriangulator();

	VARRAY m_vVertices;
	Vtx    m_vtxBBoxMin;
	Vtx    m_vtxBBoxMax;

	int    AddVertex(float x, float y, float z, void* pForeign, bool bImportant = false);
	bool   Triangulate();
	TARRAY GetTriangles();
	VARRAY GetVertices();
	bool   IsAntiClockwise(const Tri* who) const;
	void   PushUnique(int a, int b);

private:
protected:
	bool IsPerpendicular(const Vtx& v1, const Vtx& v2, const Vtx& v3) const;
	bool Calculate(Tri* pTri) const;

public:
	bool PrepForTriangulation(void);
	void FinalizeTriangulation(void);
	bool TriangulateNew(void);
};

#endif  _TRIANGULATOR_H_
