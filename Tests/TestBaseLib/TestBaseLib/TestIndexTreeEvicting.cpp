#include "BaseLib/StringHelper.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/SystemAllocator.h"
#include "BaseLib/MemoryAllocator.h"
#include "BaseLib/Logger.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/AccessDataOrderer.h"
#include "BaseLib/IndexTreeEvicting.h"
#include "BaseLib/IndexTreeEvictedList.h"
#include "BaseLib/IndexedDataEvictedList.h"
#include "BaseLib/IndexTreeHelper.h"
#include "BaseLib/IndexTreeEvictingAccess.h"
#include "BaseLib/IndexTreeEvictionStrategyRandom.h"
#include "BaseLib/IndexTreeEvictionStrategyDataOrderer.h"
#include "BaseLib/IndexTreeFileDefaultDataCallback.h"
#include "TestLib/Assert.h"
#include "TestIndexTreeEvicting.h"


void AssertTree(const char* szExpected, CIndexTreeEvicting* pcTree);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeEvictingAdd(EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse)
{
	CIndexTreeEvicting			cIndexTree;
	CIndexTreeEvictingAccess	cAccess;
	CDurableFileController		cController;
	CIndexTreeHelper			cHelper;
	int							iData;
	CChars						sz;

	cHelper.Init("Output" _FS_"IndexTreeEvicting0", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, 8 KB, NULL, LifeNull<CIndexTreeEvictionStrategy>(), NULL, eWriteThrough, eKeyReverse);
	cAccess.Init(&cIndexTree);

	iData = 78;
	cAccess.PutLongData(1LL, &iData, sizeof(int));
	sz.Init();
	cIndexTree.Print(&sz, true, false);
	if (eWriteThrough == IWT_Yes && eKeyReverse == IKR_Yes)
	{
		AssertStringApproximate(
			"= [IndexTreeEvicting]  =============================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================\n"
			"   Both:   root -> 2:6 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  ()\n"
			"Key: ------------- [0x01 00 00 00 00 00 00 00] -------------\n"
			"   Both:   0    -> 2:5  ()\n"
			"   Both:   0    -> 2:4  ()\n"
			"   Both:   0    -> 2:3  ()\n"
			"   Both:   0    -> 2:2  ()\n"
			"   Both:   0    -> 2:1  ()\n"
			"   Both:   0    -> 2:0  ()\n"
			"   Both:   0    -> 1:0  ()\n"
			"   Both:   1(X) (HAS_DATA, NODES_EMPTY)\n"
			, sz.Text());
		Pass();
	}
	else if (eWriteThrough == IWT_Yes && eKeyReverse == IKR_No)
	{
		AssertStringApproximate(
			"= [IndexTreeEvicting]  =============================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================\n"
			"   Both:   root -> . 2:6 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  ()\n"
			"Key: ------------- [0x01 00 00 00 00 00 00 00] -------------\n"
			"   Both:   1    -> 2:5  ()\n"
			"   Both:   0    -> 2:4  ()\n"
			"   Both:   0    -> 2:3  ()\n"
			"   Both:   0    -> 2:2  ()\n"
			"   Both:   0    -> 2:1  ()\n"
			"   Both:   0    -> 2:0  ()\n"
			"   Both:   0    -> 1:0  ()\n"
			"   Both:   0(X) (HAS_DATA, NODES_EMPTY)\n"
			, sz.Text());
	}
	else if (eWriteThrough == IWT_No && eKeyReverse == IKR_Yes)
	{
		AssertStringApproximate(
			"= [IndexTreeEvicting]  =============================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================\n"
			"+Memory:   root -> o . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  (DIRTY_PATH)\n"
			"-  File:   root -> . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  (DIRTY_PATH)\n"
			"Key: ------------- [0x01 00 00 00 00 00 00 00] -------------\n"
			" Memory:   0    -> o  (DIRTY_PATH)\n"
			" Memory:   0    -> o  (DIRTY_PATH)\n"
			" Memory:   0    -> o  (DIRTY_PATH)\n"
			" Memory:   0    -> o  (DIRTY_PATH)\n"
			" Memory:   0    -> o  (DIRTY_PATH)\n"
			" Memory:   0    -> o  (DIRTY_PATH)\n"
			" Memory:   0    -> o  (DIRTY_PATH)\n"
			" Memory:   1(X) (DIRTY_NODE, DIRTY_PATH, HAS_DATA, NODES_EMPTY)\n"
			, sz.Text());
		Pass();
	}
	else if (eWriteThrough == IWT_No && eKeyReverse == IKR_No)
	{
		AssertStringApproximate(
			"= [IndexTreeEvicting]  =============================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================\n"
			"+Memory:   root -> . o . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  (DIRTY_PATH)\n"
			"-  File:   root -> . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  (DIRTY_PATH)\n"
			"Key: ------------- [0x01 00 00 00 00 00 00 00] -------------\n"
			" Memory:   1    -> o  (DIRTY_PATH)\n"
			" Memory:   0    -> o  (DIRTY_PATH)\n"
			" Memory:   0    -> o  (DIRTY_PATH)\n"
			" Memory:   0    -> o  (DIRTY_PATH)\n"
			" Memory:   0    -> o  (DIRTY_PATH)\n"
			" Memory:   0    -> o  (DIRTY_PATH)\n"
			" Memory:   0    -> o  (DIRTY_PATH)\n"
			" Memory:   0(X) (DIRTY_NODE, DIRTY_PATH, HAS_DATA, NODES_EMPTY)\n"
			, sz.Text());
		Pass();
	}
	sz.Kill();

	cAccess.Flush();
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
void TestIndexTreeEvictingEvictRandom(EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse)
{
	CTestIndexTreeEvicting				cIndexTree;
	CIndexTreeHelper					cHelper;
	CDurableFileController				cController;
	CIndexTreeEvictingAccess			cAccess;
	CMemoryAllocator					cAllocator;
	CIndexTreeEvictionStrategyRandom	cStrategy;
	CIndexTreeEvictedList				cIndexTreeEvictedList;   // CIndexTreeEvictionCallback
	lsize								sSize;
	CIndexTreeFileDefaultDataCallback	cWriterCallback;

	cHelper.Init("Output" _FS_"IndexTreeEvicting0a", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);
	cIndexTreeEvictedList.Init();
	cController.Begin();
	cStrategy.Init(15);
	cIndexTree.Init(&cController, "Sub", 8 KB, &cIndexTreeEvictedList, LifeLocal<CIndexTreeEvictionStrategy>(&cStrategy), &cWriterCallback, eWriteThrough, eKeyReverse);
	cAccess.Init(&cIndexTree);


	sSize = cIndexTree.GetRoot()->CalculateNodeSize();
	AssertLong(2062LL, sSize);
	sSize = cIndexTree.GetSystemMemorySize();
#ifdef BUILD_64
	AssertLong(4136LL, sSize);
#else
	AssertLong(3100LL, sSize);
#endif // BUILD_64

	cAccess.PutLongInt(1LL, 57);
	sSize = cIndexTree.GetRoot()->CalculateNodeSize();
	AssertLong(2062LL, sSize);
	sSize = cIndexTree.GetSystemMemorySize();
#ifdef BUILD_64
	AssertLong(4596LL, sSize);
#else
	AssertLong(3424LL, sSize);
#endif // BUILD_64

	cAccess.Flush();
	sSize = cIndexTree.GetRoot()->CalculateNodeSize();
	AssertLSize(2062LL, sSize);
	sSize = cIndexTree.GetSystemMemorySize();
#ifdef BUILD_64
	AssertSize(4596LL, sSize);
#else
	AssertLSize(3424LL, sSize);
#endif // BUILD_64

	cAccess.DeleteLong(1LL);
	cAccess.Flush();
	sSize = cIndexTree.GetRoot()->CalculateNodeSize();
	AssertLSize(2062LL, sSize);
	sSize = cIndexTree.GetSystemMemorySize();
#ifdef BUILD_64
	AssertSize(4136LL, sSize);
#else
	AssertLSize(3100LL, sSize);
#endif // BUILD_64


	cController.End();
	cAccess.Kill();
	cIndexTree.Kill();
	cIndexTreeEvictedList.Kill();
	cController.Kill();
	cStrategy.Kill();
	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeEvictingPut(EIndexWriteThrough eWriteThrough)
{
	CTestIndexTreeEvicting				cIndexTree;
	CIndexTreeEvictedList				cEvictedNodes;
	CIndexTreeHelper					cHelper;
	CDurableFileController				cController;
	CIndexTreeEvictingAccess			cAccess;
	CMemoryAllocator					cAllocator;
	CGeneralMemory*						pcMemory;
	int									x;
	int32								y;
	CIndexTreeEvictionStrategyRandom	cStrategy;
	size								uiCacheSize;
	
	cAllocator.Init();
	pcMemory = cAllocator.GetMemory();
	cHelper.Init("Output" _FS_"IndexTreeEvicting1", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cEvictedNodes.Init();
	cStrategy.Init(15);
	//Room for one key.  The nodes are bigger in a 64 bit build.
#ifdef BUILD_64
	uiCacheSize = 5000;
#else
	uiCacheSize = 3656;
#endif // BUILD_64
	cIndexTree.Init(&cController, "Here", uiCacheSize, &cEvictedNodes, LifeLocal<CIndexTreeEvictionStrategy>(&cStrategy), NULL, LifeLocal<CMallocator>(&cAllocator), eWriteThrough, IKR_No);
	cAccess.Init(&cIndexTree);

#ifdef BUILD_64
	AssertLong(4128, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3096, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64
	x = 1234;
	cAccess.PutStringInt("Tomato", x);
#ifdef BUILD_64
	AssertLong(4428LL, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3316LL, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64
	AssertLSize(1, cIndexTree.NumMemoryElements());
	AssertLSize(7, cIndexTree.NumMemoryNodes());
	AssertSize(7, strlen("Tomato") + 1);  //6 nodes for Tomato + the root node.
	AssertSize(0, cEvictedNodes.NumElements());
	cEvictedNodes.Clear();

	y = 567890;
	cAccess.PutStringInt("Plantation", y);

#ifdef BUILD_64
	AssertLong(4620LL, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3460LL, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64
	AssertSize(1, cEvictedNodes.NumElements());
	AssertString("Tomato", (char*)cEvictedNodes.GetKey(0));
	AssertInt(x, *((int*)cEvictedNodes.GetData(0)));
	AssertLSize(1, cIndexTree.NumMemoryElements());
	AssertLSize(11, cIndexTree.NumMemoryNodes());
	AssertSize(11, strlen("Plantation") + 1);  //10 nodes for Plantation + the root node.
	cEvictedNodes.Clear();

	AssertInt((int32)1234, cAccess.GetStringInt("Tomato"));

#ifdef BUILD_64
	AssertLong(4444LL, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3328LL, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64
	AssertSize(1, cEvictedNodes.NumElements());
	AssertString("Plantation", (char*)cEvictedNodes.GetKey(0));
	AssertInt(y, *((int32*)cEvictedNodes.GetData(0)));
	AssertLSize(1, cIndexTree.NumMemoryElements());
	AssertLSize(7, cIndexTree.NumMemoryNodes());

	cController.End();

	cAccess.Kill();
	cIndexTree.Kill();
	cEvictedNodes.Kill();
	cStrategy.Kill();
	cController.Kill();
	cHelper.Kill(true);

	cAllocator.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeEvictingEvictWithChildren(void)
{
	CIndexTreeEvictingAccess	cAccess;
	CIndexTreeHelper			cHelper;
	char						szAlbatros[] = "albatros";
	char						szAlba[] = "alba";
	char						szAlbaquerque[] = "albaquerque";
	CGeneralMemory*				pcMemory;
	CMemoryAllocator			cAllocator;
	CDurableFileController		cController;
	CIndexTreeEvicting			cIndexTree;
	SLogConfig					sLogConfig;

	cAllocator.Init();
	pcMemory = cAllocator.GetMemory();
	cHelper.Init("Output" _FS_"IndexTreeEvicting2", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, 8192, NULL, LifeNull<CIndexTreeEvictionStrategy>(), NULL, LifeLocal<CMallocator>(&cAllocator), IWT_No, IKR_No);
	cAccess.Init(&cIndexTree);

	AssertTrue(cAccess.PutStringString(szAlbatros, szAlbatros));
	AssertTrue(cAccess.PutStringString(szAlba, szAlba));
	AssertTrue(cAccess.PutStringString(szAlbaquerque, szAlbaquerque));
#ifdef BUILD_64
	AssertLong(4962LL, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3710LL, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64

	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cAccess.EvictString(szAlba));
	gcLogger.SetConfig(&sLogConfig);
#ifdef BUILD_64
	AssertLong(4962LL, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3710LL, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64
	Pass();

	AssertTrue(cAccess.EvictString(szAlbatros));
#ifdef BUILD_64
	AssertLong(4753LL, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3557LL, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64

	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cAccess.EvictString(szAlba));
	gcLogger.SetConfig(&sLogConfig);
#ifdef BUILD_64
	AssertLong(4753LL, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3557LL, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64
	Pass();

	AssertTrue(cAccess.EvictString(szAlbaquerque));
#ifdef BUILD_64
	AssertLong(4397LL, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3293LL, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64
	Pass();

	AssertTrue(cAccess.EvictString(szAlba));
#ifdef BUILD_64
	AssertLong(4128LL, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3096LL, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64

	cController.End();
	cIndexTree.Kill();
	AssertLong(0, pcMemory->GetTotalAllocatedMemory());
	cHelper.Kill(true);

	cController.Kill();
	cAllocator.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeEvictingFlushWithChildren(void)
{
	CIndexTreeEvictingAccess	cAccess;
	CIndexTreeHelper			cHelper;
	char						szAlbatros[] = "albatros";
	char						szAlba[] = "alba";
	char						szAlbaquerque[] = "albaquerque";
	CGeneralMemory*				pcMemory;
	CMemoryAllocator			cAllocator;
	CDurableFileController		cController;
	CIndexTreeEvicting			cIndexTree;
	char						szBatmobile[] = "batmobile";
	char						szBatcave[] = "batcave";
	char						szBathroom[] = "bathroom";
	CChars						szResult;

	cAllocator.Init();
	pcMemory = cAllocator.GetMemory();
	cHelper.Init("Output" _FS_"IndexTreeEvicting3", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cController.Begin();
	cIndexTree.Init(&cController, NULL, 8192, NULL, LifeNull<CIndexTreeEvictionStrategy>(), NULL, LifeLocal<CMallocator>(&cAllocator), IWT_No, IKR_No);
	cAccess.Init(&cIndexTree);
#ifdef BUILD_64
	AssertLong(4128LL, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3096LL, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64
	Pass();

	AssertTrue(cAccess.PutStringString(szAlbatros, szAlbatros));
	AssertTrue(cAccess.PutStringString(szAlba, szAlba));
	AssertTrue(cAccess.PutStringString(szAlbaquerque, szAlbaquerque));
#ifdef BUILD_64
	AssertLong(4962LL, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3710LL, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64
	Pass();

	AssertTree(
		"= [IndexTreeEvicting]  =============================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================\n" 
		"+Memory:   root -> . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . o . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . \n" 
		"-  File:   root -> . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . \n" 
		"Key: ------------- [alba] -------------\n"
		" Memory:   a    -> o \n"
		" Memory:   l    -> o \n"
		" Memory:   b    -> o \n"
		" Memory:   a(X) -> o . . o\n"
		"Key: ------------- [albaquerque] -------------\n"
		" Memory:   a    -> o \n" 
		" Memory:   l    -> o \n" 
		" Memory:   b    -> o \n" 
		" Memory:   a(X) -> o . . o \n" 
		" Memory:   q    -> o \n" 
		" Memory:   u    -> o \n" 
		" Memory:   e    -> o \n" 
		" Memory:   r    -> o \n" 
		" Memory:   q    -> o \n" 
		" Memory:   u    -> o \n" 
		" Memory:   e(X)\n" 
		"Key: ------------- [albatros] -------------\n" 
		" Memory:   a    -> o \n" 
		" Memory:   l    -> o \n" 
		" Memory:   b    -> o \n" 
		" Memory:   a(X) -> o . . o \n" 
		" Memory:   t    -> o \n" 
		" Memory:   r    -> o \n" 
		" Memory:   o    -> o \n" 
		" Memory:   s(X)", 
		&cIndexTree);
	AssertFalse(cIndexTree.IsFlushed());

	AssertTrue(cAccess.FlushString(szAlba));
	AssertTree(
		"= [IndexTreeEvicting]  =============================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================\n"
		"   Both:   root -> . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 2:2 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . \n"
		"Key: ------------- [alba] -------------\n"
		"   Both:   a    -> 2:1 \n"
		"   Both:   l    -> 2:0 \n"
		"   Both:   b    -> 1:0 \n"
		"+Memory:   a(X) -> o . . o \n"
		"-  File:   a(X) -> . . . . \n"
		"Key: ------------- [albaquerque] -------------\n"
		"   Both:   a    -> 2:1 \n"
		"   Both:   l    -> 2:0 \n"
		"   Both:   b    -> 1:0 \n"
		"+Memory:   a(X) -> o . . o \n"
		"-  File:   a(X) -> . . . . \n"
		" Memory:   q    -> o \n"
		" Memory:   u    -> o \n"
		" Memory:   e    -> o \n"
		" Memory:   r    -> o \n"
		" Memory:   q    -> o \n"
		" Memory:   u    -> o \n"
		" Memory:   e(X)\n"
		"Key: ------------- [albatros] -------------\n"
		"   Both:   a    -> 2:1 \n"
		"   Both:   l    -> 2:0 \n"
		"   Both:   b    -> 1:0 \n"
		"+Memory:   a(X) -> o . . o \n"
		"-  File:   a(X) -> . . . . \n"
		" Memory:   t    -> o \n"
		" Memory:   r    -> o \n"
		" Memory:   o    -> o \n"
		" Memory:   s(X)",
		&cIndexTree);
	AssertFalse(cIndexTree.IsFlushed());

	AssertTrue(cAccess.FlushString(szAlbatros));
	AssertTree(
		"= [IndexTreeEvicting]  =============================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================\n"
		"   Both:   root -> . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 2:2 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . \n"
		"Key: ------------- [alba] -------------\n"
		"   Both:   a    -> 2:1 \n"
		"   Both:   l    -> 2:0 \n"
		"   Both:   b    -> 1:0 \n"
		"+Memory:   a(X) -> o . . 2:5 \n"
		"-  File:   a(X) -> . . . 2:5 \n"
		"Key: ------------- [albaquerque] -------------\n"
		"   Both:   a    -> 2:1 \n"
		"   Both:   l    -> 2:0 \n"
		"   Both:   b    -> 1:0 \n"
		"+Memory:   a(X) -> o . . 2:5 \n"
		"-  File:   a(X) -> . . . 2:5 \n"
		" Memory:   q    -> o \n"
		" Memory:   u    -> o \n"
		" Memory:   e    -> o \n"
		" Memory:   r    -> o \n"
		" Memory:   q    -> o \n"
		" Memory:   u    -> o \n"
		" Memory:   e(X)\n"
		"Key: ------------- [albatros] -------------\n"
		"   Both:   a    -> 2:1 \n"
		"   Both:   l    -> 2:0 \n"
		"   Both:   b    -> 1:0 \n"
		"+Memory:   a(X) -> o . . 2:5 \n"
		"-  File:   a(X) -> . . . 2:5 \n"
		"   Both:   t    -> 2:4 \n"
		"   Both:   r    -> 2:3 \n"
		"   Both:   o    -> 3:0 \n"
		"   Both:   s(X)",
		&cIndexTree);
	AssertFalse(cIndexTree.IsFlushed());

	AssertTrue(cAccess.FlushString(szAlbaquerque));
	AssertTree(
	"= [IndexTreeEvicting]  =============================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================\n"
		"   Both:   root -> . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 2:2 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . \n"
		"Key: ------------- [alba] -------------\n"
		"   Both:   a    -> 2:1 \n"
		"   Both:   l    -> 2:0 \n"
		"   Both:   b    -> 1:0 \n"
		"   Both:   a(X) -> 2:11 . . 2:5 \n"
		"Key: ------------- [albaquerque] -------------\n"
		"   Both:   a    -> 2:1 \n"
		"   Both:   l    -> 2:0 \n"
		"   Both:   b    -> 1:0 \n"
		"   Both:   a(X) -> 2:11 . . 2:5 \n"
		"   Both:   q    -> 2:10 \n"
		"   Both:   u    -> 2:9 \n"
		"   Both:   e    -> 2:8 \n"
		"   Both:   r    -> 2:7 \n"
		"   Both:   q    -> 2:6 \n"
		"   Both:   u    -> 4:0 \n"
		"   Both:   e(X)\n"
		"Key: ------------- [albatros] -------------\n"
		"   Both:   a    -> 2:1 \n"
		"   Both:   l    -> 2:0 \n"
		"   Both:   b    -> 1:0 \n"
		"   Both:   a(X) -> 2:11 . . 2:5 \n"
		"   Both:   t    -> 2:4 \n"
		"   Both:   r    -> 2:3 \n"
		"   Both:   o    -> 3:0 \n"
		"   Both:   s(X)",
		&cIndexTree);
	AssertTrue(cIndexTree.IsFlushed());
#ifdef BUILD_64
	AssertLong(4962LL, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3710LL, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64

	szResult = cAccess.GetStringString(szAlbatros);
	AssertString(szAlbatros, szResult.Text());	szResult.Kill();
	szResult = cAccess.GetStringString(szAlba);
	AssertString(szAlba, szResult.Text());	szResult.Kill();
	szResult = cAccess.GetStringString(szAlbaquerque);
	AssertString(szAlbaquerque, szResult.Text());	szResult.Kill();
	Pass();

	AssertTrue(cAccess.PutStringString(szAlbatros, szBatcave));
	AssertTrue(cAccess.PutStringString(szAlba, szBathroom));
#ifdef BUILD_64
	AssertLong(4965LL, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3713LL, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64
	Pass();

	szResult = cAccess.GetStringString(szAlbatros);
	AssertString(szBatcave, szResult.Text());	szResult.Kill();
	szResult = cAccess.GetStringString(szAlba);
	AssertString(szBathroom, szResult.Text());	szResult.Kill();
	AssertTree(
		"= [IndexTreeEvicting]  =============================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================\n"
		"   Both:   root -> . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 2:2 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . \n"
		"Key: ------------- [alba] -------------\n"
		"   Both:   a    -> 2:1 \n"
		"   Both:   l    -> 2:0 \n"
		"   Both:   b    -> 1:0 \n"
		"   Both:   a(X) -> 2:11 . . 2:5 \n"
		"Key: ------------- [albaquerque] -------------\n"
		"   Both:   a    -> 2:1 \n"
		"   Both:   l    -> 2:0 \n"
		"   Both:   b    -> 1:0 \n"
		"   Both:   a(X) -> 2:11 . . 2:5 \n"
		"   Both:   q    -> 2:10 \n"
		"   Both:   u    -> 2:9 \n"
		"   Both:   e    -> 2:8 \n"
		"   Both:   r    -> 2:7 \n"
		"   Both:   q    -> 2:6 \n"
		"   Both:   u    -> 4:0 \n"
		"   Both:   e(X)\n"
		"Key: ------------- [albatros] -------------\n"
		"   Both:   a    -> 2:1 \n"
		"   Both:   l    -> 2:0 \n"
		"   Both:   b    -> 1:0 \n"
		"   Both:   a(X) -> 2:11 . . 2:5 \n"
		"   Both:   t    -> 2:4 \n"
		"   Both:   r    -> 2:3 \n"
		"   Both:   o    -> 3:0 \n"
		"   Both:   s(X)",
		&cIndexTree);

	AssertTrue(cAccess.FlushString(szAlba));
	AssertTrue(cAccess.FlushString(szAlbatros));
	AssertTree(
		"= [IndexTreeEvicting]  =============================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================\n"
		"   Both:   root -> . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 2:2 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . \n"
		"Key: ------------- [alba] -------------\n"
		"   Both:   a    -> 2:1 \n"
		"   Both:   l    -> 2:0 \n"
		"   Both:   b    -> 5:0 \n"
		"   Both:   a(X) -> 2:11 . . 2:5 \n"
		"Key: ------------- [albaquerque] -------------\n"
		"   Both:   a    -> 2:1 \n"
		"   Both:   l    -> 2:0 \n"
		"   Both:   b    -> 5:0 \n"
		"   Both:   a(X) -> 2:11 . . 2:5 \n"
		"   Both:   q    -> 2:10 \n"
		"   Both:   u    -> 2:9 \n"
		"   Both:   e    -> 2:8 \n"
		"   Both:   r    -> 2:7 \n"
		"   Both:   q    -> 2:6 \n"
		"   Both:   u    -> 4:0 \n"
		"   Both:   e(X)\n"
		"Key: ------------- [albatros] -------------\n"
		"   Both:   a    -> 2:1 \n"
		"   Both:   l    -> 2:0 \n"
		"   Both:   b    -> 5:0 \n"
		"   Both:   a(X) -> 2:11 . . 2:5 \n"
		"   Both:   t    -> 2:4 \n"
		"   Both:   r    -> 2:3 \n"
		"   Both:   o    -> 2:12 \n"
		"   Both:   s(X)",
		&cIndexTree);

	AssertTrue(cAccess.PutStringString(szAlbaquerque, szBatmobile));
	AssertTrue(cAccess.FlushString(szAlbaquerque));
	AssertTrue(cIndexTree.IsFlushed());
	AssertTree(
		"= [IndexTreeEvicting]  =============================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================\n"
		"   Both:   root -> . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 2:2 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . \n"
		"Key: ------------- [alba] -------------\n"
		"   Both:   a    -> 2:1 \n"
		"   Both:   l    -> 2:0 \n"
		"   Both:   b    -> 5:0 \n"
		"   Both:   a(X) -> 2:11 . . 2:5 \n"
		"Key: ------------- [albaquerque] -------------\n"
		"   Both:   a    -> 2:1 \n"
		"   Both:   l    -> 2:0 \n"
		"   Both:   b    -> 5:0 \n"
		"   Both:   a(X) -> 2:11 . . 2:5 \n"
		"   Both:   q    -> 2:10 \n"
		"   Both:   u    -> 2:9 \n"
		"   Both:   e    -> 2:8 \n"
		"   Both:   r    -> 2:7 \n"
		"   Both:   q    -> 2:6 \n"
		"   Both:   u    -> 6:0 \n"
		"   Both:   e(X)\n"
		"Key: ------------- [albatros] -------------\n"
		"   Both:   a    -> 2:1 \n"
		"   Both:   l    -> 2:0 \n"
		"   Both:   b    -> 5:0 \n"
		"   Both:   a(X) -> 2:11 . . 2:5 \n"
		"   Both:   t    -> 2:4 \n"
		"   Both:   r    -> 2:3 \n"
		"   Both:   o    -> 2:12 \n"
		"   Both:   s(X)\n",
		&cIndexTree);

	cController.End();
	cIndexTree.Kill();
	cAccess.Kill();
	AssertLong(0, pcMemory->GetTotalAllocatedMemory());

	cController.Begin();
	cIndexTree.Init(&cController, NULL, 8192, NULL, LifeNull<CIndexTreeEvictionStrategy>(), NULL, LifeLocal<CMallocator>(&cAllocator), IWT_No, IKR_No);
	cAccess.Init(&cIndexTree);
#ifdef BUILD_64
	AssertLong(4160, pcMemory->GetTotalAllocatedMemory());
#else
	AssertLong(3120, pcMemory->GetTotalAllocatedMemory());
#endif // BUILD_64

	szResult = cAccess.GetStringString(szAlbatros);
	AssertString(szBatcave, szResult.Text());	szResult.Kill();
	szResult = cAccess.GetStringString(szAlba);
	AssertString(szBathroom, szResult.Text());	szResult.Kill();
	szResult = cAccess.GetStringString(szAlbaquerque);
	AssertString(szBatmobile, szResult.Text());	szResult.Kill();

	cController.End();
	cIndexTree.Kill();
	cAccess.Kill();
	AssertLong(0, pcMemory->GetTotalAllocatedMemory());
	cHelper.Kill(true);

	cController.Kill();
	cAllocator.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeEvictingEvictLastAccessed(void)
{
	CTestIndexTreeEvicting					cIndexTree;
	CIndexTreeHelper						cHelper;
	CDurableFileController					cController;
	CIndexTreeEvictingAccess				cAccess;
	CIndexTreeEvictionStrategyDataOrderer	cStrategy;
	CIndexTreeEvictedList					cIndexTreeEvictedList;
	CAccessDataOrderer						cOrderer;
	char									szData[1024];
	int										i;
	lsize									sSize;
	char*									szEvictedKey;
	size									uiCacheSize;

	for (i = 0; i < 1023; i++)
	{
		szData[i] = (char)('A' + i % 26);
	}
	szData[1023] = '\0';

	cHelper.Init("Output" _FS_"IndexTreeEvicting4", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);
	cIndexTreeEvictedList.Init();
	cController.Begin();
	cOrderer.Init();
	cStrategy.Init(LifeLocal<CIndexTreeDataOrderer>(&cOrderer));
	//Room for three keys.  The nodes are bigger in a 64 bit build.
#ifdef BUILD_64
	uiCacheSize = 9 KB;
#else
	uiCacheSize = 8 KB;
#endif // BUILD_64
	cIndexTree.Init(&cController, "Sub", uiCacheSize, &cIndexTreeEvictedList, LifeLocal<CIndexTreeEvictionStrategy>(&cStrategy), NULL, IWT_No, IKR_No, LifeLocal<CIndexTreeDataOrderer>(&cOrderer));
	cAccess.Init(&cIndexTree);

	sSize = cIndexTree.GetSystemMemorySize();
#ifdef BUILD_64
	AssertLong(4136LL, sSize);
#else
	AssertLong(3100LL, sSize);
#endif // BUILD_64
	AssertSize(0, cIndexTreeEvictedList.NumElements());

	cAccess.PutStringString("A0000001", szData);
	sSize = cIndexTree.GetSystemMemorySize();
#ifdef BUILD_64
	AssertLong(5616LL, sSize);
#else
	AssertLong(4444LL, sSize);
#endif // BUILD_64
	AssertSize(0, cIndexTreeEvictedList.NumElements());

	cAccess.PutStringString("B0000002", szData);
	sSize = cIndexTree.GetSystemMemorySize();
#ifdef BUILD_64
	AssertLong(7096LL, sSize);
#else
	AssertLong(5788LL, sSize);
#endif // BUILD_64
	AssertSize(0, cIndexTreeEvictedList.NumElements());

	cAccess.PutStringString("C0000003", szData);
	sSize = cIndexTree.GetSystemMemorySize();
#ifdef BUILD_64
	AssertLong(8576LL, sSize);
#else
	AssertLong(7132LL, sSize);
#endif // BUILD_64
	AssertSize(0, cIndexTreeEvictedList.NumElements());

	cAccess.PutStringString("D0000004", szData);
	sSize = cIndexTree.GetSystemMemorySize();
#ifdef BUILD_64
	AssertLong(8576LL, sSize);
#else
	AssertLong(7132LL, sSize);
#endif // BUILD_64
	AssertSize(1, cIndexTreeEvictedList.NumElements());
	szEvictedKey = (char*)cIndexTreeEvictedList.GetKey(0);
	AssertString("A0000001", szEvictedKey);
	cIndexTreeEvictedList.Clear();

	cAccess.PutStringString("E0000005", szData);
	sSize = cIndexTree.GetSystemMemorySize();
#ifdef BUILD_64
	AssertLong(8576LL, sSize);
#else
	AssertLong(7132LL, sSize);
#endif // BUILD_64
	AssertSize(1, cIndexTreeEvictedList.NumElements());
	szEvictedKey = (char*)cIndexTreeEvictedList.GetKey(0);
	AssertString("B0000002", szEvictedKey);
	cIndexTreeEvictedList.Clear();

	cAccess.PutStringString("G0000006", szData);
	sSize = cIndexTree.GetSystemMemorySize();
#ifdef BUILD_64
	AssertLong(8576LL, sSize);
#else
	AssertLong(7132LL, sSize);
#endif // BUILD_64
	AssertSize(1, cIndexTreeEvictedList.NumElements());
	szEvictedKey = (char*)cIndexTreeEvictedList.GetKey(0);
	AssertString("C0000003", szEvictedKey);

	cAccess.Flush();
	cAccess.ValidateIndex();

	cController.End();
	cAccess.Kill();
	cIndexTree.Kill();
	cIndexTreeEvictedList.Kill();
	cController.Kill();
	cStrategy.Kill();
	cOrderer.Kill();
	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeEvictingEmpty(void)
{
	CIndexTreeEvictingAccess 				cAccess;
	CIndexTreeEvicting						cIndexTree;
	CIndexTreeHelper						cHelper;
	CDurableFileController					cController;
	CIndexTreeEvictionStrategyDataOrderer	cEvictionStrategy;
	CAccessDataOrderer						cDataOrderer;

	cHelper.Init("Output" _FS_ "IndexTreeEvicting5", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);

	cDataOrderer.Init();
	cEvictionStrategy.Init(LifeLocal<CIndexTreeDataOrderer>(&cDataOrderer));

	cController.Begin();
	cIndexTree.Init(&cController, NULL, 8 KB, NULL, LifeLocal<CIndexTreeEvictionStrategy>(&cEvictionStrategy), NULL, IWT_No, IKR_No, LifeLocal<CIndexTreeDataOrderer>(&cDataOrderer));
	cAccess.Init(&cIndexTree);

	cAccess.Flush();
	cAccess.ValidateIndex();

	cController.End();
	cAccess.Kill();
	cIndexTree.Kill();
	cController.Kill();
	cEvictionStrategy.Kill();
	cDataOrderer.Kill();

	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertTree(const char* szExpected, CIndexTreeEvicting* pcTree)
{
	CChars	sz;

	sz.Init();
	pcTree->Print(&sz, false, false);
	AssertStringApproximate(szExpected, sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Keys put and then removed again and again stay within the cache: a written
// back tree keeps a removed key in memory until its removal is written, and
// writes the removals when nothing else can be evicted.
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeEvictingEvictRemoved(EIndexWriteThrough eWriteThrough)
{
	CTestIndexTreeEvicting					cIndexTree;
	CIndexTreeHelper						cHelper;
	CDurableFileController					cController;
	CIndexTreeEvictionStrategyDataOrderer	cStrategy;
	CAccessDataOrderer						cOrderer;
	char									szData[1024];
	char									szRead[1024];
	char									szKey[8];
	int										i;
	int										iRound;
	size									uiCacheSize;
	size									uiSize;

	for (i = 0; i < 1024; i++)
	{
		szData[i] = (char)('A' + i % 26);
	}

	cHelper.Init("Output" _FS_"IndexTreeEvicting5", "primary", "backup", true);
	cController.Init(cHelper.GetPrimaryDirectory(), cHelper.GetBackupDirectory());
	cController.SetSync(DS_No);
	cController.Begin();
	cOrderer.Init();
	cStrategy.Init(LifeLocal<CIndexTreeDataOrderer>(&cOrderer));
	//Room for the tree's nodes and a few keys, but not the 20 put each round.
	uiCacheSize = 16 KB;
	cIndexTree.Init(&cController, "Sub", uiCacheSize, NULL, LifeLocal<CIndexTreeEvictionStrategy>(&cStrategy), NULL, eWriteThrough, IKR_No, LifeLocal<CIndexTreeDataOrderer>(&cOrderer));

	for (iRound = 0; iRound < 5; iRound++)
	{
		for (i = 0; i < 20; i++)
		{
			snprintf(szKey, sizeof(szKey), "Key%02d", i);
			szData[0] = (char)('a' + iRound);
			AssertTrue(cIndexTree.Put((uint8*)szKey, 5, szData, 1024));
			AssertTrue(cIndexTree.GetSystemMemorySize() <= uiCacheSize);
		}
		for (i = 0; i < 20; i++)
		{
			snprintf(szKey, sizeof(szKey), "Key%02d", i);
			AssertTrue(cIndexTree.Get((uint8*)szKey, 5, szRead, &uiSize, 1024));
			AssertSize(1024, uiSize);
			AssertChar((char)('a' + iRound), szRead[0]);
			AssertTrue(cIndexTree.Remove((uint8*)szKey, 5));
			AssertTrue(cIndexTree.GetSystemMemorySize() <= uiCacheSize);
		}
		AssertFalse(cIndexTree.Get((uint8*)"Key00", 5, szRead, &uiSize, 1024));
	}

	cController.End();
	cIndexTree.Kill();
	cStrategy.Kill();
	cOrderer.Kill();
	cController.Kill();
	cHelper.Kill(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexTreeEvicting(void)
{
	TypeConverterInit();
	MemoryInit();
	DataMemoryInit();
	BeginTests();

	TestIndexTreeEvictingEmpty();
	TestIndexTreeEvictingAdd(IWT_Yes, IKR_Yes);
	TestIndexTreeEvictingAdd(IWT_Yes, IKR_No);
	TestIndexTreeEvictingAdd(IWT_No, IKR_Yes);
	TestIndexTreeEvictingAdd(IWT_No, IKR_No);
	TestIndexTreeEvictingPut(IWT_Yes);
	TestIndexTreeEvictingPut(IWT_No);
	TestIndexTreeEvictingEvictWithChildren();
	TestIndexTreeEvictingFlushWithChildren();
	TestIndexTreeEvictingEvictRandom(IWT_Yes, IKR_Yes);
	TestIndexTreeEvictingEvictRandom(IWT_Yes, IKR_No);
	TestIndexTreeEvictingEvictRandom(IWT_No, IKR_Yes);
	TestIndexTreeEvictingEvictRandom(IWT_No, IKR_No);
	TestIndexTreeEvictingEvictLastAccessed();
	TestIndexTreeEvictingEvictRemoved(IWT_No);
	TestIndexTreeEvictingEvictRemoved(IWT_Yes);
	
	TestStatistics();
	DataMemoryKill();
	MemoryKill();
	TypeConverterKill();
}

