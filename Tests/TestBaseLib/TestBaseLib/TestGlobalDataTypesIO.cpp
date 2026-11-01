#include "BaseLib/TypeNames.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/MemoryFile.h"
#include "TestLib/Assert.h"

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGlobalDataTypesIOPrimitives(void)
{
	int8			x;
	int16			y;
	uint32			z;
	float64			f;
	SDataIO*		psIOByte;
	SDataIO*		psIOShort;
	SDataIO*		psIOInt;
	SDataIO*		psIODouble;
	CMemoryFile		cMemoryFile;
	CFileBasic		cFile;

	DataIOInit();
	
	x = 0x6c;
	y = 0x39e7;
	z = 0xf07d8a2;
	f = 3.14159265358979323846;

	psIOByte = gcDataTypesIO.GetIO("Byte");
	AssertNotNull(psIOByte);
	psIOShort = gcDataTypesIO.GetIO("Short");
	psIOInt = gcDataTypesIO.GetIO("Int");
	psIODouble = gcDataTypesIO.GetIO("Double");

	cMemoryFile.Init();
	cFile.Init(&cMemoryFile);
	cFile.Open(EFM_Write_Create);
	AssertTrue((((SDataTypeIO*)&x)->*psIOByte->fWriter)(&cFile));
	AssertTrue((((SDataTypeIO*)&y)->*psIOShort->fWriter)(&cFile));
	AssertTrue((((SDataTypeIO*)&z)->*psIOInt->fWriter)(&cFile));
	AssertTrue((((SDataTypeIO*)&f)->*psIODouble->fWriter)(&cFile));
	cFile.Close();

	x = 0;
	y = 0;
	z = 0;
	f = 0;

	cFile.Open(EFM_Read);
	AssertTrue((((SDataTypeIO*)&x)->*psIOByte->fReader)(&cFile));
	AssertTrue((((SDataTypeIO*)&y)->*psIOShort->fReader)(&cFile));
	AssertTrue((((SDataTypeIO*)&z)->*psIOInt->fReader)(&cFile));
	AssertTrue((((SDataTypeIO*)&f)->*psIODouble->fReader)(&cFile));
	cFile.Close();
	cFile.Kill();
	cMemoryFile.Kill();

	AssertInt(0x6c, x);
	AssertInt(0x39e7, y);
	AssertInt(0xf07d8a2, z);
	AssertDouble(3.14159265358979323846, f, 19);

	DataIOKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGlobalDataTypesIO(void)
{
	BeginTests();
	TypesInit();

	TestGlobalDataTypesIOPrimitives();

	TypesKill();
	TestStatistics();
}

