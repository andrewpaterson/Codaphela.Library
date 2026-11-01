#include "BaseLib/FileUtil.h"
#include "BaseLib/SystemAllocator.h"
#include "BaseLib/MemoryAllocator.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/NaiveFile.h"
#include "BaseLib/MapStringString.h"
#include "BaseLib/Logger.h"
#include "BaseLib/DebugOutput.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/IndexTreeHelper.h"
#include "BaseLib/IndexTreeFile.h"
#include "BaseLib/IndexTreeFileAccess.h"
#include "TestLib/Assert.h"
#include "TestIndexTreeObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeTemplateFileStuff(void)
{
	CIndexTreeHelper								cHelper;
	CDurableFileController							cController;
	CIndexTreeFile									cIndexTree;
	CIndexTreeFileAccess							cAccess;
	CTestIndexTreeObject2							cObject1;
	CTestIndexTreeObject2							cObject2;
	CTestIndexTreeObject2							cResult;

	cHelper.Init("Output" _FS_"IndexTree0", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, IWT_Yes, IKR_No);
	cAccess.Init(&cIndexTree);
	
	cObject1.Init("Hello");
	cObject2.Init("Haplo");

	AssertTrue(cAccess.PutKeyData((uint8*)cObject1.GetName(), cObject1.NameLength(), &cObject1, sizeof(CTestIndexTreeObject2)));
	AssertTrue(cAccess.PutKeyData((uint8*)cObject2.GetName(), cObject2.NameLength(), &cObject2, sizeof(CTestIndexTreeObject2)));
	AssertInt((uint32)2, cIndexTree.NumElements());
		
	AssertTrue(cIndexTree.Get((uint8*)cObject1.GetName(), cObject1.NameLength(), &cResult, NULL, 0));
	AssertString("Hello", cResult.GetName());
	AssertTrue(cIndexTree.Get((uint8*)cObject2.GetName(), cObject2.NameLength(), &cResult, NULL, 0));
	AssertString("Haplo", cResult.GetName());
	AssertFalse(cIndexTree.Get((uint8*)"Help", 4, &cResult, NULL, 0));

	AssertTrue(cIndexTree.Remove((uint8*)cObject1.GetName(), cObject1.NameLength()));
	AssertTrue(cIndexTree.Get((uint8*)cObject2.GetName(), cObject2.NameLength(), &cResult, NULL, 0));
	AssertString("Haplo", cResult.GetName());
	AssertFalse(cIndexTree.Get((uint8*)cObject1.GetName(), cObject1.NameLength(), &cResult, NULL, 0));
	AssertFalse(cIndexTree.Get((uint8*)"Hap", 3, &cResult, NULL, 0));
	AssertInt((uint32)1, cIndexTree.NumElements());

	AssertTrue(cIndexTree.Remove((uint8*)cObject2.GetName(), cObject2.NameLength()));
	AssertFalse(cIndexTree.Get((uint8*)cObject2.GetName(), cObject2.NameLength(), &cResult, NULL, 0));
	AssertFalse(cIndexTree.Get((uint8*)cObject1.GetName(), cObject1.NameLength(), &cResult, NULL, 0));
	AssertInt((uint32)0, cIndexTree.NumElements());

	cController.End();

	cAccess.Kill();
	cIndexTree.Kill();
	cController.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeTemplateFile(void)
{
	TypeConverterInit();
	MemoryInit();
	DataMemoryInit();
	BeginTests();

	TestIndexTreeTemplateFileStuff();

	TestStatistics();
	DataMemoryKill();
	MemoryKill();
	TypeConverterKill();
}

