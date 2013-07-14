#ifndef __NAMED_HOLLOW_OBJECT_H__
#define __NAMED_HOLLOW_OBJECT_H__
#include "HollowObject.h"


class CNamedHollowObject : public CHollowObject
{
BASE_FUNCTIONS(CNamedHollowObject);
protected:
	CChars	mon;

public:
	void			Kill(void);
	CBaseObject*	Dehollow(void);

	char*			GetName(void);
	BOOL			IsNamed(void);

	BOOL			InitName(char* szName);

protected:
	CEmbeddedObject*	GetRemappedEmbeddedObject(int iIndex);
};


#endif // __NAMED_HOLLOW_OBJECT_H__

