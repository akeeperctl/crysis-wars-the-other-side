#include "StdAfx.h"
#include "ISerialize.h"
#include "SquadSystem.h"

//COrder::COrder()
//{
//	Reset();
//}
//
//void COrder::Reset()
//{
//	m_processedPos.zero();
//	m_processedId = 0;
//	m_type = eSO_GoTo;
//	m_gotoState = eGUS_Guard;
//}
//
//void COrder::Serialize(TSerialize ser)
//{
//	SER_VALUE(m_processedPos);
//	SER_VALUE(m_processedId);
//	SER_VALUE_ENUM(m_type,eSO_GoTo,eSO_ForSync);
//	SER_VALUE_ENUM(m_gotoState, eGUS_CleanAI, eGUS_ForSync);
//}
//
//void COrder::GetMemoryStatistics(ICrySizer* s)
//{
//	s->Add(*this);
//}