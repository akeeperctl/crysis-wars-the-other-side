#include "StdAfx.h"
#include "PersonalHostiles.h"

CTOSPersonalHostiles::CTOSPersonalHostiles(IEntitySystem* pEntSys) 
	: m_pEntitySystem(pEntSys)
{
}

CTOSPersonalHostiles::~CTOSPersonalHostiles()
{
}

 void CTOSPersonalHostiles::OnEntityEvent(IEntity* pEntity, SEntityEvent& event)
{
	if (pEntity && event.event == ENTITY_EVENT_DONE)
	{
		m_pEntitySystem->RemoveEntityEventListener(pEntity->GetId(), ENTITY_EVENT_DONE, this);
		m_hostiles.erase(pEntity->GetId());
	}
}

 bool CTOSPersonalHostiles::MakeHostile(EntityId one, EntityId two)
{	
	auto result = m_hostiles[one].emplace(two);

	bool maked = result.second;
	if (maked)
		m_pEntitySystem->AddEntityEventListener(one, ENTITY_EVENT_DONE, this);

	return maked;
}

 bool CTOSPersonalHostiles::RemoveHostile(EntityId one, EntityId two)
{
	auto result = m_hostiles[one].erase(two);
	return result > 0;
}

 bool CTOSPersonalHostiles::IsHaveHostiles(EntityId one) 
{
	return m_hostiles[one].size() > 0;
}

 bool CTOSPersonalHostiles::IsHostile(EntityId one, EntityId two)
{
	return m_hostiles[one].find(two) != m_hostiles[one].end();
}