#ifndef __UNMANAGED_FIELD_H__
#define __UNMANAGED_FIELD_H__
#include "Field.h"


class CBaseObject;
class CUnmanagedField : public CField
{
CONSTRUCTABLE(CUnmanagedField);
protected:
	size_t		muiLength;
	uint32		muiElementSize;
	SDataIO*	mpcIO;

public:
	void		Init(EPrimitiveType eType, ptrdiff_t iOffset, CClass* pcContainingClass, SDataIO* pcIO, char* szName);
	void		Init(EPrimitiveType eType, ptrdiff_t iOffset, CClass* pcContainingClass, size_t uiLength, SDataIO* pcIO, char* szName);
	void		Init(EPrimitiveType eType, ptrdiff_t iOffset, CClass* pcContainingClass, uint32 uiElementSize, size_t uiLength, SDataIO* pcIO, char* szName);
	void		Kill(void);

	BOOL		IsArray(void);
	size_t		GetLength(void);
	BOOL		IsUnmanaged(void);
	void*		GetData(CBaseObject* pcFieldContainer);
	uint32		GetSizeOf(void);
	size_t		GetNameOffset(void);
	SDataIO*	GetDataIO(void);
};


#endif // __UNMANAGED_FIELD_H__

