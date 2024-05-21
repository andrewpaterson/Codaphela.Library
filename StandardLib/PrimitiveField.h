#ifndef __PRIMITIVE_FIELD_H__
#define __PRIMITIVE_FIELD_H__
#include "Field.h"


class CPrimitiveObject;
class CBaseObject;
class CPrimitiveField : public CField
{
CONSTRUCTABLE(CPrimitiveField);
protected:
	SDataIO*	mpcIO;
	uint32		muiValueOffset;

public:
	void				Init(CClass* pcFieldClass, ptrdiff_t iOffset, ptrdiff_t iValueOffset, CClass* pcContainingClass, SDataIO* pcIO, char* szName);
	void				Kill(void);

	bool				IsPrimitive(void);
	uint32				GetSizeOf(void);
	size				GetNameOffset(void);
	SDataIO*			GetDataIO(void);
	CPrimitiveObject*	GetPrimitiveObject(CBaseObject* pcFieldContainer);
	void*				GetValue(CBaseObject* pcFieldContainer);
};


#endif // __PRIMITIVE_FIELD_H__

