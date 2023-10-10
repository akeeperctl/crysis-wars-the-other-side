	// CryAISystem.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "DllMain.h"
#include "aisystem.h"

#if defined(WIN32) && defined(_DEBUG) 
#include <crtdbg.h> 
#define DEBUG_NEW_NORMAL_CLIENTBLOCK(file, line) new(_NORMAL_BLOCK, file, line) 
#define new DEBUG_NEW_NORMAL_CLIENTBLOCK( __FILE__, __LINE__) 
#endif

// TheOtherSide
//#ifndef _XBOX
////#if !defined(LINUX)
//_ACCESS_POOL;
////#endif//LINUX 
//#endif //_XBOX
// ~TheOtherSide

// TheOtherSide
// Уже определен в ISystem
//////////////////////////////////////////////////////////////////////////
// Pointer to Global ISystem.
//static ISystem* gISystem = nullptr;
//ISystem* GetISystem()
//{
//	return gISystem;
//}
//////////////////////////////////////////////////////////////////////////
// ~TheOtherSide

#if !defined( _XBOX ) && !defined( PS2 ) && !defined( LINUX )
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved )
{
    return TRUE;
}
#endif

struct ISystem;

#ifndef _XBOX
CRYAIAPI IAISystem *CreateAISystem( ISystem *pSystem)
#else
IAISystem *CreateAISystem( ISystem *pSystem)
#endif
{
	//gISystem = pSystem;
	return (gEnv->pAISystem = new CAISystem(pSystem));
}

//! Reports an AI Warning to validator with WARNING severity.
//void AIWarning( const char *format,... )
//{
//	char buffer[MAX_WARNING_LENGTH];
//	va_list args;
//	va_start(args, format);
//	vsprintf(buffer, format, args);
//	va_end(args);
//	GetAISystem()->m_pSystem->Warning( VALIDATOR_MODULE_AI,VALIDATOR_WARNING,VALIDATOR_FLAG_AI,nullptr,"%s",buffer );
//}
//
////! Reports an AI Warning to validator with ERROR severity.
//void AIError( const char *format,... )
//{
//	char buffer[MAX_WARNING_LENGTH];
//	va_list args;
//	va_start(args, format);
//	vsprintf(buffer, format, args);
//	va_end(args);
//	GetAISystem()->m_pSystem->Warning( VALIDATOR_MODULE_AI,VALIDATOR_ERROR,VALIDATOR_FLAG_AI,nullptr,"%s",buffer );
//}
//////////////////////////////////////////////////////////////////////////

#include <CrtDebugStats.h>
