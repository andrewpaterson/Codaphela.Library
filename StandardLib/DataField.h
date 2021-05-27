#ifndef __DATA_FIELD_H__
#define __DATA_FIELD_H__
#include "Field.h"


//Shouldn't this be called PrimitiveField?
class CDataField : public CField
{
CONSTRUCTABLE(CDataField);
public:
	void Init(CClass* pcFieldClass, ptrdiff_t iOffset, CClass* pcContainingClass, char* szName);
	void Kill(void);
};


#endif // __DATA_FIELD_H__

