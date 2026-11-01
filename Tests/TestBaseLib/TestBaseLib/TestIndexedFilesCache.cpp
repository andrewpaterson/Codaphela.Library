#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/IndexTreeHelper.h"
#include "BaseLib/IndexedMap.h"
#include "BaseLib/IndexedDataEvictedList.h"
#include "TestLib/Assert.h"


//320 bytes in a 32 bit build.  The objects the tests expect to be evicted depend on the descriptor size, so the cache
//grows with it.
#define TEST_INDEXED_FILES_CACHE_SIZE	(248 + 3 * sizeof(SIndexedCacheDescriptor))


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedFilesEvictingFlush(EIndexWriteThrough eWriteThrough, bool bClearCache)
{
	CDurableFileController					cController;
	CIndexTreeHelper						cHelper;
	CIndexedMap								cDescriptors;
	char									szMoreover[] = "Although moreover mistaken kindness me feelings do be marianne.  Cordial village and settled she ability law herself.";
	char									szMudpuppy[] = "Although mudpuppy mistaken kindness me feelings do be marianne.  Cordial village and settled she ability law herself.";
	char									szForsaken[] = "Although mudpuppy forsaken kindness me feelings do be marianne.  Cordial village and settled she ability law herself.";
	char									sz[200];
	size										iLenMoreover;
	OIndex									oi;
	CIndexedDataDescriptor					cDescriptor;
	lsize							uiSize;

	cHelper.Init("Output" _FS_ "TestEvicting1", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cDescriptors.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX", 1024, eWriteThrough);

	AssertTrue(cController.Begin());
	iLenMoreover = strlen(szMoreover) + 1;
	oi = 45;
	AssertLSize(0, cDescriptors.NumDataCached());

	AssertTrue(cDescriptors.Add(oi, szMoreover, iLenMoreover));
	AssertTrue(cDescriptors.TestGetDescriptor(oi, NULL));
	AssertLSize(1, cDescriptors.NumDataCached());

	AssertTrue(cDescriptors.Get(oi, &uiSize, sz, 200));
	AssertLSize(iLenMoreover, uiSize);
	AssertString(szMoreover, sz);
	AssertLSize(1, cDescriptors.NumDataCached());
	AssertTrue(cDescriptors.Flush(bClearCache));
	AssertLSize(bClearCache ? 0 : 1, cDescriptors.NumDataCached());

	AssertTrue(cController.End());
	AssertTrue(cDescriptors.CanKill());
	cDescriptors.Kill();


	cDescriptors.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX", 1024, eWriteThrough);
	AssertInt(1, (int)cDescriptors.NumElements());

	AssertTrue(cController.Begin());

	AssertTrue(cDescriptors.TestGetDescriptor(oi, &cDescriptor));
	AssertFalse(cDescriptor.IsCached());
	AssertTrue(cDescriptor.HasFile());
	AssertTrue(cDescriptors.Get(oi, &uiSize, sz, 200));
	AssertLSize(iLenMoreover, uiSize);
	AssertString(szMoreover, sz);

	AssertTrue(cController.End());

	AssertTrue(cDescriptors.CanKill());	
	cDescriptors.Kill();


	cDescriptors.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX", 1024, eWriteThrough);
	AssertInt(1, (int)cDescriptors.NumElements());

	AssertTrue(cController.Begin());

	AssertTrue(cDescriptors.Set(oi, szMudpuppy));
	AssertTrue(cDescriptors.TestGetDescriptor(oi, &cDescriptor));
	AssertTrue(cDescriptor.IsCached());
	AssertLSize(1, cDescriptors.NumDataCached());
	AssertTrue(cDescriptors.Get(oi, &uiSize, sz, 200));
	AssertString(szMudpuppy, sz);
	AssertTrue(cDescriptors.TestGetDescriptor(oi, &cDescriptor));
	AssertTrue(cDescriptor.IsCached());
	AssertTrue(cDescriptor.HasFile());
	AssertLSize(1, cDescriptors.NumDataCached());
	AssertTrue(cDescriptors.Flush(bClearCache));
	AssertLSize(bClearCache ? 0 : 1, cDescriptors.NumDataCached());
	AssertTrue(cDescriptors.Get(oi, &uiSize, sz, 200));
	AssertString(szMudpuppy, sz);

	AssertTrue(cController.End());

	AssertTrue(cDescriptors.CanKill());	
	cDescriptors.Kill();


	cDescriptors.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX", 1024, eWriteThrough);
	AssertInt(1, (int)cDescriptors.NumElements());

	AssertTrue(cController.Begin());

	AssertTrue(cDescriptors.Get(oi, &uiSize, sz, 200));
	AssertTrue(cDescriptors.TestGetDescriptor(oi, &cDescriptor));
	AssertTrue(cDescriptor.IsCached());
	AssertTrue(cDescriptor.HasFile());
	AssertLSize(1, cDescriptors.NumDataCached());
	AssertTrue(cDescriptors.Set(oi, szForsaken));
	AssertLSize(1, cDescriptors.NumDataCached());
	AssertTrue(cDescriptors.Flush(bClearCache));
	AssertLSize(bClearCache ? 0 : 1, cDescriptors.NumDataCached());
	AssertTrue(cDescriptors.Get(oi, &uiSize, sz, 200));
	AssertString(szForsaken, sz);

	AssertTrue(cController.End());

	AssertTrue(cDescriptors.CanKill());	
	cDescriptors.Kill();

	cController.Kill();
	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedFilesEvictingEviction(EIndexWriteThrough eWriteThrough)
{
	CDurableFileController					cController;
	CIndexTreeHelper						cHelper;
	CIndexedMap								cDescriptors;
	CIndexedDataEvictedList					cKeyDataEvictedList;
	char									szMoreover[] = "Although moreover mistaken kindness me feelings do be marianne.  Cordial village and settled she ability law herself.";
	char									szIndulged[] = "Concerns greatest margaret him absolute entrance nay.  Door neat week do find past he.Be no surprise he honoured indulged.";
	char									szSeparate[] = "To they four in love.  Settling you has separate supplied bed.  Concluded resembled suspected his resources curiosity joy.";
	char									sz[200];
	size										iLenMoreover;
	size										iLenIndulged;
	size										iLenSeparate;
	OIndex									oiMoreover;
	OIndex									oiIndulged;
	OIndex									oiSeparate;
	CIndexedDataDescriptor					cDescriptor;
	lsize							uiSize;

	cHelper.Init("Output" _FS_ "TestEvicting2", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cKeyDataEvictedList.Init();
	cDescriptors.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX", TEST_INDEXED_FILES_CACHE_SIZE, eWriteThrough, &cKeyDataEvictedList);

	AssertTrue(cController.Begin());
	iLenMoreover = strlen(szMoreover) + 1;
	iLenIndulged = strlen(szIndulged) + 1;
	iLenSeparate = strlen(szSeparate) + 1;
	oiMoreover = 45;
	oiIndulged = 346;
	oiSeparate = 12;
	AssertLSize(0, cDescriptors.NumDataCached());

	AssertTrue(cDescriptors.Add(oiMoreover, szMoreover, iLenMoreover));
	AssertTrue(cDescriptors.TestGetDescriptor(oiMoreover, NULL));
	AssertLSize(1, cDescriptors.NumDataCached());

	AssertTrue(cDescriptors.Get(oiMoreover, &uiSize, sz, 200));
	AssertLSize(iLenMoreover, uiSize);
	AssertString(szMoreover, sz);
	AssertLSize(1, cDescriptors.NumDataCached());
	AssertSize(0, cKeyDataEvictedList.NumElements());

	AssertTrue(cDescriptors.Add(oiIndulged, szIndulged, iLenIndulged));
	AssertTrue(cDescriptors.TestGetDescriptor(oiIndulged, NULL));
	AssertLSize(2, cDescriptors.NumDataCached());
	AssertSize(0, cKeyDataEvictedList.NumElements());

	AssertTrue(cDescriptors.Add(oiSeparate, szSeparate, iLenSeparate));
	AssertTrue(cDescriptors.TestGetDescriptor(oiSeparate, NULL));
	AssertLSize(1, cDescriptors.NumDataCached());
	AssertSize(2, cKeyDataEvictedList.NumElements());	//Two were evicted because eviction wrapped back to the start of the cache
														//and the first element size szMoreover is smaller than szSeparate size
														//which means that the second element szIndulged needed to be evicted too
														//to ensure there was enough contiguous space in the cache.
	AssertString(szMoreover, (char*)cKeyDataEvictedList.GetData(0));
	AssertString(szIndulged, (char*)cKeyDataEvictedList.GetData(1));

	AssertTrue(cDescriptors.TestGetDescriptor(oiMoreover, NULL));
	AssertTrue(cDescriptors.TestGetDescriptor(oiIndulged, NULL));
	AssertTrue(cDescriptors.TestGetDescriptor(oiSeparate, NULL));
	AssertLSize(0, cDescriptors.TestGetCachedObjectSize(oiMoreover));
	AssertLSize(0, cDescriptors.TestGetCachedObjectSize(oiIndulged));
	AssertLSize(iLenSeparate + sizeof(SIndexedCacheDescriptor), cDescriptors.TestGetCachedObjectSize(oiSeparate));
	cKeyDataEvictedList.Clear();

	AssertTrue(cDescriptors.Flush(true));

	AssertTrue(cController.End());
	AssertTrue(cDescriptors.CanKill());	
	cDescriptors.Kill();
	cKeyDataEvictedList.Kill();

	cKeyDataEvictedList.Init();
	cDescriptors.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX", TEST_INDEXED_FILES_CACHE_SIZE, eWriteThrough, &cKeyDataEvictedList);
	AssertInt(3, (int)cDescriptors.NumElements());

	AssertTrue(cController.Begin());

	AssertTrue(cDescriptors.TestGetDescriptor(oiMoreover, &cDescriptor));
	AssertFalse(cDescriptor.IsCached());
	AssertTrue(cDescriptor.HasFile());
	AssertLSize(0, cDescriptors.TestGetCachedObjectSize(oiMoreover));

	AssertTrue(cDescriptors.TestGetDescriptor(oiIndulged, &cDescriptor));
	AssertFalse(cDescriptor.IsCached());
	AssertTrue(cDescriptor.HasFile());
	AssertLSize(0, cDescriptors.TestGetCachedObjectSize(oiIndulged));
	AssertTrue(cDescriptors.TestGetDescriptor(oiSeparate, &cDescriptor));
	AssertFalse(cDescriptor.IsCached());
	AssertTrue(cDescriptor.HasFile());
	AssertLSize(0, cDescriptors.TestGetCachedObjectSize(oiSeparate));

	AssertLSize(0, cDescriptors.NumDataCached());
	AssertSize(0, cKeyDataEvictedList.NumElements());

	AssertTrue(cDescriptors.Get(oiIndulged, &uiSize, sz, 200));
	AssertLSize(iLenIndulged, uiSize);
	AssertString(szIndulged, sz);
	AssertLSize(1, cDescriptors.NumDataCached());
	AssertSize(0, cKeyDataEvictedList.NumElements());
	AssertTrue(cDescriptors.Get(oiIndulged, &uiSize, sz, 200));
	AssertFalse(cDescriptors.IsDirty(oiMoreover));
	AssertFalse(cDescriptors.IsDirty(oiSeparate));
	AssertFalse(cDescriptors.IsDirty(oiMoreover));

	AssertTrue(cDescriptors.Get(oiSeparate, &uiSize, sz, 200));
	AssertLSize(iLenSeparate, uiSize);
	AssertString(szSeparate, sz);
	AssertLSize(2, cDescriptors.NumDataCached());
	AssertSize(0, cKeyDataEvictedList.NumElements());
	AssertFalse(cDescriptors.IsDirty(oiMoreover));
	AssertFalse(cDescriptors.IsDirty(oiSeparate));
	AssertFalse(cDescriptors.IsDirty(oiMoreover));
	cKeyDataEvictedList.Clear();

	AssertTrue(cDescriptors.Get(oiMoreover, &uiSize, sz, 200));
	AssertLSize(iLenMoreover, uiSize);
	AssertString(szMoreover, sz);
	AssertLSize(2, cDescriptors.NumDataCached());
	AssertSize(1, cKeyDataEvictedList.NumElements());
	AssertFalse(cDescriptors.IsDirty(oiMoreover));
	AssertFalse(cDescriptors.IsDirty(oiSeparate));
	AssertFalse(cDescriptors.IsDirty(oiMoreover));
	AssertString(szIndulged, (char*)cKeyDataEvictedList.GetData(0));
	cKeyDataEvictedList.Clear();

	AssertTrue(cController.End());
	AssertTrue(cDescriptors.CanKill());	
	cDescriptors.Kill();
	cKeyDataEvictedList.Kill();
	cController.Kill();

	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);
	cKeyDataEvictedList.Init();
	cDescriptors.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX", TEST_INDEXED_FILES_CACHE_SIZE, eWriteThrough, &cKeyDataEvictedList);
	AssertTrue(cController.Begin());
	AssertInt(3, (int)cDescriptors.NumElements());

	AssertTrue(cDescriptors.TestGetDescriptor(oiMoreover, &cDescriptor));
	AssertFalse(cDescriptor.IsCached());
	AssertTrue(cDescriptor.HasFile());
	AssertLSize(0, cDescriptors.TestGetCachedObjectSize(oiMoreover));

	AssertTrue(cDescriptors.TestGetDescriptor(oiIndulged, &cDescriptor));
	AssertFalse(cDescriptor.IsCached());
	AssertTrue(cDescriptor.HasFile());
	AssertLSize(0, cDescriptors.TestGetCachedObjectSize(oiIndulged));
	AssertTrue(cDescriptors.TestGetDescriptor(oiSeparate, &cDescriptor));
	AssertFalse(cDescriptor.IsCached());
	AssertTrue(cDescriptor.HasFile());
	AssertLSize(0, cDescriptors.TestGetCachedObjectSize(oiSeparate));

	AssertLSize(0, cDescriptors.NumDataCached());
	AssertSize(0, cKeyDataEvictedList.NumElements());

	AssertTrue(cDescriptors.Get(oiIndulged, &uiSize, sz, 200));
	AssertLSize(iLenIndulged, uiSize);
	AssertString(szIndulged, sz);
	AssertLSize(1, cDescriptors.NumDataCached());
	AssertSize(0, cKeyDataEvictedList.NumElements());
	AssertTrue(cDescriptors.Get(oiIndulged, &uiSize, sz, 200));
	AssertString(szIndulged, sz);
	AssertLSize(1, cDescriptors.NumDataCached());
	AssertSize(0, cKeyDataEvictedList.NumElements());

	AssertTrue(cDescriptors.Get(oiSeparate, &uiSize, sz, 200));
	AssertLSize(iLenSeparate, uiSize);
	AssertString(szSeparate, sz);
	AssertLSize(2, cDescriptors.NumDataCached());
	AssertSize(0, cKeyDataEvictedList.NumElements());

	AssertTrue(cDescriptors.Get(oiMoreover, &uiSize, sz, 200));
	AssertLSize(iLenMoreover, uiSize);
	AssertString(szMoreover, sz);
	AssertLSize(2, cDescriptors.NumDataCached());
	AssertSize(1, cKeyDataEvictedList.NumElements());
	AssertFalse(cDescriptors.IsDirty(oiMoreover));
	AssertFalse(cDescriptors.IsDirty(oiSeparate));
	AssertFalse(cDescriptors.IsDirty(oiIndulged));
	AssertTrue(cDescriptors.TestGetDescriptor(oiMoreover, &cDescriptor));
	AssertNotNull(cDescriptor.GetCache());
	AssertString(szMoreover, (char*)cDescriptor.GetCache());
	AssertTrue(cDescriptors.TestGetDescriptor(oiSeparate, &cDescriptor));
	AssertNotNull(cDescriptor.GetCache());
	AssertString(szSeparate, (char*)cDescriptor.GetCache());
	AssertTrue(cDescriptors.TestGetDescriptor(oiIndulged, &cDescriptor));
	AssertNull(cDescriptor.GetCache());
	AssertString(szIndulged, (char*)cKeyDataEvictedList.GetData(0));
	cKeyDataEvictedList.Clear();

	AssertTrue(cDescriptors.Get(oiIndulged, &uiSize, sz, 200));
	AssertLSize(iLenIndulged, uiSize);
	AssertString(szIndulged, sz);
	AssertLSize(2, cDescriptors.NumDataCached());
	AssertSize(1, cKeyDataEvictedList.NumElements());
	AssertString(szSeparate, (char*)cKeyDataEvictedList.GetData(0));
	cKeyDataEvictedList.Clear();

	AssertTrue(cDescriptors.Get(oiIndulged, &uiSize, sz, 200));
	AssertLSize(2, cDescriptors.NumDataCached());
	AssertSize(0, cKeyDataEvictedList.NumElements());

	AssertTrue(cDescriptors.Get(oiSeparate, &uiSize, sz, 200));
	AssertLSize(iLenSeparate, uiSize);
	AssertString(szSeparate, sz);
	AssertLSize(1, cDescriptors.NumDataCached());
	AssertSize(2, cKeyDataEvictedList.NumElements());
	AssertString(szMoreover, (char*)cKeyDataEvictedList.GetData(0));
	AssertString(szIndulged, (char*)cKeyDataEvictedList.GetData(1));
	cKeyDataEvictedList.Clear();

	AssertTrue(cDescriptors.Get(oiSeparate, &uiSize, sz, 200));
	AssertLSize(1, cDescriptors.NumDataCached());
	AssertSize(0, cKeyDataEvictedList.NumElements());

	AssertTrue(cController.End());
	AssertTrue(cDescriptors.CanKill());	
	cDescriptors.Kill();
	cKeyDataEvictedList.Kill();
	cController.Kill();
	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedFilesEvictingSetDataNoCacheNoFile(void)
{
	CDurableFileController					cController;
	CIndexTreeHelper						cHelper;
	CIndexedMap								cDescriptors;
	char									sz[200];
	char									sz6[] = "6Six6\0";
	EIndexWriteThrough						eWriteThrough;
	CIndexedDataDescriptor					cResult;

#ifdef BUILD_64
	AssertSize(40, sizeof(CIndexedDataDescriptor));
#else
	AssertSize(32, sizeof(CIndexedDataDescriptor));
#endif // BUILD_64
	eWriteThrough = IWT_No;
	cHelper.Init("Output" _FS_ "TestEvicting3", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cDescriptors.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX", TEST_INDEXED_FILES_CACHE_SIZE, eWriteThrough);

	AssertTrue(cController.Begin());
	AssertLSize(0, cDescriptors.NumDataCached());
	Pass();

	AssertTrue(cDescriptors.Put(1LL, sz6, 6));
	AssertTrue(cDescriptors.TestGetDescriptor(1LL, &cResult));
	AssertLSize(6, cResult.GetCacheDataSize());
	AssertLSize(0, cResult.GetFileDataSize());
	AssertNotNull(cResult.GetCache());
	AssertFalse(cResult.HasFile());
	AssertLSize(1, cDescriptors.NumDataCached());
	Pass();

	AssertSize(0, cDescriptors.NumFiles());
	AssertTrue(cDescriptors.Flush(true));
	AssertTrue(cDescriptors.TestGetDescriptor(1LL, &cResult));
	AssertLSize(0, cResult.GetCacheDataSize());
	AssertLSize(6, cResult.GetFileDataSize());
	AssertLSize(0, cDescriptors.NumDataCached());
	Pass();

	AssertTrue(cController.End());
	AssertTrue(cDescriptors.CanKill());	
	cDescriptors.Kill();
	Pass(); cController.Kill();

	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);
	cDescriptors.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX", TEST_INDEXED_FILES_CACHE_SIZE, eWriteThrough);
	AssertTrue(cController.Begin());
	AssertInt(1, (int)cDescriptors.NumElements());
	Pass();

	AssertTrue(cDescriptors.Get(1LL, sz));
	AssertString(sz6, sz);
	Pass();

	AssertTrue(cController.End());
	AssertTrue(cDescriptors.CanKill());	
	cDescriptors.Kill();
	Pass();

	cController.Kill();
	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedFilesEvictingSetDataCacheSameSizeNoFile(void)
{
	CDurableFileController		cController;
	CIndexTreeHelper			cHelper;
	CIndexedMap					cDescriptors;
	char						sz[200];
	char						sz6a[] = "6Six6\0";
	char						sz6b[] = "xisix\0";
	EIndexWriteThrough			eWriteThrough;

	eWriteThrough = IWT_No;
	cHelper.Init("Output" _FS_ "TestEvicting4", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cDescriptors.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX", TEST_INDEXED_FILES_CACHE_SIZE, eWriteThrough);

	AssertTrue(cController.Begin());
	AssertLSize(0, cDescriptors.NumDataCached());

	AssertTrue(cDescriptors.Put(1LL, sz6a, 6));
	AssertTrue(cDescriptors.TestGetDescriptor(1LL, NULL));
	AssertLSize(1, cDescriptors.NumDataCached());

	AssertTrue(cDescriptors.Put(1LL, sz6b, 6));
	AssertTrue(cDescriptors.TestGetDescriptor(1LL, NULL));
	AssertLSize(1, cDescriptors.NumDataCached());

	AssertSize(0, cDescriptors.NumFiles());
	AssertTrue(cDescriptors.Flush(true));

	AssertTrue(cController.End());
	AssertTrue(cDescriptors.CanKill());	
	cDescriptors.Kill();
	cController.Kill();

	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);
	cDescriptors.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX", TEST_INDEXED_FILES_CACHE_SIZE, eWriteThrough);
	AssertTrue(cController.Begin());
	AssertInt(1, (int)cDescriptors.NumElements());

	AssertTrue(cDescriptors.Get(1LL, sz));
	AssertString(sz6b, sz);

	AssertTrue(cController.End());
	AssertTrue(cDescriptors.CanKill());	
	cDescriptors.Kill();
	cController.Kill();
	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedFilesEvictingSetDataCacheDiffSizeNoFile(void)
{
	CDurableFileController		cController;
	CIndexTreeHelper			cHelper;
	CIndexedMap					cDescriptors;
	char						sz[200];
	char						sz6[] = "6Six6\0";
	char						sz5[] = "5ive\0";
	EIndexWriteThrough			eWriteThrough;

	eWriteThrough = IWT_No;
	cHelper.Init("Output" _FS_ "TestEvicting5", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cDescriptors.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX", TEST_INDEXED_FILES_CACHE_SIZE, eWriteThrough);

	AssertTrue(cController.Begin());
	AssertLSize(0, cDescriptors.NumDataCached());

	AssertTrue(cDescriptors.Put(1LL, sz6, 6));
	AssertTrue(cDescriptors.TestGetDescriptor(1LL, NULL));
	AssertLSize(1, cDescriptors.NumDataCached());

	AssertTrue(cDescriptors.Put(1LL, sz5, 5));
	AssertTrue(cDescriptors.TestGetDescriptor(1LL, NULL));
	AssertLSize(1, cDescriptors.NumDataCached());

	AssertSize(0, cDescriptors.NumFiles());
	AssertTrue(cDescriptors.Flush(true));

	AssertTrue(cController.End());
	AssertTrue(cDescriptors.CanKill());	
	cDescriptors.Kill();
	cController.Kill();

	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);
	cDescriptors.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX", TEST_INDEXED_FILES_CACHE_SIZE, eWriteThrough);
	AssertTrue(cController.Begin());
	AssertInt(1, (int)cDescriptors.NumElements());

	AssertTrue(cDescriptors.Get(1LL, sz));
	AssertString(sz5, sz);

	AssertTrue(cController.End());
	AssertTrue(cDescriptors.CanKill());	
	cDescriptors.Kill();
	cController.Kill();
	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedFilesEvictingSetDataNoCacheFileSameSize(void)
{
	CDurableFileController		cController;
	CIndexTreeHelper			cHelper;
	CIndexedMap					cDescriptors;
	char						sz[200];
	char						sz6a[] = "6Six6\0";
	char						sz6b[] = "xisix\0";
	EIndexWriteThrough			eWriteThrough;

	eWriteThrough = IWT_No;
	cHelper.Init("Output" _FS_ "TestEvicting6", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cDescriptors.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX", TEST_INDEXED_FILES_CACHE_SIZE, eWriteThrough);

	AssertTrue(cController.Begin());
	AssertLSize(0, cDescriptors.NumDataCached());

	AssertTrue(cDescriptors.Put(1LL, sz6a, 6));
	AssertTrue(cDescriptors.TestGetDescriptor(1LL, NULL));
	AssertLSize(1, cDescriptors.NumDataCached());

	AssertSize(0, cDescriptors.NumFiles());
	AssertTrue(cDescriptors.Flush(true));

	AssertTrue(cController.End());
	AssertTrue(cDescriptors.CanKill());	
	cDescriptors.Kill();
	cController.Kill();

	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);
	cDescriptors.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX", TEST_INDEXED_FILES_CACHE_SIZE, eWriteThrough);
	AssertTrue(cController.Begin());
	AssertInt(1, (int)cDescriptors.NumElements());

	AssertTrue(cDescriptors.Put(1LL, sz6b, 6));
	AssertTrue(cDescriptors.TestGetDescriptor(1LL, NULL));
	AssertLSize(1, cDescriptors.NumDataCached());

	AssertTrue(cDescriptors.Flush(true));
	AssertTrue(cController.End());
	AssertTrue(cDescriptors.CanKill());	
	cDescriptors.Kill();
	cController.Kill();

	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);
	cDescriptors.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX", TEST_INDEXED_FILES_CACHE_SIZE, eWriteThrough);
	AssertTrue(cController.Begin());
	AssertInt(1, (int)cDescriptors.NumElements());

	AssertTrue(cDescriptors.Get(1LL, sz));
	AssertString(sz6b, sz);

	AssertTrue(cController.End());
	AssertTrue(cDescriptors.CanKill());	
	cDescriptors.Kill();
	cController.Kill();
	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedFilesEvictingSetDataNoCacheFileDiffSize(void)
{
	CDurableFileController		cController;
	CIndexTreeHelper			cHelper;
	CIndexedMap					cDescriptors;
	char						sz[200];
	char						sz6[] = "6Six6\0";
	char						sz5[] = "5ive\0";
	EIndexWriteThrough			eWriteThrough;

	eWriteThrough = IWT_No;
	cHelper.Init("Output" _FS_ "TestEvicting7", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cDescriptors.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX", TEST_INDEXED_FILES_CACHE_SIZE, eWriteThrough);

	AssertTrue(cController.Begin());
	AssertLSize(0, cDescriptors.NumDataCached());

	AssertTrue(cDescriptors.Put(1LL, sz6, 6));
	AssertTrue(cDescriptors.TestGetDescriptor(1LL, NULL));
	AssertLSize(1, cDescriptors.NumDataCached());

	AssertSize(0, cDescriptors.NumFiles());
	AssertTrue(cDescriptors.Flush(true));

	AssertTrue(cController.End());
	AssertTrue(cDescriptors.CanKill());	
	cDescriptors.Kill();
	cController.Kill();

	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);
	cDescriptors.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX", TEST_INDEXED_FILES_CACHE_SIZE, eWriteThrough);
	AssertTrue(cController.Begin());
	AssertInt(1, (int)cDescriptors.NumElements());

	AssertTrue(cDescriptors.Put(1LL, sz5, 5));
	AssertTrue(cDescriptors.TestGetDescriptor(1LL, NULL));
	AssertLSize(1, cDescriptors.NumDataCached());

	AssertTrue(cDescriptors.Flush(true));
	AssertTrue(cController.End());
	AssertTrue(cDescriptors.CanKill());	
	cDescriptors.Kill();
	cController.Kill();

	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);
	cDescriptors.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX", TEST_INDEXED_FILES_CACHE_SIZE, eWriteThrough);
	AssertTrue(cController.Begin());
	AssertInt(1, (int)cDescriptors.NumElements());

	AssertTrue(cDescriptors.Get(1LL, sz));
	AssertString(sz5, sz);

	AssertTrue(cController.End());
	AssertTrue(cDescriptors.CanKill());	
	cDescriptors.Kill();
	cController.Kill();
	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedFilesEvictingSetDataCacheSameSizeFile(void)
{
	CDurableFileController					cController;
	CIndexTreeHelper						cHelper;
	CIndexedMap								cDescriptors;
	char									sz[200];
	char									sz6a[] = "6Six6\0";
	char									sz6b[] = "xisix\0";
	char									sz5[] = "5ive\0";
	EIndexWriteThrough						eWriteThrough;

	eWriteThrough = IWT_No;
	cHelper.Init("Output" _FS_ "TestEvicting8", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cDescriptors.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX", TEST_INDEXED_FILES_CACHE_SIZE, eWriteThrough);

	AssertTrue(cController.Begin());
	AssertLSize(0, cDescriptors.NumDataCached());

	AssertTrue(cDescriptors.Put(1LL, sz6a, 6));
	AssertTrue(cDescriptors.TestGetDescriptor(1LL, NULL));
	AssertLSize(1, cDescriptors.NumDataCached());

	AssertSize(0, cDescriptors.NumFiles());
	AssertTrue(cDescriptors.Flush(true));

	AssertTrue(cController.End());
	AssertTrue(cDescriptors.CanKill());	
	cDescriptors.Kill();
	cController.Kill();

	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);
	cDescriptors.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX", TEST_INDEXED_FILES_CACHE_SIZE, eWriteThrough);
	AssertTrue(cController.Begin());
	AssertInt(1, (int)cDescriptors.NumElements());

	AssertTrue(cDescriptors.Put(1LL, sz6b, 6));
	AssertTrue(cDescriptors.TestGetDescriptor(1LL, NULL));
	AssertLSize(1, cDescriptors.NumDataCached());

	AssertTrue(cDescriptors.Flush(true));
	AssertTrue(cController.End());
	AssertTrue(cDescriptors.CanKill());	
	cDescriptors.Kill();
	cController.Kill();

	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);
	cDescriptors.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX", TEST_INDEXED_FILES_CACHE_SIZE, eWriteThrough);
	AssertTrue(cController.Begin());
	AssertInt(1, (int)cDescriptors.NumElements());

	AssertTrue(cDescriptors.Put(1LL, sz5, 5));
	AssertTrue(cDescriptors.TestGetDescriptor(1LL, NULL));
	AssertLSize(1, cDescriptors.NumDataCached());

	AssertTrue(cDescriptors.Flush(true));
	AssertTrue(cController.End());
	AssertTrue(cDescriptors.CanKill());	
	cDescriptors.Kill();
	cController.Kill();

	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);
	cDescriptors.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX", TEST_INDEXED_FILES_CACHE_SIZE, eWriteThrough);
	AssertTrue(cController.Begin());
	AssertInt(1, (int)cDescriptors.NumElements());

	AssertTrue(cDescriptors.Get(1LL, sz));
	AssertString(sz5, sz);

	AssertTrue(cController.End());
	AssertTrue(cDescriptors.CanKill());	
	cDescriptors.Kill();
	cController.Kill();
	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedFilesEvictingSetDataCacheDiffSizeFile(void)
{
	CDurableFileController					cController;
	CIndexTreeHelper						cHelper;
	CIndexedMap		cDescriptors;
	char									sz[200];
	char									sz6[] = "6Six6\0";
	char									sz5[] = "5ive\0";
	char									sz4[] = "4or\0";
	EIndexWriteThrough   eWriteThrough;

	eWriteThrough = IWT_No;
	cHelper.Init("Output" _FS_ "TestEvicting8", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cDescriptors.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX", TEST_INDEXED_FILES_CACHE_SIZE, eWriteThrough);

	AssertTrue(cController.Begin());
	AssertLSize(0, cDescriptors.NumDataCached());

	AssertTrue(cDescriptors.Put(1LL, sz6, 6));
	AssertTrue(cDescriptors.TestGetDescriptor(1LL, NULL));
	AssertLSize(1, cDescriptors.NumDataCached());

	AssertSize(0, cDescriptors.NumFiles());
	AssertTrue(cDescriptors.Flush(true));

	AssertTrue(cController.End());
	AssertTrue(cDescriptors.CanKill());	
	cDescriptors.Kill();
	cController.Kill();

	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);
	cDescriptors.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX", TEST_INDEXED_FILES_CACHE_SIZE, eWriteThrough);
	AssertTrue(cController.Begin());
	AssertInt(1, (int)cDescriptors.NumElements());

	AssertTrue(cDescriptors.Put(1LL, sz5, 5));
	AssertTrue(cDescriptors.TestGetDescriptor(1LL, NULL));
	AssertLSize(1, cDescriptors.NumDataCached());

	AssertTrue(cDescriptors.Flush(true));
	AssertTrue(cController.End());
	AssertTrue(cDescriptors.CanKill());	
	cDescriptors.Kill();
	cController.Kill();

	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);
	cDescriptors.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX", TEST_INDEXED_FILES_CACHE_SIZE, eWriteThrough);
	AssertTrue(cController.Begin());
	AssertInt(1, (int)cDescriptors.NumElements());

	AssertTrue(cDescriptors.Put(1LL, sz4, 4));
	AssertTrue(cDescriptors.TestGetDescriptor(1LL, NULL));
	AssertLSize(1, cDescriptors.NumDataCached());

	AssertTrue(cDescriptors.Flush(true));
	AssertTrue(cController.End());
	AssertTrue(cDescriptors.CanKill());	
	cDescriptors.Kill();
	cController.Kill();

	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);
	cDescriptors.Init(&cController, NULL, "DAT", "Files.IDX", "_Files.IDX", TEST_INDEXED_FILES_CACHE_SIZE, eWriteThrough);
	AssertTrue(cController.Begin());
	AssertInt(1, (int)cDescriptors.NumElements());

	AssertTrue(cDescriptors.Get(1LL, sz));
	AssertString(sz4, sz);

	AssertTrue(cController.End());
	AssertTrue(cDescriptors.CanKill());	
	cDescriptors.Kill();
	cController.Kill();
	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedFilesEvicting(void)
{
	TypeConverterInit();
	MemoryInit();
	DataMemoryInit();
	BeginTests();

	TestIndexedFilesEvictingSetDataNoCacheNoFile();
	TestIndexedFilesEvictingSetDataCacheSameSizeNoFile();
	TestIndexedFilesEvictingSetDataCacheDiffSizeNoFile();
	TestIndexedFilesEvictingSetDataNoCacheFileSameSize();
	TestIndexedFilesEvictingSetDataNoCacheFileDiffSize();
	TestIndexedFilesEvictingSetDataCacheSameSizeFile();
	TestIndexedFilesEvictingSetDataCacheDiffSizeFile();

	TestIndexedFilesEvictingFlush(IWT_No, true);
	TestIndexedFilesEvictingFlush(IWT_Yes, true);
	TestIndexedFilesEvictingFlush(IWT_No, false);
	TestIndexedFilesEvictingFlush(IWT_Yes, false);
	TestIndexedFilesEvictingEviction(IWT_No);
	TestIndexedFilesEvictingEviction(IWT_Yes);

	TestStatistics();
	DataMemoryKill();
	MemoryKill();
	TypeConverterKill();
}

