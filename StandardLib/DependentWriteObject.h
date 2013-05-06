#ifndef __DEPENDENT_WRITE_OBJECT_H__
#define __DEPENDENT_WRITE_OBJECT_H__
#include "BaseLib/ArrayTemplate.h"


class CBaseObject;
class CDependentWriteObject
{
public:
	CBaseObject*	mpcObject;
	BOOL			mbWritten;

	void Init(CBaseObject* pcObject, BOOL bWritten);
};


int CompareDependentWriteObject(const void* ps1, const void* ps2);


typedef CArrayTemplate<CDependentWriteObject>	CArrayDependentWriteObject;



#endif // __DEPENDENT_WRITE_OBJECT_H__

