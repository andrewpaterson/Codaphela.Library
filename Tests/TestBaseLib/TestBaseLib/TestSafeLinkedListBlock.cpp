#include "BaseLib/IntegerHelper.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/SafeLinkedListBlock.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSafeLinkedListBlockIndexOf(void)
{
	CSafeLinkedListBlock	cList;
	int*					pi1;
	int*					pi2;
	int*					pi3;

	pi1 = NULL;
	pi2 = NULL;
	pi3 = NULL;

	cList.Init();
	pi1 = (int*)cList.InsertAfterTail(sizeof(int));
	*pi1 = 3;
	pi2 = (int*)cList.InsertAfterTail(sizeof(int));
	*pi2 = 2;
	pi3 = (int*)cList.InsertAfterTail(sizeof(int));
	*pi3 = 1;

	AssertInt(0, cList.IndexOf(pi1));
	AssertInt(1, cList.IndexOf(pi2));
	AssertInt(2, cList.IndexOf(pi3));

	cList.BubbleSort(&CompareInt);

	AssertInt(2, cList.IndexOf(pi1));
	AssertInt(1, cList.IndexOf(pi2));
	AssertInt(0, cList.IndexOf(pi3));

	cList.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSafeLinkedListBlockWrite(void)
{
	CSafeLinkedListBlock	cList;
	char					szHello[] = "Hello";
	char					szWorld[] = "World";
	char					szLog[] = "Logytech";
	char*					sz;
	CFileBasic				cFile;
	CDiskFile*				pcDisk;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "SafeLinkedListBlock";
	char					szFile[] = "Output" _FS_ "SafeLinkedListBlock" _FS_ "Test.DAT";
	bool					b;

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.TouchDir(szDirectory);

	cList.Init();
	sz = (char*)cList.InsertAfterTail((uint32)strlen(szHello) + 1);
	strcpy(sz, szHello);
	sz = (char*)cList.InsertAfterTail((uint32)strlen(szWorld) + 1);
	strcpy(sz, szWorld);
	sz = (char*)cList.InsertAfterTail((uint32)strlen(szLog) + 1);
	strcpy(sz, szLog);
	AssertInt(3, cList.NumElements());

	pcDisk = DiskFile(szFile);
	AssertTrue(pcDisk->Open(EFM_Write_Create));
	cFile.Init(pcDisk);
	AssertTrue(cFile.WriteBool(true));
	AssertTrue(cList.Write(&cFile));
	AssertTrue(pcDisk->Close());
	cFile.Kill();
	cList.Kill();

	cList.Init();
	AssertInt(0, cList.NumElements());
	cList.Kill();

	pcDisk = DiskFile(szFile);
	AssertTrue(pcDisk->Open(EFM_Read));
	cFile.Init(pcDisk);
	AssertTrue(cFile.ReadBool(&b));
	AssertTrue(cList.Read(&cFile));
	AssertTrue(pcDisk->Close());
	cFile.Kill();

	sz = (char*)cList.GetHead();
	AssertString(szHello, sz);
	sz = (char*)cList.GetNext(sz);
	AssertString(szWorld, sz);
	sz = (char*)cList.GetNext(sz);
	AssertString(szLog, sz);
	sz = (char*)cList.GetNext(sz);
	AssertNull(sz);
	cList.Kill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//	The tests of CSafeLinkedListBlock used by more than one thread are in TestThreadLib.
//
//////////////////////////////////////////////////////////////////////////
void TestSafeLinkedListBlock(void)
{
	BeginTests();
	MemoryInit();

	TestSafeLinkedListBlockIndexOf();
	TestSafeLinkedListBlockWrite();

	MemoryKill();
	TestStatistics();
}

