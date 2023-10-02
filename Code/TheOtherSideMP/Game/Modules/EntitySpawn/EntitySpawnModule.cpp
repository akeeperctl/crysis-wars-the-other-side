#include "StdAfx.h"
#include "EntitySpawnModule.h"

#include "Game.h"

#include "../../TOSGame.h"
#include "../../TOSGameEventRecorder.h"

CTOSEntitySpawnModule::CTOSEntitySpawnModule()
{
	
}

CTOSEntitySpawnModule::~CTOSEntitySpawnModule()
{
}

void CTOSEntitySpawnModule::OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event)
{
	//TOS_INIT_EVENT_VALUES(pEntity, event);
}

void CTOSEntitySpawnModule::Init()
{
	CTOSGenericModule::Init();
}

void CTOSEntitySpawnModule::Update(float frametime)
{
}

void CTOSEntitySpawnModule::Serialize(TSerialize ser)
{
}

IEntity* CTOSEntitySpawnModule::SpawnEntity(SEntitySpawnParams& params) const
{
	const auto pEntSys = gEnv->pEntitySystem;
	if (!pEntSys)
		return nullptr;

	const auto pEntity = pEntSys->SpawnEntity(params);
	assert(pEntity);

	return pEntity;
}
