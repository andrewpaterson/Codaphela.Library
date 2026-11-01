#include "BaseLib/TypeConverter.h"
#include "BaseLib/Define.h"
#include "BaseLib/StdRandom.h"
#include "BaseLib/Logger.h"
#include "BaseLib/LifeCycle.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/DurableFileController.h"
#include "BaseLib/NamedIndexedData.h"
#include "BaseLib/ValueIndexedDataConfig.h"
#include "BaseLib/ValueNamedIndexesConfig.h"
#include "BaseLib/IndexTreeEvictionCounter.h"
#include "BaseLib/IndexedDataEvictionCounter.h"
#include "TestLib/Assert.h"
#include "TestLib/Words.h"
#include "NamedIndexedDataObject.h"


//The index nodes are bigger in a 64 bit build so the index cache needs more room before it can evict anything.
#ifdef BUILD_64
#define EVICTION_TEST_INDEX_CACHE_SIZE	(12 KB)
#else
#define EVICTION_TEST_INDEX_CACHE_SIZE	(8 KB)
#endif // BUILD_64


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNamedIndexedDataInit(EIndexWriteThrough eWriteThrough)
{
	CNamedIndexedData					cDatabase;
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CLifeInit<CNamedIndexesConfig>		cNamedConfig;
	CDurableFileController				cController;
	CIndexTreeEvictionStrategyRandom	cIndexEvictionStrategy;
	CIndexTreeEvictionStrategyRandom	cNamedEvictionStrategy;
	CFileUtil							cFileUtil;
	
	cFileUtil.RemoveDir("Output" _FS_ "Database1");

	cController.Init("Output" _FS_ "Database1" _FS_ "R", "Output" _FS_ "Database1" _FS_ "W");
	cController.SetSync(DS_No);

	cNamedEvictionStrategy.Init(32);
	cIndexEvictionStrategy.Init(32);

	cIndexConfig = CValueIndexedDataConfig::Create("IndexData", 8 KB, 8 KB, eWriteThrough, LifeLocal<CIndexTreeEvictionStrategy>(&cIndexEvictionStrategy));
	cNamedConfig = CValueNamedIndexesConfig::Create("Names", 16 KB, LifeLocal<CIndexTreeEvictionStrategy>(&cNamedEvictionStrategy), eWriteThrough);
	
	cController.Begin();
	cDatabase.Init(&cController, cIndexConfig, cNamedConfig);
	cController.End();
	cNamedEvictionStrategy.Kill();
	cIndexEvictionStrategy.Kill();
	cController.Kill();

	cDatabase.Kill();
	cFileUtil.RemoveDir("Output" _FS_ "Database1");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNamedIndexedDataAddString(EIndexWriteThrough eWriteThrough)
{
	CNamedIndexedData					cDatabase;
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CLifeInit<CNamedIndexesConfig>		cNamedConfig;
	CDurableFileController				cController;
	CIndexTreeEvictionStrategyRandom	cIndexEvictionStrategy;
	CIndexTreeEvictionStrategyRandom	cNamedEvictionStrategy;
	CFileUtil							cFileUtil;
	CTestNamedIndexedDataObject			cObject1;
	CTestNamedIndexedDataObject			cObject2;
	CTestNamedIndexedDataObject			cResult;
	bool								bResult;

	cFileUtil.RemoveDir("Output" _FS_ "Database2");

	/// Add Name and Index
	cController.Init("Output" _FS_ "Database2" _FS_ "R", "Output" _FS_ "Database2" _FS_ "W");
	cController.SetSync(DS_No);
	cIndexEvictionStrategy.Init(32);
	cNamedEvictionStrategy.Init(32);
	cIndexConfig = CValueIndexedDataConfig::Create("IndexData", 8 KB, 8 KB, eWriteThrough, LifeLocal<CIndexTreeEvictionStrategy>(&cIndexEvictionStrategy));
	cNamedConfig = CValueNamedIndexesConfig::Create("Names", 16 KB, LifeLocal<CIndexTreeEvictionStrategy>(&cNamedEvictionStrategy), eWriteThrough);

	cController.Begin();
	cDatabase.Init(&cController, cIndexConfig, cNamedConfig);
	cController.End();

	cController.Begin();
	cObject1.Init("Rutherford", 1871, 1937);
	cDatabase.Add(0x0102LL, "Ernest", &cObject1, cObject1.Size());
	if (eWriteThrough == IWT_No) cDatabase.Flush();
	cController.End();

	cController.Begin();
	cObject2.Init("Bohr", 1885, 1962);
	cDatabase.Add(0x0203LL, "Niels", &cObject2, cObject2.Size());
	if (eWriteThrough == IWT_No) cDatabase.Flush();
	cController.End();

	cDatabase.Kill();
	cController.Kill();
	cNamedEvictionStrategy.Kill();
	cIndexEvictionStrategy.Kill();



	//Get by Name
	cController.Init("Output" _FS_ "Database2" _FS_ "R", "Output" _FS_ "Database2" _FS_ "W");
	cController.SetSync(DS_No);
	cNamedEvictionStrategy.Init(32);
	cIndexEvictionStrategy.Init(32);
	cIndexConfig = CValueIndexedDataConfig::Create("IndexData", 8 KB, 8 KB, eWriteThrough, LifeLocal<CIndexTreeEvictionStrategy>(&cIndexEvictionStrategy));
	cNamedConfig = CValueNamedIndexesConfig::Create("Names", 16 KB, LifeLocal<CIndexTreeEvictionStrategy>(&cNamedEvictionStrategy), eWriteThrough);

	cController.Begin();
	cDatabase.Init(&cController, cIndexConfig, cNamedConfig);
	cController.End();

	cController.Begin();
	bResult = cDatabase.Get("Ernest", &cResult);
	AssertTrue(bResult);
	AssertString("Rutherford", cResult.mszString);
	AssertLong(1871, cResult.miNumberX);
	AssertLong(1937, cResult.miNumberY);

	bResult = cDatabase.Get("Niels", &cResult);
	AssertTrue(bResult);
	AssertString("Bohr", cResult.mszString);
	AssertLong(1885, cResult.miNumberX);
	AssertLong(1962, cResult.miNumberY);
	cController.End();
	cNamedEvictionStrategy.Kill();
	cIndexEvictionStrategy.Kill();
	cController.Kill();
	cDatabase.Kill();


	//Get by Index
	cController.Init("Output" _FS_ "Database2" _FS_ "R", "Output" _FS_ "Database2" _FS_ "W");
	cController.SetSync(DS_No);
	cNamedEvictionStrategy.Init(32);
	cIndexEvictionStrategy.Init(32);
	cIndexConfig = CValueIndexedDataConfig::Create("IndexData", 8 KB, 8 KB, eWriteThrough, LifeLocal<CIndexTreeEvictionStrategy>(&cIndexEvictionStrategy));
	cNamedConfig = CValueNamedIndexesConfig::Create("Names", 16 KB, LifeLocal<CIndexTreeEvictionStrategy>(&cNamedEvictionStrategy), eWriteThrough);

	cController.Begin();
	cDatabase.Init(&cController, cIndexConfig, cNamedConfig);
	cController.End();

	cController.Begin();
	bResult = cDatabase.Get(0x0203LL, &cResult);
	AssertTrue(bResult);
	AssertString("Bohr", cResult.mszString);
	AssertLong(1885, cResult.miNumberX);
	AssertLong(1962, cResult.miNumberY);

	bResult = cDatabase.Get(0x0102, &cResult);
	AssertTrue(bResult);
	AssertString("Rutherford", cResult.mszString);
	AssertLong(1871, cResult.miNumberX);
	AssertLong(1937, cResult.miNumberY);
	cController.End();
	cNamedEvictionStrategy.Kill();
	cIndexEvictionStrategy.Kill();
	cController.Kill();
	cDatabase.Kill();


	//Size by Index
	cController.Init("Output" _FS_ "Database2" _FS_ "R", "Output" _FS_ "Database2" _FS_ "W");
	cController.SetSync(DS_No);
	cNamedEvictionStrategy.Init(32);
	cIndexEvictionStrategy.Init(32);
	cIndexConfig = CValueIndexedDataConfig::Create("IndexData", 8 KB, 8 KB, eWriteThrough, LifeLocal<CIndexTreeEvictionStrategy>(&cIndexEvictionStrategy));
	cNamedConfig = CValueNamedIndexesConfig::Create("Names", 16 KB, LifeLocal<CIndexTreeEvictionStrategy>(&cNamedEvictionStrategy), eWriteThrough);

	cController.Begin();
	cDatabase.Init(&cController, cIndexConfig, cNamedConfig);
	cController.End();

	cController.Begin();
	AssertLSize(sizeof(CTestNamedIndexedDataObject), cDatabase.Size(0x0203));
	AssertLSize(sizeof(CTestNamedIndexedDataObject), cDatabase.Size(0x0102));
	AssertLong(2, cDatabase.NumIndices());
	AssertLong(2, cDatabase.NumNames());
	cController.End();


	cIndexEvictionStrategy.Kill();
	cNamedEvictionStrategy.Kill();
	cDatabase.Kill();
	cController.Kill();


	cFileUtil.RemoveDir("Output" _FS_ "Database2");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNamedIndexedDataAddChars(EIndexWriteThrough eWriteThrough)
{
	CNamedIndexedData					cDatabase;
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CLifeInit<CNamedIndexesConfig>		cNamedConfig;
	CDurableFileController				cController;
	CIndexTreeEvictionStrategyRandom	cIndexEvictionStrategy;
	CIndexTreeEvictionStrategyRandom	cNamedEvictionStrategy;
	CFileUtil							cFileUtil;
	CTestNamedIndexedDataObject			cObject1;
	CTestNamedIndexedDataObject			cResult;
	bool								bResult;
	CChars								szName;
	lsize								uiDataSize;

	cFileUtil.RemoveDir("Output" _FS_ "Database2a");

	/// Add Name and Index
	cController.Init("Output" _FS_ "Database2a" _FS_ "R", "Output" _FS_ "Database2a" _FS_ "W");
	cController.SetSync(DS_No);
	cIndexEvictionStrategy.Init(32);
	cNamedEvictionStrategy.Init(32);
	cIndexConfig = CValueIndexedDataConfig::Create("IndexData", 8 KB, 8 KB, eWriteThrough, LifeLocal<CIndexTreeEvictionStrategy>(&cIndexEvictionStrategy));
	cNamedConfig = CValueNamedIndexesConfig::Create("Names", 16 KB, LifeLocal<CIndexTreeEvictionStrategy>(&cNamedEvictionStrategy), eWriteThrough);

	cController.Begin();
	cDatabase.Init(&cController, cIndexConfig, cNamedConfig);
	cController.End();

	cController.Begin();
	szName.Init("Ernest");
	cObject1.Init("Rutherford", 1871, 1937);
	cDatabase.Add(1LL, &szName, &cObject1, cObject1.Size());
	szName.Kill();
	if (eWriteThrough == IWT_No) cDatabase.Flush();
	cController.End();
	cIndexEvictionStrategy.Kill();
	cNamedEvictionStrategy.Kill();
	cController.Kill();
	cDatabase.Kill();


	//Get by Name
	cController.Init("Output" _FS_ "Database2a" _FS_ "R", "Output" _FS_ "Database2a" _FS_ "W");
	cController.SetSync(DS_No);
	cIndexEvictionStrategy.Init(32);
	cNamedEvictionStrategy.Init(32);
	cIndexConfig = CValueIndexedDataConfig::Create("IndexData", 8 KB, 8 KB, eWriteThrough, LifeLocal<CIndexTreeEvictionStrategy>(&cIndexEvictionStrategy));
	cNamedConfig = CValueNamedIndexesConfig::Create("Names", 16 KB, LifeLocal<CIndexTreeEvictionStrategy>(&cNamedEvictionStrategy), eWriteThrough);

	cController.Begin();
	cDatabase.Init(&cController, cIndexConfig, cNamedConfig);
	cController.End();

	cController.Begin();
	szName.Init("Ernest");
	bResult = cDatabase.Get(&szName, &uiDataSize, &cResult, cResult.Size());
	szName.Kill();
	AssertLSize(sizeof(CTestNamedIndexedDataObject), uiDataSize);
	AssertTrue(bResult);
	AssertString("Rutherford", cResult.mszString);
	AssertLong(1871, cResult.miNumberX);
	AssertLong(1937, cResult.miNumberY);
	AssertLong(1, cDatabase.NumIndices());
	AssertLong(1, cDatabase.NumNames());
	cController.End();


	cDatabase.Kill();
	cController.Kill();
	cNamedEvictionStrategy.Kill();
	cIndexEvictionStrategy.Kill();

	cFileUtil.RemoveDir("Output" _FS_ "Database2a");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNamedIndexedDataAddBad(EIndexWriteThrough eWriteThrough)
{
	CNamedIndexedData					cDatabase;
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CLifeInit<CNamedIndexesConfig>		cNamedConfig;
	CDurableFileController				cController;
	CIndexTreeEvictionStrategyRandom	cIndexEvictionStrategy;
	CIndexTreeEvictionStrategyRandom	cNamedEvictionStrategy;
	CFileUtil							cFileUtil;
	CTestNamedIndexedDataObject			cObject1;
	SLogConfig							sLogConfig;
	CChars								szName;

	cFileUtil.RemoveDir("Output" _FS_ "Database2b");

	cController.Init("Output" _FS_ "Database2b" _FS_ "R", "Output" _FS_ "Database2b" _FS_ "W");
	cController.SetSync(DS_No);
	cNamedEvictionStrategy.Init(32);
	cIndexEvictionStrategy.Init(32);
	cIndexConfig = CValueIndexedDataConfig::Create("IndexData", 8 KB, 8 KB, eWriteThrough, LifeLocal<CIndexTreeEvictionStrategy>(&cIndexEvictionStrategy));
	cNamedConfig = CValueNamedIndexesConfig::Create("Names", 16 KB, LifeLocal<CIndexTreeEvictionStrategy>(&cNamedEvictionStrategy), eWriteThrough);

	cController.Begin();
	cDatabase.Init(&cController, cIndexConfig, cNamedConfig);
	cController.End();

	cController.Begin();
	cObject1.Init("Wild Butterfly", 1871, 1937);
	szName.Init("Hello");
	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cDatabase.Add(1LL, (char*)NULL, &cObject1, cObject1.Size()));
	AssertFalse(cDatabase.Add(1LL, (CChars*)NULL, &cObject1, cObject1.Size()));
	AssertFalse(cDatabase.Add(INVALID_O_INDEX, "Hello", &cObject1, cObject1.Size()));
	AssertFalse(cDatabase.Add(INVALID_O_INDEX, &szName, &cObject1, cObject1.Size()));
	AssertFalse(cDatabase.Add(NULL_O_INDEX, &cObject1, cObject1.Size()));
	AssertFalse(cDatabase.Add(INVALID_O_INDEX, &cObject1, cObject1.Size()));
	AssertFalse(cDatabase.Add(1LL, "Hello", NULL, 0));
	AssertFalse(cDatabase.Add(1LL, &szName, NULL, 0));
	AssertFalse(cDatabase.Add(1LL, NULL, 0));
	gcLogger.SetConfig(&sLogConfig);
	szName.Kill();
	AssertLong(0, cDatabase.NumIndices());
	AssertLong(0, cDatabase.NumNames());
	cController.End();


	cDatabase.Kill();
	cController.Kill();
	cIndexEvictionStrategy.Kill();
	cNamedEvictionStrategy.Kill();

	cFileUtil.RemoveDir("Output" _FS_ "Database2b");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNamedIndexedDataAddIndex(EIndexWriteThrough eWriteThrough)
{
	CNamedIndexedData					cDatabase;
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CLifeInit<CNamedIndexesConfig>		cNamedConfig;
	CDurableFileController				cController;
	CIndexTreeEvictionStrategyRandom	cIndexEvictionStrategy;
	CIndexTreeEvictionStrategyRandom	cNamedEvictionStrategy;
	CFileUtil							cFileUtil;
	CTestNamedIndexedDataObject			cObject1;
	CTestNamedIndexedDataObject			cResult;

	cFileUtil.RemoveDir("Output" _FS_ "Database2c");

	/// Add Index
	cController.Init("Output" _FS_ "Database2c" _FS_ "R", "Output" _FS_ "Database2c" _FS_ "W");
	cController.SetSync(DS_No);
	cNamedEvictionStrategy.Init(32);
	cIndexEvictionStrategy.Init(32);
	cIndexConfig = CValueIndexedDataConfig::Create("IndexData", 8 KB, 8 KB, eWriteThrough, LifeLocal<CIndexTreeEvictionStrategy>(&cIndexEvictionStrategy));
	cNamedConfig = CValueNamedIndexesConfig::Create("Names", 16 KB, LifeLocal<CIndexTreeEvictionStrategy>(&cNamedEvictionStrategy), eWriteThrough);

	cController.Begin();
	cDatabase.Init(&cController, cIndexConfig, cNamedConfig);
	cController.End();

	cController.Begin();
	cObject1.Init("Rutherford", 1871, 1937);
	cDatabase.Add(0x56788435450563LL, &cObject1, cObject1.Size());
	if (eWriteThrough == IWT_No) cDatabase.Flush();
	cController.End();
	cNamedEvictionStrategy.Kill();
	cIndexEvictionStrategy.Kill();
	cController.Kill();
	cDatabase.Kill();


	//Get by Index
	cController.Init("Output" _FS_ "Database2c" _FS_ "R", "Output" _FS_ "Database2c" _FS_ "W");
	cController.SetSync(DS_No);
	cNamedEvictionStrategy.Init(32);
	cIndexEvictionStrategy.Init(32);
	cIndexConfig = CValueIndexedDataConfig::Create("IndexData", 8 KB, 8 KB, eWriteThrough, LifeLocal<CIndexTreeEvictionStrategy>(&cIndexEvictionStrategy));
	cNamedConfig = CValueNamedIndexesConfig::Create("Names", 16 KB, LifeLocal<CIndexTreeEvictionStrategy>(&cNamedEvictionStrategy), eWriteThrough);

	cController.Begin();
	cDatabase.Init(&cController, cIndexConfig, cNamedConfig);
	cController.End();

	cController.Begin();
	AssertFalse(cDatabase.Get("Ernest", &cResult));
	AssertTrue(cDatabase.Get(0x56788435450563LL, &cResult));
	AssertString("Rutherford", cResult.mszString);
	AssertLong(1871, cResult.miNumberX);
	AssertLong(1937, cResult.miNumberY);
	AssertLong(1, cDatabase.NumIndices());
	AssertLong(0, cDatabase.NumNames());
	cController.End();


	cIndexEvictionStrategy.Kill();
	cNamedEvictionStrategy.Kill();
	cDatabase.Kill();
	cController.Kill();

	cFileUtil.RemoveDir("Output" _FS_ "Database2c");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNamedIndexedDataGetName(EIndexWriteThrough eWriteThrough)
{
	CNamedIndexedData					cDatabase;
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CLifeInit<CNamedIndexesConfig>		cNamedConfig;
	CDurableFileController				cController;
	CIndexTreeEvictionStrategyRandom	cIndexEvictionStrategy;
	CIndexTreeEvictionStrategyRandom	cNamedEvictionStrategy;
	CFileUtil							cFileUtil;
	size								pai[512];
	CChars								szName;

	cFileUtil.RemoveDir("Output" _FS_ "Database3");

	cController.Init("Output" _FS_ "Database3" _FS_ "R", "Output" _FS_ "Database3" _FS_ "W");
	cController.SetSync(DS_No);
	cNamedEvictionStrategy.Init(32);
	cIndexEvictionStrategy.Init(32);
	cIndexConfig = CValueIndexedDataConfig::Create("IndexData", 8 KB, 8 KB, eWriteThrough, LifeLocal<CIndexTreeEvictionStrategy>(&cIndexEvictionStrategy));
	cNamedConfig = CValueNamedIndexesConfig::Create("Names", 16 KB, LifeLocal<CIndexTreeEvictionStrategy>(&cNamedEvictionStrategy), eWriteThrough);

	cController.Begin();
	cDatabase.Init(&cController, cIndexConfig, cNamedConfig);
	cController.End();

	cController.Begin();
	memset(pai, 0x04, sizeof(pai));
	cDatabase.Add(0x89072fc3, "graphics/world/city1.3ds", pai, sizeof(pai));

	szName.Init();
	AssertTrue(cDatabase.GetName(0x89072fc3, &szName));
	AssertString("graphics/world/city1.3ds", szName.Text());
	szName.Kill();
	szName.Init();
	AssertFalse(cDatabase.GetName(0x89072fc4, &szName));
	szName.Kill();

	if (eWriteThrough == IWT_No) cDatabase.Flush();
	cController.End();

	cDatabase.Kill();
	cController.Kill();
	cNamedEvictionStrategy.Kill();
	cIndexEvictionStrategy.Kill();

	cFileUtil.RemoveDir("Output" _FS_ "Database3");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNamedIndexedDataSetIndex(EIndexWriteThrough eWriteThrough)
{
	CNamedIndexedData					cDatabase;
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CLifeInit<CNamedIndexesConfig>		cNamedConfig;
	CDurableFileController				cController;
	CIndexTreeEvictionStrategyRandom	cIndexEvictionStrategy;
	CIndexTreeEvictionStrategyRandom	cNamedEvictionStrategy;
	CFileUtil							cFileUtil;
	CTestNamedIndexedDataObject			cObject1;
	CTestNamedIndexedDataObject			cResult;

	cFileUtil.RemoveDir("Output" _FS_ "Database4");

	cController.Init("Output" _FS_ "Database4" _FS_ "R", "Output" _FS_ "Database4" _FS_ "W");
	cController.SetSync(DS_No);
	cIndexEvictionStrategy.Init(32);
	cNamedEvictionStrategy.Init(32);
	cIndexConfig = CValueIndexedDataConfig::Create("IndexData", 8 KB, 8 KB, eWriteThrough, LifeLocal<CIndexTreeEvictionStrategy>(&cIndexEvictionStrategy));
	cNamedConfig = CValueNamedIndexesConfig::Create("Names", 16 KB, LifeLocal<CIndexTreeEvictionStrategy>(&cNamedEvictionStrategy), eWriteThrough);

	cController.Begin();
	cDatabase.Init(&cController, cIndexConfig, cNamedConfig);
	cController.End();

	cController.Begin();
	cObject1.Init("Those an equal point no years do.", 456754674567, 192345324532);
	cDatabase.Add(0xf97f3227894f783eLL, "Depend warmth fat but her but played", &cObject1, cObject1.Size());
	if (eWriteThrough == IWT_No) cDatabase.Flush();
	cController.End();

	cController.Begin();
	AssertTrue(cDatabase.Set(0xf97f3227894f783eLL, &cObject1));
	if (eWriteThrough == IWT_No) cDatabase.Flush();
	cController.End();

	cController.Begin();
	AssertTrue(cDatabase.Get("Depend warmth fat but her but played", &cResult));
	AssertString("Those an equal point no years do.", cResult.mszString);
	AssertLong(456754674567, cResult.miNumberX);
	AssertLong(192345324532, cResult.miNumberY);
	AssertTrue(cDatabase.Get(0xf97f3227894f783eLL, &cResult));
	AssertString("Those an equal point no years do.", cResult.mszString);
	AssertLong(456754674567, cResult.miNumberX);
	AssertLong(192345324532, cResult.miNumberY);
	cController.End();

	cDatabase.Kill();
	cController.Kill();
	cNamedEvictionStrategy.Kill();
	cIndexEvictionStrategy.Kill();

	cFileUtil.RemoveDir("Output" _FS_ "Database4");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNamedIndexedDataSet(EIndexWriteThrough eWriteThrough)
{
	CNamedIndexedData					cDatabase;
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CLifeInit<CNamedIndexesConfig>		cNamedConfig;
	CDurableFileController				cController;
	CIndexTreeEvictionStrategyRandom	cIndexEvictionStrategy;
	CIndexTreeEvictionStrategyRandom	cNamedEvictionStrategy;
	CFileUtil							cFileUtil;
	CTestNamedIndexedDataObject			cObject1;
	CTestNamedIndexedDataObject			cObject2;
	CTestNamedIndexedDataObject			cObject3;
	CTestNamedIndexedDataObject			cResult;
	CChars								sz;

	cFileUtil.RemoveDir("Output" _FS_ "Database4a");

	cController.Init("Output" _FS_ "Database4a" _FS_ "R", "Output" _FS_ "Database4a" _FS_ "W");
	cController.SetSync(DS_No);
	cIndexEvictionStrategy.Init(32);
	cNamedEvictionStrategy.Init(32);
	cIndexConfig = CValueIndexedDataConfig::Create("IndexData", 8 KB, 8 KB, eWriteThrough, LifeLocal<CIndexTreeEvictionStrategy>(&cIndexEvictionStrategy));
	cNamedConfig = CValueNamedIndexesConfig::Create("Names", 16 KB, LifeLocal<CIndexTreeEvictionStrategy>(&cNamedEvictionStrategy), eWriteThrough);

	cController.Begin();
	cDatabase.Init(&cController, cIndexConfig, cNamedConfig);
	cController.End();

	cController.Begin();
	cObject1.Init("Those an equal point no years do.", 456754674567, 192345324532);
	cDatabase.Add(0xf97f3227894f783eLL, "Depend warmth fat but her but played", &cObject1, cObject1.Size());
	cObject2.Init("Shy and subjects wondered trifling.", 7682493597, 328476355);
	cDatabase.Add(0xd678f6989c68234aLL, "Prudent cordial comfort do no on colonel as assured chicken", &cObject2, cObject2.Size());
	cObject3.Init("Terminated uncommonly at at estimating.", 87632234423, 3948765293847);
	cDatabase.Add(0xf9786d986987bcd1LL, "Ten behaviour met moonlight extremity acuteness direction", &cObject3, cObject2.Size());
	if (eWriteThrough == IWT_No) cDatabase.Flush();
	cController.End();

	cController.Begin();
	AssertTrue(cDatabase.Get(0xf97f3227894f783eLL, &cResult));
	AssertTrue(cDatabase.Get(0xd678f6989c68234aLL, &cResult));
	AssertTrue(cDatabase.Get(0xf9786d986987bcd1LL, &cResult));
	AssertFalse(cDatabase.Get(0xf97f3227894f783fLL, &cResult));
	AssertFalse(cDatabase.Get(0xe678f6989c68234aLL, &cResult));
	AssertFalse(cDatabase.Get(0xf9786d996987bcd1LL, &cResult));
	AssertString("Terminated uncommonly at at estimating.", cResult.mszString);
	AssertLong(87632234423, cResult.miNumberX);
	AssertLong(3948765293847, cResult.miNumberY);

	cObject1.Init("Certainty listening no no behaviour.", 192345324532, 456754674567);
	AssertTrue(cDatabase.Set("Ten behaviour met moonlight extremity acuteness direction", &cObject1));
	cObject2.Init("Because add why not esteems amiable him.", 867575634, 75684567698873423);
	sz.Init("Prudent cordial comfort do no on colonel as assured chicken");
	AssertTrue(cDatabase.Set(&sz, &cObject2));
	cObject3.Init("Answer living law things either sir bed length.", 4657587587534, 327649823);
	AssertTrue(cDatabase.Set(0xf97f3227894f783eLL, &cObject3));
	if (eWriteThrough == IWT_No) cDatabase.Flush();
	cController.End();

	cController.Begin();
	AssertTrue(cDatabase.Get(0xf97f3227894f783eLL, &cResult));
	AssertString("Answer living law things either sir bed length.", cResult.mszString);
	AssertLong(4657587587534, cResult.miNumberX);
	AssertLong(327649823, cResult.miNumberY);
	AssertTrue(cDatabase.Get(&sz, &cResult));
	AssertString("Because add why not esteems amiable him.", cResult.mszString);
	AssertLong(867575634, cResult.miNumberX);
	AssertLong(75684567698873423, cResult.miNumberY);
	AssertTrue(cDatabase.Get("Prudent cordial comfort do no on colonel as assured chicken", &cResult));
	AssertString("Because add why not esteems amiable him.", cResult.mszString);
	AssertLong(867575634, cResult.miNumberX);
	AssertLong(75684567698873423, cResult.miNumberY);
	sz.Kill();
	cController.End();

	cDatabase.Kill();
	cController.Kill();
	cNamedEvictionStrategy.Kill();
	cIndexEvictionStrategy.Kill();

	cFileUtil.RemoveDir("Output" _FS_ "Database4a");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNamedIndexedDataSetBad(EIndexWriteThrough eWriteThrough)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNamedIndexedDataPut(EIndexWriteThrough eWriteThrough)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNamedIndexedDataRemove(EIndexWriteThrough eWriteThrough)
{
	CNamedIndexedData					cDatabase;
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CLifeInit<CNamedIndexesConfig>		cNamedConfig;
	CDurableFileController				cController;
	CIndexTreeEvictionStrategyRandom	cIndexEvictionStrategy;
	CIndexTreeEvictionStrategyRandom	cNamedEvictionStrategy;
	CFileUtil							cFileUtil;
	CTestNamedIndexedDataObject			cObject1;
	CTestNamedIndexedDataObject			cObject2;
	CTestNamedIndexedDataObject			cObject3;
	bool								bResult;

	cFileUtil.RemoveDir("Output" _FS_ "Database7");

	cIndexEvictionStrategy.Init(32);
	cNamedEvictionStrategy.Init(32);
	cController.Init("Output" _FS_ "Database7" _FS_ "R", "Output" _FS_ "Database7" _FS_ "W");
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create("IndexData", 8 KB, 8 KB, eWriteThrough, LifeLocal<CIndexTreeEvictionStrategy>(&cIndexEvictionStrategy));
	cNamedConfig = CValueNamedIndexesConfig::Create("Names", 16 KB, LifeLocal<CIndexTreeEvictionStrategy>(&cNamedEvictionStrategy), eWriteThrough);

	cController.Begin();
	cDatabase.Init(&cController, cIndexConfig, cNamedConfig);
	cObject1.Init("Rutherford", 1871, 1937);
	cDatabase.Add(0x0102LL, "Ernest", &cObject1, cObject1.Size());
	cObject2.Init("Bohr", 1885, 1962);
	cDatabase.Add(0x0203LL, "Niels", &cObject2, cObject2.Size());
	cObject3.Init("Planck", 1858, 1947);
	cDatabase.Add(0x0304LL, &cObject3, cObject3.Size());
	AssertLong(3, cDatabase.NumIndices());
	AssertLong(2, cDatabase.NumNames());
	AssertTrue(cDatabase.Contains(0x0102LL));
	AssertTrue(cDatabase.Contains("Ernest"));
	AssertTrue(cDatabase.Contains(0x0203LL));
	AssertTrue(cDatabase.Contains("Niels"));
	AssertTrue(cDatabase.Contains(0x0304LL));
	if (eWriteThrough == IWT_No) cDatabase.Flush();
	cController.End();
	cDatabase.Kill();
	cController.Kill();
	cNamedEvictionStrategy.Kill();
	cIndexEvictionStrategy.Kill();


	cIndexEvictionStrategy.Init(32);
	cNamedEvictionStrategy.Init(32);
	cController.Init("Output" _FS_ "Database7" _FS_ "R", "Output" _FS_ "Database7" _FS_ "W");
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create("IndexData", 8 KB, 8 KB, eWriteThrough, LifeLocal<CIndexTreeEvictionStrategy>(&cIndexEvictionStrategy));
	cNamedConfig = CValueNamedIndexesConfig::Create("Names", 16 KB, LifeLocal<CIndexTreeEvictionStrategy>(&cNamedEvictionStrategy), eWriteThrough);

	cController.Begin();
	cDatabase.Init(&cController, cIndexConfig, cNamedConfig);
	cController.End();

	cController.Begin();
	AssertTrue(cDatabase.Contains(0x0102LL));
	AssertTrue(cDatabase.Contains("Ernest"));
	AssertTrue(cDatabase.Contains(0x0203LL));
	AssertTrue(cDatabase.Contains("Niels"));
	AssertTrue(cDatabase.Contains(0x0304LL));
	cController.End();
	cDatabase.Kill();
	cController.Kill();
	cNamedEvictionStrategy.Kill();
	cIndexEvictionStrategy.Kill();


	cIndexEvictionStrategy.Init(32);
	cNamedEvictionStrategy.Init(32);
	cController.Init("Output" _FS_ "Database7" _FS_ "R", "Output" _FS_ "Database7" _FS_ "W");
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create("IndexData", 8 KB, 8 KB, eWriteThrough, LifeLocal<CIndexTreeEvictionStrategy>(&cIndexEvictionStrategy));
	cNamedConfig = CValueNamedIndexesConfig::Create("Names", 16 KB, LifeLocal<CIndexTreeEvictionStrategy>(&cNamedEvictionStrategy), eWriteThrough);

	cController.Begin();
	cDatabase.Init(&cController, cIndexConfig, cNamedConfig);
	cController.End();

	cController.Begin();
	bResult = cDatabase.Remove(0x0102LL);
	AssertTrue(bResult);
	AssertFalse(cDatabase.Contains(0x0102LL));
	AssertFalse(cDatabase.Contains("Ernest"));
	if (eWriteThrough == IWT_No) cDatabase.Flush();
	cController.End();
	cDatabase.Kill();
	cController.Kill();
	cNamedEvictionStrategy.Kill();
	cIndexEvictionStrategy.Kill();


	cIndexEvictionStrategy.Init(32);
	cNamedEvictionStrategy.Init(32);
	cController.Init("Output" _FS_ "Database7" _FS_ "R", "Output" _FS_ "Database7" _FS_ "W");
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create("IndexData", 8 KB, 8 KB, eWriteThrough, LifeLocal<CIndexTreeEvictionStrategy>(&cIndexEvictionStrategy));
	cNamedConfig = CValueNamedIndexesConfig::Create("Names", 16 KB, LifeLocal<CIndexTreeEvictionStrategy>(&cNamedEvictionStrategy), eWriteThrough);

	cController.Begin();
	cDatabase.Init(&cController, cIndexConfig, cNamedConfig);
	cController.End();

	cController.Begin();
	AssertFalse(cDatabase.Contains(0x0102LL));
	AssertFalse(cDatabase.Contains("Ernest"));
	bResult = cDatabase.Remove("Niels");
	AssertTrue(bResult);
	AssertFalse(cDatabase.Contains(0x0203LL));
	AssertFalse(cDatabase.Contains("Niels"));
	bResult = cDatabase.Contains(0x0304LL);
	bResult = cDatabase.Remove(0x0304LL);
	AssertTrue(bResult);
	AssertFalse(cDatabase.Contains(0x0304LL));
	if (eWriteThrough == IWT_No) cDatabase.Flush();
	cController.End();

	cDatabase.Kill();
	cController.Kill();
	cNamedEvictionStrategy.Kill();
	cIndexEvictionStrategy.Kill();


	cFileUtil.RemoveDir("Output" _FS_ "Database7");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNamedIndexedDataIterate(EIndexWriteThrough eWriteThrough)
{
	CNamedIndexedData					cDatabase;
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CLifeInit<CNamedIndexesConfig>		cNamedConfig;
	CDurableFileController				cController;
	CIndexTreeEvictionStrategyRandom	cIndexEvictionStrategy;
	CIndexTreeEvictionStrategyRandom	cNamedEvictionStrategy;
	CFileUtil							cFileUtil;
	CTestNamedIndexedDataObject			cObject1;
	CTestNamedIndexedDataObject			cObject2;
	CTestNamedIndexedDataObject			cObject3;
	CTestNamedIndexedDataObject			cObject4;
	CTestNamedIndexedDataObject			cObject5;
	SIndexTreeFileIterator				sIter;
	CTestNamedIndexedDataObject			cResult;
	lsize								iDataSize;
	OIndex								oi;
	char								szKey[MAX_KEY_SIZE];
	bool								bResult;

	cFileUtil.RemoveDir("Output" _FS_ "Database8");

	cIndexEvictionStrategy.Init(32);
	cNamedEvictionStrategy.Init(32);
	cController.Init("Output" _FS_ "Database8" _FS_ "R", "Output" _FS_ "Database8" _FS_ "W");
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create("IndexData", 8 KB, 8 KB, eWriteThrough, LifeLocal<CIndexTreeEvictionStrategy>(&cIndexEvictionStrategy));
	cNamedConfig = CValueNamedIndexesConfig::Create("Names", 16 KB, LifeLocal<CIndexTreeEvictionStrategy>(&cNamedEvictionStrategy), eWriteThrough);

	cController.Begin();
	cDatabase.Init(&cController, cIndexConfig, cNamedConfig);
	cObject1.Init("Rutherford", 1871, 1937);
	cDatabase.Add(0x0102LL, "Ernest", &cObject1, cObject1.Size());
	cObject2.Init("Bohr", 1885, 1962);
	cDatabase.Add(0x0203LL, "Niels", &cObject2, cObject2.Size());
	cObject3.Init("Planck", 1858, 1947);
	cDatabase.Add(0x0304LL, &cObject3, cObject3.Size());
	cObject4.Init("Einstein", 1879, 1955);
	cDatabase.Add(0x405LL, "Albert", &cObject4, cObject4.Size());
	cObject5.Init("Newton", 1643, 1727);
	cDatabase.Add(0x506LL, "Isaac", &cObject5, cObject5.Size());
	AssertLong(5, cDatabase.NumIndices());
	AssertLong(4, cDatabase.NumNames());

	oi = cDatabase.StartIndexIteration(&sIter, &cResult, &iDataSize, sizeof(CTestNamedIndexedDataObject));
	AssertLong(0x0102LL, oi);
	AssertLSize(sizeof(CTestNamedIndexedDataObject), iDataSize);
	AssertMemory(&cObject1, &cResult, iDataSize);

	oi = cDatabase.IndexIterate(&sIter, &cResult, &iDataSize, sizeof(CTestNamedIndexedDataObject));
	AssertLong(0x0203LL, oi);
	AssertLSize(sizeof(CTestNamedIndexedDataObject), iDataSize);
	AssertMemory(&cObject2, &cResult, iDataSize);

	oi = cDatabase.IndexIterate(&sIter, &cResult, &iDataSize, sizeof(CTestNamedIndexedDataObject));
	AssertLong(0x0304LL, oi);
	AssertLSize(sizeof(CTestNamedIndexedDataObject), iDataSize);
	AssertMemory(&cObject3, &cResult, iDataSize);

	oi = cDatabase.IndexIterate(&sIter, &cResult, &iDataSize, sizeof(CTestNamedIndexedDataObject));
	AssertLong(0x0405LL, oi);
	AssertLSize(sizeof(CTestNamedIndexedDataObject), iDataSize);
	AssertMemory(&cObject4, &cResult, iDataSize);

	oi = cDatabase.IndexIterate(&sIter, &cResult, &iDataSize, sizeof(CTestNamedIndexedDataObject));
	AssertLong(0x0506LL, oi);
	AssertLSize(sizeof(CTestNamedIndexedDataObject), iDataSize);
	AssertMemory(&cObject5, &cResult, iDataSize);

	oi = cDatabase.IndexIterate(&sIter, &cResult, &iDataSize, sizeof(CTestNamedIndexedDataObject));
	AssertLong(INVALID_O_INDEX, oi);


	bResult = cDatabase.StartNameIteration(&sIter, szKey, &oi);
	AssertTrue(bResult);
	AssertLong(0x0405LL, oi);
	AssertString("Albert", szKey);

	bResult = cDatabase.NameIterate(&sIter, szKey, &oi);
	AssertTrue(bResult);
	AssertLong(0x0102LL, oi);
	AssertString("Ernest", szKey);

	bResult = cDatabase.NameIterate(&sIter, szKey, &oi);
	AssertTrue(bResult);
	AssertLong(0x0506LL, oi);
	AssertString("Isaac", szKey);

	bResult = cDatabase.NameIterate(&sIter, szKey, &oi);
	AssertTrue(bResult);
	AssertLong(0x0203LL, oi);
	AssertString("Niels", szKey);

	bResult = cDatabase.NameIterate(&sIter, szKey, &oi);
	AssertFalse(bResult);


	oi = cDatabase.StartIndexIteration(&sIter, &cResult, &iDataSize, 0);
	AssertLong(0x0102LL, oi);

	oi = cDatabase.IndexIterate(&sIter, &cResult, &iDataSize, 0);
	AssertLong(0x0203LL, oi);

	oi = cDatabase.IndexIterate(&sIter, &cResult, &iDataSize, 0);
	AssertLong(0x0304LL, oi);

	oi = cDatabase.IndexIterate(&sIter, &cResult, &iDataSize, 0);
	AssertLong(0x0405LL, oi);

	oi = cDatabase.IndexIterate(&sIter, &cResult, &iDataSize, 0);
	AssertLong(0x0506LL, oi);

	oi = cDatabase.IndexIterate(&sIter, &cResult, &iDataSize, 0);
	sIter.Kill();
	AssertLong(INVALID_O_INDEX, oi);


	if (eWriteThrough == IWT_No) cDatabase.Flush();
	cController.End();
	cDatabase.Kill();
	cController.Kill();
	cNamedEvictionStrategy.Kill();
	cIndexEvictionStrategy.Kill();

	cFileUtil.RemoveDir("Output" _FS_ "Database8");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNamedIndexedDataIterateDuringTreeChange(EIndexWriteThrough eWriteThrough)
{
	CNamedIndexedData					cDatabase;
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CLifeInit<CNamedIndexesConfig>		cNamedConfig;
	CDurableFileController				cController;
	CIndexTreeEvictionStrategyRandom	cIndexEvictionStrategy;
	CIndexTreeEvictionStrategyRandom	cNamedEvictionStrategy;
	CFileUtil							cFileUtil;
	CTestNamedIndexedDataObject			cObject;
	SIndexTreeFileIterator				sIter;
	CTestNamedIndexedDataObject			cResult;
	lsize								iDataSize;
	OIndex								oi;
	char								szKey[MAX_KEY_SIZE];
	bool								bResult;
	CArrayChars							aszWords;
	CRandom								cRandom;
	CChars								sz;
	size								iNumWords;
	size								iIndex;
	size								iWord;
	CArrayChars							aszNames;
	size								iIter;
	size								iExpectedOi;
	CIndexTreeEvictionCounter			cIndexEvictionCounter;
	CIndexedDataEvictionCounter			cDataEvictionCounter;
	CIndexTreeEvictionCounter			cNameEvictionCounter;
	bool								bFailed;

	aszWords.Init();
	GetCommonWords(&aszWords);
	iNumWords = aszWords.NumElements();
	cRandom.Init(8976);

	cFileUtil.RemoveDir("Output" _FS_ "Database9");

	aszNames.Init();
	cIndexEvictionCounter.Init();
	cDataEvictionCounter.Init();
	cNameEvictionCounter.Init();

	cIndexEvictionStrategy.Init(32);
	cNamedEvictionStrategy.Init(32);
	cController.Init("Output" _FS_ "Database9" _FS_ "R", "Output" _FS_ "Database9" _FS_ "W");
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create("IndexData", 8 KB, EVICTION_TEST_INDEX_CACHE_SIZE, eWriteThrough, LifeLocal<CIndexTreeEvictionStrategy>(&cIndexEvictionStrategy), &cIndexEvictionCounter, &cDataEvictionCounter);
	cNamedConfig = CValueNamedIndexesConfig::Create("Names", 16 KB, LifeLocal<CIndexTreeEvictionStrategy>(&cNamedEvictionStrategy), eWriteThrough, &cNameEvictionCounter);
	
	cController.Begin();
	cDatabase.Init(&cController, cIndexConfig, cNamedConfig);

#ifdef BUILD_64
	AssertLong(4136, cDatabase.GetIndiciesSystemMemorySize());
#else
	AssertLong(3100, cDatabase.GetIndiciesSystemMemorySize());
#endif // BUILD_64
	AssertLong(0, cDatabase.GetDataSystemMemorySize());
#ifdef BUILD_64
	AssertLong(4136, cDatabase.GetNamesSystemMemorySize());
#else
	AssertLong(3100, cDatabase.GetNamesSystemMemorySize());
#endif // BUILD_64

	for (oi = 1; oi < 1024; oi++)
	{
		sz.Init();
		for (iWord = 0; iWord < 3; iWord++)
		{
			if (iWord != 0)
			{
				sz.Append(" ");
			}
			iIndex = cRandom.Next(0, (int32)(iNumWords - 1));
			sz.Append(aszWords.Get(iIndex));
		}
		cObject.Init(sz.Text(), oi-1, oi * 7);
		cDatabase.Add(oi * 7, sz.Text(), &cObject, cObject.Size());
		aszNames.Add(sz.Text());
		sz.Kill();
	}

	if (eWriteThrough == IWT_Yes)
	{
#ifdef BUILD_64
		AssertLong(10648, cDatabase.GetIndiciesSystemMemorySize());
		AssertLong(5711, cDatabase.GetDataSystemMemorySize());
		AssertLong(13968, cDatabase.GetNamesSystemMemorySize());
		AssertLong(17, cDatabase.NumIndicesCached());
		AssertLong(17, cDatabase.NumDataCached());
		AssertLong(6, cDatabase.NumNamesCached());
		AssertLong(1006, cIndexEvictionCounter.EvictionCount());
		AssertLong(1006, cDataEvictionCounter.EvictionCount());
		AssertLong(1017, cNameEvictionCounter.EvictionCount());
#else
		AssertLong(8156, cDatabase.GetIndiciesSystemMemorySize());
		AssertLong(5756, cDatabase.GetDataSystemMemorySize());
		AssertLong(16308, cDatabase.GetNamesSystemMemorySize());
		AssertLong(21, cDatabase.NumIndicesCached());
		AssertLong(18, cDatabase.NumDataCached());
		AssertLong(10, cDatabase.NumNamesCached());
		AssertLong(1002, cIndexEvictionCounter.EvictionCount());
		AssertLong(1005, cDataEvictionCounter.EvictionCount());
		AssertLong(1013, cNameEvictionCounter.EvictionCount());
#endif // BUILD_64
		AssertLong(1023, cDatabase.NumIndices());  //This causes evictions.
		AssertLong(1023, cDatabase.NumNames());
	}
	else if (eWriteThrough == IWT_No)
	{
#ifdef BUILD_64
		AssertLong(10648, cDatabase.GetIndiciesSystemMemorySize());
		AssertLong(5711, cDatabase.GetDataSystemMemorySize());
		AssertLong(13968, cDatabase.GetNamesSystemMemorySize());
		AssertLong(17, cDatabase.NumIndicesCached());
		AssertLong(17, cDatabase.NumDataCached());
		AssertLong(6, cDatabase.NumNamesCached());
		AssertLong(1006, cIndexEvictionCounter.EvictionCount());
		AssertLong(1006, cDataEvictionCounter.EvictionCount());
		AssertLong(1017, cNameEvictionCounter.EvictionCount());
#else
		AssertLong(8156, cDatabase.GetIndiciesSystemMemorySize());
		AssertLong(5756, cDatabase.GetDataSystemMemorySize());
		AssertLong(16308, cDatabase.GetNamesSystemMemorySize());
		AssertLong(21, cDatabase.NumIndicesCached());
		AssertLong(18, cDatabase.NumDataCached());
		AssertLong(10, cDatabase.NumNamesCached());
		AssertLong(1002, cIndexEvictionCounter.EvictionCount());
		AssertLong(1005, cDataEvictionCounter.EvictionCount());
		AssertLong(1013, cNameEvictionCounter.EvictionCount());
#endif // BUILD_64
		AssertLong(1023, cDatabase.NumIndices());  //This causes evictions.
		AssertLong(1023, cDatabase.NumNames());
	}
	else
	{
		Fail();
	}

	cIndexEvictionCounter.Reset();
	cDataEvictionCounter.Reset();
	cNameEvictionCounter.Reset();
	Pass();

	bFailed = false;
	iIter = 0;
	oi = cDatabase.StartIndexIteration(&sIter, &cResult, &iDataSize, sizeof(CTestNamedIndexedDataObject));
	while (oi != INVALID_O_INDEX)
	{
		iExpectedOi = (iIter + 1) * 7;
		if ((OIndex)iExpectedOi != oi) bFailed = true;
		if (sizeof(CTestNamedIndexedDataObject) != iDataSize) bFailed = true;
		if ((int64)iIter != cResult.miNumberX) bFailed = true;
		if ((int64)iExpectedOi != cResult.miNumberY)  bFailed = true;
		if (!aszNames.Get(iIter)->Equals(cResult.mszString)) bFailed = true;
		AssertFalse(bFailed);

		iIter++;
		oi = cDatabase.IndexIterate(&sIter, &cResult, &iDataSize, sizeof(CTestNamedIndexedDataObject));
	}
	Pass();

	if (eWriteThrough == IWT_Yes)
	{
#ifdef BUILD_64
		AssertLong(12240, cDatabase.GetIndiciesSystemMemorySize());
		AssertLong(5699, cDatabase.GetDataSystemMemorySize());
		AssertLong(15696, cDatabase.GetNamesSystemMemorySize());
		AssertLong(28, cDatabase.NumIndicesCached());
		AssertLong(17, cDatabase.NumDataCached());
		AssertLong(8, cDatabase.NumNamesCached());
#else
		AssertLong(8156, cDatabase.GetIndiciesSystemMemorySize());
		AssertLong(4478, cDatabase.GetDataSystemMemorySize());
		AssertLong(15668, cDatabase.GetNamesSystemMemorySize());
		AssertLong(18, cDatabase.NumIndicesCached());
		AssertLong(14, cDatabase.NumDataCached());
		AssertLong(15, cDatabase.NumNamesCached());
#endif // BUILD_64
		AssertLong(1023, cIndexEvictionCounter.EvictionCount());
#ifdef BUILD_64
		AssertLong(1006, cDataEvictionCounter.EvictionCount());
#else
		AssertLong(1009, cDataEvictionCounter.EvictionCount());
#endif // BUILD_64
		AssertLong(0, cNameEvictionCounter.EvictionCount());
		AssertLong(1023, cDatabase.NumIndices());	//This causes evictions.
		AssertLong(1023, cDatabase.NumNames());
		AssertLong(0, cDatabase.NumDataCached()); 
	}
	else if (eWriteThrough == IWT_No)
	{
#ifdef BUILD_64
		AssertLong(12240, cDatabase.GetIndiciesSystemMemorySize());
		AssertLong(5699, cDatabase.GetDataSystemMemorySize());
		AssertLong(15696, cDatabase.GetNamesSystemMemorySize());
		AssertLong(28, cDatabase.NumIndicesCached());
		AssertLong(17, cDatabase.NumDataCached());
		AssertLong(8, cDatabase.NumNamesCached());
#else
		AssertLong(8156, cDatabase.GetIndiciesSystemMemorySize());
		AssertLong(4478, cDatabase.GetDataSystemMemorySize());
		AssertLong(15668, cDatabase.GetNamesSystemMemorySize());
		AssertLong(18, cDatabase.NumIndicesCached());
		AssertLong(14, cDatabase.NumDataCached());
		AssertLong(15, cDatabase.NumNamesCached());
#endif // BUILD_64
		AssertLong(1023, cIndexEvictionCounter.EvictionCount());
#ifdef BUILD_64
		AssertLong(1006, cDataEvictionCounter.EvictionCount());
#else
		AssertLong(1009, cDataEvictionCounter.EvictionCount());
#endif // BUILD_64
		AssertLong(0, cNameEvictionCounter.EvictionCount());
		AssertLong(1023, cDatabase.NumIndices());	//This causes evictions.
		AssertLong(1023, cDatabase.NumNames());
		AssertLong(0, cDatabase.NumDataCached());
	}
	else
	{
		Fail();
	}
	cIndexEvictionCounter.Reset();
	cDataEvictionCounter.Reset();
	cNameEvictionCounter.Reset();
	
	aszNames.QuickSort();

	iIter = 0;
	bResult = cDatabase.StartNameIteration(&sIter, szKey, &oi);
	while (bResult)
	{
		AssertString(aszNames.Get(iIter)->Text(), szKey);

		iIter++;
		bResult = cDatabase.NameIterate(&sIter, szKey, &oi);
	}

#ifdef BUILD_64
	AssertLong(28, cDatabase.NumIndicesCached());
#else
	AssertLong(18, cDatabase.NumIndicesCached());
#endif // BUILD_64
	AssertLong(0, cDatabase.NumDataCached());
#ifdef BUILD_64
	AssertLong(9, cDatabase.NumNamesCached());
#else
	AssertLong(16, cDatabase.NumNamesCached());
#endif // BUILD_64
	AssertLong(0, cIndexEvictionCounter.EvictionCount());
	AssertLong(0, cDataEvictionCounter.EvictionCount());
#ifdef BUILD_64
	AssertLong(1023, cNameEvictionCounter.EvictionCount());
#else
	AssertLong(1022, cNameEvictionCounter.EvictionCount());
#endif // BUILD_64
	AssertLong(1023, cDatabase.NumIndices());
	AssertLong(1023, cDatabase.NumNames());


	if (eWriteThrough == IWT_No) cDatabase.Flush();
	cController.End();
	cDatabase.Kill();
	cController.Kill();
	cNamedEvictionStrategy.Kill();
	cIndexEvictionStrategy.Kill();

	cFileUtil.RemoveDir("Output" _FS_ "Database8");

	aszWords.Kill();
	aszNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNamedIndexedDataGetDoesNotExceedCache(EIndexWriteThrough eWriteThrough)
{
	CNamedIndexedData					cDatabase;
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CLifeInit<CNamedIndexesConfig>		cNamedConfig;
	CDurableFileController				cController;
	CIndexTreeEvictionStrategyRandom	cIndexEvictionStrategy;
	CIndexTreeEvictionStrategyRandom	cNamedEvictionStrategy;
	CFileUtil							cFileUtil;
	CTestNamedIndexedDataObject			cObject;
	CTestNamedIndexedDataObject			cResult;
	OIndex								oi;
	CArrayChars							aszWords;
	CRandom								cRandom;
	CChars								sz;
	size								iNumWords;
	size								iIndex;
	size								iWord;
	CArrayChars							aszNames;
	CIndexTreeEvictionCounter			cIndexEvictionCounter;
	CIndexedDataEvictionCounter			cDataEvictionCounter;
	CIndexTreeEvictionCounter			cNameEvictionCounter;
	size								i;
	lsize								uiDataSize;
	char*								szName;

	aszWords.Init();
	GetCommonWords(&aszWords);
	iNumWords = aszWords.NumElements();
	cRandom.Init(8976);

	cFileUtil.RemoveDir("Output" _FS_ "Database9");

	aszNames.Init();
	cIndexEvictionCounter.Init();
	cDataEvictionCounter.Init();
	cNameEvictionCounter.Init();

	cIndexEvictionStrategy.Init(32);
	cNamedEvictionStrategy.Init(32);
	cController.Init("Output" _FS_ "Database9" _FS_ "R", "Output" _FS_ "Database9" _FS_ "W");
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create("IndexData", 8 KB, EVICTION_TEST_INDEX_CACHE_SIZE, eWriteThrough, LifeLocal<CIndexTreeEvictionStrategy>(&cIndexEvictionStrategy), &cIndexEvictionCounter, &cDataEvictionCounter);
	cNamedConfig = CValueNamedIndexesConfig::Create("Names", 16 KB, LifeLocal<CIndexTreeEvictionStrategy>(&cNamedEvictionStrategy), eWriteThrough, &cNameEvictionCounter);

	cController.Begin();
	cDatabase.Init(&cController, cIndexConfig, cNamedConfig);

#ifdef BUILD_64
	AssertLong(4136, cDatabase.GetIndiciesSystemMemorySize());
#else
	AssertLong(3100, cDatabase.GetIndiciesSystemMemorySize());
#endif // BUILD_64
	AssertLong(0, cDatabase.GetDataSystemMemorySize());
#ifdef BUILD_64
	AssertLong(4136, cDatabase.GetNamesSystemMemorySize());
#else
	AssertLong(3100, cDatabase.GetNamesSystemMemorySize());
#endif // BUILD_64

	for (oi = 1; oi < 1024; oi++)
	{
		sz.Init();
		for (iWord = 0; iWord < 3; iWord++)
		{
			if (iWord != 0)
			{
				sz.Append(" ");
			}
			iIndex = cRandom.Next(0, (int32)(iNumWords - 1));
			sz.Append(aszWords.Get(iIndex));
		}

		szName = sz.Text();
		uiDataSize = cObject.Size();
		cObject.Init(szName, oi - 1, oi * 7);
		cDatabase.Add(oi * 7, szName, &cObject, uiDataSize);
		aszNames.Add(szName);
		sz.Kill();

		cDatabase.ValidateCache();
	}

	if (eWriteThrough == IWT_Yes)
	{
#ifdef BUILD_64
		AssertLong(10648, cDatabase.GetIndiciesSystemMemorySize());
		AssertLong(5711, cDatabase.GetDataSystemMemorySize());
		AssertLong(13968, cDatabase.GetNamesSystemMemorySize());
		AssertLong(17, cDatabase.NumIndicesCached());
		AssertLong(17, cDatabase.NumDataCached());
		AssertLong(6, cDatabase.NumNamesCached());
		AssertLong(1006, cIndexEvictionCounter.EvictionCount());
		AssertLong(1006, cDataEvictionCounter.EvictionCount());
		AssertLong(1017, cNameEvictionCounter.EvictionCount());
#else
		AssertLong(8156, cDatabase.GetIndiciesSystemMemorySize());
		AssertLong(5756, cDatabase.GetDataSystemMemorySize());
		AssertLong(16308, cDatabase.GetNamesSystemMemorySize());
		AssertLong(21, cDatabase.NumIndicesCached());
		AssertLong(18, cDatabase.NumDataCached());
		AssertLong(10, cDatabase.NumNamesCached());
		AssertLong(1002, cIndexEvictionCounter.EvictionCount());
		AssertLong(1005, cDataEvictionCounter.EvictionCount());
		AssertLong(1013, cNameEvictionCounter.EvictionCount());
#endif // BUILD_64
		AssertLong(1023, cDatabase.NumIndices());  //This causes evictions.
		AssertLong(1023, cDatabase.NumNames());
	}
	else if (eWriteThrough == IWT_No)
	{
#ifdef BUILD_64
		AssertLong(10648, cDatabase.GetIndiciesSystemMemorySize());
		AssertLong(5711, cDatabase.GetDataSystemMemorySize());
		AssertLong(13968, cDatabase.GetNamesSystemMemorySize());
		AssertLong(17, cDatabase.NumIndicesCached());
		AssertLong(17, cDatabase.NumDataCached());
		AssertLong(6, cDatabase.NumNamesCached());
		AssertLong(1006, cIndexEvictionCounter.EvictionCount());
		AssertLong(1006, cDataEvictionCounter.EvictionCount());
		AssertLong(1017, cNameEvictionCounter.EvictionCount());
#else
		AssertLong(8156, cDatabase.GetIndiciesSystemMemorySize());
		AssertLong(5756, cDatabase.GetDataSystemMemorySize());
		AssertLong(16308, cDatabase.GetNamesSystemMemorySize());
		AssertLong(21, cDatabase.NumIndicesCached());
		AssertLong(18, cDatabase.NumDataCached());
		AssertLong(10, cDatabase.NumNamesCached());
		AssertLong(1002, cIndexEvictionCounter.EvictionCount());
		AssertLong(1005, cDataEvictionCounter.EvictionCount());
		AssertLong(1013, cNameEvictionCounter.EvictionCount());
#endif // BUILD_64
		AssertLong(1023, cDatabase.NumIndices());  //This causes evictions.
		AssertLong(1023, cDatabase.NumNames());
	}
	else
	{
		Fail();
	}

	cIndexEvictionCounter.Reset();
	cDataEvictionCounter.Reset();
	cNameEvictionCounter.Reset();
	aszNames.Shuffle(&cRandom);
	for (i = 0; i < aszNames.NumElements(); i++)
	{
		cDatabase.Get(aszNames.Get(i), &uiDataSize, &cResult, sizeof(CTestNamedIndexedDataObject));
	}

#ifdef BUILD_64
	AssertSize(9000, cDatabase.GetIndiciesSystemMemorySize());
	AssertSize(330, cDatabase.GetDataSystemMemorySize());
	AssertSize(15920, cDatabase.GetNamesSystemMemorySize());
#else
	AssertLSize(6728, cDatabase.GetIndiciesSystemMemorySize());
	AssertLSize(314, cDatabase.GetDataSystemMemorySize());
	AssertLSize(16252, cDatabase.GetNamesSystemMemorySize());
#endif // BUILD_64
	AssertLong(1, cDatabase.NumIndicesCached());
	AssertLong(1, cDatabase.NumDataCached());
#ifdef BUILD_64
	AssertLong(5, cDatabase.NumNamesCached());
	AssertLong(1050, cIndexEvictionCounter.EvictionCount());
#else
	AssertLong(9, cDatabase.NumNamesCached());
	AssertLong(1040, cIndexEvictionCounter.EvictionCount());
#endif // BUILD_64
	AssertLong(1022, cDataEvictionCounter.EvictionCount());
#ifdef BUILD_64
	AssertLong(1026, cNameEvictionCounter.EvictionCount());
#else
	AssertLong(1028, cNameEvictionCounter.EvictionCount());
#endif // BUILD_64
	AssertLong(1023, cDatabase.NumIndices());
	AssertLong(1023, cDatabase.NumNames());

	cIndexEvictionCounter.Reset();
	cDataEvictionCounter.Reset();
	cNameEvictionCounter.Reset();
	aszNames.Shuffle(&cRandom);
	for (i = 0; i < aszNames.NumElements(); i++)
	{
		cDatabase.Get(aszNames.Get(i), &uiDataSize, &cResult, sizeof(CTestNamedIndexedDataObject));
	}

#ifdef BUILD_64
	AssertSize(9112, cDatabase.GetIndiciesSystemMemorySize());
	AssertSize(339, cDatabase.GetDataSystemMemorySize());
	AssertSize(16048, cDatabase.GetNamesSystemMemorySize());
#else
	AssertLSize(6812, cDatabase.GetIndiciesSystemMemorySize());
	AssertLSize(323, cDatabase.GetDataSystemMemorySize());
	AssertLSize(15708, cDatabase.GetNamesSystemMemorySize());
#endif // BUILD_64
	AssertLong(1, cDatabase.NumIndicesCached());
	AssertLong(1, cDatabase.NumDataCached());
#ifdef BUILD_64
	AssertLong(6, cDatabase.NumNamesCached());
	AssertLong(1050, cIndexEvictionCounter.EvictionCount());
#else
	AssertLong(9, cDatabase.NumNamesCached());
	AssertLong(1040, cIndexEvictionCounter.EvictionCount());
#endif // BUILD_64
	AssertLong(1022, cDataEvictionCounter.EvictionCount());
#ifdef BUILD_64
	AssertLong(1026, cNameEvictionCounter.EvictionCount());
#else
	AssertLong(1029, cNameEvictionCounter.EvictionCount());
#endif // BUILD_64
	AssertLong(1023, cDatabase.NumIndices());
	AssertLong(1023, cDatabase.NumNames());

	if (eWriteThrough == IWT_No) cDatabase.Flush();
	cController.End();
	cDatabase.Kill();
	cController.Kill();
	cNamedEvictionStrategy.Kill();
	cIndexEvictionStrategy.Kill();

	cFileUtil.RemoveDir("Output" _FS_ "Database8");

	aszNames.Kill();
	aszWords.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNamedIndexedData(void)
{
	TypeConverterInit();
	MemoryInit();
	DataMemoryInit();
	BeginTests();

	TestNamedIndexedDataInit(IWT_Yes);
	TestNamedIndexedDataInit(IWT_No);
	TestNamedIndexedDataAddString(IWT_Yes);
	TestNamedIndexedDataAddString(IWT_No);
	TestNamedIndexedDataAddChars(IWT_Yes);
	TestNamedIndexedDataAddChars(IWT_No);
	TestNamedIndexedDataAddIndex(IWT_Yes);
	TestNamedIndexedDataAddIndex(IWT_No);
	TestNamedIndexedDataAddBad(IWT_Yes);
	TestNamedIndexedDataAddBad(IWT_No);
	TestNamedIndexedDataGetName(IWT_Yes);
	TestNamedIndexedDataGetName(IWT_No);
	TestNamedIndexedDataSetIndex(IWT_Yes);
	TestNamedIndexedDataSetIndex(IWT_No);
	TestNamedIndexedDataSet(IWT_Yes);
	TestNamedIndexedDataSet(IWT_No);
	TestNamedIndexedDataSetBad(IWT_Yes);	//Why isn't this implemented?
	TestNamedIndexedDataSetBad(IWT_No);
	TestNamedIndexedDataPut(IWT_Yes);		//Why isn't this implemented?
	TestNamedIndexedDataPut(IWT_No);
	TestNamedIndexedDataRemove(IWT_Yes);
	TestNamedIndexedDataRemove(IWT_No);
	TestNamedIndexedDataIterate(IWT_Yes);
	TestNamedIndexedDataIterate(IWT_No);
	TestNamedIndexedDataGetDoesNotExceedCache(IWT_Yes);
	TestNamedIndexedDataGetDoesNotExceedCache(IWT_No);
	TestNamedIndexedDataIterateDuringTreeChange(IWT_Yes);
	TestNamedIndexedDataIterateDuringTreeChange(IWT_No);

	TestStatistics();
	DataMemoryKill();
	MemoryKill();
	TypeConverterKill();
}

