/////////////////////////////////////////////////////////////////
// Copyright (C), RenEvo Software & Designs, 2008
// FGPlugin Source File
//
// IGameFlashAnimation.h
//
// Purpose: Interface to describe a flash animation object
//
// History:
//	- 3/29/09 : File created - KAK
/////////////////////////////////////////////////////////////////

#ifndef _IGAMEFLASHANIMATION_H_
#define _IGAMEFLASHANIMATION_H_

#include "IFlashPlayer.h"

struct SFlashVarValue;

enum EFlashDock
{
	eFD_Stretch			= (1 << 0),
	eFD_Center			= (1 << 1),
	eFD_Left			= (1 << 3),
	eFD_Right			= (1 << 4)
};

enum EFlashAnimFlags
{
	eFAF_ThisHandler	= (1 << 5),
	eFAF_Visible		= (1 << 6),
	eFAF_ManualRender	= (1 << 7),
	eFAF_Default		= (eFAF_ThisHandler|eFAF_Visible)
};

struct IGameFlashAnimation
{
	virtual ~IGameFlashAnimation() {}

	// these functions act on the flash player
	virtual void SetVisible(bool visible) = 0;
	virtual bool GetVisible() const = 0;
	virtual bool IsAvailable(const char* pPathToVar) const = 0;
	virtual bool SetVariable(const char* pPathToVar, const SFlashVarValue& value) = 0;
	virtual bool CheckedSetVariable(const char* pPathToVar, const SFlashVarValue& value) = 0;
	virtual bool Invoke(const char* pMethodName, const SFlashVarValue* pArgs, unsigned int numArgs, SFlashVarValue* pResult = 0) = 0;
	virtual bool CheckedInvoke(const char* pMethodName, const SFlashVarValue* pArgs, unsigned int numArgs, SFlashVarValue* pResult = 0) = 0;
	virtual bool SetFSCommandHandler(IFSCommandHandler* pHandler) = 0;
	virtual void Advance( float deltaTime ) = 0;
	virtual void Render() = 0;

	// invoke helpers
	virtual bool Invoke(const char* pMethodName, SFlashVarValue* pResult = 0) = 0;
	virtual bool Invoke(const char* pMethodName, const SFlashVarValue& arg, SFlashVarValue* pResult = 0) = 0;
	virtual bool CheckedInvoke(const char* pMethodName, SFlashVarValue* pResult = 0) = 0;
	virtual bool CheckedInvoke(const char* pMethodName, const SFlashVarValue& arg, SFlashVarValue* pResult = 0) = 0;

	virtual void Init(const char *strFileName, EFlashDock docking = eFD_Center, uint32 flags = eFAF_Default) = 0;
	virtual bool Load(const char *strFileName, EFlashDock docking = eFD_Center, uint32 flags = eFAF_Default) = 0;
	virtual bool Reload(bool forceUnload=false) = 0;
	virtual void Unload() = 0;
	virtual void RepositionDock(EFlashDock docking) = 0;
	virtual uint32 GetFlags() const = 0;
	virtual void AddVariable(const char *strControl,const char *strVariable,	const char *strToken,float fScale,float fOffset) = 0;
	virtual void ReInitVariables() = 0;
};

#endif //_IGAMEFLASHANIMATION_H_
