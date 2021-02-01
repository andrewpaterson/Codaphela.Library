#ifndef __NAMED_HOLLOW_OBJECT_H__
#define __NAMED_HOLLOW_OBJECT_H__
#include "HollowObject.h"


class CNamedHollowObject : public CHollowObject
{
CONSTRUCTABLE(CNamedHollowObject);
protected:
	CChars	mon;

public:
	CBaseObject*	Dehollow(void);

	char*			GetName(void);
	BOOL			IsNamed(void);

	BOOL			InitName(char* szName);

protected:
	void				FreeIdentifiers(void);
	CEmbeddedObject*	GetRemappedEmbeddedObject(int iIndex);
	void				ClearName(void);
};


#endif // __NAMED_HOLLOW_OBJECT_H__

