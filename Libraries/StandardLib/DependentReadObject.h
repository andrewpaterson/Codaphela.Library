#ifndef __DEPENDENT_READ_OBJECT_H__
#define __DEPENDENT_READ_OBJECT_H__
#include "BaseLib/ArrayTemplate.h"
#include "ObjectHeader.h"


#define DEPENDENT_READ_OBJECT_FLAG_READ		0x01
#define DEPENDENT_READ_OBJECT_FLAG_EXISTED	0x02


class CBaseObject;
class CDependentReadObject : public CObjectIdentifier
{
protected:
	uint16	muiFlags;

public:
	void	Init(CObjectIdentifier* pcObjectIdentifier);
	void	Kill(void);

	char*	GetName(void);
	OIndex	GetOldIndex(void);
	void	SetRead(void);
	void	SetExisting(void);
	bool	IsRead(void);
	bool	ExistsInObjects(void);
};




typedef CArrayTemplate<CDependentReadObject> CArrayDependentReadObject;


#endif // __DEPENDENT_READ_OBJECT_H__

