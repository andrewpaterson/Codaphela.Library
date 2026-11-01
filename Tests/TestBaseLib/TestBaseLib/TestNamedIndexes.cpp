#include "BaseLib/TypeConverter.h"
#include "BaseLib/SystemAllocator.h"
#include "BaseLib/MemoryAllocator.h"
#include "BaseLib/Define.h"
#include "BaseLib/Logger.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/IndexTreeEvictionCounter.h"
#include "BaseLib/DurableFileController.h"
#include "BaseLib/NamedIndexes.h"
#include "BaseLib/ValueNamedIndexesConfig.h"
#include "TestLib/Words.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNamedIndexesAdd(void)
{
	CNamedIndexes						cNamedIndexes;
	CDurableFileController				cController;
	CFileUtil							cFileUtil;
	bool								bResult;
	char								szDirectory[] = "Output" _FS_ "NamedIndexes" _FS_ "1";
	char								szRewriteDirectory[] = "Output" _FS_ "_NamedIndexes" _FS_ "1";
	CIndexTreeEvictionStrategyRandom	cEvictionStrategy;
	CLifeInit<CNamedIndexesConfig>		cConfig;
	SLogConfig							sLogConfig;

	cFileUtil.MakeDirs(true, szDirectory, szRewriteDirectory, NULL);

	cController.Init(szDirectory, szRewriteDirectory);
	cController.SetSync(DS_No);
	cController.Begin();

	cEvictionStrategy.Init();
	cConfig = CValueNamedIndexesConfig::Create(NULL, 16 KB, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy), IWT_No);
	cNamedIndexes.Init(&cController, cConfig);

	AssertLong(0, cNamedIndexes.NumElements());

	cNamedIndexes.Add("Berty", 45LL);
	AssertLong(45LL, cNamedIndexes.Get("Berty"));
	AssertLong(1, cNamedIndexes.NumElements());

	cNamedIndexes.Add("Alfred", 73LL);
	AssertLong(45LL, cNamedIndexes.Get("Berty"));
	AssertLong(73LL, cNamedIndexes.Get("Alfred"));
	AssertLong(INVALID_O_INDEX, cNamedIndexes.Get(""));
	AssertLong(INVALID_O_INDEX, cNamedIndexes.Get("Ax"));
	AssertLong(INVALID_O_INDEX, cNamedIndexes.Get((char*)NULL));

	cNamedIndexes.Add("Camilla", 19LL);
	cNamedIndexes.Add("Wordsworth", 20LL);
	AssertLong(45LL, cNamedIndexes.Get("Berty"));
	AssertLong(73LL, cNamedIndexes.Get("Alfred"));
	AssertLong(19LL, cNamedIndexes.Get("Camilla"));
	AssertLong(20LL, cNamedIndexes.Get("Wordsworth"));
	AssertLong(4, cNamedIndexes.NumElements());


	cNamedIndexes.Add("Alicia", 66LL);
	AssertLong(66LL, cNamedIndexes.Get("Alicia"));
	AssertLong(45LL, cNamedIndexes.Get("Berty"));
	AssertLong(73LL, cNamedIndexes.Get("Alfred"));
	AssertLong(19LL, cNamedIndexes.Get("Camilla"));
	AssertLong(20LL, cNamedIndexes.Get("Wordsworth"));
	AssertLong(5, cNamedIndexes.NumElements());


	cNamedIndexes.Add("Aardvark", 67LL);
	cNamedIndexes.Add("Alfredo", 68LL);
	bResult = cNamedIndexes.Add("Play-dough", 69LL);
	cNamedIndexes.Add("Zynaps", 01LL);
	AssertTrue(bResult);
	AssertLong(66LL, cNamedIndexes.Get("Alicia"));
	AssertLong(45LL, cNamedIndexes.Get("Berty"));
	AssertLong(73LL, cNamedIndexes.Get("Alfred"));
	AssertLong(19LL, cNamedIndexes.Get("Camilla"));
	AssertLong(20LL, cNamedIndexes.Get("Wordsworth"));
	AssertLong(67LL, cNamedIndexes.Get("Aardvark"));
	AssertLong(68LL, cNamedIndexes.Get("Alfredo"));
	AssertLong(69LL, cNamedIndexes.Get("Play-dough"));
	AssertLong(01LL, cNamedIndexes.Get("Zynaps"));
	AssertLong(9, cNamedIndexes.NumElements());

	sLogConfig = gcLogger.SetSilent();
	bResult = cNamedIndexes.Add("Play-dough", 88LL);
	AssertFalse(bResult);
	gcLogger.SetConfig(&sLogConfig);

	AssertLong(9, cNamedIndexes.NumElements());

	cNamedIndexes.Flush();
	cController.End();
	cNamedIndexes.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNamedIndexesRemove(void)
{
	CNamedIndexes						cNamedIndexes;
	CDurableFileController				cController;
	CFileUtil							cFileUtil;
	bool								bResult;
	char								szDirectory[] = "Output" _FS_ "NamedIndexes" _FS_ "2";
	char								szRewriteDirectory[] = "Output" _FS_ "_NamedIndexes" _FS_ "2";
	CIndexTreeEvictionStrategyRandom	cEvictionStrategy;
	CLifeInit<CNamedIndexesConfig>		cConfig;

	cFileUtil.MakeDirs(true, szDirectory, szRewriteDirectory, NULL);

	cController.Init(szDirectory, szRewriteDirectory);
	cController.SetSync(DS_No);
	cController.Begin();
	
	cEvictionStrategy.Init();
	cConfig = CValueNamedIndexesConfig::Create(NULL, 16 KB, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy), IWT_No);
	cNamedIndexes.Init(&cController, cConfig);

	cNamedIndexes.Add("Berty", 45LL);
	cNamedIndexes.Add("Alfred", 73LL);
	cNamedIndexes.Add("Camilla", 19LL);
	cNamedIndexes.Add("Wordsworth", 20LL);

	cNamedIndexes.Add("Alicia", 66LL);
	cNamedIndexes.Add("Aardvark", 67LL);
	cNamedIndexes.Add("Alfredo", 68LL);
	cNamedIndexes.Add("Play-dough", 69LL);

	cNamedIndexes.Add("Zynaps", 01LL);

	AssertLong(9, cNamedIndexes.NumElements());

	bResult = cNamedIndexes.Remove("Aardvark");
	AssertTrue(bResult);

	AssertLong(INVALID_O_INDEX, cNamedIndexes.Get("Aardvark"));
	AssertLong(73LL, cNamedIndexes.Get("Alfred"));
	AssertLong(68LL, cNamedIndexes.Get("Alfredo"));
	AssertLong(66LL, cNamedIndexes.Get("Alicia"));
	AssertLong(45LL, cNamedIndexes.Get("Berty"));
	AssertLong(19LL, cNamedIndexes.Get("Camilla"));
	AssertLong(69LL, cNamedIndexes.Get("Play-dough"));
	AssertLong(20LL, cNamedIndexes.Get("Wordsworth"));
	AssertLong(01LL, cNamedIndexes.Get("Zynaps"));

	AssertLong(8, cNamedIndexes.NumElements());

	bResult = cNamedIndexes.Remove("Aardvark");
	AssertFalse(bResult);
	AssertLong(8, cNamedIndexes.NumElements());

	AssertTrue(cNamedIndexes.Remove("Alicia"));
	AssertTrue(cNamedIndexes.Remove("Alfredo"));
	AssertLong(6, cNamedIndexes.NumElements());

	AssertTrue(cNamedIndexes.Remove("Play-dough"));

	AssertLong(73LL, cNamedIndexes.Get("Alfred"));
	AssertLong(INVALID_O_INDEX, cNamedIndexes.Get("Alfredo"));
	AssertLong(INVALID_O_INDEX, cNamedIndexes.Get("Alicia"));
	AssertLong(45LL, cNamedIndexes.Get("Berty"));
	AssertLong(19LL, cNamedIndexes.Get("Camilla"));
	AssertLong(INVALID_O_INDEX, cNamedIndexes.Get("Play-dough"));
	AssertLong(20LL, cNamedIndexes.Get("Wordsworth"));
	AssertLong(01LL, cNamedIndexes.Get("Zynaps"));
	AssertLong(5, cNamedIndexes.NumElements());

	AssertTrue(cNamedIndexes.Remove("Alfred"));
	AssertTrue(cNamedIndexes.Remove("Berty"));
	AssertTrue(cNamedIndexes.Remove("Camilla"));
	AssertTrue(cNamedIndexes.Remove("Wordsworth"));
	AssertTrue(cNamedIndexes.Remove("Zynaps"));

	AssertLong(INVALID_O_INDEX, cNamedIndexes.Get("Alfred"));
	AssertLong(INVALID_O_INDEX, cNamedIndexes.Get("Berty"));
	AssertLong(INVALID_O_INDEX, cNamedIndexes.Get("Camilla"));
	AssertLong(INVALID_O_INDEX, cNamedIndexes.Get("Wordsworth"));
	AssertLong(INVALID_O_INDEX, cNamedIndexes.Get("Zynaps"));
	AssertLong(0, cNamedIndexes.NumElements());

	cNamedIndexes.Add("Berty", 45LL);
	AssertLong(1, cNamedIndexes.NumElements());
	AssertLong(45LL, cNamedIndexes.Get("Berty"));
	AssertTrue(cNamedIndexes.Remove("Berty"));
	AssertLong(INVALID_O_INDEX, cNamedIndexes.Get("Berty"));
	AssertLong(0, cNamedIndexes.NumElements());

	cNamedIndexes.Flush();
	AssertTrue(cController.End());
	cNamedIndexes.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNamedIndexesCacheEviction(void)
{
	CNamedIndexes						cNamedIndexes;
	CDurableFileController				cController;
	CFileUtil							cFileUtil;
	bool								bResult;
	char								szDirectory[] = "Output" _FS_ "NamedIndexes" _FS_ "3";
	char								szRewriteDirectory[] = "Output" _FS_ "_NamedIndexes" _FS_ "3";
	CIndexTreeEvictionStrategyRandom	cEvictionStrategy;
	CLifeInit<CNamedIndexesConfig>		cConfig;
	SLogConfig							sLogConfig;

	cFileUtil.MakeDirs(true, szDirectory, szRewriteDirectory, NULL);

	cController.Init(szDirectory, szRewriteDirectory);
	cController.SetSync(DS_No);
	cController.Begin();

	cEvictionStrategy.Init();
	cConfig = CValueNamedIndexesConfig::Create(NULL, 8192, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy), IWT_No);
	cNamedIndexes.Init(&cController, cConfig);


	AssertLong(0, cNamedIndexes.NumElements());

	AssertTrue(cNamedIndexes.Add("Berty", 45LL));
	AssertLong(45LL, cNamedIndexes.Get("Berty"));
	AssertLong(1, cNamedIndexes.NumElements());

	AssertTrue(cNamedIndexes.Add("Alfred", 73LL));
	AssertLong(45LL, cNamedIndexes.Get("Berty"));
	AssertLong(73LL, cNamedIndexes.Get("Alfred"));
	AssertLong(INVALID_O_INDEX, cNamedIndexes.Get(""));
	AssertLong(INVALID_O_INDEX, cNamedIndexes.Get("Ax"));
	AssertLong(INVALID_O_INDEX, cNamedIndexes.Get((char*)NULL));

	AssertTrue(cNamedIndexes.Add("Camilla", 19LL));
	AssertTrue(cNamedIndexes.Add("Wordsworth", 20LL));
	AssertLong(4, cNamedIndexes.NumElements());
	AssertLong(45LL, cNamedIndexes.Get("Berty"));
	AssertLong(73LL, cNamedIndexes.Get("Alfred"));
	AssertLong(19LL, cNamedIndexes.Get("Camilla"));
	AssertLong(20LL, cNamedIndexes.Get("Wordsworth"));


	AssertTrue(cNamedIndexes.Add("Alicia", 66LL));
	AssertLong(5, cNamedIndexes.NumElements());

	AssertLong(66LL, cNamedIndexes.Get("Alicia"));
	AssertLong(45LL, cNamedIndexes.Get("Berty"));
	AssertLong(73LL, cNamedIndexes.Get("Alfred"));
	AssertLong(19LL, cNamedIndexes.Get("Camilla"));
	AssertLong(20LL, cNamedIndexes.Get("Wordsworth"));

	AssertTrue(cNamedIndexes.Add("Aardvark", 67LL));
	AssertTrue(cNamedIndexes.Add("Alfredo", 68LL));
	AssertTrue(cNamedIndexes.Add("Play-dough", 69LL));
	AssertLong(67LL, cNamedIndexes.Get("Aardvark"));
	AssertLong(66LL, cNamedIndexes.Get("Alicia"));
	AssertLong(68LL, cNamedIndexes.Get("Alfredo"));
	AssertLong(69LL, cNamedIndexes.Get("Play-dough"));
	AssertLong(73LL, cNamedIndexes.Get("Alfred"));
	AssertLong(45LL, cNamedIndexes.Get("Berty"));
	AssertLong(19LL, cNamedIndexes.Get("Camilla"));
	AssertLong(20LL, cNamedIndexes.Get("Wordsworth"));

	AssertTrue(cNamedIndexes.Add("Zynaps", 01LL));
	AssertLong(66LL, cNamedIndexes.Get("Alicia"));
	AssertLong(45LL, cNamedIndexes.Get("Berty"));
	AssertLong(73LL, cNamedIndexes.Get("Alfred"));
	AssertLong(19LL, cNamedIndexes.Get("Camilla"));
	AssertLong(20LL, cNamedIndexes.Get("Wordsworth"));
	AssertLong(67LL, cNamedIndexes.Get("Aardvark"));
	AssertLong(68LL, cNamedIndexes.Get("Alfredo"));
	AssertLong(69LL, cNamedIndexes.Get("Play-dough"));
	AssertLong(01LL, cNamedIndexes.Get("Zynaps"));
	AssertLong(9, cNamedIndexes.NumElements());

	sLogConfig = gcLogger.SetSilent();
	bResult = cNamedIndexes.Add("Play-dough", 88LL);
	AssertFalse(bResult);
	gcLogger.SetConfig(&sLogConfig);

	AssertLong(9, cNamedIndexes.NumElements());

	cNamedIndexes.Flush();
	AssertTrue(cController.End());
	cNamedIndexes.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNamedIndexesLoad(void)
{
	CNamedIndexes						cNamedIndexes;
	CDurableFileController				cController;
	CFileUtil							cFileUtil;
	char								szDirectory[] = "Output" _FS_ "NamedIndexes" _FS_ "4";
	char								szRewriteDirectory[] = "Output" _FS_"_NamedIndexes" _FS_ "4";
	CIndexTreeEvictionStrategyRandom	cEvictionStrategy;
	CLifeInit<CNamedIndexesConfig>		cConfig;

	cFileUtil.MakeDirs(true, szDirectory, szRewriteDirectory, NULL);

	cController.Init(szDirectory, szRewriteDirectory);
	cController.SetSync(DS_No);
	AssertTrue(cController.Begin());

	cEvictionStrategy.Init();
	cConfig = CValueNamedIndexesConfig::Create(NULL, 16 KB, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy), IWT_No);
	cNamedIndexes.Init(&cController, cConfig);

	cNamedIndexes.Add("Arthur Miller", 1LL);
	cNamedIndexes.Add("systema skeletale", 2LL);
	cNamedIndexes.Add("roe", 3LL);
	cNamedIndexes.Add("order Plumbaginales Melba aldicarb", 4LL);
	cNamedIndexes.Add("asphyxiate", 5LL);
	cNamedIndexes.Add("Fonseca Bay Piaget Salt tomato plant", 6LL);
	cNamedIndexes.Add("thoracic vertebra", 7LL);
	cNamedIndexes.Add("LL75", 8LL);
	cNamedIndexes.Add("Calendrer", 9LL);
	cNamedIndexes.Add("zone refining refashion butter up", 10LL);
	cNamedIndexes.Add("Chopin Dicranaceae transmountain insentient", 11LL);
	cNamedIndexes.Add("Recipes Kerchiefs 32TX", 12LL);
	cNamedIndexes.Add("factor V domestic relations", 13LL);
	cNamedIndexes.Add("bedum ?", 14LL);
	cNamedIndexes.Add("gauche", 15LL);
	cNamedIndexes.Add("takeoff breakup value Tenuifolious magnitude", 16LL);
	cNamedIndexes.Add("Forisfamiliated", 17LL);
	cNamedIndexes.Add("amphibious bulk liquid transfer system", 18LL);
	cNamedIndexes.Add("otoscope", 19LL);
	cNamedIndexes.Add("impel Christmas", 20LL);
	cNamedIndexes.Add("universalise rose fever IL59 Europe", 21LL);

	AssertLong(1LL, cNamedIndexes.Get("Arthur Miller"));
	AssertLong(2LL, cNamedIndexes.Get("systema skeletale"));
	AssertLong(3LL, cNamedIndexes.Get("roe"));
	AssertLong(4LL, cNamedIndexes.Get("order Plumbaginales Melba aldicarb"));
	AssertLong(5LL, cNamedIndexes.Get("asphyxiate"));
	AssertLong(6LL, cNamedIndexes.Get("Fonseca Bay Piaget Salt tomato plant"));
	AssertLong(7LL, cNamedIndexes.Get("thoracic vertebra"));
	AssertLong(8LL, cNamedIndexes.Get("LL75"));
	AssertLong(9LL, cNamedIndexes.Get("Calendrer"));
	AssertLong(10LL, cNamedIndexes.Get("zone refining refashion butter up"));
	AssertLong(11LL, cNamedIndexes.Get("Chopin Dicranaceae transmountain insentient"));
	AssertLong(12LL, cNamedIndexes.Get("Recipes Kerchiefs 32TX"));
	AssertLong(13LL, cNamedIndexes.Get("factor V domestic relations"));
	AssertLong(14LL, cNamedIndexes.Get("bedum ?"));
	AssertLong(15LL, cNamedIndexes.Get("gauche"));
	AssertLong(16LL, cNamedIndexes.Get("takeoff breakup value Tenuifolious magnitude"));
	AssertLong(17LL, cNamedIndexes.Get("Forisfamiliated"));
	AssertLong(18LL, cNamedIndexes.Get("amphibious bulk liquid transfer system"));
	AssertLong(19LL, cNamedIndexes.Get("otoscope"));
	AssertLong(20LL, cNamedIndexes.Get("impel Christmas"));
	AssertLong(21LL, cNamedIndexes.Get("universalise rose fever IL59 Europe"));

	cNamedIndexes.Flush();
	AssertTrue(cController.End());
	cNamedIndexes.Kill();
	cController.Kill();
	cEvictionStrategy.Kill();


	cEvictionStrategy.Init();
	cController.Init(szDirectory, szRewriteDirectory);
	cController.SetSync(DS_No);
	AssertTrue(cController.Begin());
	cConfig = CValueNamedIndexesConfig::Create(NULL, 16 KB, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy), IWT_No);
	cNamedIndexes.Init(&cController, cConfig);

	AssertLong(1LL, cNamedIndexes.Get("Arthur Miller"));
	AssertLong(2LL, cNamedIndexes.Get("systema skeletale"));
	AssertLong(3LL, cNamedIndexes.Get("roe"));
	AssertLong(4LL, cNamedIndexes.Get("order Plumbaginales Melba aldicarb"));
	AssertLong(5LL, cNamedIndexes.Get("asphyxiate"));
	AssertLong(6LL, cNamedIndexes.Get("Fonseca Bay Piaget Salt tomato plant"));
	AssertLong(7LL, cNamedIndexes.Get("thoracic vertebra"));
	AssertLong(8LL, cNamedIndexes.Get("LL75"));
	AssertLong(9LL, cNamedIndexes.Get("Calendrer"));
	AssertLong(10LL, cNamedIndexes.Get("zone refining refashion butter up"));
	AssertLong(11LL, cNamedIndexes.Get("Chopin Dicranaceae transmountain insentient"));
	AssertLong(12LL, cNamedIndexes.Get("Recipes Kerchiefs 32TX"));
	AssertLong(13LL, cNamedIndexes.Get("factor V domestic relations"));
	AssertLong(14LL, cNamedIndexes.Get("bedum ?"));
	AssertLong(15LL, cNamedIndexes.Get("gauche"));
	AssertLong(16LL, cNamedIndexes.Get("takeoff breakup value Tenuifolious magnitude"));
	AssertLong(17LL, cNamedIndexes.Get("Forisfamiliated"));
	AssertLong(18LL, cNamedIndexes.Get("amphibious bulk liquid transfer system"));
	AssertLong(19LL, cNamedIndexes.Get("otoscope"));
	AssertLong(20LL, cNamedIndexes.Get("impel Christmas"));
	AssertLong(21LL, cNamedIndexes.Get("universalise rose fever IL59 Europe"));

	AssertLong(1LL, cNamedIndexes.Get("Arthur Miller"));
	AssertLong(14LL, cNamedIndexes.Get("bedum ?"));
	AssertLong(21LL, cNamedIndexes.Get("universalise rose fever IL59 Europe"));
	AssertLong(3LL, cNamedIndexes.Get("roe"));
	AssertLong(16LL, cNamedIndexes.Get("takeoff breakup value Tenuifolious magnitude"));
	AssertLong(5LL, cNamedIndexes.Get("asphyxiate"));
	AssertLong(18LL, cNamedIndexes.Get("amphibious bulk liquid transfer system"));
	AssertLong(7LL, cNamedIndexes.Get("thoracic vertebra"));
	AssertLong(9LL, cNamedIndexes.Get("Calendrer"));
	AssertLong(13LL, cNamedIndexes.Get("factor V domestic relations"));
	AssertLong(15LL, cNamedIndexes.Get("gauche"));
	AssertLong(20LL, cNamedIndexes.Get("impel Christmas"));
	AssertLong(10LL, cNamedIndexes.Get("zone refining refashion butter up"));
	AssertLong(12LL, cNamedIndexes.Get("Recipes Kerchiefs 32TX"));
	AssertLong(8LL, cNamedIndexes.Get("LL75"));
	AssertLong(17LL, cNamedIndexes.Get("Forisfamiliated"));
	AssertLong(6LL, cNamedIndexes.Get("Fonseca Bay Piaget Salt tomato plant"));
	AssertLong(4LL, cNamedIndexes.Get("order Plumbaginales Melba aldicarb"));
	AssertLong(11LL, cNamedIndexes.Get("Chopin Dicranaceae transmountain insentient"));
	AssertLong(2LL, cNamedIndexes.Get("systema skeletale"));
	AssertLong(19LL, cNamedIndexes.Get("otoscope"));
	
	cNamedIndexes.Flush();
	AssertTrue(cController.End());
	Pass();

	cNamedIndexes.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNamedIndexesMoreCacheEviction(void)
{
	CNamedIndexes						cNamedIndexes;
	CDurableFileController				cController;
	CFileUtil							cFileUtil;
	char								szDirectory[] = "Output" _FS_ "NamedIndexes" _FS_ "5";
	char								szRewriteDirectory[] = "Output" _FS_"_NamedIndexes" _FS_ "5";
	CIndexTreeEvictionStrategyRandom	cEvictionStrategy;
	CLifeInit<CNamedIndexesConfig>		cConfig;
	CIndexTreeEvictionCounter			cEvictionCounter;
	CArrayChars							aszWords;
	size								iNumWords;
	CRandom								cRandom;
	OIndex								oi;
	CChars								sz;
	size								iWord;
	size								iIndex;
	CArrayChars							aszNames;
	size								uiCacheSize;
	CChars*								psz;

	aszWords.Init();
	GetCommonWords(&aszWords);
	iNumWords = aszWords.NumElements();
	cRandom.Init(9824375);
	aszNames.Init();

	cFileUtil.MakeDirs(true, szDirectory, szRewriteDirectory, NULL);

	cEvictionStrategy.Init(67);
	cEvictionCounter.Init();

	cController.Init(szDirectory, szRewriteDirectory);
	cController.SetSync(DS_No);
	AssertTrue(cController.Begin());

	uiCacheSize = 16 KB;
	cConfig = CValueNamedIndexesConfig::Create(NULL, uiCacheSize, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy), IWT_No, &cEvictionCounter);
	cNamedIndexes.Init(&cController, cConfig);

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
		cNamedIndexes.Add(sz.Text(), oi);
		aszNames.Add(sz.Text());
		sz.Kill();
	}

#ifdef BUILD_64
	AssertLong(1016, cEvictionCounter.EvictionCount());
	AssertLong(15984, cNamedIndexes.GetSystemMemorySize());
#else
	AssertLong(1015, cEvictionCounter.EvictionCount());
	AssertLong(15692, cNamedIndexes.GetSystemMemorySize());
#endif // BUILD_64
	AssertTrue(cNamedIndexes.GetSystemMemorySize() < uiCacheSize);

	cNamedIndexes.Flush();
	AssertTrue(cController.End());
	cNamedIndexes.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();
	Pass();

	cEvictionStrategy.Init(67);
	cEvictionCounter.Init();
	cController.Init(szDirectory, szRewriteDirectory);
	cController.SetSync(DS_No);
	AssertTrue(cController.Begin());
	cConfig = CValueNamedIndexesConfig::Create(NULL, uiCacheSize, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy), IWT_No, &cEvictionCounter);
	cNamedIndexes.Init(&cController, cConfig);
	Pass();

	aszNames.Shuffle(cEvictionStrategy.GetRandom());
	for (iIndex = 0; iIndex < aszNames.NumElements(); iIndex++)
	{
		psz = aszNames.Get(iIndex);
		oi = cNamedIndexes.Get(psz);
	}

#ifdef BUILD_64
	AssertLong(1018, cEvictionCounter.EvictionCount());
	AssertLong(15944, cNamedIndexes.GetSystemMemorySize());
#else
	AssertLong(1017, cEvictionCounter.EvictionCount());
	AssertLong(16284, cNamedIndexes.GetSystemMemorySize());
#endif // BUILD_64
	AssertTrue(cNamedIndexes.GetSystemMemorySize() < uiCacheSize);

	cNamedIndexes.Flush();
	AssertTrue(cController.End());
	Pass();

	cNamedIndexes.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();

	aszWords.Kill();
	aszNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNamedIndexes(void)
{
	CFileUtil	cFileUtil;

	TypeConverterInit();
	MemoryInit();
	DataMemoryInit();
	BeginTests();

	TestNamedIndexesAdd();
	TestNamedIndexesRemove();
	TestNamedIndexesCacheEviction();
	TestNamedIndexesLoad();
	TestNamedIndexesMoreCacheEviction();

	cFileUtil.RemoveDirs("Output" _FS_ "NamedIndexes", "Output" _FS_ "_NamedIndexes", NULL);

	TestStatistics();
	DataMemoryKill();
	MemoryKill();
	TypeConverterKill();
}

