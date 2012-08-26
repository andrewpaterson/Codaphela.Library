#ifndef __DEPENDENT_READ_OBJECT_H__
#define __DEPENDENT_READ_OBJECT_H__
#include "BaseLib/ArrayTemplate.h"
#include "ObjectHeader.h"


class CBaseObject;
class CDependentReadObject : public CPointerHeader
{
public:
	BOOL			mbRead;

	void	Init(CPointerHeader* pcObjectPtr);
	void	Kill(void);

	BOOL	IsNamed(void);
	char*	GetName(void);
	OIndex	GetIndex(void);
};


int CompareDependentReadObject(const void* ps1, const void* ps2);


typedef CArrayTemplate<CDependentReadObject> CArrayDependentReadObject;


#endif // __DEPENDENT_READ_OBJECT_H__

