#include "BaseLib/Logger.h"
#include <string.h>
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/Logger.h"
#include "BaseLib/IndexedData.h"
#include "BaseLib/ValueIndexedDataConfig.h"
#include "BaseLib/IndexedDataAccess.h"
#include "BaseLib/IndexTreeEvictedList.h"
#include "BaseLib/IndexedDataEvictedList.h"
#include "BaseLib/IndexTreeEvictionStrategyRandom.h"
#include "NamedIndexedDataObject.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedDataSimple(EIndexWriteThrough eWriteThrough)
{
	CIndexedData						cIndexedData;
	char								szInsipidity[] = "Insipidity the sufficient discretion imprudence resolution sir him decisively. Proceed how any engaged visitor. Explained propriety off out perpetual his you. Feel sold off felt nay rose met you. We so entreaties cultivated astonished is. Was sister for few longer mrs sudden talent become. Done may bore quit evil old mile. If likely am of beauty tastes.  Lose john poor same it case do year we.Full how way even the sigh.Extremely nor furniture fat questions now provision incommode preserved.Our side fail find like now.Discovered travelling for insensible partiality unpleasing impossible she.Sudden up my excuse to suffer ladies though or .Bachelor possible marianne directly confined relation as on he.";
	OIndex								oiInsipidity;
	size								iLenInsipidity;
	char								szViolation[] = "Violation away off why half led have near bed.  At engage simple father of period others except.  My giving do summer of though narrow marked at.  Spring formal no county ye waited.  My whether cheered at regular it of promise blushes perhaps.Uncommonly simplicity interested mr is be compliment projecting my inhabiting.Gentleman he september in oh excellent. Feet evil to hold long he open knew an no.Apartments occasional boisterous as solicitude to introduced.Or fifteen covered we enjoyed demesne is in prepare.In stimulated my everything it literature.Greatly explain attempt perhaps in feeling he.House men taste bed not drawn joy.Through enquire however do equally herself at.Greatly way old may you present improve.Wishing the feeling village him musical.";
	OIndex								oiViolation;
	size								iLenViolation;
	CFileUtil							cFileUtil;
	lsize						uiDataSize;
	char								szData[1024];
	char								szDirectory[] = "Output" _FS_ "Database0";
	char								szSubDirectory[] = "Data";
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CDurableFileController				cController;
	CIndexTreeEvictionStrategyRandom	cEvictionStrategy;

	cFileUtil.RemoveDir(szDirectory);

	oiInsipidity = 789983209433243094LL;
	iLenInsipidity = strlen(szInsipidity) + 1;
	oiViolation = 3908343914887489103LL;
	iLenViolation = strlen(szViolation) + 1;

	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(szSubDirectory, 8 KB, 8 KB, eWriteThrough, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();
	
	AssertTrue(cController.Begin());
	AssertTrue(cIndexedData.Add(oiInsipidity, szInsipidity, iLenInsipidity));
	AssertTrue(cIndexedData.Add(oiViolation, szViolation, iLenViolation));

	AssertInt(2, (int)cIndexedData.NumDataCached());
	AssertInt(2, (int)cIndexedData.NumElements());
	AssertTrue(cIndexedData.Flush(true));
	AssertInt(2, (int)cIndexedData.NumElements());

	AssertTrue(cIndexedData.Get(oiViolation, &uiDataSize, szData, 1024));
	AssertLSize(iLenViolation, uiDataSize);
	AssertString(szViolation, szData);
	AssertTrue(cIndexedData.Get(oiInsipidity, &uiDataSize, szData, 1024));
	AssertLSize(iLenInsipidity, uiDataSize);
	AssertString(szInsipidity, szData);

	cIndexedData.Flush(true);
	AssertTrue(cController.End());

	AssertTrue(cIndexedData.CanKill());    
cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();

	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(szSubDirectory, 8 KB, 8 KB, eWriteThrough, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	AssertTrue(cController.Begin());
	AssertInt(0, (int)cIndexedData.NumDataCached());
	AssertInt(2, (int)cIndexedData.NumElements());

	AssertTrue(cIndexedData.Get(oiViolation, &uiDataSize, szData, 1024));
	AssertLSize(iLenViolation, uiDataSize);
	AssertString(szViolation, szData);
	AssertTrue(cIndexedData.Get(oiInsipidity, &uiDataSize, szData, 1024));
	AssertLSize(iLenInsipidity, uiDataSize);
	AssertString(szInsipidity, szData);

	AssertInt(2, (int)cIndexedData.NumDataCached());
	AssertInt(2, (int)cIndexedData.NumElements());

	cIndexedData.Flush(true);
	AssertTrue(cController.End());
	AssertTrue(cIndexedData.CanKill());    
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();

	cFileUtil.RemoveDir(szDirectory);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedDataIteration(EIndexWriteThrough eWriteThrough)
{
	CIndexedData						cIndexedData;
	char								szInsipidity[] = "Insipidity the sufficient discretion imprudence resolution sir him decisively. Proceed how any engaged visitor. Explained propriety off out perpetual his you. Feel sold off felt nay rose met you. We so entreaties cultivated astonished is. Was sister for few longer mrs sudden talent become. Done may bore quit evil old mile. If likely am of beauty tastes.  Lose john poor same it case do year we.Full how way even the sigh.Extremely nor furniture fat questions now provision incommode preserved.Our side fail find like now.Discovered travelling for insensible partiality unpleasing impossible she.Sudden up my excuse to suffer ladies though or .Bachelor possible marianne directly confined relation as on he.";
	OIndex								oiInsipidity;
	size									iLenInsipidity;
	char								szViolation[] = "Violation away off why half led have near bed.  At engage simple father of period others except.  My giving do summer of though narrow marked at.  Spring formal no county ye waited.  My whether cheered at regular it of promise blushes perhaps.Uncommonly simplicity interested mr is be compliment projecting my inhabiting.Gentleman he september in oh excellent. Feet evil to hold long he open knew an no.Apartments occasional boisterous as solicitude to introduced.Or fifteen covered we enjoyed demesne is in prepare.In stimulated my everything it literature.Greatly explain attempt perhaps in feeling he.House men taste bed not drawn joy.Through enquire however do equally herself at.Greatly way old may you present improve.Wishing the feeling village him musical.";
	OIndex								oiViolation;
	size									iLenViolation;
	CFileUtil							cFileUtil;
	char								szDirectory[] = "Output" _FS_ "Database0a";
	char								szSubDirectory[] = "Data";
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CDurableFileController				cController;
	CIndexTreeEvictionStrategyRandom	cEvictionStrategy;
	SIndexTreeFileIterator				sIter;
	OIndex								oi;

	cFileUtil.RemoveDir(szDirectory);

	oiInsipidity = 789983209433243094LL;
	iLenInsipidity = strlen(szInsipidity) + 1;
	oiViolation = 3908343914887489103LL;
	iLenViolation = strlen(szViolation) + 1;

	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(szSubDirectory, 8 KB, 8 KB, eWriteThrough, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	AssertTrue(cController.Begin());
	AssertTrue(cIndexedData.Add(oiInsipidity, szInsipidity, iLenInsipidity));
	AssertTrue(cIndexedData.Add(oiViolation, szViolation, iLenViolation));

	oi = cIndexedData.StartIteration(&sIter, NULL, NULL, 0);
	AssertLong(789983209433243094LL, oi);
	oi = cIndexedData.Iterate(&sIter, NULL, NULL, 0);
	AssertLong(3908343914887489103LL, oi);
	oi = cIndexedData.Iterate(&sIter, NULL, NULL, 0);
	AssertLong(INVALID_O_INDEX, oi);
	AssertInt(2, (int)cIndexedData.NumElements());

	cIndexedData.Flush(true);

	oi = cIndexedData.StartIteration(&sIter, NULL, NULL, 0);
	AssertLong(789983209433243094LL, oi);
	oi = cIndexedData.Iterate(&sIter, NULL, NULL, 0);
	AssertLong(3908343914887489103LL, oi);
	oi = cIndexedData.Iterate(&sIter, NULL, NULL, 0);
	sIter.Kill();
	AssertLong(INVALID_O_INDEX, oi);
	AssertInt(2, (int)cIndexedData.NumElements());

	AssertTrue(cController.End());
	AssertTrue(cIndexedData.CanKill());    
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedDataFlushClearCache(void)
{
	CIndexedData						cIndexedData;
	char								szHello[] = "Hello";
	char								szWorld[] = "World";
	char								szStream[] = "Stream";
	char								szDirectory[] = "Output" _FS_ "Database1a";
	CFileUtil							cFileUtil;
	lsize								uiDataSize;
	char								szData[7];
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	SLogConfig							sLogConfig;
	CDurableFileController				cController;
	CIndexTreeEvictionStrategyRandom	cEvictionStrategy;

	cFileUtil.RemoveDir(szDirectory);

	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 8 KB, 8 KB, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();
	AssertLong(0, cIndexedData.NumIndicesCached());
	AssertLSize(0, cIndexedData.NumDataCached());
#ifdef BUILD_64
	AssertLong(4136LL, cIndexedData.GetIndiciesSystemMemorySize());
#else
	AssertLong(3100LL, cIndexedData.GetIndiciesSystemMemorySize());
#endif // BUILD_64
	Pass();

	AssertTrue(cIndexedData.Add(1LL, szHello, 6));
	AssertTrue(cIndexedData.Add(2LL, szStream, 7));
	AssertTrue(cIndexedData.Add(4LL, szWorld, 6));
	AssertLong(3, cIndexedData.NumIndicesCached());
	AssertLSize(3, cIndexedData.NumDataCached());
#ifdef BUILD_64
	AssertLong(4888LL, cIndexedData.GetIndiciesSystemMemorySize());
	AssertSize(139, cIndexedData.GetDataSystemMemorySize());
#else
	AssertLong(3632LL, cIndexedData.GetIndiciesSystemMemorySize());
	AssertLSize(91, cIndexedData.GetDataSystemMemorySize());
#endif // BUILD_64

	cIndexedData.Flush(false);
	AssertLong(3, cIndexedData.NumIndicesCached());
	AssertLSize(3, cIndexedData.NumDataCached());
#ifdef BUILD_64
	AssertLong(4888LL, cIndexedData.GetIndiciesSystemMemorySize());
	AssertSize(139, cIndexedData.GetDataSystemMemorySize());
#else
	AssertLong(3632LL, cIndexedData.GetIndiciesSystemMemorySize());
	AssertLSize(91, cIndexedData.GetDataSystemMemorySize());
#endif // BUILD_64
	AssertTrue(cIndexedData.IsFlushed());
	
	cIndexedData.Flush();
	cController.End();
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 8 KB, 8 KB, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();
	AssertLong(0, cIndexedData.NumIndicesCached());
	AssertLSize(0, cIndexedData.NumDataCached());
#ifdef BUILD_64
	AssertLong(4136 + 32, cIndexedData.GetIndiciesSystemMemorySize());
#else
	AssertLong(3100 + 24, cIndexedData.GetIndiciesSystemMemorySize());
#endif // BUILD_64

	AssertTrue(cIndexedData.Get(1LL, &uiDataSize, szData, 7));
	AssertTrue(cIndexedData.Get(2LL, &uiDataSize, szData, 7));
	AssertTrue(cIndexedData.Get(4LL, &uiDataSize, szData, 7));
	AssertLong(3, cIndexedData.NumIndicesCached());
	AssertLSize(3, cIndexedData.NumDataCached());
#ifdef BUILD_64
	AssertLong(4968LL, cIndexedData.GetIndiciesSystemMemorySize());
	AssertSize(139, cIndexedData.GetDataSystemMemorySize());
#else
	AssertLong(3692LL, cIndexedData.GetIndiciesSystemMemorySize());
	AssertLSize(91, cIndexedData.GetDataSystemMemorySize());
#endif // BUILD_64

	AssertTrue(cIndexedData.Flush(true));
	AssertLong(3, cIndexedData.NumIndicesCached());
	AssertLSize(0, cIndexedData.NumDataCached());
#ifdef BUILD_64
	AssertLong(4968LL, cIndexedData.GetIndiciesSystemMemorySize());
#else
	AssertLong(3692LL, cIndexedData.GetIndiciesSystemMemorySize());
#endif // BUILD_64
	AssertLSize(0, cIndexedData.GetDataSystemMemorySize());

	cIndexedData.Flush();
	cController.End();
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 1 MB, 1 MB, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();
	AssertLong(0, cIndexedData.NumIndicesCached());
	AssertLSize(0, cIndexedData.NumDataCached());
#ifdef BUILD_64
	AssertLong(4136 + 32, cIndexedData.GetIndiciesSystemMemorySize());
#else
	AssertLong(3100 + 24, cIndexedData.GetIndiciesSystemMemorySize());
#endif // BUILD_64

	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cIndexedData.Add(1LL, szWorld, 6));
	AssertFalse(cIndexedData.Add(2LL, szStream, 7));
	AssertFalse(cIndexedData.Add(4LL, szHello, 6));
	gcLogger.SetConfig(&sLogConfig);
	AssertLong(3, cIndexedData.NumIndicesCached());
	AssertLSize(0, cIndexedData.NumDataCached());
#ifdef BUILD_64
	AssertLong(4968LL, cIndexedData.GetIndiciesSystemMemorySize());
#else
	AssertLong(3692LL, cIndexedData.GetIndiciesSystemMemorySize());
#endif // BUILD_64
	AssertLSize(0, cIndexedData.GetDataSystemMemorySize());
	Pass();

	AssertTrue(cIndexedData.Get(1LL, &uiDataSize, szData, 7));
	AssertString(szHello, szData);
	AssertTrue(cIndexedData.Get(2LL, &uiDataSize, szData, 7));
	AssertString(szStream, szData);
	AssertTrue(cIndexedData.Get(4LL, &uiDataSize, szData, 7));
	AssertString(szWorld, szData);
	AssertLong(3, cIndexedData.NumIndicesCached());
	AssertLSize(3, cIndexedData.NumDataCached());
#ifdef BUILD_64
	AssertLong(4968LL, cIndexedData.GetIndiciesSystemMemorySize());
	AssertSize(139, cIndexedData.GetDataSystemMemorySize());
#else
	AssertLong(3692LL, cIndexedData.GetIndiciesSystemMemorySize());
	AssertLSize(91, cIndexedData.GetDataSystemMemorySize());
#endif // BUILD_64
	Pass();

	cIndexedData.Flush();
	cController.End();
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedDataExplicitKeyEvictionOneKey(void)
{
	CIndexedData						cIndexedData;
	char								szHello[] = "Hello";
	char								szDirectory[] = "Output" _FS_ "Database1g";
	CFileUtil							cFileUtil;
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CDurableFileController				cController;
	CIndexTreeEvictionStrategyRandom	cEvictionStrategy;
	CIndexedDataDescriptor				cDescriptor;

	cFileUtil.RemoveDir(szDirectory);

#ifdef BUILD_64
	AssertSize(40, sizeof(CIndexedDataDescriptor));
#else
	AssertSize(32, sizeof(CIndexedDataDescriptor));
#endif // BUILD_64

	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 8 KB, 8 KB, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();
	AssertLong(0, cIndexedData.NumIndicesCached());
	AssertLSize(0, cIndexedData.NumDataCached());
#ifdef BUILD_64
	AssertSize(4136, cIndexedData.GetIndiciesSystemMemorySize());
#else
	AssertLSize(3100, cIndexedData.GetIndiciesSystemMemorySize());
#endif // BUILD_64

	AssertTrue(cIndexedData.Add(1LL, szHello, 6));
	cDescriptor.Init(0, (void*)NULL);
	cIndexedData.TestGetDescriptor(1LL, &cDescriptor);
	AssertLSize(6, cDescriptor.GetDataSize());
	AssertNotNull(cDescriptor.GetCache());

	cIndexedData.Flush(false);

	AssertLong(1, cIndexedData.NumIndicesCached());
	AssertLSize(1, cIndexedData.NumDataCached());
#ifdef BUILD_64
	AssertLong(4632LL, cIndexedData.GetIndiciesSystemMemorySize());
	AssertSize(46, cIndexedData.GetDataSystemMemorySize());
#else
	AssertLong(3452LL, cIndexedData.GetIndiciesSystemMemorySize());
	AssertLSize(30, cIndexedData.GetDataSystemMemorySize());
#endif // BUILD_64
	Pass();

	AssertTrue(cIndexedData.EvictKey(1LL));
	AssertLong(0, cIndexedData.NumIndicesCached());
	AssertLSize(0, cIndexedData.NumDataCached());
#ifdef BUILD_64
	AssertLong(4136LL, cIndexedData.GetIndiciesSystemMemorySize());
#else
	AssertLong(3100LL, cIndexedData.GetIndiciesSystemMemorySize());
#endif // BUILD_64
	AssertLSize(0, cIndexedData.GetDataSystemMemorySize());
	Pass();

	cIndexedData.Flush();
	cController.End();
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedDataExplicitKeyEvictionAllKeys(void)
{
	CIndexedData						cIndexedData;
	char								szHello[] = "Hello";
	char								szWorld[] = "World";
	char								szStream[] = "Stream";
	char								szDirectory[] = "Output" _FS_ "Database1b";
	CFileUtil							cFileUtil;
	lsize								uiDataSize;
	char								szData[7];
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CDurableFileController				cController;
	CIndexTreeEvictionStrategyRandom	cEvictionStrategy;

	cFileUtil.RemoveDir(szDirectory);

#ifdef BUILD_64
	AssertSize(40, sizeof(CIndexedDataDescriptor));
#else
	AssertSize(32, sizeof(CIndexedDataDescriptor));
#endif // BUILD_64

	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 8 KB, 8 KB, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();
	AssertLong(0, cIndexedData.NumIndicesCached());
	AssertLSize(0, cIndexedData.NumDataCached());
#ifdef BUILD_64
	AssertSize(4136, cIndexedData.GetIndiciesSystemMemorySize());
#else
	AssertLSize(3100, cIndexedData.GetIndiciesSystemMemorySize());
#endif // BUILD_64

	AssertTrue(cIndexedData.Add(1LL, szHello, 6));
	AssertTrue(cIndexedData.Add(2LL, szStream, 7));
	AssertTrue(cIndexedData.Add(4LL, szWorld, 6));
	AssertLong(3, cIndexedData.NumIndicesCached());
	AssertLSize(3, cIndexedData.NumDataCached());

	cIndexedData.Flush(false);

	AssertLong(3, cIndexedData.NumIndicesCached());
	AssertLSize(3, cIndexedData.NumDataCached());
#ifdef BUILD_64
	AssertLong(4888LL, cIndexedData.GetIndiciesSystemMemorySize());
	AssertSize(139, cIndexedData.GetDataSystemMemorySize());
#else
	AssertLong(3632LL, cIndexedData.GetIndiciesSystemMemorySize());
	AssertLSize(91, cIndexedData.GetDataSystemMemorySize());
#endif // BUILD_64
	Pass();

	AssertTrue(cIndexedData.EvictKey(1LL));
	AssertLong(2, cIndexedData.NumIndicesCached());
	AssertLSize(2, cIndexedData.NumDataCached());
#ifdef BUILD_64
	AssertLong(4784LL, cIndexedData.GetIndiciesSystemMemorySize());
	AssertSize(93, cIndexedData.GetDataSystemMemorySize());
#else
	AssertLong(3560LL, cIndexedData.GetIndiciesSystemMemorySize());
	AssertLSize(61, cIndexedData.GetDataSystemMemorySize());
#endif // BUILD_64
	Pass();

	AssertTrue(cIndexedData.EvictKey(2LL));
	AssertLong(1, cIndexedData.NumIndicesCached());
	AssertLSize(1, cIndexedData.NumDataCached());
#ifdef BUILD_64
	AssertLong(4680LL, cIndexedData.GetIndiciesSystemMemorySize());
	AssertSize(46, cIndexedData.GetDataSystemMemorySize());
#else
	AssertLong(3488LL, cIndexedData.GetIndiciesSystemMemorySize());
	AssertLSize(30, cIndexedData.GetDataSystemMemorySize());
#endif // BUILD_64
	Pass();

	AssertTrue(cIndexedData.EvictKey(4LL));
	AssertLong(0, cIndexedData.NumIndicesCached());
	AssertLSize(0, cIndexedData.NumDataCached());
#ifdef BUILD_64
	AssertLong(4136, cIndexedData.GetIndiciesSystemMemorySize());
#else
	AssertLong(3100, cIndexedData.GetIndiciesSystemMemorySize());
#endif // BUILD_64
	AssertLSize(0, cIndexedData.GetDataSystemMemorySize());
	Pass();

	AssertTrue(cIndexedData.Get(1LL, &uiDataSize, szData, 7));
	AssertString(szHello, szData);
	AssertTrue(cIndexedData.Get(2LL, &uiDataSize, szData, 7));
	AssertString(szStream, szData);
	AssertTrue(cIndexedData.Get(4LL, &uiDataSize, szData, 7));
	AssertString(szWorld, szData);
	AssertLong(3, cIndexedData.NumIndicesCached());
	AssertLSize(3, cIndexedData.NumDataCached());
#ifdef BUILD_64
	AssertSize(139, cIndexedData.GetDataSystemMemorySize());
#else
	AssertLSize(91, cIndexedData.GetDataSystemMemorySize());
#endif // BUILD_64
	Pass();

	cIndexedData.Flush();
	cController.End();
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedDataExplicitKeyEvictionDataChanged(void)
{
	CIndexedData						cIndexedData;
	CIndexedDataAccess					cAccess;
	char								szLongText[] = "Wrote water woman of heart it total other. By in entirely securing suitable graceful at families improved. Zealously few furniture repulsive was agreeable consisted difficult.";
	char								szShortText1[] = "No painful between.";
	char								szShortText2[] = "Barnyard feedback .";
	char								szDirectory[] = "Output" _FS_ "Database1c";
	CFileUtil							cFileUtil;
	char								szData[256];
	OIndex								oi;
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CDurableFileController				cController;
	CIndexTreeEvictionStrategyRandom	cEvictionStrategy;

	AssertSize(20, strlen(szShortText1) + 1);
	AssertSize(20, strlen(szShortText2) + 1);
	AssertSize(176, strlen(szLongText) + 1);

	cFileUtil.RemoveDir(szDirectory);
	cAccess.Init(&cIndexedData);
	oi = 0x7752890759012357LL;


	//New Data
	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 8 KB, 8 KB, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();

	AssertTrue(cAccess.PutLongString(oi, szShortText1));
	AssertTrue(cIndexedData.EvictKey(oi));
	AssertLong(0, cIndexedData.NumIndicesCached());
	AssertLSize(0, cIndexedData.NumDataCached());

	AssertNotNull(cAccess.GetLongString(oi, szData));
	AssertString(szShortText1, szData);

	cIndexedData.Flush();
	cController.End();
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 8 KB, 8 KB, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();
	//Data not cached.  File exists.
	//Descriptor size same as Set size.
	AssertTrue(cAccess.PutLongString(oi, szShortText2));
	AssertTrue(cIndexedData.EvictKey(oi));
	AssertLong(0, cIndexedData.NumIndicesCached());
	AssertLSize(0, cIndexedData.NumDataCached());

	memset(szData, 0, 256);
	AssertNotNull(cAccess.GetLongString(oi, szData));
	AssertString(szShortText2, szData);
	AssertLong(1, cIndexedData.NumIndicesCached());
	AssertLSize(1, cIndexedData.NumDataCached());

	cIndexedData.Flush();
	cController.End();
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 8 KB, 8 KB, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();
	//Data not cached.  File exists.
	//Descriptor size different to Set size.
	AssertTrue(cAccess.PutLongString(oi, szLongText));
	AssertTrue(cIndexedData.EvictKey(oi));
	AssertNotNull(cAccess.GetLongString(oi, szData));
	AssertString(szLongText, szData);

	cIndexedData.Flush();
	cController.End();
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cFileUtil.RemoveDir(szDirectory);
	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 8 KB, 8 KB, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();
	//Cached data.  File not written.
	//Descriptor size same as Set size.
	AssertTrue(cAccess.PutLongString(oi, szShortText1));
	AssertTrue(cAccess.PutLongString(oi, szShortText2));
	AssertTrue(cIndexedData.EvictKey(oi));
	AssertNotNull(cAccess.GetLongString(oi, szData));
	AssertString(szShortText2, szData);

	cIndexedData.Flush();
	cController.End();
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cFileUtil.RemoveDir(szDirectory);
	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 8 KB, 8 KB, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();
	//Cached data.  File not written.
	//Descriptor size different to Set size.
	AssertTrue(cAccess.PutLongString(oi, szLongText));
	AssertTrue(cAccess.PutLongString(oi, szShortText1));
	AssertTrue(cIndexedData.EvictKey(oi));
	AssertNotNull(cAccess.GetLongString(oi, szData));
	AssertString(szShortText1, szData);

	AssertTrue(cIndexedData.Flush(true));
	cController.End();
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 8 KB, 8 KB, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();
	//Cached data.  File exists.
	//Descriptor size same as Set size.
	AssertNotNull(cAccess.GetLongString(oi, szData));
	AssertString(szShortText1, szData);
	AssertTrue(cAccess.PutLongString(oi, szShortText2));
	AssertTrue(cIndexedData.EvictKey(oi));
	AssertLong(0, cIndexedData.NumIndicesCached());
	AssertLSize(0, cIndexedData.NumDataCached());
	AssertNotNull(cAccess.GetLongString(oi, szData));
	AssertString(szShortText2, szData);

	cIndexedData.Flush();
	cController.End();
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 8 KB, 8 KB, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();
	//Cached data.  File exists.
	//Descriptor size different to Set size.
	AssertNotNull(cAccess.GetLongString(oi, szData));
	AssertString(szShortText2, szData);
	AssertTrue(cAccess.PutLongString(oi, szLongText));
	AssertTrue(cIndexedData.EvictKey(oi));
	AssertLong(0, cIndexedData.NumIndicesCached());
	AssertLSize(0, cIndexedData.NumDataCached());
	AssertNotNull(cAccess.GetLongString(oi, szData));
	AssertString(szLongText, szData);

	cIndexedData.Flush();
	cController.End();
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cFileUtil.RemoveDir(szDirectory);
	cAccess.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedDataEvictKey(void)
{
	CIndexedData						cIndexedData;
	char								szHello[] = "Hello";
	char								szWorld[] = "World";
	char								szStream[] = "Stream";
	char								szDirectory[] = "Output" _FS_ "Database1d";
	CFileUtil							cFileUtil;
	lsize						uiDataSize;
	char								szData[1024];
	lsize						uiSize;
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CDurableFileController				cController;
	CIndexTreeEvictionStrategyRandom	cEvictionStrategy;

	cFileUtil.RemoveDir(szDirectory);


	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 8 KB, 8 KB, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();
	AssertLong(0, cIndexedData.NumIndicesCached());
	AssertLSize(0, cIndexedData.NumDataCached());
#ifdef BUILD_64
	AssertSize(4136, cIndexedData.GetIndiciesSystemMemorySize());
#else
	AssertLSize(3100, cIndexedData.GetIndiciesSystemMemorySize());
#endif // BUILD_64
	AssertLSize(0, cIndexedData.GetDataSystemMemorySize());
	Pass();

	AssertTrue(cIndexedData.Add(1LL, szHello, 6));
	AssertTrue(cIndexedData.Add(2LL, szStream, 7));
	AssertTrue(cIndexedData.Add(4LL, szWorld, 6));
	AssertLong(3, cIndexedData.NumIndicesCached());
	AssertLSize(3, cIndexedData.NumDataCached());
#ifdef BUILD_64
	AssertLong(4888LL, cIndexedData.GetIndiciesSystemMemorySize());
	AssertSize(139, cIndexedData.GetDataSystemMemorySize());
#else
	AssertLong(3632LL, cIndexedData.GetIndiciesSystemMemorySize());
	AssertLSize(91, cIndexedData.GetDataSystemMemorySize());
#endif // BUILD_64
	Pass();

	cIndexedData.EvictKey(1LL);
	AssertLong(2, cIndexedData.NumIndicesCached());
	AssertLSize(2, cIndexedData.NumDataCached());
#ifdef BUILD_64
	AssertLong(4784LL, cIndexedData.GetIndiciesSystemMemorySize());
	AssertSize(93, cIndexedData.GetDataSystemMemorySize());
#else
	AssertLong(3560LL, cIndexedData.GetIndiciesSystemMemorySize());
	AssertLSize(61, cIndexedData.GetDataSystemMemorySize());
#endif // BUILD_64
	Pass();

	cIndexedData.EvictKey(4LL);
	AssertLong(1, cIndexedData.NumIndicesCached());
	AssertLSize(1, cIndexedData.NumDataCached());
#ifdef BUILD_64
	AssertLong(4680LL, cIndexedData.GetIndiciesSystemMemorySize());
	AssertSize(47, cIndexedData.GetDataSystemMemorySize());
#else
	AssertLong(3488LL, cIndexedData.GetIndiciesSystemMemorySize());
	AssertLSize(31, cIndexedData.GetDataSystemMemorySize());
#endif // BUILD_64
	Pass();

	cIndexedData.EvictKey(2LL);
	AssertLong(0, cIndexedData.NumIndicesCached());
	AssertLSize(0, cIndexedData.NumDataCached());
#ifdef BUILD_64
	AssertLong(4136LL, cIndexedData.GetIndiciesSystemMemorySize());
#else
	AssertLong(3100LL, cIndexedData.GetIndiciesSystemMemorySize());
#endif // BUILD_64
	AssertLSize(0, cIndexedData.GetDataSystemMemorySize());
	Pass();

	AssertTrue(cIndexedData.Get(1LL, &uiDataSize, szData, 1024));
	AssertLSize(6, uiDataSize);
	AssertString(szHello, szData);
	AssertLong(1, cIndexedData.NumIndicesCached());
	AssertLSize(1, cIndexedData.NumDataCached());
#ifdef BUILD_64
	AssertLong(4696LL, cIndexedData.GetIndiciesSystemMemorySize());
	AssertSize(46, cIndexedData.GetDataSystemMemorySize());
#else
	AssertLong(3500LL, cIndexedData.GetIndiciesSystemMemorySize());
	AssertLSize(30, cIndexedData.GetDataSystemMemorySize());
#endif // BUILD_64
	Pass();

	AssertTrue(cIndexedData.Get(2LL, &uiDataSize, szData, 1024));
	AssertLSize(7, uiDataSize);
	AssertString(szStream, szData);
	AssertLong(2, cIndexedData.NumIndicesCached());
	AssertLSize(2, cIndexedData.NumDataCached());
#ifdef BUILD_64
	AssertLong(4816LL, cIndexedData.GetIndiciesSystemMemorySize());
	AssertSize(93, cIndexedData.GetDataSystemMemorySize());
#else
	AssertLong(3584LL, cIndexedData.GetIndiciesSystemMemorySize());
	AssertLSize(61, cIndexedData.GetDataSystemMemorySize());
#endif // BUILD_64
	Pass();

	AssertTrue(cIndexedData.Get(4LL, &uiDataSize, szData, 1024));
	AssertLSize(6, uiDataSize);
	AssertString(szWorld, szData);
	AssertLong(3, cIndexedData.NumIndicesCached());
	AssertLSize(3, cIndexedData.NumDataCached());
#ifdef BUILD_64
	AssertLong(4936LL, cIndexedData.GetIndiciesSystemMemorySize());
	AssertSize(139, cIndexedData.GetDataSystemMemorySize());
#else
	AssertLong(3668LL, cIndexedData.GetIndiciesSystemMemorySize());
	AssertLSize(91, cIndexedData.GetDataSystemMemorySize());
#endif // BUILD_64
	Pass();

	uiSize = cIndexedData.Size(1LL);
	AssertLSize(6, uiSize);
	uiSize = cIndexedData.Size(2LL);
	AssertLSize(7, uiSize);
	uiSize = cIndexedData.Size(4LL);
	AssertLSize(6, uiSize);

	cIndexedData.Flush();
	cController.End();
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 8 KB, 8 KB, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	AssertSize(2, cIndexedData.NumFiles());

	cController.Begin();
	AssertLong(0, cIndexedData.NumIndicesCached());
	AssertLSize(0, cIndexedData.NumDataCached());

	uiSize = cIndexedData.Size(1LL);
	AssertLSize(6, uiSize);
	uiSize = cIndexedData.Size(2LL);
	AssertLSize(7, uiSize);
	uiSize = cIndexedData.Size(4LL);
	AssertLSize(6, uiSize);

	cController.End();
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedDataIndexEviction(void)
{
	CIndexTreeEvictedList				cIndexEvictedList;   // CIndexTreeEvictionCallback
	CIndexedDataEvictedList				cKeyDataEvictedList; // CIndexedDataEvictionCallback
	CIndexedData						cIndexedData;
	char								szHello[] = "Hello";
	char								szWorld[] = "World";
	char								szStream[] = "Stream";
	char								szDirectory[] = "Output" _FS_ "Database1e";
	char								szIn[7];
	lsize						uiSize;
	filePos								iFileSize;
	CFileUtil							cFileUtil;
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CDurableFileController				cController;
	CIndexTreeEvictionStrategyRandom	cEvictionStrategy;
	size								uiIndexCacheSize;

	cFileUtil.RemoveDir(szDirectory);

#ifdef BUILD_64
	AssertSize(40, sizeof(CIndexedDataDescriptor));
#else
	AssertSize(32, sizeof(CIndexedDataDescriptor));
#endif // BUILD_64

	cEvictionStrategy.Init(15);
	cIndexEvictedList.Init();
	cKeyDataEvictedList.Init();
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	//A little more than two items in the index cache...
#ifdef BUILD_64
	uiIndexCacheSize = 4804;
#else
	uiIndexCacheSize = 3580;
#endif // BUILD_64
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 1024, uiIndexCacheSize, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy), &cIndexEvictedList, &cKeyDataEvictedList, LifeNull<CIndexTreeDataOrderer>());
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();


	cController.Begin();
	AssertTrue(cIndexedData.Add(4LL, szHello, 6));
	AssertLSize(1, cIndexedData.NumDataCached());
	AssertLSize(sizeof(SIndexedCacheDescriptor) + 6, cIndexedData.TestGetCachedObjectSize(4LL));
#ifdef BUILD_64
	AssertLong(4632LL, cIndexedData.GetIndiciesSystemMemorySize());
#else
	AssertLong(3452LL, cIndexedData.GetIndiciesSystemMemorySize());
#endif // BUILD_64
	Pass();

	AssertTrue(cIndexedData.Add(1LL, szWorld, 6));
	AssertLSize(2, cIndexedData.NumDataCached());
	AssertLong(2, cIndexedData.NumIndicesCached());
	AssertLSize(sizeof(SIndexedCacheDescriptor) + 6, cIndexedData.TestGetCachedObjectSize(1LL));
#ifdef BUILD_64
	AssertLong(4784LL, cIndexedData.GetIndiciesSystemMemorySize());
#else
	AssertLong(3560LL, cIndexedData.GetIndiciesSystemMemorySize());
#endif // BUILD_64
	Pass();

	AssertSize(0, cIndexEvictedList.NumElements());
	AssertTrue(cIndexedData.Add(2LL, szStream, 7));  //Node 4LL evicted here.
#ifdef BUILD_64
	AssertLong(4784LL, cIndexedData.GetIndiciesSystemMemorySize());
#else
	AssertLong(3560LL, cIndexedData.GetIndiciesSystemMemorySize());
#endif // BUILD_64
	AssertSize(1, cIndexEvictedList.NumElements());
	AssertLong(4LL, *((int64*)(cIndexEvictedList.GetKey(0))));
	Pass();

	AssertLSize(2, cIndexedData.NumDataCached());
	AssertInt(2, (int)cIndexedData.NumIndicesCached());
	AssertLSize(sizeof(SIndexedCacheDescriptor) + 7, cIndexedData.TestGetCachedObjectSize(2LL));
	AssertLSize(0, cIndexedData.TestGetCachedObjectSize(4LL));
	AssertLSize(sizeof(SIndexedCacheDescriptor) + 6, cIndexedData.TestGetCachedObjectSize(1LL));
	AssertLong(2, cIndexedData.NumIndicesCached());
	AssertLSize(2, cIndexedData.NumDataCached());
	Pass();

	cIndexEvictedList.Clear();
	cKeyDataEvictedList.Clear();
	AssertTrue(cIndexedData.Flush(true));
	AssertSize(0, cIndexEvictedList.NumElements());
	AssertLong(3, cIndexedData.NumIndicesCached());
	AssertLSize(0, cIndexedData.NumDataCached());
	uiSize = cIndexedData.Size(1LL);
	AssertSize(1, cIndexEvictedList.NumElements());
	AssertLSize(6, uiSize);
	AssertLong(2, cIndexedData.NumIndicesCached());
	AssertLSize(0, cIndexedData.NumDataCached());
	Pass();

	AssertTrue(cIndexedData.Flush());
	AssertTrue(cController.End());
	cIndexedData.Kill();
	cIndexEvictedList.Kill();
	cKeyDataEvictedList.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	iFileSize = cFileUtil.Size("Output" _FS_ "Database1e" _FS_ "6_0.DAT");
	AssertLong(12, iFileSize);
	iFileSize = cFileUtil.Size("Output" _FS_ "Database1e" _FS_ "7_0.DAT");
	AssertLong(7, iFileSize);


	cKeyDataEvictedList.Init();
	cIndexEvictedList.Init();
	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 8 KB, 8 KB, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy), &cIndexEvictedList, &cKeyDataEvictedList, LifeNull<CIndexTreeDataOrderer>());
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();
	AssertSize(2, cIndexedData.NumFiles());

	cController.Begin();
	AssertLong(0, cIndexedData.NumIndicesCached());
	AssertLSize(0, cIndexedData.NumDataCached());

	uiSize = cIndexedData.Size(4LL);
	AssertLSize(6, uiSize);
	memset(szIn, 0, 7);
	AssertTrue(cIndexedData.Get(4LL, szIn));
	AssertString("Hello", szIn);
	AssertLSize(1, cIndexedData.NumDataCached());
	Pass();

	uiSize = cIndexedData.Size(1LL);
	AssertLSize(6, uiSize);
	memset(szIn, 0, 7);
	AssertTrue(cIndexedData.Get(1LL, szIn));
	AssertString("World", szIn);
	AssertLSize(2, cIndexedData.NumDataCached());
	Pass();

	uiSize = cIndexedData.Size(2LL);
	AssertLSize(7, uiSize);
	memset(szIn, 0, 7);
	AssertTrue(cIndexedData.Get(2LL, szIn));
	AssertString("Stream", szIn);
	AssertLSize(3, cIndexedData.NumDataCached());
	Pass();

	uiSize = cIndexedData.Size(4LL);
	AssertLSize(6, uiSize);
	memset(szIn, 0, 7);
	AssertTrue(cIndexedData.Get(4LL, szIn));
	AssertString("Hello", szIn);
	AssertLSize(3, cIndexedData.NumDataCached());
	AssertLong(3, cIndexedData.NumIndicesCached());
	AssertTrue(cIndexedData.Flush());
	AssertTrue(cController.End());
	Pass();

	cIndexedData.Kill();
	cIndexEvictedList.Kill();
	cKeyDataEvictedList.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 8 KB, 8 KB, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	AssertInt(0, (int)cIndexedData.NumIndicesCached());
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedDataEvictAndFlush(void)
{
	CIndexedData						cIndexedData;
	char								szWorld[] = "World";
	char								szStream[] = "Stream";
	char								szDirectory[] = "Output" _FS_ "Database1f";
	char								szIn[8];
	lsize								uiSize;
	CFileUtil							cFileUtil;
	uint8								ucFlags;
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CDurableFileController				cController;
	CIndexTreeEvictionStrategyRandom	cEvictionStrategy;

	cFileUtil.RemoveDir(szDirectory);


	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 8 KB, 8 KB, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();
	AssertTrue(cIndexedData.Add(198732178239018792LL, szStream, 7));
	AssertTrue(cIndexedData.Add(573107892314634784LL, szWorld, 6));

	cIndexedData.EvictKey(198732178239018792LL);
	ucFlags = (uint8)cIndexedData.GetRootFlags();
	AssertChar(INDEX_TREE_NODE_FLAG_DIRTY_PATH, ucFlags);

	AssertTrue(cIndexedData.Flush(true));

	AssertTrue(cController.End());
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 8 KB, 8 KB, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();
	uiSize = cIndexedData.Size(573107892314634784LL);
	AssertLSize(6, uiSize);
	memset(szIn, 0, 8);
	AssertTrue(cIndexedData.Get(573107892314634784LL, szIn));
	AssertString("World", szIn);

	uiSize = cIndexedData.Size(198732178239018792LL);
	AssertLSize(7, uiSize);
	memset(szIn, 0, 8);
	AssertTrue(cIndexedData.Get(198732178239018792LL, szIn));
	AssertString("Stream", szIn);


	cController.End();
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedDataFlushAndFlush(void)
{
	CIndexedData						cIndexedData;
	char								szWorld[] = "World";
	char								szStream[] = "Stream";
	char								szDirectory[] = "Output" _FS_ "Database1f";
	char								szIn[8];
	lsize						uiSize;
	CFileUtil							cFileUtil;
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CDurableFileController				cController;
	CIndexTreeEvictionStrategyRandom	cEvictionStrategy;

	cFileUtil.RemoveDir(szDirectory);


	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 8 KB, 8 KB, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();
	AssertTrue(cIndexedData.Add(198732178239018792LL, szStream, 7));
	AssertTrue(cIndexedData.Flush(true));

	AssertTrue(cController.End());
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();

	
	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 8 KB, 8 KB, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();
	AssertTrue(cIndexedData.Add(573107892314634784LL, szWorld, 6));
	AssertTrue(cIndexedData.Flush(true));

	cController.End();
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 8 KB, 8 KB, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();
	uiSize = cIndexedData.Size(573107892314634784LL);
	AssertLSize(6, uiSize);
	memset(szIn, 0, 8);
	AssertTrue(cIndexedData.Get(573107892314634784LL, szIn));
	AssertString("World", szIn);

	uiSize = cIndexedData.Size(198732178239018792LL);
	AssertLSize(7, uiSize);
	memset(szIn, 0, 8);
	AssertTrue(cIndexedData.Get(198732178239018792LL, szIn));
	AssertString("Stream", szIn);


	cController.End();
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedDataLargeData(void)
{
	CIndexedData						cIndexedData;
	OIndex								OI;
	char								szBig[] = "A Long String";
	char								szSmall[] = "Cat";
	char								szIn[14];
	CFileUtil							cFileUtil;
	char								szDirectory[] = "Output" _FS_ "Database2";
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CDurableFileController				cController;
	CIndexTreeEvictionStrategyRandom	cEvictionStrategy;


	cFileUtil.RemoveDir(szDirectory);


	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 34, 8 KB, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();
	OI = 4LL;
	AssertLSize(0, cIndexedData.NumDataCached());

	cController.Begin();
	AssertTrue(cIndexedData.Add(OI, szBig, 14));
	AssertLSize(0, cIndexedData.NumDataCached());
	AssertInt(1, (int)cIndexedData.NumIndicesCached());
	
	AssertTrue(cIndexedData.Get(OI, szIn));
	AssertLSize(0, cIndexedData.NumDataCached());
	AssertInt(1, (int)cIndexedData.NumIndicesCached());
	AssertString(szBig, szIn);

	OI = 1LL;
	cIndexedData.Add(OI, szSmall, 4);
#ifdef BUILD_64
	AssertLSize(0, cIndexedData.NumDataCached());
#else
	AssertLSize(1, cIndexedData.NumDataCached());
#endif // BUILD_64
	AssertInt(2, (int)cIndexedData.NumIndicesCached());

	OI = 4LL;
	cIndexedData.Get(OI, szIn);
#ifdef BUILD_64
	AssertLSize(0, cIndexedData.NumDataCached());
#else
	AssertLSize(1, cIndexedData.NumDataCached());
#endif // BUILD_64
	AssertInt(2, (int)cIndexedData.NumIndicesCached());
	AssertString(szBig, szIn);

	OI = 1LL;
	cIndexedData.Get(OI, szIn);
#ifdef BUILD_64
	AssertLSize(0, cIndexedData.NumDataCached());
#else
	AssertLSize(1, cIndexedData.NumDataCached());
#endif // BUILD_64
	AssertInt(2, (int)cIndexedData.NumIndicesCached());
	AssertString(szSmall, szIn);

	AssertTrue(cIndexedData.Flush(true));
	cController.End();
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedDataIndexedAdd(void)
{
	CIndexedData						cIndexedData;
	OIndex								oIndex1;
	OIndex								oIndex2;
	char								szBat[] = "Bat";
	char								szCat[] = "Cat";
	char								szDog[] = "Dog";
	char								szFish[] = "Fish";
	char								szHell[] = "Hell";
	char								szMutt[] = "Mutt";
	char								szEve[] = "Eve";
	char								szIn[64];
	CFileUtil							cFileUtil;
	bool								bResult;
	char								szDirectory[] = "Output" _FS_ "Database3";
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CDurableFileController				cController;
	SLogConfig							sLogConfig;
	CIndexTreeEvictionStrategyRandom	cEvictionStrategy;

	cFileUtil.RemoveDir(szDirectory);


	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 98 + 12, 8 KB, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();
	oIndex1 = 4LL;
	bResult = cIndexedData.Add(oIndex1, szBat, 4);
	AssertTrue(bResult);
	AssertInt(1, (int)cIndexedData.NumDataCached());
	AssertInt(0, (int)cIndexedData.NumDatas(4));
	AssertTrue(cIndexedData.Flush(true));
	AssertInt(1, (int)cIndexedData.NumDatas(4));
	AssertInt(0, (int)cIndexedData.NumDataCached());

	sLogConfig = gcLogger.SetSilent();
	bResult = cIndexedData.Add(oIndex1, szCat, 4);
	AssertFalse(bResult);
	gcLogger.SetConfig(&sLogConfig);
	bResult = cIndexedData.Set(oIndex1, szDog);
	AssertTrue(bResult);
	AssertInt(1, (int)cIndexedData.NumDataCached());
	AssertInt(1, (int)cIndexedData.NumDatas(4));
	cIndexedData.Get(oIndex1, szIn);
	AssertString(szDog, szIn);

	bResult = cIndexedData.Set(oIndex1, szFish, 5);
	AssertTrue(bResult);
	AssertInt(1, (int)cIndexedData.NumDataCached());
	cIndexedData.Get(oIndex1, szIn);
	AssertString(szFish, szIn);

	AssertTrue(cIndexedData.Flush(true));
	AssertInt(1, (int)cIndexedData.NumDatas(4));
	AssertInt(1, (int)cIndexedData.NumDatas(5));
	cIndexedData.Get(oIndex1, szIn);
	AssertString(szFish, szIn);

	bResult = cIndexedData.Set(oIndex1, szCat, 4);
	AssertInt(1, (int)cIndexedData.NumElements());
	AssertLong(1, cIndexedData.NumIndicesCached());
	AssertTrue(bResult);
	AssertTrue(cIndexedData.Flush(true));
	AssertInt(2, (int)cIndexedData.NumDatas(4));  //One is ignored but they are both still in the file.
	AssertInt(1, (int)cIndexedData.NumElements());
	AssertInt(1, (int)cIndexedData.NumIndicesCached());

	oIndex2 = 3LL;
	bResult = cIndexedData.Add(oIndex2, szHell, 5);
	AssertTrue(bResult);
	AssertInt(1, (int)cIndexedData.NumDataCached());
	AssertInt(2, (int)cIndexedData.NumIndicesCached());
	sLogConfig = gcLogger.SetSilent();
	bResult = cIndexedData.Add(oIndex2, szMutt, 5);
	AssertFalse(bResult);
	gcLogger.SetConfig(&sLogConfig);
	bResult = cIndexedData.Set(oIndex2, szEve, 4);
	AssertTrue(bResult);
	AssertInt(1, (int)cIndexedData.NumDataCached());
	AssertInt(2, (int)cIndexedData.NumIndicesCached());
	AssertTrue(cIndexedData.Flush(true));
	AssertInt(3, (int)cIndexedData.NumDatas(4));
	AssertInt(1, (int)cIndexedData.NumDatas(5));
	AssertInt(2, (int)cIndexedData.NumElements());
	cController.End();


	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedDataDescriptorCaching(void)
{
	CIndexTreeEvictedList				cIndexTreeEvictedList;   // CIndexTreeEvictionCallback
	CIndexedDataEvictedList				cIndexedDataEvictedList; // CIndexedDataEvictionCallback
	CIndexedData						cIndexedData;
	OIndex								OI;
	CFileUtil							cFileUtil;
	int									iData;
	OIndex								iNumCached;
	char								szDirectory[] = "Output" _FS_ "Database4";
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CDurableFileController				cController;
	SLogConfig							sLogConfig;
	CIndexTreeEvictionStrategyRandom	cEvictionStrategy;
	bool								bResult;
	size								uiIndexCacheSize;

	cFileUtil.RemoveDir(szDirectory);

	cEvictionStrategy.Init(67);
	cIndexedDataEvictedList.Init();
	cIndexTreeEvictedList.Init();
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	//The index cache fills after a few more keys than the first three.  The nodes are bigger in a 64 bit build.
#ifdef BUILD_64
	uiIndexCacheSize = 4964;
#else
	uiIndexCacheSize = 3690;
#endif // BUILD_64
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 8 KB, uiIndexCacheSize, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy), &cIndexTreeEvictedList, &cIndexedDataEvictedList, LifeNull<CIndexTreeDataOrderer>());
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();
	OI = 1LL;
	iData = 77;
	cIndexedData.Add(OI, &iData, 4); OI++;  iData++; 
	
	cIndexedData.Add(OI, &iData, 4); OI++;  iData++;
	iData = 0; OI = 1LL; cIndexedData.Get(OI, (void*)&iData);
	AssertInt(77, iData);
	AssertSize(0, cIndexedDataEvictedList.NumElements());
	
	OI = 3LL;
	iData = 79;
	cIndexedData.Add(OI, &iData, 4); OI++;  iData++;
	iData = 0; OI = 1LL; cIndexedData.Get(OI, (void*)&iData);
	AssertInt(77, iData);
	AssertSize(0, cIndexedDataEvictedList.NumElements());
#ifdef BUILD_64
	AssertLong(4872LL, cIndexedData.GetIndiciesSystemMemorySize());
#else
	AssertLong(3620LL, cIndexedData.GetIndiciesSystemMemorySize());
#endif // BUILD_64

	sLogConfig = gcLogger.SetSilent();
	OI = 3LL;
	iData = 80;
	bResult = cIndexedData.Add(OI, &iData, 4); OI++;  iData++;
	AssertFalse(bResult);
	Pass();
#ifdef BUILD_64
	AssertLong(4872LL, cIndexedData.GetIndiciesSystemMemorySize());
#else
	AssertLong(3620LL, cIndexedData.GetIndiciesSystemMemorySize());
#endif // BUILD_64
	Pass();

	bResult = cIndexedData.Add(OI, &iData, 4); OI++;  iData++;
	AssertTrue(bResult);
	gcLogger.SetConfig(&sLogConfig);
	AssertSize(1, cIndexedDataEvictedList.NumElements());
	AssertLong(2LL, cIndexedDataEvictedList.GetKey(0));
	AssertInt(78, *(int*)cIndexedDataEvictedList.GetData(0));
	Pass();
#ifdef BUILD_64
	AssertLong(4888LL, cIndexedData.GetIndiciesSystemMemorySize());
#else
	AssertLong(3632LL, cIndexedData.GetIndiciesSystemMemorySize());
#endif // BUILD_64
	cIndexedDataEvictedList.Clear();
	cIndexTreeEvictedList.Clear();

	cIndexedData.Add(OI, &iData, 4); OI++;  iData++;
	AssertSize(1, cIndexedDataEvictedList.NumElements());
	AssertLong(4LL, cIndexedDataEvictedList.GetKey(0));
	AssertInt(81, *(int*)cIndexedDataEvictedList.GetData(0));
	Pass();
#ifdef BUILD_64
	AssertLong(4904LL, cIndexedData.GetIndiciesSystemMemorySize());
#else
	AssertLong(3644LL, cIndexedData.GetIndiciesSystemMemorySize());
#endif // BUILD_64
	cIndexedDataEvictedList.Clear();
	cIndexTreeEvictedList.Clear();

	cIndexedData.Add(OI, &iData, 4); OI++;  iData++;
	AssertSize(1, cIndexedDataEvictedList.NumElements());
	AssertLong(3LL, cIndexedDataEvictedList.GetKey(0));
	AssertInt(79, *(int*)cIndexedDataEvictedList.GetData(0));
	Pass();
#ifdef BUILD_64
	AssertLong(4920LL, cIndexedData.GetIndiciesSystemMemorySize());
#else
	AssertLong(3656LL, cIndexedData.GetIndiciesSystemMemorySize());
#endif // BUILD_64
	cIndexedDataEvictedList.Clear();
	cIndexTreeEvictedList.Clear();

	cIndexedData.Add(OI, &iData, 4); OI++;  iData++;
	AssertSize(1, cIndexedDataEvictedList.NumElements());
	AssertLong(5LL, cIndexedDataEvictedList.GetKey(0));
	AssertInt(82, *(int*)cIndexedDataEvictedList.GetData(0));
	Pass();
#ifdef BUILD_64
	AssertLong(4936LL, cIndexedData.GetIndiciesSystemMemorySize());
#else
	AssertLong(3668LL, cIndexedData.GetIndiciesSystemMemorySize());
#endif // BUILD_64
	cIndexedDataEvictedList.Clear();
	cIndexTreeEvictedList.Clear();

	cIndexedData.Add(OI, &iData, 4); OI++;  iData++;
	AssertSize(1, cIndexedDataEvictedList.NumElements());
	AssertLong(6LL, cIndexedDataEvictedList.GetKey(0));
	AssertInt(83, *(int*)cIndexedDataEvictedList.GetData(0));
	Pass();
#ifdef BUILD_64
	AssertLong(4952LL, cIndexedData.GetIndiciesSystemMemorySize());
#else
	AssertLong(3680LL, cIndexedData.GetIndiciesSystemMemorySize());
#endif // BUILD_64
	AssertInt(3, (int)cIndexedData.NumIndicesCached());
	cIndexedDataEvictedList.Clear();
	cIndexTreeEvictedList.Clear();

	iData = 0; OI = 7LL; cIndexedData.Get(OI, (void*)&iData);
	AssertInt(84, iData);
	AssertSize(0, cIndexedDataEvictedList.NumElements());
#ifdef BUILD_64
	AssertLong(4952LL, cIndexedData.GetIndiciesSystemMemorySize());
#else
	AssertLong(3680LL, cIndexedData.GetIndiciesSystemMemorySize());
#endif // BUILD_64
	AssertInt(3, (int)cIndexedData.NumIndicesCached());
	Pass();

	iData = 0; OI = 3LL; cIndexedData.Get(OI, (void*)&iData);
	AssertInt(79, iData);
	AssertSize(2, cIndexedDataEvictedList.NumElements());
	AssertSize(2, cIndexTreeEvictedList.NumElements());
#ifdef BUILD_64
	AssertLong(4864LL, cIndexedData.GetIndiciesSystemMemorySize());
#else
	AssertLong(3620LL, cIndexedData.GetIndiciesSystemMemorySize());
#endif // BUILD_64
	Pass();
	AssertLong(8LL, cIndexedDataEvictedList.GetKey(0));
	AssertInt(85, *(int*)cIndexedDataEvictedList.GetData(0));
	AssertLong(1LL, cIndexedDataEvictedList.GetKey(1));
	AssertInt(77, *(int*)cIndexedDataEvictedList.GetData(1));
	Pass();
	AssertLong(8LL, *(int64*)cIndexTreeEvictedList.GetKey(0));
	AssertLong(-1LL, ((CIndexedDataDescriptor*)cIndexTreeEvictedList.GetData(0))->GetFilePosIndex()->mulliFilePos);
	AssertLong(1LL, *(int64*)cIndexTreeEvictedList.GetKey(1));
	AssertLong(-1LL, ((CIndexedDataDescriptor*)cIndexTreeEvictedList.GetData(1))->GetFilePosIndex()->mulliFilePos);
	Pass();
	AssertInt(2, (int)cIndexedData.NumIndicesCached());
	Pass();

	AssertTrue(cIndexedData.Flush(true));
	cController.End();
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();

	
	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 96, 8 KB, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();
	iNumCached = cIndexedData.NumIndicesCached();
	AssertInt(0, (int)iNumCached);

	cController.End();
	cIndexedData.Kill();
	cIndexedDataEvictedList.Kill();
	cIndexTreeEvictedList.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedDataNoCaching(void)
{
	CIndexedData						cIndexedData;
	OIndex								OI;
	CFileUtil							cFileUtil;
	int									iData;
	int64								iNumIndicesCached;
	int64								iNumDataCached;
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CDurableFileController				cController;
	SLogConfig							sLogConfig;
	char								szDirectory[] = "Output" _FS_ "Database5";
	CIndexTreeEvictionStrategyRandom	cEvictionStrategy;
	size								uiIndexCacheSize;

	cFileUtil.RemoveDir(szDirectory);


	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	//The nodes are bigger in a 64 bit build.
#ifdef BUILD_64
	uiIndexCacheSize = 4920;
#else
	uiIndexCacheSize = 3670;
#endif // BUILD_64
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 0, uiIndexCacheSize, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();
	OI = 1LL;
	iData = 77;
	cIndexedData.Add(OI, &iData, 4); OI++; 
	cIndexedData.Add(OI, &iData, 4); OI++;
	iData = 0; OI = 1LL; cIndexedData.Get(OI, (void*)&iData);
	AssertInt(77, iData);

	iNumIndicesCached = cIndexedData.NumIndicesCached();
	AssertInt(2, (int)iNumIndicesCached);
	iNumDataCached = cIndexedData.NumDataCached();
	AssertLong(0, iNumDataCached);

	OI = 3LL;
	cIndexedData.Add(OI, &iData, 4); OI++;
	iData = 0; OI = 1LL; cIndexedData.Get(OI, (void*)&iData);
	AssertInt(77, iData);

	iNumIndicesCached = cIndexedData.NumIndicesCached();
	AssertInt(3, (int)iNumIndicesCached);
	iNumDataCached = cIndexedData.NumDataCached();
	AssertLong(0, iNumDataCached);

	sLogConfig = gcLogger.SetSilent();
	OI = 3LL;
	cIndexedData.Add(OI, &iData, 4); OI++;
	cIndexedData.Add(OI, &iData, 4); OI++;
	cIndexedData.Add(OI, &iData, 4); OI++;
	cIndexedData.Add(OI, &iData, 4); OI++;
	cIndexedData.Add(OI, &iData, 4); OI++;
	cIndexedData.Add(OI, &iData, 4); OI++;
	gcLogger.SetConfig(&sLogConfig);
	iData = 0; OI--; cIndexedData.Get(OI, (void*)&iData);;

	AssertInt(77, iData);
	iData = 0; OI = 1LL; cIndexedData.Get(OI, (void*)&iData);
	AssertInt(77, iData);

	iNumIndicesCached = cIndexedData.NumIndicesCached();
	AssertInt(2, (int)iNumIndicesCached);
	iNumDataCached = cIndexedData.NumDataCached();
	AssertLong(0, iNumDataCached);

	AssertTrue(cIndexedData.Flush());
	AssertTrue(cController.End());


	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedDataGet(void)
{
	CIndexedData						cIndexedData;
	CFileUtil							cFileUtil;
	char								szData[256];
	lsize						uiSize;
	char								szSmellsLikeTeenSpirit[] = {"Smells Like Teen Spirit"};
	char								szSeizedPotPlants[] = {"Seized pot plants turn out to be daisies"};
	char								szCallingFromWindows[] = {"I am calling you from Windows"};
	char								szDirectory[] = "Output" _FS_ "Database6";
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CDurableFileController				cController;
	CIndexTreeEvictionStrategyRandom	cEvictionStrategy;


	cFileUtil.RemoveDir(szDirectory);


	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 8 KB, 8 KB, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();
	cIndexedData.Add(0x7634, szSmellsLikeTeenSpirit, strlen(szSmellsLikeTeenSpirit)+1);
	cIndexedData.Add(0x3589, szSeizedPotPlants, strlen(szSeizedPotPlants)+1);
	cIndexedData.Add(0x8743, szCallingFromWindows, strlen(szCallingFromWindows)+1);
	
	AssertLong(3, cIndexedData.NumElements());

	AssertTrue(cIndexedData.Flush(true));
	cController.End();
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 8 KB, 8 KB, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();
	AssertLong(3, cIndexedData.NumElements());

	AssertTrue(cIndexedData.Get(0x7634, &uiSize, szData, 256));
	AssertLSize(strlen(szSmellsLikeTeenSpirit)+1, uiSize);
	AssertString(szSmellsLikeTeenSpirit, szData);

	AssertTrue(cIndexedData.Get(0x3589, &uiSize, szData, 256));
	AssertLSize(strlen(szSeizedPotPlants)+1, uiSize);
	AssertString(szSeizedPotPlants, szData);
	
	AssertTrue(cIndexedData.Get(0x8743, &uiSize, szData, 256));
	AssertLSize(strlen(szCallingFromWindows)+1, uiSize);
	AssertString(szCallingFromWindows, szData);


	cController.End();
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedDataSubDirectories(void)
{
	CIndexedData						cIndexedData;
	CFileUtil							cFileUtil;
	char								szGroot[] = { "I am Groot" };
	char								szDirectory[] = "Output" _FS_ "Database7" _FS_ "Write";
	char								szRewriteDirectory[] = "Output" _FS_ "Database7" _FS_ "Rewrite";
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CDurableFileController				cController;
	CIndexedDataAccess					cAccess;
	CIndexTreeEvictionStrategyRandom	cEvictionStrategy;

	cFileUtil.RemoveDir("Output" _FS_ "Database7");


	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, szRewriteDirectory);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create("Forest of Solitude", 8 KB, 8 KB, IWT_No, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cAccess.Init(&cIndexedData);
	cController.Begin();
	cAccess.PutLongString(0x7fffffffffffffffLL, szGroot);

	AssertLong(1, cIndexedData.NumElements());

	cIndexedData.Flush();
	cAccess.Kill();
	cController.End();
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cFileUtil.RemoveDir("Output" _FS_ "Database7");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedDataRemove(EIndexWriteThrough eWriteThrough)
{
	CIndexedData						cIndexedData;
	CFileUtil							cFileUtil;
	char								szData[256];
	lsize						uiSize;
	char								szSmellsLikeTeenSpirit[] = { "Smells Like Teen Spirit" };
	char								szSeizedPotPlants[] = { "Seized pot plants turn out to be daisies" };
	char								szCallingFromWindows[] = { "I am calling you from Windows" };
	char								szDirectory[] = "Output" _FS_ "Database8";
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CDurableFileController				cController;
	CIndexTreeEvictionStrategyRandom	cEvictionStrategy;

	cFileUtil.RemoveDir(szDirectory);


	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 8 KB, 8 KB, eWriteThrough, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();
	cIndexedData.Add(0x7634, szSmellsLikeTeenSpirit, strlen(szSmellsLikeTeenSpirit) + 1);
	cIndexedData.Add(0x3589, szSeizedPotPlants, strlen(szSeizedPotPlants) + 1);
	cIndexedData.Add(0x8743, szCallingFromWindows, strlen(szCallingFromWindows) + 1);
	AssertLong(3, cIndexedData.NumElements());
	AssertTrue(cIndexedData.Contains(0x7634));
	AssertTrue(cIndexedData.Contains(0x3589));
	AssertTrue(cIndexedData.Contains(0x8743));

	AssertTrue(cIndexedData.Flush(true));
	cController.End();
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 8 KB, 8 KB, eWriteThrough, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();
	AssertTrue(cIndexedData.Contains(0x7634));
	AssertTrue(cIndexedData.Contains(0x3589));
	AssertTrue(cIndexedData.Contains(0x8743));

	cController.End();
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 8 KB, 8 KB, eWriteThrough, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();
	AssertTrue(cIndexedData.Remove(0x7634));
	AssertLong(2, cIndexedData.NumElements());
	AssertFalse(cIndexedData.Contains(0x7634));
	AssertTrue(cIndexedData.Contains(0x3589));
	AssertTrue(cIndexedData.Contains(0x8743));
	AssertFalse(cIndexedData.Get(0x7634, &uiSize, szData, 256));

	AssertTrue(cIndexedData.Flush(true));
	cController.End();
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 8 KB, 8 KB, eWriteThrough, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();
	AssertTrue(cIndexedData.Remove(0x3589));
	AssertTrue(cIndexedData.Remove(0x8743));
	AssertFalse(cIndexedData.Contains(0x7634));
	AssertFalse(cIndexedData.Contains(0x3589));
	AssertFalse(cIndexedData.Contains(0x8743));
	AssertLong(0, cIndexedData.NumElements());
	AssertFalse(cIndexedData.Remove(0x7634));
	AssertFalse(cIndexedData.Remove(0x3589));
	AssertFalse(cIndexedData.Remove(0x8743));

	AssertTrue(cIndexedData.Flush(true));
	AssertFalse(cIndexedData.Remove(0x7634));
	AssertFalse(cIndexedData.Remove(0x3589));
	AssertFalse(cIndexedData.Remove(0x8743));


	cController.End();
	cIndexedData.Kill();
	cEvictionStrategy.Kill();
	cController.Kill();


	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedDataRemove2(EIndexWriteThrough eWriteThrough)
{
	CIndexedData						cIndexedData;
	CFileUtil							cFileUtil;
	CTestNamedIndexedDataObject			cObject1;
	CTestNamedIndexedDataObject			cObject2;
	CTestNamedIndexedDataObject			cObject3;
	CTestNamedIndexedDataObject			cResult;
	char								szDirectory[] = "Output" _FS_ "Database8a";
	CLifeInit<CIndexedDataConfig>		cIndexConfig;
	CDurableFileController				cController;
	bool								bResult;
	CIndexTreeEvictionStrategyRandom	cEvictionStrategy;

	cFileUtil.RemoveDir(szDirectory);


	cEvictionStrategy.Init(15);
	cController.Init(szDirectory, NULL);
	cController.SetSync(DS_No);
	cIndexConfig = CValueIndexedDataConfig::Create(NULL, 8 KB, 8 KB, eWriteThrough, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy));
	cController.Begin();
	cIndexedData.Init(&cController, cIndexConfig);
	cController.End();

	cController.Begin();
	cObject1.Init("Rutherford", 1871, 1937);
	cIndexedData.Add(0x0102LL, &cObject1, cObject1.Size());
	cObject2.Init("Bohr", 1885, 1962);
	cIndexedData.Add(0x0203LL, &cObject2, cObject2.Size());
	cObject3.Init("Planck", 1858, 1947);
	cIndexedData.Add(0x0304LL, &cObject3, cObject3.Size());
	AssertLong(3, cIndexedData.NumIndices());
	AssertTrue(cIndexedData.Contains(0x0102LL));
	AssertTrue(cIndexedData.Contains(0x0203LL));
	AssertTrue(cIndexedData.Contains(0x0304LL));

	cResult.Init();
	bResult = cIndexedData.Get(0x0102LL, &cResult);
	bResult = cIndexedData.Remove(0x0102LL);
	AssertTrue(bResult);
	AssertFalse(cIndexedData.Contains(0x0102LL));
	AssertTrue(cIndexedData.Contains(0x0203LL));
	AssertTrue(cIndexedData.Contains(0x0304LL));
	if (eWriteThrough == IWT_No) cIndexedData.Flush();
	cController.End();


	cIndexedData.Kill();
	cController.Kill();
	cEvictionStrategy.Kill();


	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedData(void)
{
	TypeConverterInit();
	MemoryInit();
	DataMemoryInit();
	BeginTests();

	TestIndexedDataSimple(IWT_Yes);
	TestIndexedDataSimple(IWT_No);
	TestIndexedDataIteration(IWT_Yes);
	TestIndexedDataIteration(IWT_No);
	TestIndexedDataFlushClearCache();
	TestIndexedDataExplicitKeyEvictionOneKey();
	TestIndexedDataEvictKey();
	TestIndexedDataExplicitKeyEvictionAllKeys();
	TestIndexedDataExplicitKeyEvictionDataChanged();
	TestIndexedDataEvictAndFlush();
	TestIndexedDataFlushAndFlush();
	TestIndexedDataIndexEviction();
	TestIndexedDataIndexedAdd();
	TestIndexedDataDescriptorCaching();
	TestIndexedDataNoCaching();
	TestIndexedDataGet();
	TestIndexedDataLargeData();
	TestIndexedDataSubDirectories();
	TestIndexedDataRemove(IWT_Yes);
	TestIndexedDataRemove(IWT_No);
	TestIndexedDataRemove2(IWT_Yes);
	TestIndexedDataRemove2(IWT_No);

	TestStatistics();
	DataMemoryKill();
	MemoryKill();
	TypeConverterKill();
}


