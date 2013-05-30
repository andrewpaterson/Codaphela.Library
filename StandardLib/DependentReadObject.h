#ifndef __DEPENDENT_READ_OBJECT_H__
#define __DEPENDENT_READ_OBJECT_H__
#include "BaseLib/ArrayTemplate.h"
#include "ObjectHeader.h"


#define DEPENDENT_READ_OBJECT_FLAG_READ		0x01
#define DEPENDENT_READ_OBJECT_FLAG_EXISTED	0x02


class CBaseObject;
class CDependentReadObject : public CPointerHeader
{
protected:
	BOOL	miFlags;

public:
	void	Init(CPointerHeader* pcObjectPtr);
	void	Kill(void);

	char*	GetName(void);
	OIndex	GetOldIndex(void);
	void	SetRead(void);
	void	SetExisting(void);
	BOOL	IsRead(void);
	BOOL	PreExisted(void);
};


int CompareDependentReadObject(const void* ps1, const void* ps2);


typedef CArrayTemplate<CDependentReadObject> CArrayDependentReadObject;


#endif // __DEPENDENT_READ_OBJECT_H__

