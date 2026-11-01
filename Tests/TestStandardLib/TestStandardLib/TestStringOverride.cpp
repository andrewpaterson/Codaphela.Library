#include "BaseLib/GlobalMemory.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/LogToMemory.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/SequenceFactory.h"
#include "StandardLib/Objects.h"
#include "StandardLib/String.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStringOverrideCharPointerPlusDeconstructor(void)
{
	Ptr<CString>	pString1 = OMalloc<CString>("Hello ");
	Ptr<CString>	pString3 = NULL;
	AssertLong(2, gcObjects.NumMemoryIndexes());

	pString3 = pString1 + " World";
	AssertLong(3, gcObjects.NumMemoryIndexes());

	AssertString("Hello  World", pString3->Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStringOverrideCharPointerPlus(void)
{
	MemoryInit();
	TypesInit();
	DataIOInit();
	ObjectsInit();

	AssertLong(0, gcObjects.NumMemoryIndexes());
	TestStringOverrideCharPointerPlusDeconstructor();
	AssertLong(0, gcObjects.NumMemoryIndexes());

	ObjectsKill();
	DataIOKill();
	TypesKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStringOverrideStringPlusDeconstructor(void)
{
	Ptr<CString>	pString1 = OMalloc<CString>("Hello ");
	Ptr<CString>	pString2 = OMalloc<CString>(" World");
	Ptr<CString>	pString3 = NULL;
	AssertLong(2, gcObjects.NumMemoryIndexes());

	pString3 = pString1 + pString2;
	AssertLong(3, gcObjects.NumMemoryIndexes());

	AssertString("Hello  World", pString3->Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStringOverrideStringPlus(void)
{
	MemoryInit();
	TypesInit();
	DataIOInit();
	ObjectsInit();

	AssertLong(0, gcObjects.NumMemoryIndexes());
	TestStringOverrideStringPlusDeconstructor();
	AssertLong(0, gcObjects.NumMemoryIndexes());

	ObjectsKill();
	DataIOKill();
	TypesKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStringOverride(void)
{
	BeginTests();

	TestStringOverrideStringPlus();

	TestStatistics();
}

