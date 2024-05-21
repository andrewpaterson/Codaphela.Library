#ifndef __UNMANAGED_FIELD_H__
#define __UNMANAGED_FIELD_H__
#include "Field.h"


class CBaseObject;
class CUnmanagedField : public CField
{
CONSTRUCTABLE(CUnmanagedField);
protected:
	size		muiLength;
	uint32		muiElementSize;

public:
	void		Init(EPrimitiveType eType, ptrdiff_t iOffset, CClass* pcContainingClass, char* szName);
	void		Init(EPrimitiveType eType, ptrdiff_t iOffset, CClass* pcContainingClass, size uiLength, char* szName);
	void		Init(EPrimitiveType eType, ptrdiff_t iOffset, CClass* pcContainingClass, uint32 uiElementSize, size uiLength, char* szName);
	void		Kill(void);

	bool		IsArray(void);
	size		GetLength(void);
	bool		IsUnmanaged(void);
	void*		GetData(CBaseObject* pcFieldContainer);
	uint32		GetSizeOf(void);
	size		GetNameOffset(void);
};


#endif // __UNMANAGED_FIELD_H__

