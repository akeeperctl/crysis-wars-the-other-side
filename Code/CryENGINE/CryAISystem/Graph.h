// Graph.h: interface for the CGraph class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GRAPH_H__6D059D2E_5A74_4352_B3BF_2C88D446A2E1__INCLUDED_)
#define AFX_GRAPH_H__6D059D2E_5A74_4352_B3BF_2C88D446A2E1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IAgent.h"
#include "Heuristic.h"
#include <list>
#include <map>
#include <vector>

#ifdef LINUX
#include <winbase.h>
#endif

#define BAI_FILE_VERSION 30

struct IRenderer;
class CCryFile;

#define PATHFINDER_STILLTRACING				0
#define PATHFINDER_WALKINGBACK				1
#define PATHFINDER_BEAUTIFYINGPATH			2
#define PATHFINDER_POPNEWREQUEST			3
#define PATHFINDER_CLEANING_GRAPH			4

#define PATHFINDER_PATHFOUND				10
#define PATHFINDER_NOPATH					11


#define PATHFINDER_ITERATIONS		 30


class CAISystem;
struct IStatObj;
class ICrySizer;
class CAIObject;

struct IVisArea;

typedef struct NodeDescriptor
{
	int64        id; //AMD Port
	bool         bCreated;
	int          building;
	GameNodeData data;
	bool         bEntrance;
	bool         bExit;
	int          nObstacles;
	int          obstacle[10];
	int          pad; //padding to make it aligned to a multiple of 8 byte, be careful when changing it
}                NodeDescriptor;

typedef std::list<GraphNode*>   ListNodes;
typedef std::vector<GraphNode*> VectorNodes;

typedef struct NodeWithHistory
{
	GraphNode* pNode;
	ListNodes  lstParents;
}              NodeWithHistory;

typedef struct LinkDescriptor
{
	int64 nSourceNode; //AMD Port
	int64 nTargetNode;
	float fMaxPassRadius;
	char  nStartIndex, nEndIndex;
	Vec3  vEdgeCenter;
	Vec3  vWayOut;
}         LinkDescriptor;


class CHeuristic;
typedef std::multimap<float, GraphNode*>      CandidateMap;
typedef std::multimap<float, NodeWithHistory> CandidateHistoryMap;

// NOTE: INT_PTR here avoids a tiny performance impact on 32-bit platform
// for the cost of loss of full compatibility: 64-bit generated BAI files
// can't be used on 32-bit platform safely. Change the key to int64 to 
// make it fully compatible. The code that uses this map will be recompiled
// to use the full 64-bit key on both 32-bit and 64-bit platforms.
typedef std::multimap<INT_PTR, GraphNode*> EntranceMap;
typedef std::list<Vec3>                    ListPositions;
typedef std::list<ObstacleData>            ListObstacles;
typedef std::list<GraphNode*>::iterator    graphnodeit;
typedef std::vector<NodeDescriptor>        NodeBuffer;
typedef std::vector<GraphNode>             NodeMemory;
typedef std::vector<int>                   LinkBuffer;
typedef std::vector<LinkDescriptor>        LinkDescBuffer;


class CGraph : public IGraph
{
protected:
	int        m_nAStarDistance;
	GraphNode* m_pCurrent;
	GraphNode* m_pPathfinderCurrent;
	//	GraphNode *m_pFirst;
	GraphNode*  m_pPathBegin;
	GraphNode*  m_pWalkBackCurrent;
	CHeuristic* m_pHeuristic;


	CandidateHistoryMap m_mapCandidates; // used by pathfinder
	CandidateMap        m_mapGreedyWalkCandidates; // used by get enclosing

	VectorNodes m_lstTagTracker; // for quick cleaning of the tag
	VectorNodes m_lstMarkTracker; // for quick cleaning of the mark

	ListNodes m_lstDeleteStack; // for non-recursive deletion of the graph (stack emulator)
	ListNodes m_lstNodeStack;

	ListNodes m_lstLastPath;


	NodeBuffer     m_vBuffer;
	LinkBuffer     m_vLinks;
	LinkDescBuffer m_vLinksDesc;
	EntranceMap    m_mapReadNodes; // when the graph is read
	NodeMemory     m_vNodes;

	Vec3 m_vBBoxMin;
	Vec3 m_vBBoxMax;


	CAISystem* m_pAISystem;

	ListNodes::iterator m_iFirst, m_iSecond, m_iThird;
	Vec3                m_vBeautifierStart;
	Vec3                m_vLastIntersection;
	bool                m_bBeautifying;

	CAIObject* m_pRequester; // the puppet which whant's the path

public:
	void SetRequester(CAIObject* rq)
	{
		m_pRequester = rq;
	}

	CAIObject* GetRequester() const
	{
		return m_pRequester;
	}

	bool       ClearTags();
	GraphNode* CheckClosest(GraphNode* pCurrent, const Vec3& pos);
	int        ContinueAStar(GraphNode* pEnd, int& nIterations);
	int        WalkAStar(GraphNode* pBegin, GraphNode* pEnd, int& nIterations);
	int        WalkBack(GraphNode* pBegin, GraphNode* pEnd, int& nIterations);
	void       ClearDebugFlag(GraphNode* pNode) const;
	void       DEBUG_DrawCenters(GraphNode* pNode, IRenderer* pRenderer, int dist) const;
	void       DrawPath(IRenderer* pRenderer);
	void       GetFieldCenter(Vec3& pos) const;

	void WriteToFile(const char* pname);
	void Connect(GraphNode* one, GraphNode* two);

	void DisableInSphere(const Vec3& pos, float fRadius);
	void EnableInSphere(const Vec3& pos, float fRadius);


	CGraph(CAISystem*);
	~CGraph() override;

	CandidateMap  m_lstVisited; // debug map... remove later
	EntranceMap   m_mapEntrances;
	EntranceMap   m_mapExits;
	GraphNode*    m_pFirst;
	GraphNode*    m_pSafeFirst;
	ListPositions m_lstPath;

	ListNodes m_lstTrapNodes;

	ListNodes m_lstSaveStack;
	ListNodes m_lstCurrentHistory;

	int   nNodes;
	float m_fDistance;
	Vec3  m_vRealPathfinderEnd;

	ListNodes     m_lstNodesInsideSphere;
	ListObstacles m_lstSelected;

	GraphNode*         GetCurrent() const;
	virtual GraphNode* GetEnclosing(const Vec3& pos, GraphNode* pStart = nullptr, bool bOutsideOnly = false);

protected:
	int  GetNodesInSphere(const Vec3& pos, float fRadius);
	void DeleteGraph(GraphNode*, int depth);
	void ClearPath();
	void EvaluateNode(GraphNode* pNode, GraphNode* pEnd, GraphNode* pParent);

	GraphNode* ASTARStep(GraphNode* pBegin, GraphNode* pEnd);
	void       DebugWalk(GraphNode* pNode, const Vec3& pos);


#ifndef __MWERKS__
	GraphNode* WriteLine(GraphNode* pNode);
#endif

private:
	int m_nTagged;

public:
	void TagNode(GraphNode* pNode);
	void Disconnect(GraphNode* pDisconnected, bool bDelete = true);
	// walk that will always produce a result, for indoors
	void IndoorDebugWalk(GraphNode* pNode, const Vec3& pos, IVisArea* pArea = nullptr);
	// Clears the tags of the graph without time-slicing the operation
	void ClearTagsNow(void);

	// Check whether a position is within a node's triangle
	bool PointInTriangle(const Vec3& pos, GraphNode* pNode);

	// uses mark for internal graph operation without disturbing the pathfinder
	void MarkNode(GraphNode* pNode);

public:
	// clears the marked nodes
	void ClearMarks(bool bJustClear = false);

protected:
	// iterative function to quickly converge on the target position in the graph
	GraphNode* GREEDYStep(GraphNode* pBegin, const Vec3& pos, bool bIndoor = false);

public:
	// adds an entrance for easy traversing later
	void AddIndoorEntrance(int nBuildingID, GraphNode* pNode, bool bExitOnly = false);
	// Reads the AI graph from a specified file
	bool ReadFromFile(const char* szName);
	// reads all the nodes in a map
	bool ReadNodes(CCryFile& file);
	// defines bounding rectangle of this graph
	void SetBBox(const Vec3& min, const Vec3& max);
	// how is that for descriptive naming of functions ??
	bool OutsideOfBBox(const Vec3& pos) const;
	bool RemoveEntrance(int nBuildingID, GraphNode* pNode);


	//GraphNode* CreateNewNode(bool bFromTriangulation = false);
	GraphNode* CreateNewNode(bool bFromTriangulation = false) const;
	unsigned   CreateNewNode(IAISystem::ENavigationType type, const Vec3& pos, unsigned ID = 0) override;


	int        BeautifyPath(int& nIterations, const Vec3& start, const Vec3& end);
	int        BeautifyPathCar(int& nIterations, const Vec3& start, const Vec3& end);
	int        BeautifyPathCarOld(int& nIterations, const Vec3& start, const Vec3& end);
	int        SelectNodesInSphere(const Vec3& vCenter, float fRadius, GraphNode* pStart = nullptr);
	void       AddHidePoint(GraphNode* pOwner, const Vec3& pos, const Vec3& dir);
	void       DeleteNode(GraphNode* pNode);
	void       DisconnectLink(GraphNode* one, GraphNode* two, bool bOneWay = false);
	void       DisconnectUnreachable(void);
	void       FillGreedyMap(GraphNode* pNode, const Vec3& pos, IVisArea* pTargetArea, bool bStayInArea);
	void       REC_RemoveNodes(GraphNode* pNode);
	void       RemoveHidePoint(GraphNode* pOwner, const Vec3& pos, const Vec3& dir);
	void       RemoveIndoorNodes(void);
	void       ResolveLinkData(GraphNode* pOne, GraphNode* pTwo);
	void       SelectNodeRecursive(GraphNode* pNode, const Vec3& vCenter, float fRadius);
	void       SelectNodesRecursiveIndoors(GraphNode* pNode, const Vec3& vCenter, float fRadius, float fDistance);

	// merging, optimization stuff
	typedef std::multimap<float, GraphNode*> NodesList;

	//******
	typedef std::list<int> ObstacleIndexList;

	int  Rearrange(ListNodes& nodesList, const Vec3& cutStart, const Vec3& cutEnd);
	bool ProcessRearrange(GraphNode* node1, GraphNode* node2, ListNodes& nodesList);
	int  ProcessRearrange(ListNodes& nodesList, const Vec3& cutStart, const Vec3& cutEnd);
	//	bool				ProcessMerge( GraphNode	*curNode, ListNodes& nodesList );
	int ProcessMegaMerge(ListNodes& nodesList, const Vec3& cutStart, const Vec3& cutEnd);
	//	bool				CreateOutline( ListNodes& insideNodes, ListNodes& nodesList, ListPositions&	outline);
	//******
	bool CreateOutline(const ListNodes& insideNodes, ListNodes& nodesList, ObstacleIndexList& outline);
	//******
	void TriangulateOutline(ListNodes& nodesList, ObstacleIndexList& outline, bool orientation);

	bool ProcessMerge(GraphNode* curNode, NodesList& ndList);

	//	GraphNode*	CanMerge( GraphNode	*curNode, int& curIdxToDelete, int& curIdxToKeep, int& nbrIdxToDelete, int& nbrIdxToKeep );
	//	bool				CanMergeNbr( GraphNode	*nbr1, GraphNode	*nbr2 );

	//	GraphNode*	DoMerge( GraphNode	*node1, GraphNode	*node2, int curIdxToDelete, int curIdxToKeep, int nbrIdxToDelete, int nbrIdxToKeep );

	void ConnectNodes(ListNodes& lstNodes);
	void FillGraphNodeData(GraphNode* pNode);

	size_t MemStats();

	bool       DbgCheckList(ListNodes& nodesList) const;
	void       SetCurrentHeuristic(unsigned int heuristic_type);
	void       ResolveTotalLinkData(void);
	bool       CanReuseLastPath(GraphNode* pBegin);
	GraphNode* GetThirdNode(const Vec3& vFirst, const Vec3& vSecond, const Vec3& vThird);

	ListPositions m_DEBUG_outlineListL;
	ListPositions m_DEBUG_outlineListR;

	void       Reset(void);
	void       FindTrapNodes(GraphNode* pNode, int recCount);
	GraphNode* GetEntrance(int nBuildingID, const Vec3& pos);
	void       RemoveDegenerateTriangle(GraphNode* pDegenerate, bool bRecurse = true);
	void       FixDegenerateTriangles(void);
};


#endif // !defined(AFX_GRAPH_H__6D059D2E_5A74_4352_B3BF_2C88D446A2E1__INCLUDED_)
