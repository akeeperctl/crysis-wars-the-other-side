/*************************************************************************
Crysis Co-op Source File.
Copyright (C), Crysis Co-op
**************************************************************************/

#include <StdAfx.h>
#include "AnimationGraphState.h"
#include "../TOSActor.h"

// Forward to owner actor.

bool CAnimationGraphState::SetInput(const InputID id, const float value, TAnimationGraphQueryID* pQueryID)
{
	const bool bSucceeded = m_pAnimationGraphState->SetInput(id, value, pQueryID);
	if (gEnv->bServer)
		m_pOwner->OnAGSetInput(bSucceeded, id, value, pQueryID);

	return bSucceeded;
}

bool CAnimationGraphState::SetInput(const InputID id, const int value, TAnimationGraphQueryID* pQueryID)
{
	const bool bSucceeded = m_pAnimationGraphState->SetInput(id, value, pQueryID);
	if (gEnv->bServer)
		m_pOwner->OnAGSetInput(bSucceeded, id, value, pQueryID);

	return bSucceeded;
}

bool CAnimationGraphState::SetInput(const InputID id, const char* value, TAnimationGraphQueryID* pQueryID)
{
	const bool bSucceeded = m_pAnimationGraphState->SetInput(id, value, pQueryID);
	if (gEnv->bServer)
		m_pOwner->OnAGSetInput(bSucceeded, id, value, pQueryID);

	return bSucceeded;
};
