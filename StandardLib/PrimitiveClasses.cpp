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
	
	mpcInt8 = AddClassWithConstructorAndIO<Int8>(PT_int8);
	mpcInt16 = AddClassWithConstructorAndIO<Int16>(PT_int16);
	mpcInt32 = AddClassWithConstructorAndIO<Int32>(PT_int32);
	mpcInt64 = AddClassWithConstructorAndIO<Int64>(PT_int64);
	mpcUInt8 = AddClassWithConstructorAndIO<UInt8>(PT_uint8);
	mpcUInt16 = AddClassWithConstructorAndIO<UInt16>(PT_uint16);
	mpcUInt32 = AddClassWithConstructorAndIO<UInt32>(PT_uint32);
	mpcUInt64 = AddClassWithConstructorAndIO<UInt64>(PT_uint64);
	mpcFloat32 = AddClassWithConstructorAndIO<Char8>(PT_char8);
	mpcFloat64 = AddClassWithConstructorAndIO<Char16>(PT_char16);
	mpcBool = AddClassWithConstructorAndIO<Bool>(PT_bool);
	mpcChar8 = AddClassWithConstructorAndIO<Float32>(PT_float32);
	mpcChar16 = AddClassWithConstructorAndIO<Float64>(PT_float64);

	mpcVoid = AddVoid();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPrimitiveClasses::Kill(void)
{
	mpcClasses = NULL;
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
CClass* CPrimitiveClasses::AddVoid(void)
{
	Void	cVoid;
	CClass* pcClass;

	pcClass = mpcClasses->Add(cVoid.ClassName(), cVoid.ClassSize());
	pcClass->Primitive();
	pcClass->System();
	pcClass->Complete();

	mpcClasses->AddConstructorAndIO<Void>();

	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
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

