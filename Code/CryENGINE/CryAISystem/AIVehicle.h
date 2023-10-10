#ifndef _AI_VEHICLE_
#define _AI_VEHICLE_

//#include "aiobject.h"
#include "Puppet.h"

#include "AISystem.h"
#include "PipeUser.h"
//#include "AgentParams.h"
//#include "IAgent.h"
//#include "GoalPipe.h"
//#include "Graph.h"
//#include <list>
//#include <map>
//#include <vector>


class CPuppet;

class CAIVehicle :
//	public CPipeUser, IVehicle
	public CPuppet
{
public:
	CAIVehicle(void);
	~CAIVehicle(void) override;

	void Update() override;
	void Steer(const Vec3 & vTargetPos, GraphNode * pNode) override;
//	void UpdateVehicleInternalState();
	void Navigate(CAIObject * pTarget) override;
	void Event(unsigned short eType, SAIEVENT * pEvent) override;

	void Reset(void) override;
	void ParseParameters(const AIObjectParameters & params) override;
	void OnObjectRemoved(CAIObject * pObject) override;

	bool CanBeConvertedTo(unsigned short type, void **pConverted) override;

	void	SetParameters(AgentParameters & sParams);
	AgentParameters GetPuppetParameters() { return GetParameters();}
	void SetPuppetParameters(AgentParameters &pParams) { SetParameters(pParams);}

	void SetVehicleType(unsigned short type) { m_VehicleType = type; }
	unsigned short GetVehicleType( ) { return m_VehicleType; }

	void UpdateThread();

	IUnknownProxy* GetProxy() override
	{ return m_pProxy; };
	void Bind(IAIObject* bind) override;
	void Unbind( ) override;
//	IAIObject* GetBound( )  { return m_Gunner; }
//	void	SetGunner( IAIObject *pGunner );				//CPuppet	*gunner);
//	void Event(unsigned short eType, SAIEVENT *pEvent);

	IVehicleProxy	*m_pProxy;

	CAIObject		*m_Threat;

private:

	unsigned short m_VehicleType;

	CPuppet	*m_Gunner;

public:
	void AlertPuppets(void);
	void Save(CStream & stm) override;
	void Load(CStream & stm) override;
};

#endif