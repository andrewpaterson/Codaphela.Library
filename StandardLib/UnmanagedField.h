#ifndef __UNMANAGED_FIELD_H__
#define __UNMANAGED_FIELD_H__
#include "Field.h"


class CUnmanagedField : public CField
{
CONSTRUCTABLE(CUnmanagedField);
public:
	void Init(EPrimitiveType eType, ptrdiff_t iOffset, CClass* pcContainingClass, char* szName);
	void Kill(void);
};


#endif // __UNMANAGED_FIELD_H__

