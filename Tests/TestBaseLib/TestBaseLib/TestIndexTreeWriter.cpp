#include "BaseLib/IndexTreeMemory.h"
#include "BaseLib/IndexTreeMemoryAccess.h"
#include "BaseLib/MapStringString.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalMemory.h"
#include "TestLib/Assert.h"
#include "BaseLib/IndexTreeWriter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeWriterWrite(void)
{
	char*					pvData;
	char*					pvKey;
	size						iDataSize;
	bool					bExists;
	size						iKeyLength;
	CIndexTreeMemory		cIndexTree;
	CIndexTreeMemoryAccess	cAccess;
	CMapStringString		cMap;
	SMapIterator			sMapIter;
	CIndexTreeWriter		cWriter;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "IndexTreeMemoryWriter";

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
	cAccess.Init(&cIndexTree);

	bExists = cMap.StartIteration(&sMapIter, (void**)&pvKey, NULL, (void**)&pvData, NULL);
	while (bExists)
	{
		iKeyLength = strlen((char*)pvKey);
		iDataSize = strlen((char*)pvData);
		cAccess.PutKeyData((uint8*)pvKey, iKeyLength, pvData, iDataSize);

		bExists = cMap.Iterate(&sMapIter, (void**)&pvKey, NULL, (void**)&pvData, NULL);
	}

	AssertSize(12, cIndexTree.NumElements());
	Pass();
	AssertTrue(cIndexTree.ValidateIndexTree());

	cFileUtil.RemoveDir(szDirectory);

	cWriter.Write(&cIndexTree, szDirectory);

	cAccess.Kill();
	cIndexTree.Kill();
	cMap.Kill();

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeWriter(void)
{
	TypeConverterInit();
	MemoryInit();
	DataMemoryInit();
	BeginTests();

	TestIndexTreeWriterWrite();

	TestStatistics();
	DataMemoryKill();
	MemoryKill();
	TypeConverterKill();
}

