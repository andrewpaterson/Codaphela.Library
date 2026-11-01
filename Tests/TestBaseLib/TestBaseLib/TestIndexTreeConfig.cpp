#include "BaseLib/IndexTreeMemory.h"
#include "BaseLib/IndexTreeMemoryAccess.h"
#include "BaseLib/IndexTreeConfig.h"
#include "BaseLib/CreationDataOrderer.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/MemoryAllocator.h"
#include "BaseLib/GlobalMemoryAllocator.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/LocalMallocatorConstructors.h"
#include "BaseLib/DataOrderers.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeMemoryConfigGlobalMallocatorReading(void)
{
	CIndexTreeMemory					cIndexTree;
	CIndexTreeMemoryAccess				cAccess;
	CLifeInit<CIndexTreeDataOrderer>	cOrderer;
	CFileBasic							cConfigFile;
	bool								bResult;
	CFileUtil							cFileUtil;
	CFileBasic							cTreeFile;
	CIndexTreeMemoryConfig				cTreeConfig;
	CLifeInit<CMallocator>				cMallocator;

	cMallocator.Init(&gcMemoryAllocator, false, false);
	cOrderer = CCreationDataOrderer::Create();

	cIndexTree.Init(cMallocator, IKR_Yes, 512, 8, cOrderer);
	cAccess.Init(&cIndexTree);

	cFileUtil.MakeDir("Output" _FS_ "IndexTreeConfig1");
	cConfigFile.Init(DiskFile("Output" _FS_ "IndexTreeConfig1" _FS_ "Tree.cfg"));
	cConfigFile.Open(EFM_ReadWrite_Create);
	bResult = cIndexTree.WriteConfig(&cConfigFile);
	AssertTrue(bResult);
	cConfigFile.Close();
	cConfigFile.Kill();

	cAccess.PutStringChar("0000", 'a');
	cAccess.PutStringChar("0100", 'b');
	cAccess.PutStringChar("0010", 'c');
	cAccess.PutStringChar("0110", 'd');

	cTreeFile.Init(DiskFile("Output" _FS_ "IndexTreeConfig1" _FS_ "Tree.dat"));
	cTreeFile.Open(EFM_ReadWrite_Create);
	bResult = cIndexTree.Write(&cTreeFile);
	AssertTrue(bResult);
	cTreeFile.Close();
	cTreeFile.Kill();

	cIndexTree.Kill();
	cAccess.Kill();

	cConfigFile.Init(DiskFile("Output" _FS_ "IndexTreeConfig1" _FS_ "Tree.cfg"));
	cConfigFile.Open(EFM_Read);
	bResult = cTreeConfig.Init(&cConfigFile);
	AssertTrue(bResult);
	cConfigFile.Close();
	cConfigFile.Kill();

	cIndexTree.Init(&cTreeConfig, &cIndexTree, &cIndexTree, NULL);
	cIndexTree.Kill();

	cTreeFile.Init(DiskFile("Output" _FS_ "IndexTreeConfig1" _FS_ "Tree.dat"));
	cTreeFile.Open(EFM_Read);
	bResult = cIndexTree.Read(&cTreeFile);
	AssertTrue(bResult);
	cTreeFile.Close();
	cTreeFile.Kill();

	cIndexTree.Kill();

	cFileUtil.RemoveDir("Output" _FS_ "IndexTreeConfig1");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeMemoryConfigLocalMallocatorReading(void)
{
	CIndexTreeMemory					cIndexTree;
	CIndexTreeMemoryAccess				cAccess;
	CLifeInit<CIndexTreeDataOrderer>	cOrderer;
	CFileBasic							cConfigFile;
	bool								bResult;
	CFileUtil							cFileUtil;
	CFileBasic							cTreeFile;
	CIndexTreeMemoryConfig				cTreeConfig;
	CLifeInit<CMallocator>				cMallocator;

	cMallocator = CMemoryAllocator::Create();
	cOrderer = CCreationDataOrderer::Create();

	cIndexTree.Init(cMallocator, IKR_Yes, 512, 8, cOrderer);
	cAccess.Init(&cIndexTree);

	cFileUtil.MakeDir("Output" _FS_ "IndexTreeConfig2");
	cConfigFile.Init(DiskFile("Output" _FS_ "IndexTreeConfig2" _FS_ "Tree.cfg"));
	cConfigFile.Open(EFM_ReadWrite_Create);
	bResult = cIndexTree.WriteConfig(&cConfigFile);
	AssertTrue(bResult);
	cConfigFile.Close();
	cConfigFile.Kill();

	cAccess.PutStringChar("0000", 'a');
	cAccess.PutStringChar("0100", 'b');
	cAccess.PutStringChar("0010", 'c');
	cAccess.PutStringChar("0110", 'd');

	cTreeFile.Init(DiskFile("Output" _FS_ "IndexTreeConfig2" _FS_ "Tree.dat"));
	cTreeFile.Open(EFM_ReadWrite_Create);
	//Write writes out the Config into the Tree.dat file.
	bResult = cIndexTree.Write(&cTreeFile);
	AssertTrue(bResult);
	cTreeFile.Close();
	cTreeFile.Kill();

	cIndexTree.Kill();
	cAccess.Kill();

	cConfigFile.Init(DiskFile("Output" _FS_ "IndexTreeConfig2" _FS_ "Tree.cfg"));
	cConfigFile.Open(EFM_Read);
	bResult = cTreeConfig.Init(&cConfigFile);
	AssertTrue(bResult);
	cConfigFile.Close();
	cConfigFile.Kill();

	cIndexTree.Init(&cTreeConfig, &cIndexTree, &cIndexTree, NULL);
	cTreeConfig.Kill();
	cIndexTree.Kill();

	cTreeFile.Init(DiskFile("Output" _FS_ "IndexTreeConfig2" _FS_ "Tree.dat"));
	cTreeFile.Open(EFM_Read);
	//Read reads in the config embedded in the Tree.dat file.
	bResult = cIndexTree.Read(&cTreeFile);
	AssertTrue(bResult);
	cTreeFile.Close();
	cTreeFile.Kill();

	cIndexTree.Kill();

	cFileUtil.RemoveDir("Output" _FS_ "IndexTreeConfig2");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeMemoryConfig(void)
{
	BeginTests();
	MemoryInit();
	LocalMallocatorsInit(&gcConstructors, &gcMallocators);
	DataOrderersInit();

	TestIndexTreeMemoryConfigLocalMallocatorReading();
	TestIndexTreeMemoryConfigGlobalMallocatorReading();

	DataOrderersKill();
	MemoryKill();
	TestStatistics();
}

