#ifndef _CAISYSTEM_H_
#define _CAISYSTEM_H_

#include <map>

#include "buildingidmanager.h"
#include "CTriangulator.h"
#include "Formation.h"
#include "Graph.h"
#include "IAgent.h"
#include "IAISystem.h"
#include "vertexlist.h"

// TheOtherSide
#include "AISystemCVars.h"
// ~TheOtherSide


class CGraph;
struct GraphNode;
struct IRenderer;
struct ISystem;
struct IGame;
class CAIObject;
struct IAIObject;
struct ICVar;
class CPuppet;
class CPipeUser;
class CAIAutoBalance;



struct IAgentProxy;
struct IEntity;
class CGoalPipe;
struct IPhysicalEntity;


const float cm_epsilon = 0.01f;


typedef struct AuxSignalDesc
{
	float fTimeout;
	string strMessage;
} AuxSignalDesc;

typedef struct BeaconStruct
{
	CAIObject *pBeacon;
	CAIObject *pOwner;

	BeaconStruct() 
	{
		pBeacon = nullptr;
		pOwner = nullptr;
	}
} BeaconStruct;

typedef std::multimap<unsigned short , CAIObject *> AIObjects;
typedef std::multimap<int,int> IntIntMultiMap;
typedef std::list<CAIObject *> ListAIObjects;
typedef std::map<string, FormationDescriptor> DescriptorMap;
typedef std::map<int, CFormation*> FormationMap;
typedef std::map<unsigned short, BeaconStruct> BeaconMap;
typedef std::map<string, CGoalPipe *> GoalMap;	
typedef std::map<string, ListPositions> DesignerPathMap;
typedef std::vector<CPuppet *> ListPuppets;
typedef std::map<int, float> MapMultipliers;
typedef std::multimap<unsigned short, AuxSignalDesc> MapSignalStrings;
typedef std::list<unsigned short> ListOfUnsignedShort;

//<<FIXME>> just used for profiling
typedef std::map<string,float> TimingMap;

#ifdef __MWERKS__
#pragma warn_hidevirtual off
#endif

#define AI_THINKINTERVAL 0.1f
#define AISYSTEM_PUPPETRESPAWNTIME 2.f

struct SpecialArea
{
	ListPositions	lstPolygon;
	float fMinZ,fMaxZ;
	float fHeight;
	int	nBuildingID;

	SpecialArea()
		: nBuildingID(0)
	{
		fMinZ = 9999.f;
		fMaxZ = -9999.f;
		fHeight = 0;
	}
};

typedef std::map<string, SpecialArea> SpecialAreaMap;


struct PathFindRequest
{
	GraphNode *pStart;
	GraphNode *pEnd;
	Vec3 endpos;
	Vec3 startpos;
	bool bSuccess;
	CAIObject *pRequester;
	unsigned int	m_nSelectedHeuristic;

	PathFindRequest()
		: bSuccess(false)
	{
		m_nSelectedHeuristic = AIHEURISTIC_STANDARD;
		pStart = pEnd = nullptr;
		pRequester = nullptr;
	}
};

typedef std::list<PathFindRequest *> PathQueue;
struct IVisArea;


typedef struct CutEdgeIdx
{
	int idx1;
	int idx2;

	CutEdgeIdx( int i1, int i2 ) 
	{
		idx1 = i1;
		idx2 = i2;
	}

} CutEdgeIdx;

//typedef std::map<GraphNode*, CutEdgeIdx> NewCutsMap;
typedef std::vector<CutEdgeIdx> NewCutsVector;




class CAISystem : public IAISystem
{
	bool m_bRepopulateUpdateList;
	IVisArea *m_pAreaList[100];
	std::list<IVisArea*> lstSectors;
	int m_nRaysThisUpdateFrame;

	unsigned int m_nNumBuildings;
	IntIntMultiMap m_mapBuildingMap;

	
	CAIAutoBalance *m_pAutoBalance;

	CBuildingIDManager	m_BuildingIDManager;

	ListNodes m_lstNewNodes;
	ListNodes m_lstOldNodes;

//	NewCutsMap	m_NewCutsMap;
	NewCutsVector	m_NewCutsVector;

	ListPositions m_lstVisibilityRays;

	unsigned int m_nTickCount;
	bool m_bInitialized;

	CAIObject *DEBUG_object;

	IPhysicalWorld *m_pWorld;

	CTriangulator *m_pTriangulator;

	std::vector<Tri*>	m_vTriangles;
	VARRAY  m_vVertices;


	float	m_fLastPathfindTimeStart;

	
	ListAIObjects m_lstDummies;
	
	ListPuppets m_lstWaitingToBeUpdated;	
	ListPuppets m_lstAlreadyUpdated;	

	
	PathQueue	m_lstPathQueue;
	PathFindRequest *m_pCurrentRequest;
	
	CGraph *m_pGraph;
	CGraph *m_pHideGraph;

	FormationMap	m_mapActiveFormations;
	DescriptorMap m_mapFormationDescriptors;
	BeaconMap			m_mapBeacons;

	MapSignalStrings	m_mapAuxSignalsFired;

	DesignerPathMap m_mapDesignerPaths;
	DesignerPathMap m_mapOcclusionPlanes;
	DesignerPathMap m_mapForbiddenAreas;
	SpecialAreaMap	m_mapSpecialAreas;	// define where to disable automatic AI processing.	

	ListOfUnsignedShort	m_lstVisible;

	IPhysicalEntity **m_pObstacles;

	// keeps all possible goal pipes that the agents can use
	GoalMap m_mapGoals;

	float fLastUpdateTime;

	CAIObject *m_pLastHidePlace;

	int m_nPathfinderResult;

	float	m_fDistortionTime,m_fDistortionTimeStart;

public:
	
	void        SupressSoundEvent(const Vec3 & pos,float & fAffectedRadius);
	void        SoundEvent(int soundid, const Vec3 & pos,float fRadius, float fThreat, float fInterest,IAIObject * pObject);
	void        RemoveObject(IAIObject * pObject) override;
	CAIObject * CreateDummyObject();
	void        RemoveDummyObject(CAIObject * pObject);
	CAIObject * GetAIObjectByName(const char * pName);
	void        TracePath(const Vec3 & start, const Vec3 & end, CAIObject * pRequester);
	IAIObject * GetAIObjectByName(unsigned short type, const char * pName);
	CAIObject * GetPlayer();
	IAIObject * CreateAIObject(unsigned short type, void * pAssociation);
	int         RayOcclusionPlaneIntersection(const Vec3 & start,const Vec3 & end);
	

	ISystem *m_pSystem;

	AIObjects m_Objects;

	float	m_fAutoBalanceCollectingTimeout;
	bool	m_bCollectingAllowed;

	AIObjects m_mapGroups;
	AIObjects m_mapLeaders;
	AIObjects m_mapSpecies;
	MapMultipliers	m_mapMultipliers;
	MapMultipliers	m_mapSpeciesThreatMultipliers;

	TimingMap m_mapDEBUGTiming;
	TimingMap m_mapDEBUGTimingGOALS;


	CVertexList m_VertexList;

	CGraph *         GetGraph() const;
	IPhysicalWorld * GetPhysicalWorld() const;

		
	CAISystem(ISystem *pSystem);
	~CAISystem() override;

	void Release() override
	{delete this;}

	IGoalPipe *CreateGoalPipe(const char *pName) override;

	IGoalPipe *OpenGoalPipe(const char *pName) override;

	
	bool Init(ISystem *pSystem,const char *szLevel, const char *szMission);
	void Update();
	void ShutDown() override;

	void DebugDraw(IRenderer * pRenderer) override;
	void DebugDrawAlter(IRenderer * pRenderer);
	void DebugDrawVehicle(IRenderer * pRenderer);
	void DebugDrawDirections(IRenderer * pRenderer);
	// for inter dll use

	float f1,f2,f3,f4,f5,f6,f7,f8;
	int m_nNumRaysShot;
	

	// pointer to the indoor manager
	//IIndoorBase *m_pIndoor;

	
	CGoalPipe *IsGoalPipe(const string &name);

	 ICVar *m_cvShowGroup;
	 ICVar *m_cvAllowedTimeForPathfinding;
	 ICVar *m_cvViewField;
	 ICVar *m_cvAgentStats;
	 ICVar *m_cvDrawBalls;
	 ICVar *m_cvAiSystem;
	 ICVar *m_cvDebugDraw;
	 ICVar *m_cvBadTriangleRecursionDepth;
	 ICVar *m_cvSoundPerception;
	 ICVar *m_cvIgnorePlayer;
	 ICVar *m_cvTriangulate;
	 ICVar *m_cvCalcIndoorGraph;
	 ICVar *m_cvDrawPlayerNode;
 	 ICVar *m_cvDrawPlayerNodeFlat;
	 ICVar *m_cvDrawPath;				
	 ICVar *m_cvAllTime;				
	 ICVar *m_cvProfileGoals;				
	 ICVar *m_cvRunAccuracyMultiplier;				
	 ICVar *m_cvTargetMovingAccuracyMultiplier;
	 ICVar *m_cvLateralMovementAccuracyDecay;
	 ICVar *m_cvDrawHide;
	 ICVar *m_cvBeautifyPath;
	 ICVar *m_cvPercentSound;
	 ICVar *m_cvBallSightRangeReliable;
	 ICVar *m_cvBallSightRangeTotal;
	 ICVar *m_cvBallAttackRange;
	 ICVar *m_cvBallSoundRange;
	 ICVar *m_cvBallCommunicationRange;
	 ICVar *m_cvOptimizeGraph;
	 ICVar *m_cvUpdateProxy;
	 ICVar *m_cvSOM_Speed;
	 ICVar *m_cvDrawAnchors;
	 ICVar *m_cvAreaInfo;
	 ICVar *m_cvStatsTarget;
	 ICVar *m_cvSampleFrequency;
	 ICVar *m_cvAIUpdateInterval;
	 ICVar *m_cvAIVisRaysPerFrame;

protected:
	void	UpdatePathFinder();
	void	PathFind(const PathFindRequest &request);

	// tells if the sound is hearable by the puppet
	bool IsSoundHearable(const CPuppet * pPuppet,const Vec3 & vSoundPos,float fSoundRadius);

public:
	// Sends a signal using the desired filter to the desired agents
	void SendSignal(unsigned char cFilter, int nSignalId,const char *szText,  IAIObject *pSenderObject);
	// adds an object to a group
	void AddToGroup(CAIObject * pObject, unsigned short nGroup);
	// adds an object to a species
	void AddToSpecies(CAIObject * pObject, unsigned short nSpecies);
	// creates a formation and associates it with a group of agents
	CFormation *CreateFormation(int nGroupID, const char * szFormationName);
	// retrieves the next available formation point if a formation exists for the group of the requester
	CAIObject * GetFormationPoint(CPipeUser * pRequester);
	// Resets all agent states to initial
	void Reset(void);
	// copies a designer path into provided list if a path of such name is found
	bool GetDesignerPath(const char * szName, ListPositions & lstPath);
	// adds a point to a designer path specified by the name.. If path non existant, one is created 
	void AddPointToPath(const Vec3 & pos, const char * szPathName, EnumAreaType aAreaType = AREATYPE_PATH);
	// gets how many agents are in a specified group
	int GetGroupCount(int nGroupID) const;
	// removes specified object from group
	void RemoveFromGroup(int nGroupID, const CAIObject * pObject);
	// parses ai information into file
	void ParseIntoFile(const char * szFileName, CGraph *pGraph, bool bForbidden = false);
	CGraph * GetHideGraph(void) override;
	// // loads the triangulation for this level and mission
	void LoadTriangulation(const char * szLevel, const char * szMission);
	// deletes designer created path
	void     DeletePath(const char * szName);
	void     ReleaseFormation(int nGroupID);
	void     FreeFormationPoint(int nGroupID, CAIObject * pLastHolder);
	bool     NoFriendsInWay(CPuppet * pShooter, const Vec3 & vDirection);
	IGraph * GetNodeGraph(void) override;
	void     FlushSystem(void) override;
	float    GetPerceptionValue(IAIObject * pObject) const;
	int      GetAITickCount(void) override;
protected:
	unsigned int m_nNumPuppets;
	IPhysicalEntity *m_pTheSkip;	// phys entity which will be skipped on raytracing when determinig visibility

public:
	void        SendAnonimousSignal(int nSignalID, const char * szText, const Vec3 & vPos, float fRadius, IAIObject * pObject);
	void        ReleaseFormationPoint(CAIObject * pReserved) const;
	IAIObject * GetNearestObjectOfType(const Vec3 & pos, unsigned int nTypeID,float fRadius=0, const IAIObject* pSkip= nullptr);
	void        UpdateBeacon(unsigned short nGroupID, const Vec3 & vPos, CAIObject * pOwner = nullptr);
	CAIObject * GetBeacon(unsigned short nGroupID) override;
	void        CancelAnyPathsFor(const CPuppet * pRequester);
	void        SetAssesmentMultiplier(unsigned short type, float fMultiplier) override;
	IAIObject * GetNearestToObject(IAIObject * pRef, unsigned short nType, float fRadius);
protected:
	void AddForbiddenAreas(void);
public:
	GraphNode *RefineTriangle(GraphNode * pNode,const Vec3 &start, const Vec3 &end);
	//void FillGraphNodeData(GraphNode* pNode);
	IAIObject * GetNearestObjectOfType(IAIObject * pObject, unsigned int nTypeID, float fRadius, int nOption=0);
	void CalculatePassRadiuses();
	void CreateNewTriangle(const ObstacleData & od1, const ObstacleData & od2, const ObstacleData & od3, bool tag=false);

	void GetMemoryStatistics(ICrySizer *pSizer) override;

	bool  DEBUG_LISTCORRUPT(ListNodes & lstNodes);
	bool  IsForbidden(const Vec3 & start, const Vec3 & end) const;
	bool  CheckInside(const Vec3 & pos, int & nBuildingID, IVisArea *& pArea, bool bSkipSpecialAreas = false);
	bool  PointInsidePolygon(ListPositions & lstPolygon, const Vec3 & pos) const;
	float PointLineDistance(const Vec3 & vLineStart, const Vec3 & vLineEnd, const Vec3 & vPoint);
	bool  PointsOnLine(const Vec3 & vLineStart, const Vec3 & vLineEnd, const Vec3 & vPoint1, const Vec3 & vPoint2) const;
	bool  PointOnLine(const Vec3 & vLineStart, const Vec3 & vLineEnd, const Vec3 & vPoint, float fPrecision=0) const;
	bool  SegmentsIntersect(const Vec3 & vSegmentAStart, const Vec3 & vSegmentADir, const Vec3 & vSegmentBStart, const Vec3 & vSegmentBDir, float & fCutA, float & fCutB) const;
	bool  TriangleLineIntersection(GraphNode * pNode, const Vec3 & vStart, const Vec3 & vEnd);
	bool  SegmentInTriangle(GraphNode * pNode, const Vec3 & vStart, const Vec3 & vEnd);
	bool  CreatePath(const char * szName,EnumAreaType eAreaType, float fHeight = 0);
	bool  BehindForbidden(const Vec3 & vStart, const Vec3 & vEnd,string & forb);
	bool  NoFriendInVicinity(const Vec3 & vPos, float fRadius , CPipeUser * pChecker);
	bool  NoSameHidingPlace(CPipeUser * pHider, const Vec3 & vPos);
	bool  CrowdControl(CPipeUser * pMain, const Vec3 & pos);

	void SingleDryUpdate(CPuppet * pObject) const;
	bool SingleFullUpdate(CPuppet * pPuppet);
	void CheckVisibility(CPuppet * pPuppet, unsigned short TypeToCheck);
	Vec3 IntersectPolygon(const Vec3 & start, const Vec3 & end, ListPositions & lstPolygon) const;
	bool BehindSpecialArea(const Vec3 & vStart, const Vec3 & vEnd, string & strSpecial);
	bool IntersectsForbidden(const Vec3 & vStart, const Vec3 & vEnd, Vec3 & vClosestPoint);
	bool IntersectsSpecialArea(const Vec3 & vStart, const Vec3 & vEnd, Vec3 & vClosestPoint);
	bool ForbiddenAreasOverlap(void);


	void       AddTheCut( int vIdx1, int vIdx2 );
	GraphNode* FindMarkNodeBy2Vertex( int vIdx1, int vIdx2, const GraphNode* exclude );
	void       CreatePossibleCutList( const Vec3 & vStart, const Vec3 & vEnd, ListNodes & lstNodes );

	const ObstacleData GetObstacle(int nIndex) override;
	// it removes all references to this object from all objects of the specified type
	void RemoveObjectFromAllOfType(int nType, CAIObject* pRemovedObject);
	bool OnForbiddenEdge(const Vec3 & pos);

	void           SetTheSkip(IPhysicalEntity * pSkip) { m_pTheSkip=pSkip; }
	IAutoBalance * GetAutoBalanceInterface(void) override;
	int            ApplyDifficulty(float fAccuracy, float fAggression, float fHealth);
	bool           ExitNodeImpossible(GraphNode * pNode, float fRadius);
	void           DrawPuppetAutobalanceValues(IRenderer * pRenderer);
	void           SetSpeciesThreatMultiplier(int nSpeciesID, float fMultiplier) override;
	void           DumpStateOf(IAIObject * pObject) override;
	int            GetNumberOfObjects(unsigned short type) const;
	bool           ThroughVehicle(const Vec3 & start, const Vec3 & end);

	// TheOtherSide
	SAISystemCVars m_CVars;
	// TheOtherSide

};

#ifdef __MWERKS__
#pragma warn_hidevirtual reset
#endif



#endif // _CAISYSTEM_H_
