#ifndef __CLASS_H__
#define __CLASS_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/ArrayVoidPtr.h"
#include "BaseLib/ArrayTemplateEmbedded.h"
#include "BaseLib/LinkedListBlock.h"
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/Number.h"
#include "BaseLib/Date.h"
#include "BaseLib/Float2.h"
#include "BaseLib/Float3.h"
#include "BaseLib/Float4.h"
#include "BaseLib/Float4x4.h"
#include "Integer.h"
#include "Float.h"
#include "Character.h"
#include "PrimitiveVoid.h"
#include "PointerField.h"
#include "EmbeddedObjectField.h"
#include "PrimitiveField.h"
#include "UnmanagedField.h"


#define CLASS_FLAGS_UNMANAGED		0x01
#define CLASS_FLAGS_PRIMITIVE		0x02
#define CLASS_FLAGS_COMPLETE		0x04
#define CLASS_FLAGS_SYSTEM			0x08


class CPointer;
class CBaseObject;
class CPrimitiveObject;
class CClasses;
class CClass : public CPostMalloc<CField>
{
protected:
	CChars				mszClassName;
	size				muiSize;
	EPrimitiveType		meType;
	uint16				muiFlags;
	CClasses*			mpcClassesThisIn;

	CLinkedListBlock	macFields;

	CArrayVoidPtr		mapcPointers;
	CArrayVoidPtr		mapcEmbeddedObjects;
	CArrayVoidPtr		mapcPrimitives;
	CArrayVoidPtr		mapcUnmanaged;

public:
	void				Init(char* szClassName, size uiSize, EPrimitiveType eType, CClasses* pcClasses);
	void				Kill(void);

	void				Complete(void);

	void				Pointer(CBaseObject* pcThis, CPointer* pcPointer, char* szFieldName);
	void				Primitive(CBaseObject* pcThis, CPrimitiveObject* pcPrimitive, void* pvPrimitive, char* szFieldName);
	void				Embedded(CBaseObject* pcThis, CBaseObject* pcObject, char* szFieldName);

	CClasses*			GetClasses(void);
	char*				GetName(void);
	EPrimitiveType		GetType(void);
	size				GetSizeOf(void);

	CField*				GetField(char* szFieldName);

	void				Unmanaged(void);
	void				System(void);
	void				Primitive(void);

	bool				IsComplete(void);
	bool				IsUnmanaged(void);
	bool				IsPrimitive(void);
	bool				IsSystem(void);

	CArrayVoidPtr*		GetPointerFields(void);
	CArrayVoidPtr*		GetEmbeddedObjectFields(void);
	CArrayVoidPtr*		GetPrimitiveFields(void);
	CArrayVoidPtr*		GetUnmanagedFields(void);

	CClass*				GetClass(EPrimitiveType eType);

	CUnmanagedField*	AddUnmanaged(char* szFieldName);
	CPrimitiveField*	AddPrimitive(char* szFieldName);

public:
	void				Primitive(CBaseObject* pcThis, Int8* pcPrimitive, char* szFieldName);
	void				Primitive(CBaseObject* pcThis, UInt8* pcPrimitive, char* szFieldName);
	void				Primitive(CBaseObject* pcThis, Int16* pcPrimitive, char* szFieldName);
	void				Primitive(CBaseObject* pcThis, UInt16* pcPrimitive, char* szFieldName);
	void				Primitive(CBaseObject* pcThis, Int32* pcPrimitive, char* szFieldName);
	void				Primitive(CBaseObject* pcThis, UInt32* pcPrimitive, char* szFieldName);
	void				Primitive(CBaseObject* pcThis, Int64* pcPrimitive, char* szFieldName);
	void				Primitive(CBaseObject* pcThis, UInt64* pcPrimitive, char* szFieldName);
	void				Primitive(CBaseObject* pcThis, Bool* pcPrimitive, char* szFieldName);
	void				Primitive(CBaseObject* pcThis, Float32* pcPrimitive, char* szFieldName);
	void				Primitive(CBaseObject* pcThis, Float64* pcPrimitive, char* szFieldName);
	void				Primitive(CBaseObject* pcThis, Char8* pcPrimitive, char* szFieldName);
	void				Primitive(CBaseObject* pcThis, Char16* pcPrimitive, char* szFieldName);

	void				UnmanagedInt8(CBaseObject* pcThis, int8* pc, char* szFieldName);
	void				UnmanagedInt8(CBaseObject* pcThis, int8* pac, size uiLength, char* szFieldName);
	void				UnmanagedInt16(CBaseObject* pcThis, int16* pi, char* szFieldName);
	void				UnmanagedInt16(CBaseObject* pcThis, int16* pai, size uiLength, char* szFieldName);
	void				UnmanagedInt32(CBaseObject* pcThis, int32* pi, char* szFieldName);
	void				UnmanagedInt32(CBaseObject* pcThis, int32* pai, size uiLength, char* szFieldName);
	void				UnmanagedInt64(CBaseObject* pcThis, int64* pi, char* szFieldName);
	void				UnmanagedInt64(CBaseObject* pcThis, int64* pai, size uiLength, char* szFieldName);
	void				UnmanagedSInt(CBaseObject* pcThis, int* pi, char* szFieldName);
	void				UnmanagedSInt(CBaseObject* pcThis, int* pai, size uiLength, char* szFieldName);
	void				UnmanagedUInt8(CBaseObject* pcThis, uint8* pc, char* szFieldName);
	void				UnmanagedUInt8(CBaseObject* pcThis, uint8* pac, size uiLength, char* szFieldName);
	void				UnmanagedUInt16(CBaseObject* pcThis, uint16* pi, char* szFieldName);
	void				UnmanagedUInt16(CBaseObject* pcThis, uint16* pai, size uiLength, char* szFieldName);
	void				UnmanagedUInt32(CBaseObject* pcThis, uint32* pi, char* szFieldName);
	void				UnmanagedUInt32(CBaseObject* pcThis, uint32* pai, size uiLength, char* szFieldName);
	void				UnmanagedUInt64(CBaseObject* pcThis, uint64* pi, char* szFieldName);
	void				UnmanagedUInt64(CBaseObject* pcThis, uint64* pai, size uiLength, char* szFieldName);
	void				UnmanagedSUInt(CBaseObject* pcThis, uint* pi, char* szFieldName);
	void				UnmanagedSUInt(CBaseObject* pcThis, uint* pai, size uiLength, char* szFieldName);
	void				UnmanagedSize(CBaseObject* pcThis, size* pi, char* szFieldName);
	void				UnmanagedSize(CBaseObject* pcThis, size* pai, size uiLength, char* szFieldName);
	void				UnmanagedFloat(CBaseObject* pcThis, float32* pf, char* szFieldName);
	void				UnmanagedFloat(CBaseObject* pcThis, float32* paf, size uiLength, char* szFieldName);
	void				UnmanagedFloat(CBaseObject* pcThis, float64* pf, char* szFieldName);
	void				UnmanagedFloat(CBaseObject* pcThis, float64* paf, size uiLength, char* szFieldName);
	void				UnmanagedFloat(CBaseObject* pcThis, float96* pf, char* szFieldName);
	void				UnmanagedFloat(CBaseObject* pcThis, float96* paf, size uiLength, char* szFieldName);
	void				UnmanagedChar(CBaseObject* pcThis, char8* pc, char* szFieldName);
	void				UnmanagedChar(CBaseObject* pcThis, char8* pac, size uiLength, char* szFieldName);
	void				UnmanagedChar(CBaseObject* pcThis, char16* pc, char* szFieldName);
	void				UnmanagedChar(CBaseObject* pcThis, char16* pac, size uiLength, char* szFieldName);
	void				UnmanagedBool(CBaseObject* pcThis, bool* pb, char* szFieldName);
	void				UnmanagedBool(CBaseObject* pcThis, bool* pab, size uiLength, char* szFieldName);

	void				UnmanagedEnum(CBaseObject* pcThis, void* pe, char* szFieldName);

	void				UnmanagedString(CBaseObject* pcThis, CChars* pcChars, char* szFieldName);
	void				UnmanagedString(CBaseObject* pcThis, CCharsImmutable* pcChars, char* szFieldName);
	void				UnmanagedNumber(CBaseObject* pcThis, CNumber* pcNumber, char* szFieldName);
	void				UnmanagedDate(CBaseObject* pcThis, CDate* pcDate, char* szFieldName);

	void				UnmanagedData(CBaseObject* pcThis, void* pv, size uiSizeof, char* szFieldName);
	void				UnmanagedVoid(CBaseObject* pcThis, void* pv, char* szFieldName);
	void				UnmanagedUnknown(CBaseObject* pcThis, void* pv, size uiSizeof, char* szFieldName);
	void				UnmanagedPointer(CBaseObject* pcThis, void** pv, char* szFieldName);

	void				Unmanaged2Float(CBaseObject* pcThis, SFloat2* pf, char* szFieldName);
	void				Unmanaged2Float(CBaseObject* pcThis, SFloat2* pf, size uiLength, char* szFieldName);
	void				Unmanaged3Float(CBaseObject* pcThis, SFloat3* pf, char* szFieldName);
	void				Unmanaged3Float(CBaseObject* pcThis, SFloat3* pf, size uiLength, char* szFieldName);
	void				Unmanaged4Float(CBaseObject* pcThis, SFloat4* pf, char* szFieldName);
	void				Unmanaged4Float(CBaseObject* pcThis, SFloat4* pf, size uiLength, char* szFieldName);
	void				Unmanaged4x4Float(CBaseObject* pcThis, SFloat4x4* pf, char* szFieldName);
	void				Unmanaged4x4Float(CBaseObject* pcThis, SFloat4x4* pf, size uiLength, char* szFieldName);

protected:
	CField*				AddField(size uiFieldSize, char* szName);
	void				Unmanaged(CBaseObject* pcThis, EPrimitiveType eType, void* pv, char* szFieldName);
	void				Unmanaged(CBaseObject* pcThis, EPrimitiveType eType, void* pv, size uiLength, char* szFieldName);
	size				ComputeSize(void);
};

#endif // __CLASS_H__

