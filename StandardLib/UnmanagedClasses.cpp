#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "Classes.h"
#include "UnmanagedClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUnmanagedClasses::Init(CClasses* pcClasses)
{
	mpcClasses = pcClasses;

	mpcUndefined = AddClassByPrettyName(PT_Undefined);

	mpcVoidPointer = AddClassByPrettyName(PT_Pointer);
	mpcInt8 = AddClassByPrettyName(PT_int8);
	mpcInt16 = AddClassByPrettyName(PT_int16);
	mpcInt32 = AddClassByPrettyName(PT_int32);
	mpcInt64 = AddClassByPrettyName(PT_int64);
	mpcUInt8 = AddClassByPrettyName(PT_uint8);
	mpcUInt16 = AddClassByPrettyName(PT_uint16);
	mpcUInt32 = AddClassByPrettyName(PT_uint32);
	mpcUInt64 = AddClassByPrettyName(PT_uint64);
	mpcFloat32 = AddClassByPrettyName(PT_float32);
	mpcFloat64 = AddClassByPrettyName(PT_float64);
	mpcM2float32 = AddClassByPrettyName(PT_M2float32);
	mpcM3float32 = AddClassByPrettyName(PT_M3float32);
	mpcM4float32 = AddClassByPrettyName(PT_M4float32);
	mpcM2float64 = AddClassByPrettyName(PT_M2float64);
	mpcM3float64 = AddClassByPrettyName(PT_M3float64);
	mpcM4float64 = AddClassByPrettyName(PT_M4float64);
	mpcM2int32 = AddClassByPrettyName(PT_M2int32);
	mpcM3int32 = AddClassByPrettyName(PT_M3int32);
	mpcM4int32 = AddClassByPrettyName(PT_M4int32);
	mpcBool = AddClassByPrettyName(PT_bool);
	mpcVoid = AddClassByPrettyName(PT_void);
	mpcChar8 = AddClassByPrettyName(PT_char8);
	mpcChar16 = AddClassByPrettyName(PT_char16);

	mpcBit = AddClassByPrettyName(PT_bit);
	mpcCrumb = AddClassByPrettyName(PT_crumb);		
	mpcTribble = AddClassByPrettyName(PT_tribble);
	mpcNybble = AddClassByPrettyName(PT_nybble);
	mpcNickle = AddClassByPrettyName(PT_nickle);
	mpcSixbits = AddClassByPrettyName(PT_sixbits);

	mpcString = AddClassByCPPName(PT_String);
	mpcNumber = AddClassByCPPName(PT_Number);
	mpcDate = AddClassByCPPName(PT_Date);
	mpcDateTime = AddClassByCPPName(PT_DateTime);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUnmanagedClasses::Kill(void)
{
	mpcClasses = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CClass* CUnmanagedClasses::AddClassByPrettyName(EPrimitiveType eType)
{
	const char* pszName;
	int			iSize;

	TypesValidate();

	pszName = gcTypeNames.GetPrettyName(eType);
	iSize = gcTypeNames.GetByteSize(eType);
	return AddClass(eType, pszName, iSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CClass* CUnmanagedClasses::AddClassByCPPName(EPrimitiveType eType)
{
	const char*	pszName;
	int			iSize;

	TypesValidate();

	pszName = gcTypeNames.GetCPPName(eType);
	iSize = gcTypeNames.GetByteSize(eType);
	return AddClass(eType, pszName, iSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CClass* CUnmanagedClasses::AddClass(EPrimitiveType eType, const char* szName, int iSize)
{
	CClass*			pcClass;

	pcClass = mpcClasses->Add(szName, iSize, eType);
	pcClass->System();
	pcClass->Unmanaged();
	pcClass->Complete();
	return pcClass;
}


CClass* CUnmanagedClasses::GetUndefined(void) { return mpcUndefined; }
CClass* CUnmanagedClasses::GetVoidPointer(void) { return mpcVoidPointer; }
CClass* CUnmanagedClasses::GetByte(void) { return mpcInt8; }
CClass* CUnmanagedClasses::GetShort(void) { return mpcInt16; }
CClass* CUnmanagedClasses::GetInt(void) { return mpcInt32; }
CClass* CUnmanagedClasses::GetLong(void) { return mpcInt64; }
CClass* CUnmanagedClasses::GetUByte(void) { return mpcUInt8; }
CClass* CUnmanagedClasses::GetUShort(void) { return mpcUInt16; }
CClass* CUnmanagedClasses::GetUInt(void) { return mpcUInt32; }
CClass* CUnmanagedClasses::GetULong(void) { return mpcUInt64; }
CClass* CUnmanagedClasses::GetFloat(void) { return mpcFloat32; }
CClass* CUnmanagedClasses::GetDouble(void) { return mpcFloat64; }
CClass* CUnmanagedClasses::GetFloat2(void) { return mpcM2float32; }
CClass* CUnmanagedClasses::GetFloat3(void) { return mpcM3float64; }
CClass* CUnmanagedClasses::GetFloat4(void) { return mpcM4float64; }
CClass* CUnmanagedClasses::GetDouble2(void) { return mpcM2float64; }
CClass* CUnmanagedClasses::GetDouble3(void) { return mpcM3float64; }
CClass* CUnmanagedClasses::GetDouble4(void) { return mpcM4float64; }
CClass* CUnmanagedClasses::GetInt2(void) { return mpcM2int32; }
CClass* CUnmanagedClasses::GetInt3(void) { return mpcM3int32; }
CClass* CUnmanagedClasses::GetInt4(void) { return mpcM4int32; }
CClass* CUnmanagedClasses::GetBool(void) { return mpcBool; }
CClass* CUnmanagedClasses::GetVoid(void) { return mpcVoid; }
CClass* CUnmanagedClasses::GetChar(void) { return mpcChar8; }
CClass* CUnmanagedClasses::GetWidechar(void) { return mpcChar16; }
CClass* CUnmanagedClasses::GetString(void) { return mpcString; }
CClass* CUnmanagedClasses::GetNumber(void) { return mpcNumber; }
CClass* CUnmanagedClasses::GetDate(void) { return mpcDate; }
CClass* CUnmanagedClasses::GetDateTime(void) { return mpcDateTime; }
CClass* CUnmanagedClasses::GetBit(void) { return mpcBit; }
CClass* CUnmanagedClasses::GetCrumb(void) { return mpcCrumb; }
CClass* CUnmanagedClasses::GetTribble(void) { return mpcTribble; }
CClass* CUnmanagedClasses::GetNybble(void) { return mpcNybble; }
CClass* CUnmanagedClasses::GetNickle(void) { return mpcNickle; }
CClass* CUnmanagedClasses::GetSixbits(void) { return mpcSixbits; }

