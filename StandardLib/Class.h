#ifndef __CLASS_H__
#define __CLASS_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/ArrayVoidPtr.h"
#include "BaseLib/ArrayTemplateEmbedded.h"
#include "BaseLib/LinkedListBlock.h"
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/Number.h"
#include "BaseLib/Date.h"
#include "PointerField.h"
#include "EmbeddedObjectField.h"
#include "DataField.h"
#include "UnmanagedField.h"


#define CLASS_FLAGS_PRIMITIVE	0x01
#define CLASS_FLAGS_COMPLETE	0x02


class CPointer;
class CBaseObject;
class CPrimitiveObject;
class CClasses;
class CClass : public CPostMalloc<CField>
{
protected:
	CChars				mszClassName;
	uint32				muiSize;
	EPrimitiveType		meType;
	uint16				muiFlags;
	CClasses*			mpcClassesThisIn;

	CLinkedListBlock	macFields;

	CArrayVoidPtr		mapcPointers;
	CArrayVoidPtr		mapcEmbeddedObjects;
	CArrayVoidPtr		mapcDatas;
	CArrayVoidPtr		mapcUnmanaged;

public:
	void			Init(char* szClassName, uint32 uiSize, EPrimitiveType eType, CClasses* pcClasses);
	void			Kill(void);

	void			Complete(void);

	void			Pointer(CBaseObject* pcThis, CPointer* pcPointer, char* szName);
	void			Primitive(CBaseObject* pcThis, CPrimitiveObject* pcPrimitive, char* szName);
	void			Embedded(CBaseObject* pcThis, CBaseObject* pcObject, char* szName);

	CClasses*		GetClasses(void);
	char*			GetName(void);
	EPrimitiveType	GetType(void);

	BOOL			IsComplete(void);
	void			Primitive(void);
	BOOL			IsPrimitive(void);

	CClass*			GetClass(EPrimitiveType eType);

	void			Unmanaged(CBaseObject* pcThis, int* pi, char* szFieldName);
	void			Unmanaged(CBaseObject* pcThis, int* pai, size_t uiLength, char* szFieldName);
	void			Unmanaged(CBaseObject* pcThis, float* pf, char* szFieldName);
	void			Unmanaged(CBaseObject* pcThis, float* paf, size_t uiLength, char* szFieldName);
	void			Unmanaged(CBaseObject* pcThis, char* pc, char* szFieldName);
	void			Unmanaged(CBaseObject* pcThis, char* pac, size_t uiLength, char* szFieldName);

	void			Unmanaged(CBaseObject* pcThis, CChars* pcChars, char* szFieldName);
	void			Unmanaged(CBaseObject* pcThis, CNumber* pcNumber, char* szFieldName);
	void			Unmanaged(CBaseObject* pcThis, CDate* pcDate, char* szFieldName);

	void			Unmanaged(CBaseObject* pcThis, void* pv, size_t uiSizeof, char* szFieldName);

protected:
	CField*			AddField(size_t uiFieldSize, char* szName);
	void			Unmanaged(CBaseObject* pcThis, EPrimitiveType eType, void* pv, char* szFieldName);
	void			Unmanaged(CBaseObject* pcThis, EPrimitiveType eType, void* pv, size_t uiLength, char* szFieldName);
};

#endif // __CLASS_H__

