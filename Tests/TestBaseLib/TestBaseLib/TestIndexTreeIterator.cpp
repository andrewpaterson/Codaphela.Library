#include "BaseLib/TypeConverter.h"
#include "BaseLib/DebugOutput.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/IndexTreeMemory.h"
#include "BaseLib/MapStringString.h"
#include "BaseLib/IndexTreeMemoryIterator.h"
#include "BaseLib/IndexTreeMemoryAccess.h"
#include "TestLib/Words.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeIteratorUnsafeIterate(void)
{
	SIndexTreeMemoryUnsafeIterator	sIter;
	char*							pvData;
	uint8*							pvKey;
	size							iDataSize;
	bool							bExists;
	uint8							pacKey[9+1];
	size							iKeyLength;
	CIndexTreeMemory				cIndexTree;
	CMapStringString				cMap;
	SMapIterator					sMapIter;
	char*							pacData;
	size							iResult;
	size							iMapDataSize;

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

	cIndexTree.Init();

	bExists = cMap.StartIteration(&sMapIter, (void**)&pvKey, NULL, (void**)&pvData, NULL);
	while (bExists)
	{
		iKeyLength = strlen((char*)pvKey);
		iDataSize = strlen((char*)pvData);
		cIndexTree.Put(pvKey, iKeyLength, pvData, iDataSize);

		bExists = cMap.Iterate(&sMapIter, (void**)&pvKey, NULL, (void**)&pvData, NULL);
	}

	AssertSize(12, cIndexTree.NumElements());

	bExists = cIndexTree.StartIteration(&sIter, (void**)&pvData, &iDataSize);
	while (bExists)
	{
		iKeyLength = cIndexTree.GetKey(pvData, pacKey, 9+1);

		pacData = cMap.Get((char*)pacKey);
		iMapDataSize = strlen(pacData);
		AssertSize(iMapDataSize, iDataSize);

		iResult = memcmp(pacData, pvData, (int)iDataSize);
		AssertSize(0, iResult);

		bExists = cIndexTree.Iterate(&sIter, (void**)&pvData, &iDataSize);
	}

	cIndexTree.Kill();
	cMap.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeIteratorIterate(void)
{
	size					iData;
	char*					pszKey;
	size					i;
	CChars*					pszKey2;
	CIndexTreeMemory		cIndex;
	CIndexTreeMemoryAccess	cAccess;
	CIndexTreeIterator*		pcIter;
	bool					bExists;

	cIndex.Init();
	cAccess.Init(&cIndex);

	for (i = 0; i < gaszCommonWords.NumElements(); i++)
	{
		pszKey2 = gaszCommonWords.Get(i);
		pszKey = pszKey2->Text();
		iData = i;

		cAccess.PutStringInt(pszKey, (int32)iData);
	}

	AssertLong(1000, cAccess.NumElements());

	pcIter = cAccess.CreateIterator();

	for (i = 0; i < gaszCommonWords.NumElements(); i++)
	{
		pszKey2 = gaszCommonWords.Get(i);
		pszKey = pszKey2->Text();

		bExists = pcIter->Iterate();
		AssertString(pszKey, pcIter->GetKey());
	}
	bExists = pcIter->Iterate();
	AssertFalse(bExists);

	cAccess.FreeIterator(pcIter);

	cIndex.Kill();
	cAccess.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeMemoryIterator(void)
{
	TypeConverterInit();
	MemoryInit();
	WordsInit();
	BeginTests();


	TestIndexTreeIteratorUnsafeIterate();
	TestIndexTreeIteratorIterate();

	TestStatistics();
	WordsKill();
	MemoryKill();
	TypeConverterKill();
}

