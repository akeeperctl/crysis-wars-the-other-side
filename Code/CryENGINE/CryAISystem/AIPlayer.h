#ifndef _AI_PLAYER_
#define _AI_PLAYER_

#include "aiobject.h"

class CAIPlayer :	public CAIObject
{

	float m_fMaxPerception;
	float m_fLastPerceptionSnapshot;
	IUnknownProxy *m_pProxy;

	bool m_bSendPerceptionResetNotification;

public:
	CAIPlayer(void);
	~CAIPlayer(void) override;
	void ParseParameters(const AIObjectParameters & params) override;
	bool CanBeConvertedTo(unsigned short type, void ** pConverted) override;

	AgentParameters m_Parameters;
	void            RegisterPerception(float fValue);
	void            SetPerception(float fValue);
	float           GetPerception(void);
	void            SnapshotPerception(void);
	IUnknownProxy*  GetProxy() override
	{ return m_pProxy; };

};

#endif