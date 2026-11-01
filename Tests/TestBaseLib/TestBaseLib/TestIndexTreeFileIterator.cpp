#include "BaseLib/TypeConverter.h"
#include "BaseLib/DebugOutput.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/MapStringString.h"
#include "BaseLib/IndexTreeFile.h"
#include "BaseLib/IndexTreeHelper.h"
#include "BaseLib/IndexTreeFileIterator.h"
#include "BaseLib/IndexTreeFileAccess.h"
#include "TestLib/Words.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileIteratorUnsafeIterate(void)
{
	SIndexTreeFileUnsafeIterator	sIter;
	char*							pvData;
	uint8*							pvKey;
	size							iDataSize;
	bool							bExists;
	uint8							pacKey[9 + 1];
	size							iKeyLength;
	CIndexTreeFile					cIndexTree;
	CMapStringString				cMap;
	SMapIterator					sMapIter;
	char*							pacData;
	int								iResult;
	size							iMapDataSize;
	CIndexTreeFileAccess			cAccess;
	CIndexTreeHelper				cHelper;
	CDurableFileController			cController;

	cHelper.Init("Output" _FS_ "IndexTreeIterator1", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, IWT_No, IKR_No);
	cAccess.Init(&cIndexTree);

	cMap.Init();
	cMap.Put("AA", "nutritious");
	cMap.Put("AAA", "follow");
	cMap.Put("AB", "lighten");
	cMap.Put("BAAB", "ear");
	cMap.Put("BB", "spotted");
	cMap.Put("AAAAAAAAA", "hanging");
	cMap.Put("ABBA", "mate");
	cMap.Put("ABA", "blot");
	cMap.Put("BCD", "smash");
	cMap.Put("BBC", "cycle");
	cMap.Put("BBB", "waggish");
	cMap.Put("BBA", "debt");

	AssertSize(12, cMap.NumElements());

	bExists = cMap.StartIteration(&sMapIter, (void**)&pvKey, NULL, (void**)&pvData, NULL);
	while (bExists)
	{
		iKeyLength = strlen((char*)pvKey);
		iDataSize = strlen((char*)pvData);
		cIndexTree.Put(pvKey, iKeyLength, pvData, (uint8)iDataSize);

		bExists = cMap.Iterate(&sMapIter, (void**)&pvKey, NULL, (void**)&pvData, NULL);
	}

	AssertInt((uint32)12, cIndexTree.NumElements());

	bExists = cIndexTree.StartUnsafeIteration(&sIter, pacKey, &iKeyLength, 10, (void**)&pvData, &iDataSize);
	while (bExists)
	{
		pacData = cMap.Get(pacKey);
		iMapDataSize = strlen(pacData);
		AssertSize(iMapDataSize, iDataSize);

		iResult = memcmp(pacData, pvData, (int)iDataSize);
		AssertInt(0, iResult);

		bExists = cIndexTree.UnsafeIterate(&sIter, pacKey, &iKeyLength, 10, (void**)&pvData, &iDataSize);
	}

	cIndexTree.Flush();
	cController.End();

	cAccess.Kill();
	cIndexTree.Kill();
	cController.Kill();

	cHelper.Kill(true);
	cMap.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeFileIteratorIterate(void)
{
	size					iData;
	uint8*					pszKey;
	size					i;
	CChars*					pszKey2;
	CIndexTreeFile			cIndexTree;
	CIndexTreeFileAccess	cAccess;
	CIndexTreeIterator*		pcIter;
	bool					bExists;
	CIndexTreeHelper		cHelper;
	CDurableFileController	cController;

	cHelper.Init("Output" _FS_ "IndexTreeIterator2", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, IWT_No, IKR_No);
	cAccess.Init(&cIndexTree);

	for (i = 0; i < gaszCommonWords.NumElements(); i++)
	{
		pszKey2 = gaszCommonWords.Get(i);
		pszKey = (uint8*)pszKey2->Text();
		iData = i;

		cAccess.PutStringInt((char*)pszKey, (int32)iData);
	}

	AssertLong(1000, cAccess.NumElements());

	pcIter = cAccess.CreateIterator();

	for (i = 0; i < gaszCommonWords.NumElements(); i++)
	{
		pszKey2 = gaszCommonWords.Get(i);
		pszKey = (uint8*)pszKey2->Text();

		bExists = pcIter->Iterate();
		AssertString((char*)pszKey, pcIter->GetKey());
	}
	bExists = pcIter->Iterate();
	AssertFalse(bExists);

	cAccess.FreeIterator(pcIter);

	cIndexTree.Flush();
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
void TestIndexTreeFileIterator(void)
{
	TypeConverterInit();
	MemoryInit();
	WordsInit();
	DataMemoryInit();
	BeginTests();

	TestIndexTreeFileIteratorUnsafeIterate();
	TestIndexTreeFileIteratorIterate();

	TestStatistics();
	DataMemoryKill();
	WordsKill();
	MemoryKill();
	TypeConverterKill();
}


