#ifndef __PRIMITIVE_FIELD_H__
#define __PRIMITIVE_FIELD_H__
#include "Field.h"


class CPrimitiveObject;
class CBaseObject;
class CPrimitiveField : public CField
{
CONSTRUCTABLE(CPrimitiveField)
protected:
	SDataIO*	mpcIO;
	size		muiValueOffset;

public:
	void				Init(CClass* pcFieldClass, ptrdiff_t iOffset, ptrdiff_t iValueOffset, CClass* pcContainingClass, SDataIO* pcIO, const char* szName);
	void				Kill(void);

	bool				IsPrimitive(void);
	size				GetSizeOf(void);
	size				GetNameOffset(void);
	SDataIO*			GetDataIO(void);
	CPrimitiveObject*	GetPrimitiveObject(CBaseObject* pcFieldContainer);
	void*				GetValue(CBaseObject* pcFieldContainer);
};


#endif // __PRIMITIVE_FIELD_H__

