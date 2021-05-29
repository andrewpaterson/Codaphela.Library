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
#include "PrimitiveField.h"
#include "UnmanagedField.h"


#define CLASS_FLAGS_PRIMITIVE		0x01
#define CLASS_FLAGS_COMPLETE		0x02
#define CLASS_FLAGS_SYSTEM			0x04
#define CLASS_FLAGS_SIZE_COMPUTED	0x08


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
	CArrayVoidPtr		mapcPrimitives;
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
	uint32			GetSizeOf(void);

	BOOL			IsComplete(void);
	void			Primitive(void);
	BOOL			IsPrimitive(void);
	void			System(void);
	BOOL			IsSystem(void);
	CField*			GetField(char* szFieldName);
	void			SetSize(uint32 uiSize);

	CArrayVoidPtr*	GetPointerFields(void);
	CArrayVoidPtr*	GetEmbeddedObjectFields(void);
	CArrayVoidPtr*	GetPrimitiveFields(void);
	CArrayVoidPtr*	GetUnmanagedFields(void);

	CClass*			GetClass(EPrimitiveType eType);

public:
	void			UnmanagedInt(CBaseObject* pcThis, int8* pc, char* szFieldName);
	void			UnmanagedInt(CBaseObject* pcThis, int8* pac, size_t uiLength, char* szFieldName);
	void			UnmanagedInt(CBaseObject* pcThis, int16* pi, char* szFieldName);
	void			UnmanagedInt(CBaseObject* pcThis, int16* pai, size_t uiLength, char* szFieldName);
	void			UnmanagedInt(CBaseObject* pcThis, int32* pi, char* szFieldName);
	void			UnmanagedInt(CBaseObject* pcThis, int32* pai, size_t uiLength, char* szFieldName);
	void			UnmanagedInt(CBaseObject* pcThis, int64* pi, char* szFieldName);
	void			UnmanagedInt(CBaseObject* pcThis, int64* pai, size_t uiLength, char* szFieldName);
	void			UnmanagedInt(CBaseObject* pcThis, uint8* pc, char* szFieldName);
	void			UnmanagedInt(CBaseObject* pcThis, uint8* pac, size_t uiLength, char* szFieldName);
	void			UnmanagedInt(CBaseObject* pcThis, uint16* pi, char* szFieldName);
	void			UnmanagedInt(CBaseObject* pcThis, uint16* pai, size_t uiLength, char* szFieldName);
	void			UnmanagedInt(CBaseObject* pcThis, uint32* pi, char* szFieldName);
	void			UnmanagedInt(CBaseObject* pcThis, uint32* pai, size_t uiLength, char* szFieldName);
	void			UnmanagedInt(CBaseObject* pcThis, uint64* pi, char* szFieldName);
	void			UnmanagedInt(CBaseObject* pcThis, uint64* pai, size_t uiLength, char* szFieldName);
	void			UnmanagedFloat(CBaseObject* pcThis, float32* pf, char* szFieldName);
	void			UnmanagedFloat(CBaseObject* pcThis, float32* paf, size_t uiLength, char* szFieldName);
	void			UnmanagedFloat(CBaseObject* pcThis, float64* pf, char* szFieldName);
	void			UnmanagedFloat(CBaseObject* pcThis, float64* paf, size_t uiLength, char* szFieldName);
	void			UnmanagedChar(CBaseObject* pcThis, char8* pc, char* szFieldName);
	void			UnmanagedChar(CBaseObject* pcThis, char8* pac, size_t uiLength, char* szFieldName);
	void			UnmanagedChar(CBaseObject* pcThis, char16* pc, char* szFieldName);
	void			UnmanagedChar(CBaseObject* pcThis, char16* pac, size_t uiLength, char* szFieldName);

	void			UnmanagedString(CBaseObject* pcThis, CChars* pcChars, char* szFieldName);
	void			UnmanagedNumber(CBaseObject* pcThis, CNumber* pcNumber, char* szFieldName);
	void			UnmanagedDate(CBaseObject* pcThis, CDate* pcDate, char* szFieldName);

	void			UnmanagedData(CBaseObject* pcThis, void* pv, size_t uiSizeof, char* szFieldName);
	void			UnmanagedVoid(CBaseObject* pcThis, void* pv, char* szFieldName);

protected:
	CField*				AddField(size_t uiFieldSize, char* szName);
	void				Unmanaged(CBaseObject* pcThis, EPrimitiveType eType, void* pv, char* szFieldName);
	void				Unmanaged(CBaseObject* pcThis, EPrimitiveType eType, void* pv, size_t uiLength, char* szFieldName);
	uint32				ComputeSize(void);
	CUnmanagedField*	AddUnmanaged(CBaseObject* pcThis, EPrimitiveType eType, void* pv, char* szFieldName);
};

#endif // __CLASS_H__

