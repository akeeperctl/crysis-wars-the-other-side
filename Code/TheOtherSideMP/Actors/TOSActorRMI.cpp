#include "StdAfx.h"
#include "TOSActor.h"
#include <TheOtherSideMP/Helpers/TOS_Entity.h>
#include <Fists.h>

IMPLEMENT_RMI(CTOSActor, SvRequestPlayAnimation)
{
	// Описываем здесь всё, что будет выполняться на сервере

	GetGameObject()->InvokeRMI(ClPlayAnimation(), params, eRMI_ToRemoteClients);

	return true;
}

IMPLEMENT_RMI(CTOSActor, ClPlayAnimation)
{
	// Описываем здесь всё, что будет выполняться на клиенте

	IAnimationGraphState* pGraphState = (GetAnimatedCharacter() ? GetAnimatedCharacter()->GetAnimationGraphState() : nullptr);
	string mode;

	if (pGraphState)
	{
		if (params.mode == AIANIM_SIGNAL)
		{
			mode = "Signal";
		}
		else if (params.mode == AIANIM_ACTION)
		{
			mode = "Action";
		}

		pGraphState->SetInput(mode.c_str(), params.animation.c_str());
	}

	//CryLogAlways("[C++][%s][%s][%s] mode = %s, animation = %s", 
	//	TOS_Debug::GetEnv(), 
	//	TOS_Debug::GetAct(3), 
	//	__FUNCTION__, 
	//	mode.c_str(), params.animation.c_str());

	return true;
}

//IMPLEMENT_RMI(CTOSActor, SvRequestMarkMeAsMaster)
//{
//	// Описываем здесь всё, что будет выполняться на сервере
//
//	m_isMaster = params.value;
//	GetGameObject()->InvokeRMI(ClMarkMeAsMaster(), params, eRMI_ToAllClients);
//
//	GetGameObject()->ChangedNetworkState(TOS_NET::SERVER_ASPECT_STATIC);
//
//	CryLog("[C++][%s][%s][%s][%s] mark as master = %i",
//		TOS_Debug::GetEnv(),
//		TOS_Debug::GetAct(3),
//		__FUNCTION__,
//		m_debugName,
//		params.value);
//
//
//	return true;
//}
//
//IMPLEMENT_RMI(CTOSActor, ClMarkMeAsMaster)
//{
//	// Описываем здесь всё, что будет выполняться на клиенте
//
//	m_isMaster = params.value;
//
//	CryLog("[C++][%s][%s][%s][%s] mark as master = %i",
//		TOS_Debug::GetEnv(),
//		TOS_Debug::GetAct(3),
//		__FUNCTION__,
//		m_debugName,
//		params.value);
//
//	return true;
//}
//
//IMPLEMENT_RMI(CTOSActor, SvRequestMarkMeAsSlave)
//{
//	// Описываем здесь всё, что будет выполняться на сервере
//
//	m_isSlave = params.value;
//
//	GetGameObject()->ChangedNetworkState(TOS_NET::SERVER_ASPECT_STATIC);
//
//	CryLog("[C++][%s][%s][%s][%s] mark as slave = %i",
//		TOS_Debug::GetEnv(),
//		TOS_Debug::GetAct(3),
//		__FUNCTION__,
//		m_debugName,
//		params.value);
//	
//
//	return true;
//}
//
//IMPLEMENT_RMI(CTOSActor, ClMarkMeAsSlave)
//{
//	// Описываем здесь всё, что будет выполняться на клиенте
//
//	m_isSlave = params.value;
//
//	CryLog("[C++][%s][%s][%s][%s] mark as slave = %i",
//		TOS_Debug::GetEnv(),
//		TOS_Debug::GetAct(3),
//		__FUNCTION__,
//		m_debugName,
//		params.value);
//
//	return true;
//}

IMPLEMENT_RMI(CTOSActor, SvRequestHideMe)
{
	// Описываем здесь всё, что будет выполняться на сервере

	// 13.01.2024 Akeeper: Не уверен на счёт этих строк, но пусть они тут будут (519-521)
	const auto* pFists = static_cast<CFists*>(GetItemByClass(CItem::sFistsClass));
	if (pFists)
		g_pGame->GetIGameFramework()->GetIItemSystem()->SetActorItem(this, pFists->GetEntityId());

	GetGameObject()->SetAspectProfile(eEA_Physics, GetSpectatorMode() != 0 || params.hide ? eAP_Spectator : eAP_Alive);
	GetGameObject()->InvokeRMI(ClMarkHideMe(), params, eRMI_ToAllClients);

	return true;
}

IMPLEMENT_RMI(CTOSActor, ClMarkHideMe)
{
	// Описываем здесь всё, что будет выполняться на клиенте

	HideMe(params.hide);
	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSActor, ClTOSJump)
{
	CMovementRequest request;
	request.SetJump();
	GetMovementController()->RequestMovement(request);

	CryLog("[%s] Received jump", m_debugName);
	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSActor, SvRequestTOSJump)
{
	auto channelId = g_pGame->GetIGameFramework()->GetGameChannelId(pNetChannel);
	GetGameObject()->InvokeRMI(ClTOSJump(), params, eRMI_ToOtherClients | eRMI_NoLocalCalls, channelId);
	GetGameObject()->Pulse('bang');

	if ((IsSlave() && !IsLocalSlave()) || IsClient())
	{
		CMovementRequest request;
		request.SetJump();
		GetMovementController()->RequestMovement(request);

		CryLog("[%s] Requesting jump to channel %i", m_debugName, channelId);
	}

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSActor, ClAttachChild)
{
	IEntity* pChild = TOS_GET_ENTITY(params.id);
	if (pChild)
	{
		GetEntity()->AttachChild(pChild, params.flags);
		CryLog("[%s] Received attach child '%s'", m_debugName, pChild->GetName());
	}

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSActor, SvRequestAttachChild)
{
	IEntity* pChild = TOS_GET_ENTITY(params.id);
	if (pChild)
	{
		CryLog("[%s] Requesting attach child '%s'", m_debugName, pChild->GetName());
		GetEntity()->AttachChild(pChild, params.flags);
	}

	GetGameObject()->InvokeRMI(ClAttachChild(), params, eRMI_ToRemoteClients);

	return true;
}

//------------------------------------------------------------------------
IMPLEMENT_RMI(CTOSActor, ClClearInventory)
{
	CryLog("[C++][%s][%s][ClClearInventory]",
		TOS_Debug::GetEnv(), TOS_Debug::GetAct(3));

	RemoveAllItems();
	return true;
}