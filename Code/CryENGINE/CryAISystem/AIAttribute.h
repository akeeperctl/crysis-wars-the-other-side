#ifndef _AI_ATTRIBUTE_
#define _AI_ATTRIBUTE_

#include "aiobject.h"

class CAIAttribute : public CAIObject
{

	// this attribute is attributed to this REAL object
	CAIObject *m_pPrincipalObject;

public:
	CAIAttribute(void);
	~CAIAttribute(void) override;

	void        ParseParameters( const AIObjectParameters & params) override;
	void        Update() override;
	void        Event(unsigned short eType, SAIEVENT * pEvent) override;
	bool        CanBeConvertedTo(unsigned short type, void ** pConverted) override;
	void        OnObjectRemoved(CAIObject * pObject) override;
	CAIObject * GetPrincipalObject(void);

	void Bind(IAIObject* bind) override
	{ m_pPrincipalObject = (CAIObject*) bind; }
	void Unbind() override
	{ m_pPrincipalObject = nullptr;	}
};

#endif