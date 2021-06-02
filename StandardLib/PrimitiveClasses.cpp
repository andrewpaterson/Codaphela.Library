#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "Classes.h"
#include "Integer.h"
#include "Float.h"
#include "Character.h"
#include "PrimitiveVoid.h"
#include "PrimitiveClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPrimitiveClasses::Init(CClasses* pcClasses)
{
	mpcClasses = pcClasses;

	gcDataTypesIO.Add<Int8>();
	gcDataTypesIO.Add<Int16>();
	gcDataTypesIO.Add<Int32>();
	gcDataTypesIO.Add<Int64>();
	gcDataTypesIO.Add<UInt8>();
	gcDataTypesIO.Add<UInt16>();
	gcDataTypesIO.Add<UInt32>();
	gcDataTypesIO.Add<UInt64>();
	gcDataTypesIO.Add<Char8>();
	gcDataTypesIO.Add<Char16>();
	gcDataTypesIO.Add<Bool>();
	gcDataTypesIO.Add<Float32>();
	gcDataTypesIO.Add<Float64>();
	gcDataTypesIO.Add<Void>();
	
	mpcInt8 = AddClass<Int8>(PT_int8);
	mpcInt16 = AddClass<Int16>(PT_int16);
	mpcInt32 = AddClass<Int32>(PT_int32);
	mpcInt64 = AddClass<Int64>(PT_int64);
	mpcUInt8 = AddClass<UInt8>(PT_uint8);
	mpcUInt16 = AddClass<UInt16>(PT_uint16);
	mpcUInt32 = AddClass<UInt32>(PT_uint32);
	mpcUInt64 = AddClass<UInt64>(PT_uint64);
	mpcFloat32 = AddClass<Char8>(PT_char8);
	mpcFloat64 = AddClass<Char16>(PT_char16);
	mpcBool = AddClass<Bool>(PT_bool);
	mpcChar8 = AddClass<Float32>(PT_float32);
	mpcChar16 = AddClass<Float64>(PT_float64);

	Void	cVoid;
	mpcVoid = AddClass(cVoid.ClassName(), cVoid.ClassSize(), PT_void, sizeof(CPrimitiveObject));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CClass* CPrimitiveClasses::AddClass(const char* szClassName, size_t uiClassSize, EPrimitiveType eFieldType, uint32 eFieldOffest) 
{
	CUnmanagedField*	pcField;
	CClass*				pcClass;

	pcClass = mpcClasses->Add(szClassName, uiClassSize);

	pcField = pcClass->AddUnmanaged("mVal");
	pcField->Init(eFieldType, eFieldOffest, pcClass, "mVal");

	pcClass->Primitive();
	pcClass->System();
	pcClass->Complete();

	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPrimitiveClasses::Kill(void)
{
	mpcClasses = NULL;
}


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
CClass* CPrimitiveClasses::GetBool(void) { return mpcBool; }
CClass* CPrimitiveClasses::GetVoid(void) { return mpcVoid; }
CClass* CPrimitiveClasses::GetChar(void) { return mpcChar8; }
CClass* CPrimitiveClasses::GetWidechar(void) { return mpcChar16; }

