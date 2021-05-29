#include "BaseLib/TypeNames.h"
#include "Classes.h"
#include "PrimitiveClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPrimitiveClasses::Init(CClasses* pcClasses)
{
	mpcClasses = pcClasses;

	mpcUndefined = AddClass(PT_Undefined);
	mpcVoidPointer = AddClass(PT_Pointer);
	mpcInt8 = AddClass(PT_int8);
	mpcInt16 = AddClass(PT_int16);
	mpcInt32 = AddClass(PT_int32);
	mpcInt64 = AddClass(PT_int64);
	mpcUInt8 = AddClass(PT_uint8);
	mpcUInt16 = AddClass(PT_uint16);
	mpcUInt32 = AddClass(PT_uint32);
	mpcUInt64 = AddClass(PT_uint64);
	mpcFloat32 = AddClass(PT_float32);
	mpcFloat64 = AddClass(PT_float64);
	mpcM2float32 = AddClass(PT_M2float32);
	mpcM3float32 = AddClass(PT_M3float32);
	mpcM4float32 = AddClass(PT_M4float32);
	mpcM2float64 = AddClass(PT_M2float64);
	mpcM3float64 = AddClass(PT_M3float64);
	mpcM4float64 = AddClass(PT_M4float64);
	mpcM2int32 = AddClass(PT_M2int32);
	mpcM3int32 = AddClass(PT_M3int32);
	mpcM4int32 = AddClass(PT_M4int32);
	mpcBool = AddClass(PT_bool);
	mpcVoid = AddClass(PT_void);
	mpcChar8 = AddClass(PT_char8);
	mpcChar16 = AddClass(PT_char16);
	mpcBit = AddClass(PT_bit);
	mpcCrumb = AddClass(PT_crumb);		
	mpcTribble = AddClass(PT_tribble);
	mpcNybble = AddClass(PT_nybble);
	mpcNickle = AddClass(PT_nickle);
	mpcSixbits = AddClass(PT_sixbits);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPrimitiveClasses::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CClass* CPrimitiveClasses::AddClass(EPrimitiveType eType)
{
	const char*		pszName;
	int				iSize;
	CClass*			pcClass;

	TypesValidate();

	pszName = gcTypeNames.GetPrettyName(eType);
	iSize = gcTypeNames.GetByteSize(eType);
	pcClass = mpcClasses->Add(pszName, iSize, eType);
	pcClass->Primitive();
	pcClass->Complete();
	return pcClass;
}


CClass* CPrimitiveClasses::GetUndefined(void) { return mpcUndefined; }
CClass* CPrimitiveClasses::GetVoidPointer(void) { return mpcVoidPointer; }
CClass* CPrimitiveClasses::GetByte(void) { return mpcInt8; }
CClass* CPrimitiveClasses::GetShort(void) { return mpcInt16; }
CClass* CPrimitiveClasses::GetInt(void) { return mpcInt32; }
CClass* CPrimitiveClasses::GetLong(void) { return mpcInt64; }
CClass* CPrimitiveClasses::GetUByte(void) { return mpcUInt8; }
CClass* CPrimitiveClasses::GetUShort(void) { return mpcUInt16; }
CClass* CPrimitiveClasses::GetUInt(void) { return mpcUInt32; }
CClass* CPrimitiveClasses::GetULong(void) { return mpcUInt64; }
CClass* CPrimitiveClasses::GetFloat(void) { return mpcFloat32; }
CClass* CPrimitiveClasses::GetDouble(void) { return mpcFloat64; }
CClass* CPrimitiveClasses::GetFloat2(void) { return mpcM2float32; }
CClass* CPrimitiveClasses::GetFloat3(void) { return mpcM3float64; }
CClass* CPrimitiveClasses::GetFloat4(void) { return mpcM4float64; }
CClass* CPrimitiveClasses::GetDouble2(void) { return mpcM2float64; }
CClass* CPrimitiveClasses::GetDouble3(void) { return mpcM3float64; }
CClass* CPrimitiveClasses::GetDouble4(void) { return mpcM4float64; }
CClass* CPrimitiveClasses::GetInt2(void) { return mpcM2int32; }
CClass* CPrimitiveClasses::GetInt3(void) { return mpcM3int32; }
CClass* CPrimitiveClasses::GetInt4(void) { return mpcM4int32; }
CClass* CPrimitiveClasses::GetBool(void) { return mpcBool; }
CClass* CPrimitiveClasses::GetVoid(void) { return mpcVoid; }
CClass* CPrimitiveClasses::GetChar(void) { return mpcChar8; }
CClass* CPrimitiveClasses::GetWidechar(void) { return mpcChar16; }
CClass* CPrimitiveClasses::GetString(void) { return mpcString; }
CClass* CPrimitiveClasses::GetNumber(void) { return mpcNumber; }
CClass* CPrimitiveClasses::GetDate(void) { return mpcDate; }
CClass* CPrimitiveClasses::GetBit(void) { return mpcBit; }
CClass* CPrimitiveClasses::GetCrumb(void) { return mpcCrumb; }
CClass* CPrimitiveClasses::GetTribble(void) { return mpcTribble; }
CClass* CPrimitiveClasses::GetNybble(void) { return mpcNybble; }
CClass* CPrimitiveClasses::GetNickle(void) { return mpcNickle; }
CClass* CPrimitiveClasses::GetSixbits(void) { return mpcSixbits; }

