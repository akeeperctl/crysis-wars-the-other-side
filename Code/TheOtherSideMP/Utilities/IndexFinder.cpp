/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#include "StdAfx.h"
#include "IndexFinder.h"

// In the cpp file
IndexFinder::fake_vtable_t IndexFinder::fake_vtable = {
	&IndexFinder::method0 ,
	&IndexFinder::method1 ,
	&IndexFinder::method2 ,
	&IndexFinder::method3 ,
	&IndexFinder::method4 ,
	&IndexFinder::method5 ,
	&IndexFinder::method6 ,
	&IndexFinder::method7 ,
	&IndexFinder::method8 ,
	&IndexFinder::method9 ,
	&IndexFinder::method10,
	&IndexFinder::method11,
	&IndexFinder::method12,
	&IndexFinder::method13,
	&IndexFinder::method14,
	&IndexFinder::method15,
	&IndexFinder::method16,
	&IndexFinder::method17,
	&IndexFinder::method18,
	&IndexFinder::method19,
	&IndexFinder::method20,
	&IndexFinder::method21,
	&IndexFinder::method22,
	&IndexFinder::method23,
	&IndexFinder::method24,
	&IndexFinder::method25,
	&IndexFinder::method26,
	&IndexFinder::method27,
	&IndexFinder::method28,
	&IndexFinder::method29,
	&IndexFinder::method30,
	&IndexFinder::method31,
	&IndexFinder::method32,
	&IndexFinder::method33,
	&IndexFinder::method34,
	&IndexFinder::method35,
	&IndexFinder::method36,
	&IndexFinder::method37,
	&IndexFinder::method38,
	&IndexFinder::method39,
	&IndexFinder::method40,
	&IndexFinder::method41,
	&IndexFinder::method42,
	&IndexFinder::method43,
	&IndexFinder::method44,
	&IndexFinder::method45,
	&IndexFinder::method46,
	&IndexFinder::method47,
	&IndexFinder::method48,
	&IndexFinder::method49,
	&IndexFinder::method50,
	&IndexFinder::method51,
	&IndexFinder::method52,
	&IndexFinder::method53,
	&IndexFinder::method54,
	&IndexFinder::method55,
	&IndexFinder::method56,
	&IndexFinder::method57,
	&IndexFinder::method58,
	&IndexFinder::method59,
	&IndexFinder::method60,
	&IndexFinder::method61,
	&IndexFinder::method62,
	&IndexFinder::method63,
	&IndexFinder::method64,
	&IndexFinder::method65,
	&IndexFinder::method66,
	&IndexFinder::method67,
	&IndexFinder::method68,
	&IndexFinder::method69,
	&IndexFinder::method70,
	&IndexFinder::method71,
	&IndexFinder::method72,
	&IndexFinder::method73,
	&IndexFinder::method74,
	&IndexFinder::method75,
	&IndexFinder::method76,
	&IndexFinder::method77,
	&IndexFinder::method78,
	&IndexFinder::method79,
	&IndexFinder::method80,
	&IndexFinder::method81,
	&IndexFinder::method82,
	&IndexFinder::method83,
	&IndexFinder::method84,
	&IndexFinder::method85,
	&IndexFinder::method86,
	&IndexFinder::method87,
	&IndexFinder::method88,
	&IndexFinder::method89,
	&IndexFinder::method90,
	&IndexFinder::method91,
	&IndexFinder::method92,
	&IndexFinder::method93,
	&IndexFinder::method94,
	&IndexFinder::method95,
	&IndexFinder::method96,
	&IndexFinder::method97,
	&IndexFinder::method98,
	&IndexFinder::method99,
	&IndexFinder::method100,
	&IndexFinder::method101,
	&IndexFinder::method102,
	&IndexFinder::method103,
	&IndexFinder::method104,
	&IndexFinder::method105,
	&IndexFinder::method106,
	&IndexFinder::method107,
	&IndexFinder::method108,
	&IndexFinder::method109,
	&IndexFinder::method110,
	&IndexFinder::method111,
	&IndexFinder::method112,
	&IndexFinder::method113,
	&IndexFinder::method114,
	&IndexFinder::method115,
	&IndexFinder::method116,
	&IndexFinder::method117,
	&IndexFinder::method118,
	&IndexFinder::method119,
	&IndexFinder::method120,
	&IndexFinder::method121,
	&IndexFinder::method122,
	&IndexFinder::method123,
	&IndexFinder::method124,
	&IndexFinder::method125,
	&IndexFinder::method126,
	&IndexFinder::method127,
	&IndexFinder::method128,
	&IndexFinder::method129,
	&IndexFinder::method130,
	&IndexFinder::method131,
	&IndexFinder::method132,
	&IndexFinder::method133,
	&IndexFinder::method134,
	&IndexFinder::method135,
	&IndexFinder::method136,
	&IndexFinder::method137,
	&IndexFinder::method138,
	&IndexFinder::method139,
	&IndexFinder::method140,
	&IndexFinder::method141,
	&IndexFinder::method142,
	&IndexFinder::method143,
	&IndexFinder::method144,
	&IndexFinder::method145,
	&IndexFinder::method146,
	&IndexFinder::method147,
	&IndexFinder::method148,
	&IndexFinder::method149,
	&IndexFinder::method150,
	&IndexFinder::method151,
	&IndexFinder::method152,
	&IndexFinder::method153,
	&IndexFinder::method154,
	&IndexFinder::method155,
	&IndexFinder::method156,
	&IndexFinder::method157,
	&IndexFinder::method158,
	&IndexFinder::method159,
	&IndexFinder::method160,
	&IndexFinder::method161,
	&IndexFinder::method162,
	&IndexFinder::method163,
	&IndexFinder::method164,
	&IndexFinder::method165,
	&IndexFinder::method166,
	&IndexFinder::method167,
	&IndexFinder::method168,
	&IndexFinder::method169,
	&IndexFinder::method170,
	&IndexFinder::method171,
	&IndexFinder::method172,
	&IndexFinder::method173,
	&IndexFinder::method174,
	&IndexFinder::method175,
	&IndexFinder::method176,
	&IndexFinder::method177,
	&IndexFinder::method178,
	&IndexFinder::method179,
	&IndexFinder::method180,
	&IndexFinder::method181,
	&IndexFinder::method182,
	&IndexFinder::method183,
	&IndexFinder::method184,
	&IndexFinder::method185,
	&IndexFinder::method186,
	&IndexFinder::method187,
	&IndexFinder::method188,
	&IndexFinder::method189,
	&IndexFinder::method190,
	&IndexFinder::method191,
	&IndexFinder::method192,
	&IndexFinder::method193,
	&IndexFinder::method194,
	&IndexFinder::method195,
	&IndexFinder::method196,
	&IndexFinder::method197,
	&IndexFinder::method198,
	&IndexFinder::method199,
	&IndexFinder::method200,
};

void* IndexFinder::fake_vtable_ptr = &IndexFinder::fake_vtable;

//inline void InitCanAcquireTargetHook(CAIActor* pCAIActor, IAIActor* pIAIActor, IAIObject* pAI)
//{
//	auto pCAIObject = static_cast<CAIObject*>(pAI);
//
//	unsigned int aiVTableAdress = 0x000000003122bda0;
//	auto pAIVTable1 = reinterpret_cast<void**>(aiVTableAdress); // 0x000000003122bda0
//	void** pAIVTable2 = *reinterpret_cast<void***>(pCAIObject); // 0x000000003122bda0
//	void** pCAIActorVTable = *reinterpret_cast<void***>(pCAIActor); // 0x000000003122bda0
//	void** pIAIActorVTable = *reinterpret_cast<void***>(pIAIActor); // 0x0000000041acae44
//	void** pAISystemVTable = *reinterpret_cast<void***>(gEnv->pAISystem); // 0x000000003121c290
//
//	// IAIObject::IsHostile 35
//	// IAISystem::ExecuteAIAction 146
//	// CAIActor::IsObserver 14
//
//	auto pFuncPointer = &CAIObject::IsObservable;
//	int index = IndexFinder::getIndexOf(pFuncPointer);
//
//	auto pVTableFuncPointer = reinterpret_cast<IsObservableFunc&>(pAIVTable2[index]);
//	bool value = (pCAIObject->*pVTableFuncPointer)();
//
//	CallVTableFunction<bool, IsObservableFunc>(pAIVTable1, 1, pCAIObject);
//	
//	//auto pVTableFunctionPointer = reinterpret_cast<AllocGoalPipeIdFunc>(pAISystemVTable[index]);
//	//auto pVTableFunctionPointer = reinterpret_cast<AllocGoalPipeIdFunc*>(&pAISystemVTable[index]);
//	//auto pVTableFunctionPointer = reinterpret_cast<AllocGoalPipeIdFunc>(pAISystemVTable[index]);
//	//int result = pVTableFunctionPointer(gEnv->pAISystem);
//
//	// index of IAIActor::CanAcquireTarget
//	//constexpr unsigned int FUNC_INDEX = 11;
//
//	// vtable hook
//	//CanAcquireTargetFunc newFunc = &AIActorHook::CanAcquireTarget;
//	//WinAPI::FillMem(&pAIActorVTable[FUNC_INDEX], &reinterpret_cast<void*&>(newFunc), sizeof(void*));
//}

