#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "PrimitiveObject.h"
#include "BaseObject.h"
#include "Classes.h"
#include "Class.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClass::Init(char* szClassName, size uiSize, EPrimitiveType eType, CClasses* pcClasses)
{
	mszClassName.Init(szClassName);
	mpcClassesThisIn = pcClasses;

	muiSize = uiSize;
	meType = eType;
	muiFlags = 0;

	macFields.Init();

	mapcPointers.Init();
	mapcEmbeddedObjects.Init();
	mapcPrimitives.Init();
	mapcUnmanaged.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClass::Kill(void)
{
	size		i;
	CField*		pcField;
	size		uiFields;

	mapcUnmanaged.Kill();
	mapcPrimitives.Kill();
	mapcEmbeddedObjects.Kill();
	mapcPointers.Kill();

	uiFields = macFields.NumElements();
	for (i = 0; i < uiFields; i++)
	{
		pcField = (CField*)macFields.Get(i);
		pcField->Kill();
	}
	macFields.Kill();

	muiFlags = 0;
	muiSize = 0;
	meType = PT_Undefined;

	mszClassName.Kill();
	mpcClassesThisIn = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClass::Complete(void)
{
	muiFlags |= CLASS_FLAGS_COMPLETE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClass::Primitive(void)
{
	muiFlags |= CLASS_FLAGS_PRIMITIVE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClass::Unmanaged(void)
{
	muiFlags |= CLASS_FLAGS_UNMANAGED;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClass::System(void)
{
	muiFlags |= CLASS_FLAGS_SYSTEM;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CClass::IsComplete(void)
{
	return muiFlags & CLASS_FLAGS_COMPLETE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CClass::IsUnmanaged(void)
{
	return muiFlags & CLASS_FLAGS_UNMANAGED;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CClass::IsPrimitive(void)
{
	return muiFlags & CLASS_FLAGS_PRIMITIVE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CClass::IsSystem(void)
{
	return muiFlags & CLASS_FLAGS_SYSTEM;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CField* CClass::GetField(char* szFieldName)
{
	CField*		pcField;

	if (StrEmpty(szFieldName))
	{
		return NULL;
	}

	pcField = (CField*)macFields.GetHead();
	while (pcField)
	{
		if (strcmp(pcField->GetName(), szFieldName) == 0)
		{
			return pcField;
		}
		pcField = (CField*)macFields.GetNext(pcField);
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClass::Pointer(CBaseObject* pcThis, CPointer* pcPointer, char* szFieldName)
{
	CPointerField*	pcPointerField;
	ptrdiff_t		iOffset;

	iOffset = (size)pcPointer - (size)pcThis;
	pcPointerField = (CPointerField*)AddField(sizeof(CPointerField), szFieldName);
	PostMalloc<CPointerField>(pcPointerField);
	mapcPointers.Add(pcPointerField);
	pcPointerField->Init(iOffset, this, szFieldName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClass::Primitive(CBaseObject* pcThis, CPrimitiveObject* pcPrimitive, void* pvPrimitive, char* szFieldName)
{
	CPrimitiveField*	pcDataField;
	ptrdiff_t			iObjectOffset;
	ptrdiff_t			iValueOffset;
	CClass*				pcClass;
	SDataIO*			psIO;

	pcPrimitive->SetEmbedding((CObject*)pcThis);

	iObjectOffset = (size)pcPrimitive - (size)pcThis;
	iValueOffset = (size)pvPrimitive - (size)pcThis;
	pcDataField = (CPrimitiveField*)AddField(sizeof(CPrimitiveField), szFieldName);
	PostMalloc<CPrimitiveField>(pcDataField);
	mapcPrimitives.Add(pcDataField);
	pcClass = GetClass(pcPrimitive->GetClassType());
	psIO = gcDataTypesIO.GetIO(pcClass->GetName());
	pcDataField->Init(pcClass, iObjectOffset, iValueOffset, this, psIO, szFieldName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClass::Embedded(CBaseObject* pcThis, CBaseObject* pcObject, char* szFieldName)
{
	CEmbeddedObjectField*	pcEmbeddedObjectField;
	ptrdiff_t				iOffset;

	iOffset = (size)pcObject - (size)pcThis;
	pcEmbeddedObjectField = (CEmbeddedObjectField*)AddField(sizeof(CEmbeddedObjectField), szFieldName);
	PostMalloc<CEmbeddedObjectField>(pcEmbeddedObjectField);
	mapcEmbeddedObjects.Add(pcEmbeddedObjectField);
	pcEmbeddedObjectField->Init(pcObject->GetClass(), iOffset, this, szFieldName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClass::Primitive(CBaseObject* pcThis, Int8* pcPrimitive, char* szFieldName) { Primitive(pcThis, pcPrimitive, pcPrimitive->GetPrimitivePointer(), szFieldName); }
void CClass::Primitive(CBaseObject* pcThis, UInt8* pcPrimitive, char* szFieldName) { Primitive(pcThis, pcPrimitive, pcPrimitive->GetPrimitivePointer(), szFieldName); }
void CClass::Primitive(CBaseObject* pcThis, Int16* pcPrimitive, char* szFieldName) { Primitive(pcThis, pcPrimitive, pcPrimitive->GetPrimitivePointer(), szFieldName); }
void CClass::Primitive(CBaseObject* pcThis, UInt16* pcPrimitive, char* szFieldName) { Primitive(pcThis, pcPrimitive, pcPrimitive->GetPrimitivePointer(), szFieldName); }
void CClass::Primitive(CBaseObject* pcThis, Int32* pcPrimitive, char* szFieldName) { Primitive(pcThis, pcPrimitive, pcPrimitive->GetPrimitivePointer(), szFieldName); }
void CClass::Primitive(CBaseObject* pcThis, UInt32* pcPrimitive, char* szFieldName) { Primitive(pcThis, pcPrimitive, pcPrimitive->GetPrimitivePointer(), szFieldName); }
void CClass::Primitive(CBaseObject* pcThis, Int64* pcPrimitive, char* szFieldName) { Primitive(pcThis, pcPrimitive, pcPrimitive->GetPrimitivePointer(), szFieldName); }
void CClass::Primitive(CBaseObject* pcThis, UInt64* pcPrimitive, char* szFieldName) { Primitive(pcThis, pcPrimitive, pcPrimitive->GetPrimitivePointer(), szFieldName); }
void CClass::Primitive(CBaseObject* pcThis, Bool* pcPrimitive, char* szFieldName) { Primitive(pcThis, pcPrimitive, pcPrimitive->GetPrimitivePointer(), szFieldName); }
void CClass::Primitive(CBaseObject* pcThis, Float32* pcPrimitive, char* szFieldName) { Primitive(pcThis, pcPrimitive, pcPrimitive->GetPrimitivePointer(), szFieldName); }
void CClass::Primitive(CBaseObject* pcThis, Float64* pcPrimitive, char* szFieldName) { Primitive(pcThis, pcPrimitive, pcPrimitive->GetPrimitivePointer(), szFieldName); }
void CClass::Primitive(CBaseObject* pcThis, Char8* pcPrimitive, char* szFieldName) { Primitive(pcThis, pcPrimitive, pcPrimitive->GetPrimitivePointer(), szFieldName); }
void CClass::Primitive(CBaseObject* pcThis, Char16* pcPrimitive, char* szFieldName) { Primitive(pcThis, pcPrimitive, pcPrimitive->GetPrimitivePointer(), szFieldName); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClass::UnmanagedInt8(CBaseObject* pcThis, int8* pc, char* szFieldName)						{ Unmanaged(pcThis, PT_int8, pc, szFieldName); }
void CClass::UnmanagedInt8(CBaseObject* pcThis, int8* pac, size uiLength, char* szFieldName)		{ Unmanaged(pcThis, PT_int8, pac, uiLength, szFieldName); }
void CClass::UnmanagedInt16(CBaseObject* pcThis, int16* pc, char* szFieldName)						{ Unmanaged(pcThis, PT_int16, pc, szFieldName); }
void CClass::UnmanagedInt16(CBaseObject* pcThis, int16* pac, size uiLength, char* szFieldName)		{ Unmanaged(pcThis, PT_int16, pac, uiLength, szFieldName); }
void CClass::UnmanagedInt32(CBaseObject* pcThis, int32* pi, char* szFieldName)						{ Unmanaged(pcThis, PT_int32, pi, szFieldName); }
void CClass::UnmanagedInt32(CBaseObject* pcThis, int32* pai, size uiLength, char* szFieldName)		{ Unmanaged(pcThis, PT_int32, pai, uiLength, szFieldName); }
void CClass::UnmanagedInt64(CBaseObject* pcThis, int64* pi, char* szFieldName)						{ Unmanaged(pcThis, PT_int64, pi, szFieldName); }
void CClass::UnmanagedInt64(CBaseObject* pcThis, int64* pai, size uiLength, char* szFieldName)		{ Unmanaged(pcThis, PT_int64, pai, uiLength, szFieldName); }
void CClass::UnmanagedSInt(CBaseObject* pcThis, int* pi, char* szFieldName)							{ Unmanaged(pcThis, IntPrimitive(), pi, szFieldName); }
void CClass::UnmanagedSInt(CBaseObject* pcThis, int* pai, size uiLength, char* szFieldName)			{ Unmanaged(pcThis, IntPrimitive(), pai, uiLength, szFieldName); }
void CClass::UnmanagedUInt8(CBaseObject* pcThis, uint8* pc, char* szFieldName)						{ Unmanaged(pcThis, PT_uint8, pc, szFieldName); }
void CClass::UnmanagedUInt8(CBaseObject* pcThis, uint8* pac, size uiLength, char* szFieldName)		{ Unmanaged(pcThis, PT_uint8, pac, uiLength, szFieldName); }
void CClass::UnmanagedUInt16(CBaseObject* pcThis, uint16* pc, char* szFieldName)					{ Unmanaged(pcThis, PT_uint16, pc, szFieldName); }
void CClass::UnmanagedUInt16(CBaseObject* pcThis, uint16* pac, size uiLength, char* szFieldName)	{ Unmanaged(pcThis, PT_uint16, pac, uiLength, szFieldName); }
void CClass::UnmanagedUInt32(CBaseObject* pcThis, uint32* pi, char* szFieldName)					{ Unmanaged(pcThis, PT_uint32, pi, szFieldName); }
void CClass::UnmanagedUInt32(CBaseObject* pcThis, uint32* pai, size uiLength, char* szFieldName)	{ Unmanaged(pcThis, PT_uint32, pai, uiLength, szFieldName); }
void CClass::UnmanagedUInt64(CBaseObject* pcThis, uint64* pi, char* szFieldName)					{ Unmanaged(pcThis, PT_uint64, pi, szFieldName); }
void CClass::UnmanagedUInt64(CBaseObject* pcThis, uint64* pai, size uiLength, char* szFieldName)	{ Unmanaged(pcThis, PT_uint64, pai, uiLength, szFieldName); }
void CClass::UnmanagedSUInt(CBaseObject* pcThis, uint* pi, char* szFieldName)						{ Unmanaged(pcThis, UIntPrimitive(), pi, szFieldName); }
void CClass::UnmanagedSUInt(CBaseObject* pcThis, uint* pai, size uiLength, char* szFieldName)		{ Unmanaged(pcThis, UIntPrimitive(), pai, uiLength, szFieldName); }
void CClass::UnmanagedSize(CBaseObject* pcThis, size* pi, char* szFieldName)						{ Unmanaged(pcThis, SizePrimitive(), pi, szFieldName); }
void CClass::UnmanagedSize(CBaseObject* pcThis, size* pai, size uiLength, char* szFieldName)		{ Unmanaged(pcThis, SizePrimitive(), pai, uiLength, szFieldName); }
void CClass::UnmanagedFloat(CBaseObject* pcThis, float32* pf, char* szFieldName)					{ Unmanaged(pcThis, PT_float32, pf, szFieldName); }
void CClass::UnmanagedFloat(CBaseObject* pcThis, float32* paf, size uiLength, char* szFieldName)	{ Unmanaged(pcThis, PT_float32, paf, uiLength, szFieldName); }
void CClass::UnmanagedFloat(CBaseObject* pcThis, float64* pf, char* szFieldName)					{ Unmanaged(pcThis, PT_float64, pf, szFieldName); }
void CClass::UnmanagedFloat(CBaseObject* pcThis, float64* paf, size uiLength, char* szFieldName)	{ Unmanaged(pcThis, PT_float64, paf, uiLength, szFieldName); }
void CClass::UnmanagedChar(CBaseObject* pcThis, char8* pc, char* szFieldName)						{ Unmanaged(pcThis, PT_char8, pc, szFieldName); }
void CClass::UnmanagedChar(CBaseObject* pcThis, char8* pac, size uiLength, char* szFieldName)		{ Unmanaged(pcThis, PT_char8, pac, uiLength, szFieldName); }
void CClass::UnmanagedChar(CBaseObject* pcThis, char16* pc, char* szFieldName)						{ Unmanaged(pcThis, PT_char16, pc, szFieldName); }
void CClass::UnmanagedChar(CBaseObject* pcThis, char16* pac, size uiLength, char* szFieldName)		{ Unmanaged(pcThis, PT_char16, pac, uiLength, szFieldName); }
void CClass::UnmanagedBool(CBaseObject* pcThis, bool* pb, char* szFieldName)						{ Unmanaged(pcThis, PT_bool, pb, szFieldName); }
void CClass::UnmanagedBool(CBaseObject* pcThis, bool* pab, size uiLength, char* szFieldName)		{ Unmanaged(pcThis, PT_bool, pab, uiLength, szFieldName); }

void CClass::UnmanagedEnum(CBaseObject* pcThis, void* pe, char* szFieldName)						{ Unmanaged(pcThis, PT_Enum, pe, szFieldName); }

void CClass::UnmanagedString(CBaseObject* pcThis, CChars* pcChars, char* szFieldName)				{ Unmanaged(pcThis, PT_String, pcChars, szFieldName); }
void CClass::UnmanagedNumber(CBaseObject* pcThis, CNumber* pcNumber, char* szFieldName)				{ Unmanaged(pcThis, PT_Number, pcNumber, szFieldName); }
void CClass::UnmanagedDate(CBaseObject* pcThis, CDate* pcDate, char* szFieldName)					{ Unmanaged(pcThis, PT_Date, pcDate, szFieldName); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClass::UnmanagedData(CBaseObject* pcThis, void* pv, size uiSizeof, char* szFieldName)
{
	CUnmanagedField*	pcUnmanagedField;

	pcUnmanagedField = AddUnmanaged(szFieldName);
	pcUnmanagedField->Init(PT_Data, (size)pv - (size)pcThis, this, uiSizeof, 1, szFieldName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClass::UnmanagedVoid(CBaseObject* pcThis, void* pv, char* szFieldName)
{
	CUnmanagedField*	pcUnmanagedField;

	pcUnmanagedField = AddUnmanaged(szFieldName);
	pcUnmanagedField->Init(PT_void, (size)pv - (size)pcThis, this, 0, szFieldName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClass::Unmanaged(CBaseObject* pcThis, EPrimitiveType eType, void* pv, char* szFieldName)
{
	CUnmanagedField*	pcUnmanagedField;

	pcUnmanagedField = AddUnmanaged(szFieldName);
	pcUnmanagedField->Init(eType, (size)pv - (size)pcThis, this, szFieldName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClass::Unmanaged(CBaseObject* pcThis, EPrimitiveType eType, void* pv, size uiLength, char* szFieldName)
{
	CUnmanagedField*	pcUnmanagedField;

	pcUnmanagedField = AddUnmanaged(szFieldName);
	pcUnmanagedField->Init(eType, (size)pv - (size)pcThis, this, uiLength, szFieldName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CClasses* CClass::GetClasses(void)
{
	return mpcClassesThisIn;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CClass::GetName(void)
{
	return mszClassName.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EPrimitiveType CClass::GetType(void)
{
	return meType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CClass::GetSizeOf(void)
{
	return muiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CClass* CClass::GetClass(EPrimitiveType eType)
{
	return mpcClassesThisIn->Get(eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CField* CClass::AddField(size uiFieldSize, char* szName)
{
	size	uiNameLength;
	CField* pcField;

	uiNameLength = strlen(szName) + 1;
	pcField = (CField*)macFields.Add(uiFieldSize + uiNameLength);
	return pcField;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnmanagedField* CClass::AddUnmanaged(char* szFieldName)
{
	CUnmanagedField* pcUnmanagedField;

	pcUnmanagedField = (CUnmanagedField*)AddField(sizeof(CUnmanagedField), szFieldName);
	PostMalloc<CUnmanagedField>(pcUnmanagedField);
	mapcUnmanaged.Add(pcUnmanagedField);
	return pcUnmanagedField;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPrimitiveField* CClass::AddPrimitive(char* szFieldName)
{
	CPrimitiveField* pcPrimitiveField;

	pcPrimitiveField = (CPrimitiveField*)AddField(sizeof(CPrimitiveField), szFieldName);
	PostMalloc<CPrimitiveField>(pcPrimitiveField);
	mapcPrimitives.Add(pcPrimitiveField);
	return pcPrimitiveField;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayVoidPtr* CClass::GetPointerFields(void)
{
	return &mapcPointers;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayVoidPtr* CClass::GetEmbeddedObjectFields(void)
{
	return &mapcEmbeddedObjects;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayVoidPtr* CClass::GetPrimitiveFields(void)
{
	return &mapcPrimitives;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayVoidPtr* CClass::GetUnmanagedFields(void)
{
	return &mapcUnmanaged;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CClass::ComputeSize(void)
{
	return 0;
}

