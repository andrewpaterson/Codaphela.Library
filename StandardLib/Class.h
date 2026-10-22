#ifndef __CLASS_H__
#define __CLASS_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/ArrayVoidPtr.h"
#include "BaseLib/ArrayTemplateEmbedded.h"
#include "BaseLib/LinkedListBlock.h"
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/Number.h"
#include "BaseLib/Date.h"
#include "BaseLib/Float32Vec2.h"
#include "BaseLib/Float32Vec3.h"
#include "BaseLib/Float32Vec4.h"
#include "BaseLib/Float4x4.h"
#include "BaseLib/Int32Vec2.h"
#include "BaseLib/Int32Vec3.h"
#include "BaseLib/Int32Vec4.h"
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
#define CLASS_FLAGS_HAS_NES			0x10


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
	void					Init(const char* szClassName, size uiSize, EPrimitiveType eType, CClasses* pcClasses);
	void					Kill(void);

	void					Complete(void);

	CPointerField*			Pointer(CBaseObject* pcThis, CPointer* pcPointer, const char* szFieldName);
	CPrimitiveField*		Primitive(CBaseObject* pcThis, CPrimitiveObject* pcPrimitive, void* pvPrimitive, const char* szFieldName);
	CEmbeddedObjectField*	Embedded(CBaseObject* pcThis, CBaseObject* pcObject, const char* szFieldName);

	CClasses*				GetClasses(void);
	char*					GetName(void);
	EPrimitiveType			GetType(void);
	size					GetSizeOf(void);

	CField*					GetField(const char* szFieldName);

	void					Unmanaged(void);
	void					System(void);
	void					Primitive(void);
	void					Nes(void);

	bool					IsComplete(void);
	bool					IsUnmanaged(void);
	bool					IsPrimitive(void);
	bool					IsSystem(void);
	bool					HasNes(void);

	CArrayVoidPtr*			GetPointerFields(void);
	CArrayVoidPtr*			GetEmbeddedObjectFields(void);
	CArrayVoidPtr*			GetPrimitiveFields(void);
	CArrayVoidPtr*			GetUnmanagedFields(void);

	CClass*					GetClass(EPrimitiveType eType);

	CUnmanagedField*		AddUnmanaged(const char* szFieldName);
	CPrimitiveField*		AddPrimitive(const char* szFieldName);

public:
	CPrimitiveField*		Primitive(CBaseObject* pcThis, SInt* pcPrimitive, const char* szFieldName);
	CPrimitiveField*		Primitive(CBaseObject* pcThis, SSize* pcPrimitive, const char* szFieldName);
	CPrimitiveField*		Primitive(CBaseObject* pcThis, Int8* pcPrimitive, const char* szFieldName);
	CPrimitiveField*		Primitive(CBaseObject* pcThis, UInt8* pcPrimitive, const char* szFieldName);
	CPrimitiveField*		Primitive(CBaseObject* pcThis, Int16* pcPrimitive, const char* szFieldName);
	CPrimitiveField*		Primitive(CBaseObject* pcThis, UInt16* pcPrimitive, const char* szFieldName);
	CPrimitiveField*		Primitive(CBaseObject* pcThis, Int32* pcPrimitive, const char* szFieldName);
	CPrimitiveField*		Primitive(CBaseObject* pcThis, UInt32* pcPrimitive, const char* szFieldName);
	CPrimitiveField*		Primitive(CBaseObject* pcThis, Int64* pcPrimitive, const char* szFieldName);
	CPrimitiveField*		Primitive(CBaseObject* pcThis, UInt64* pcPrimitive, const char* szFieldName);
	CPrimitiveField*		Primitive(CBaseObject* pcThis, Bool* pcPrimitive, const char* szFieldName);
	CPrimitiveField*		Primitive(CBaseObject* pcThis, Float32* pcPrimitive, const char* szFieldName);
	CPrimitiveField*		Primitive(CBaseObject* pcThis, Float64* pcPrimitive, const char* szFieldName);
	CPrimitiveField*		Primitive(CBaseObject* pcThis, Char8* pcPrimitive, const char* szFieldName);
	CPrimitiveField*		Primitive(CBaseObject* pcThis, Char16* pcPrimitive, const char* szFieldName);

	void					UnmanagedInt8(CBaseObject* pcThis, int8* pc, const char* szFieldName);
	void					UnmanagedInt8(CBaseObject* pcThis, int8* pac, size uiLength, const char* szFieldName);
	void					UnmanagedInt16(CBaseObject* pcThis, int16* pi, const char* szFieldName);
	void					UnmanagedInt16(CBaseObject* pcThis, int16* pai, size uiLength, const char* szFieldName);
	void					UnmanagedInt32(CBaseObject* pcThis, int32* pi, const char* szFieldName);
	void					UnmanagedInt32(CBaseObject* pcThis, int32* pai, size uiLength, const char* szFieldName);
	void					UnmanagedInt64(CBaseObject* pcThis, int64* pi, const char* szFieldName);
	void					UnmanagedInt64(CBaseObject* pcThis, int64* pai, size uiLength, const char* szFieldName);
	void					UnmanagedSInt(CBaseObject* pcThis, int* pi, const char* szFieldName);
	void					UnmanagedSInt(CBaseObject* pcThis, int* pai, size uiLength, const char* szFieldName);
	void					UnmanagedUInt8(CBaseObject* pcThis, uint8* pc, const char* szFieldName);
	void					UnmanagedUInt8(CBaseObject* pcThis, uint8* pac, size uiLength, const char* szFieldName);
	void					UnmanagedUInt16(CBaseObject* pcThis, uint16* pi, const char* szFieldName);
	void					UnmanagedUInt16(CBaseObject* pcThis, uint16* pai, size uiLength, const char* szFieldName);
	void					UnmanagedUInt32(CBaseObject* pcThis, uint32* pi, const char* szFieldName);
	void					UnmanagedUInt32(CBaseObject* pcThis, uint32* pai, size uiLength, const char* szFieldName);
	void					UnmanagedUInt64(CBaseObject* pcThis, uint64* pi, const char* szFieldName);
	void					UnmanagedUInt64(CBaseObject* pcThis, uint64* pai, size uiLength, const char* szFieldName);
	void					UnmanagedSUInt(CBaseObject* pcThis, uint* pi, const char* szFieldName);
	void					UnmanagedSUInt(CBaseObject* pcThis, uint* pai, size uiLength, const char* szFieldName);
	void					UnmanagedSize(CBaseObject* pcThis, size* pi, const char* szFieldName);
	void					UnmanagedSize(CBaseObject* pcThis, size* pai, size uiLength, const char* szFieldName);
	void					UnmanagedFloat(CBaseObject* pcThis, float32* pf, const char* szFieldName);
	void					UnmanagedFloat(CBaseObject* pcThis, float32* paf, size uiLength, const char* szFieldName);
	void					UnmanagedFloat(CBaseObject* pcThis, float64* pf, const char* szFieldName);
	void					UnmanagedFloat(CBaseObject* pcThis, float64* paf, size uiLength, const char* szFieldName);
	void					UnmanagedFloat(CBaseObject* pcThis, float128* pf, const char* szFieldName);
	void					UnmanagedFloat(CBaseObject* pcThis, float128* paf, size uiLength, const char* szFieldName);
	void					UnmanagedChar(CBaseObject* pcThis, char8* pc, const char* szFieldName);
	void					UnmanagedChar(CBaseObject* pcThis, char8* pac, size uiLength, const char* szFieldName);
	void					UnmanagedChar(CBaseObject* pcThis, char16* pc, const char* szFieldName);
	void					UnmanagedChar(CBaseObject* pcThis, char16* pac, size uiLength, const char* szFieldName);
	void					UnmanagedBool(CBaseObject* pcThis, bool* pb, const char* szFieldName);
	void					UnmanagedBool(CBaseObject* pcThis, bool* pab, size uiLength, const char* szFieldName);

	void					UnmanagedEnum(CBaseObject* pcThis, void* pe, const char* szFieldName);

	void					UnmanagedString(CBaseObject* pcThis, CChars* pcChars, const char* szFieldName);
	void					UnmanagedString(CBaseObject* pcThis, CCharsImmutable* pcChars, const char* szFieldName);
	void					UnmanagedNumber(CBaseObject* pcThis, CNumber* pcNumber, const char* szFieldName);
	void					UnmanagedDate(CBaseObject* pcThis, CDate* pcDate, const char* szFieldName);

	void					UnmanagedData(CBaseObject* pcThis, void* pv, size uiSizeof, const char* szFieldName);
	void					UnmanagedVoid(CBaseObject* pcThis, void* pv, const char* szFieldName);
	void					UnmanagedUnknown(CBaseObject* pcThis, void* pv, size uiSizeof, const char* szFieldName);
	void					UnmanagedPointer(CBaseObject* pcThis, void** pv, const char* szFieldName);

	void					Unmanaged2Float(CBaseObject* pcThis, SFloat32Vec2* pf, const char* szFieldName);
	void					Unmanaged2Float(CBaseObject* pcThis, SFloat32Vec2* pf, size uiLength, const char* szFieldName);
	void					Unmanaged3Float(CBaseObject* pcThis, SFloat32Vec3* pf, const char* szFieldName);
	void					Unmanaged3Float(CBaseObject* pcThis, SFloat32Vec3* pf, size uiLength, const char* szFieldName);
	void					Unmanaged4Float(CBaseObject* pcThis, SFloat32Vec4* pf, const char* szFieldName);
	void					Unmanaged4Float(CBaseObject* pcThis, SFloat32Vec4* pf, size uiLength, const char* szFieldName);
	void					Unmanaged4x4Float(CBaseObject* pcThis, SFloat4x4* pf, const char* szFieldName);
	void					Unmanaged4x4Float(CBaseObject* pcThis, SFloat4x4* pf, size uiLength, const char* szFieldName);

	void					Unmanaged2Int(CBaseObject* pcThis, SInt32Vec2* pf, const char* szFieldName);
	void					Unmanaged2Int(CBaseObject* pcThis, SInt32Vec2* pf, size uiLength, const char* szFieldName);
	void					Unmanaged3Int(CBaseObject* pcThis, SInt32Vec3* pf, const char* szFieldName);
	void					Unmanaged3Int(CBaseObject* pcThis, SInt32Vec3* pf, size uiLength, const char* szFieldName);
	void					Unmanaged4Int(CBaseObject* pcThis, SInt32Vec4* pf, const char* szFieldName);
	void					Unmanaged4Int(CBaseObject* pcThis, SInt32Vec4* pf, size uiLength, const char* szFieldName);

protected:
	CField*					AddField(size uiFieldSize, const char* szName);
	void					Unmanaged(CBaseObject* pcThis, EPrimitiveType eType, void* pv, const char* szFieldName);
	void					Unmanaged(CBaseObject* pcThis, EPrimitiveType eType, void* pv, size uiLength, const char* szFieldName);
	size					ComputeSize(void);
};


#endif // __CLASS_H__

