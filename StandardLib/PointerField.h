#ifndef __POINTER_FIELD_H__
#define __POINTER_FIELD_H__
#include "Field.h"


class CPointer;
class CBaseObject;
class CPointerField : public CField
{
CONSTRUCTABLE(CPointerField);
public:
	void		Init(ptrdiff_t iOffset, CClass* pcContainingClass, char* szName);
	void		Kill(void);

	BOOL		IsPointer(void);
	uint32		GetSizeOf(void);
	size_t		GetNameOffset(void);
	CPointer*	GetPointer(CBaseObject* pcFieldContainer);
};


#endif // __POINTER_FIELD_H__

