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
#include "IndexTreeFileDataTestCallback.h"
#include "IndexTreeCallbackTestObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileCallbackWorks(void)
{
	CIndexTreeHelper									cHelper;
	CDurableFileController								cController;
	CIndexTreeFile										cIndexTree;
	CIndexTreeFileAccess								cAccess;
	CIndexTreeFileDataTestCallback							cDataCallback;
	CTestIndexTreeMemoryObject							cTestObject1;
	char												szKey1[3] = "AB";
	CTestIndexTreeMemoryObject							cTestObject2;
	char												szKey2[3] = "AD";
	CTestIndexTreeMemoryObject							cResult;


	cHelper.Init("Output" _FS_"IndexTreeCallback", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, &cDataCallback, IWT_Yes, IKR_No);
	cAccess.Init(&cIndexTree);

	cTestObject1.Init(1, 2, 3, 4);
	AssertTrue(cAccess.PutStringData(szKey1, &cTestObject1, sizeof(CTestIndexTreeMemoryObject)));

	cController.End();
	cIndexTree.Kill();

	cController.Begin();
	cIndexTree.Init(&cController, NULL, &cDataCallback, IWT_Yes, IKR_No);

	memset(&cResult, 0, sizeof(CTestIndexTreeMemoryObject));
	AssertTrue(cIndexTree.Get((uint8*)szKey1, 2, &cResult, NULL, 0));
	AssertInt(17, cResult.mi1);
	AssertInt(2, cResult.mi2);
	AssertInt(56, cResult.mi3);
	AssertInt(4, cResult.mi4);

	cController.End();
	cIndexTree.Kill();

	cController.Begin();
	cIndexTree.Init(&cController, NULL, &cDataCallback, IWT_No, IKR_No);

	cTestObject1.Init(5, 6, 7, 8);
	AssertTrue(cAccess.PutStringData(szKey1, &cTestObject1, sizeof(CTestIndexTreeMemoryObject)));
	cTestObject2.Init(9, 10, 11, 12);
	AssertTrue(cAccess.PutStringData(szKey2, &cTestObject2, sizeof(CTestIndexTreeMemoryObject)));
	
	cIndexTree.Flush();
	cController.End();
	cIndexTree.Kill();

	cController.Begin();
	cIndexTree.Init(&cController, NULL, &cDataCallback, IWT_No, IKR_No);

	memset(&cResult, 0, sizeof(CTestIndexTreeMemoryObject));
	AssertTrue(cIndexTree.Get((uint8*)szKey1, 2, &cResult, NULL, 0));
	AssertInt(17, cResult.mi1);
	AssertInt(6, cResult.mi2);
	AssertInt(56, cResult.mi3);
	AssertInt(8, cResult.mi4);

	memset(&cResult, 0, sizeof(CTestIndexTreeMemoryObject));
	AssertTrue(cIndexTree.Get((uint8*)szKey2, 2, &cResult, NULL, 0));
	AssertInt(17, cResult.mi1);
	AssertInt(10, cResult.mi2);
	AssertInt(56, cResult.mi3);
	AssertInt(12, cResult.mi4);

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
void TestIndexTreeFileCallback(void)
{
	TypeConverterInit();
	MemoryInit();
	DataMemoryInit();
	BeginTests();

	TestIndexTreeFileCallbackWorks();

	TestStatistics();
	DataMemoryKill();
	MemoryKill();
	TypeConverterKill();
}

