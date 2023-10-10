// AIObject.cpp: implementation of the CAIObject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AIObject.h"
#include "CAIsystem.h"
#include <float.h>
#include <ISystem.h>

#ifdef LINUX
#	include <platform.h>
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAIObject::CAIObject()
	: m_nObjectType(0),
	m_pAssociation(nullptr),
	m_fEyeHeight(0),
	m_bIsBoind(false),
	m_vBoundPosition(0, 0, 0),
	m_DEBUGFLOAT(0),
	m_bForceTargetPos(false),
	m_bNeedsPathOutdoor(true),
	m_bNeedsPathIndoor(true),
	m_fPassRadius(0.6f)
{
	m_bDEBUGDRAWBALLS = false;
	m_bEnabled = true;
	m_bSleeping = false;
	m_pLastNode = nullptr;
	m_pAISystem = nullptr;
	m_bMoving = false;
	m_pProxy = nullptr;
	m_fRadius = 0;
	m_bCloaked = false;
	m_bCanReceiveSignals = true;
}

CAIObject::~CAIObject()
{
	for (const auto pChild : m_lstBindings)
	{
		GetAISystem()->RemoveObject(pChild);
	}
	m_lstBindings.clear();
}

const Vec3& CAIObject::GetPos() const
{
	return m_vPosition;
}

unsigned short CAIObject::GetType() const
{
	return m_nObjectType;
}

void CAIObject::SetType(const unsigned short type)
{
	m_nObjectType = type;
}

void* CAIObject::GetAssociation() const
{
	return m_pAssociation;
}

void CAIObject::SetAssociation(void* pAssociation)
{
	m_pAssociation = pAssociation;
}

void CAIObject::Update()
{
	if (m_pProxy)
		m_pProxy->Update(&m_State);

	if (!m_lstBindings.empty())
		UpdateHierarchy();
}

void CAIObject::UpdateHierarchy() const
{
	// TheOtherSide
	Matrix44 mat = Matrix34::CreateRotationXYZ(static_cast<Ang3>(m_vOrientation), m_vPosition - Vec3(0, 0, m_fEyeHeight));
	mat = GetTransposed44(mat); //TODO: remove this after E3 and use Matrix34 instead of Matrix44

	for (const auto pChild : m_lstBindings)
	{
		//pChild->SetPos(mat.TransformPointOLD(pChild->GetPosBound()));
		pChild->SetPos(mat.TransformPoint(pChild->GetPosBound()));
		//		m_vPosition = mat.TransformPoint(m_vBoundPosition);

		//mat.NoScale();
		mat.SetIdentity();

		CryQuat cxquat = Quat(mat);
		CryQuat rxquat;
		rxquat.SetRotationXYZ(Ang3(0, 0, 0));
		CryQuat result = cxquat * rxquat;

		Ang3 ang1 = Ang3::GetAnglesXYZ(Matrix33(result));
		Vec3 finalangles = static_cast<Vec3>(ang1);
		pChild->SetAngles(RAD2DEG(finalangles));
	}

	// ~TheOtherSide
}

void CAIObject::CreateBoundObject(const unsigned short type, const Vec3& vBindPos, const Vec3& vBindAngl)
{
	CAIObject* pChild = static_cast<CAIObject*>(m_pAISystem->CreateAIObject(type, nullptr));
	string     name;
	char       buffer[5];
	sprintf(buffer, "%d\0", m_lstBindings.size() + 1);
	name = GetName() + string("_bound_") + string(buffer);
	pChild->SetName(name.c_str());
	m_lstBindings.push_back(pChild);
	pChild->SetPosBound(vBindPos);
	UpdateHierarchy();
}

void CAIObject::SetPosBound(const Vec3& pos)
{
	m_vBoundPosition = pos;
}

const Vec3& CAIObject::GetPosBound() const
{
	return m_vBoundPosition;
}

void CAIObject::SetEyeHeight(const float height)
{
	if (_isnan(height))
	{
		AIWarning(" NotANumber set for eye height of AI Object %s", m_sName.c_str());
		return;
	}

	m_fEyeHeight = height;
}

/*
void CAIObject::SetMinAlt(float height)
{
	m_fEyeHeight = height;
}
*/

void CAIObject::ParseParameters(const AIObjectParameters& params)
{
	//	m_fEyeHeight = params.fEyeHeight;
}


bool CAIObject::CanBeConvertedTo(unsigned short type, void** pConverted)
{
	*pConverted = nullptr;
	return false;
}

void CAIObject::SetName(const char* pName)
{
	char str[128];
	strcpy(str, pName);
	int i = 1;
	while (GetAISystem()->GetAIObjectByName(str))
	{
		sprintf(str, "%s_%02d", pName, i);
		i++;
	}
	m_sName = str;
}

const char* CAIObject::GetName() const
{
	return m_sName.c_str();
}

void CAIObject::SetAngles(const Vec3& angles)
{
	int ax = static_cast<int>(angles.x);
	ax /= 360;
	int ay = static_cast<int>(angles.y);
	ay /= 360;
	int az = static_cast<int>(angles.z);
	az /= 360;

	m_vOrientation.x = angles.x - (ax * 360);
	m_vOrientation.y = angles.y - (ay * 360);
	m_vOrientation.z = angles.z - (az * 360);
}

bool CAIObject::IsEnabled() const
{
	return m_bEnabled;
}

void CAIObject::SetEnabled(const bool enable)
{
	m_bEnabled = enable;
}

void CAIObject::SetPos(const Vec3& pos, const Vec3& dirForw)
{
	if (_isnan(pos.x) || _isnan(pos.y) || _isnan(pos.z))
	{
		AIWarning("NotANumber tried to be set for position of AI entity %s", m_sName.c_str());
		return;
	}
	m_vLastPosition = m_vPosition;
	m_vPosition = pos;


	// fixed eyeHeight for vehicles
	// m_fEyeHeight used for other stuff 
	if (GetType() == AIOBJECT_VEHICLE)
	{
		constexpr float vehicleEyeHeight = 2.5f;
		m_vLastPosition.z -= vehicleEyeHeight;

		if (!IsEquivalent(m_vLastPosition, pos, VEC_EPSILON))
			m_bMoving = true;
		else
			m_bMoving = false;

		m_vPosition.z += vehicleEyeHeight;
		return;
	}

	// TheOtherSide
	// Akeeper: Я ниибу зачем это надо, но пусть оно будет включено
	//if (bKeepEyeHeight)
	m_vLastPosition.z -= m_fEyeHeight;

	if (!IsEquivalent(m_vLastPosition, pos, VEC_EPSILON))
		m_bMoving = true;
	else
		m_bMoving = false;

	//if (bKeepEyeHeight)
	m_vPosition.z += m_fEyeHeight;

	// ~TheOtherSide
}

void CAIObject::GetLastPosition(Vec3& pos) const
{
	pos = m_vLastPosition;
}

void CAIObject::SetAISystem(CAISystem* pSystem)
{
	m_pAISystem = pSystem;
}

void CAIObject::Reset(void)
{
	m_pLastNode = nullptr;
}

float CAIObject::GetEyeHeight(void) const
{
	return m_fEyeHeight;
}

// returns the state of this object
SOBJECTSTATE* CAIObject::GetState(void)
{
	return &m_State;
}

// nSignalID = 73 allow duplicating signals
//
void CAIObject::SetSignal(const int nSignalID, const char* szText, IEntity* pSender /*= nullptr*/)
{
	if (nSignalID != 10)
	{
		// TheOtherSide
		std::vector<AISIGNAL>::iterator ai;
		for (const auto& aisignal : m_State.vSignals)
		{
			if (!stricmp((aisignal).strText, szText))
				return;
		}

		//for (ai = m_State.vSignals.begin(); ai != m_State.vSignals.end(); ++ai)
		//{
		//	if (!stricmp((*ai).strText, szText))
		//		return;
		//}

		// ~TheOtherSide
	}
	if (!stricmp(szText, "wakeup"))
		Event(AIEVENT_WAKEUP, nullptr);
	//		return;

	if (!m_bEnabled && (nSignalID != 0))
		return;

	if ((nSignalID >= 0) && !m_bCanReceiveSignals)
		return;


	//GetAISystem()->m_pSystem->GetILog()->Log("\001 ENEMY %s processing signal %s.",m_sName.c_str(),szText);

	AISIGNAL signal;
	signal.nSignal = nSignalID;
	//signal.strText = szText;
	strcpy(signal.strText, szText);
	signal.pSender = pSender;

	m_State.vSignals.push_back(signal);


	//	m_bSleeping = false;
	//	m_bEnabled = true;

	/*
	if (m_State.nSignal == 0) 
	{
		m_State.nSignal = nSignalID;
		m_State.szSignalText = szText;
	}s
	else
	{
		int a=1;
	}
	*/
}


void CAIObject::EDITOR_DrawRanges(const bool bEnable)
{
	m_bDEBUGDRAWBALLS = bEnable;
}

void CAIObject::SetRadius(const float fRadius)
{
	m_fRadius = fRadius;
}

void CAIObject::Save(CStream& stm) {}

void CAIObject::Load(CStream& stm) {}
