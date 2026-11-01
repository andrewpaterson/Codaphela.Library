#include "BaseLib/Float32Vec3.h"
#include "BaseLib/IntVec2.h"
#include "BaseLib/DatasIO.h"
#include "BaseLib/MemoryFile.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDataTypesIOInt2(void)
{
	CDatasIO			cDataIO;
	SIntVec2			si2In;
	DataIO_FileWriter	fSave;
	DataIO_FileReader	fLoad;
	bool				bResult;
	CMemoryFile			cMemoryFile;
	CFileBasic			cFile;
	SIntVec2			si2Out;

	cDataIO.Init();
	cDataIO.Add<SIntVec2>("SIntVec2");
	fSave = cDataIO.GetFileWriter("SIntVec2");
	fLoad = cDataIO.GetFileReader("SIntVec2");

	cMemoryFile.Init();
	cFile.Init(&cMemoryFile);
	cFile.Open(EFM_Write_Create);
#ifdef __W65C816__
	si2In.Init(0x1020, (int)0x80a0);  //An int is 16 bits.
#else
	si2In.Init(0x10203040, 0x80a0b0c0);
#endif
	bResult = (((SDataTypeIO*)&si2In)->*fSave)(&cFile);
	AssertTrue(bResult);
	cFile.Close();

	cFile.Open(EFM_Read);
	bResult = (((SDataTypeIO*)&si2Out)->*fLoad)(&cFile);
	AssertTrue(bResult);
	cFile.Close();
	cFile.Kill();
	cMemoryFile.Kill();

#ifdef __W65C816__
	AssertInt((int)0x1020, si2Out.x);
	AssertInt((int)0x80a0, si2Out.y);
#else
	AssertInt((int)0x10203040, si2Out.x);
	AssertInt((int)0x80a0b0c0, si2Out.y);
#endif

	cDataIO.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDataTypesIOMultiple(void)
{
	CDatasIO			cDataIO;
	SFloat32Vec2				sf2In;
	SFloat32Vec3				sf3In;
	SFloat32Vec4				sf4In;
	bool				bResult;
	CMemoryFile			cMemoryFile;
	CFileBasic			cFile;
	SFloat32Vec2				sf2Out;
	SFloat32Vec3				sf3Out;
	SFloat32Vec4				sf4Out;

	cDataIO.Init();
	cDataIO.Add<SFloat32Vec2>();
	cDataIO.Add<SFloat32Vec3>();
	cDataIO.Add<SFloat32Vec4>();

	sf2In.Init(0.1f, 2.3f);
	sf3In.Init(4.5f, 6.7f, 8.9f);
	sf4In.Init(0.9f, 1.8f, 2.7f, 3.6f);

	cMemoryFile.Init();
	cFile.Init(&cMemoryFile);
	cFile.Open(EFM_Write_Create);
	bResult = (((SDataTypeIO*)&sf2In)->*(cDataIO.GetFileWriter<SFloat32Vec2>()))(&cFile);
	AssertTrue(bResult);
	bResult = (((SDataTypeIO*)&sf3In)->*(cDataIO.GetFileWriter<SFloat32Vec3>()))(&cFile);
	AssertTrue(bResult);
	bResult = (((SDataTypeIO*)&sf4In)->*(cDataIO.GetFileWriter<SFloat32Vec4>()))(&cFile);
	AssertTrue(bResult);
	cFile.Close();

	cFile.Open(EFM_Read);
	bResult = (((SDataTypeIO*)&sf2Out)->*(cDataIO.GetFileReader<SFloat32Vec2>()))(&cFile);
	AssertTrue(bResult);
	bResult = (((SDataTypeIO*)&sf3Out)->*(cDataIO.GetFileReader<SFloat32Vec3>()))(&cFile);
	AssertTrue(bResult);
	bResult = (((SDataTypeIO*)&sf4Out)->*(cDataIO.GetFileReader<SFloat32Vec4>()))(&cFile);
	AssertTrue(bResult);
	cFile.Close();
	cFile.Kill();
	cMemoryFile.Kill();

	AssertFloat(0.1f, sf2Out.x, 3);
	AssertFloat(2.3f, sf2Out.y, 3);

	AssertFloat(4.5f, sf3Out.x, 3);
	AssertFloat(6.7f, sf3Out.y, 3);
	AssertFloat(8.9f, sf3Out.z, 3);

	AssertFloat(0.9f, sf4Out.x, 3);
	AssertFloat(1.8f, sf4Out.y, 3);
	AssertFloat(2.7f, sf4Out.z, 3);
	AssertFloat(3.6f, sf4Out.w, 3);

	cDataIO.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDataTypesIO(void)
{
	BeginTests();

	TestDataTypesIOInt2();
	TestDataTypesIOMultiple();

	TestStatistics();
}

