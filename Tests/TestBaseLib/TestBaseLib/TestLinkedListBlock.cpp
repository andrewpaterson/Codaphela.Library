#include "BaseLib/IntegerHelper.h"
#include "BaseLib/LinkedListBlock.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/GlobalMemory.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLinkedListBlockIndexOf(void)
{
	CLinkedListBlock	cList;
	int*				pi1;
	int*				pi2;
	int*				pi3;

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

	AssertSize(0, cList.IndexOf(pi1));
	AssertSize(1, cList.IndexOf(pi2));
	AssertSize(2, cList.IndexOf(pi3));

	cList.BubbleSort(&CompareInt);

	AssertSize(2, cList.IndexOf(pi1));
	AssertSize(1, cList.IndexOf(pi2));
	AssertSize(0, cList.IndexOf(pi3));

	cList.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLinkedListBlockWrite(void)
{
	CLinkedListBlock	cList;
	char				szHello[] = "Hello";
	char				szWorld[] = "World";
	char				szLog[] = "Logytech";
	char*				sz;
	CFileBasic			cFile;
	CDiskFile*			pcDisk;
	CFileUtil			cFileUtil;
	char				szDirectory[] = "Output" _FS_ "LinkedListBlock";
	char				szFile[] = "Output" _FS_ "LinkedListBlock" _FS_ "Test.DAT";
	bool				b;

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.TouchDir(szDirectory);

	cList.Init();
	sz = (char*)cList.InsertAfterTail(strlen(szHello) + 1);
	strcpy(sz, szHello);
	sz = (char*)cList.InsertAfterTail(strlen(szWorld) + 1);
	strcpy(sz, szWorld);
	sz = (char*)cList.InsertAfterTail(strlen(szLog) + 1);
	strcpy(sz, szLog);
	AssertSize(3, cList.NumElements());

	pcDisk = DiskFile(szFile);
	AssertTrue(pcDisk->Open(EFM_Write_Create));
	cFile.Init(pcDisk);
	AssertTrue(cFile.WriteBool(true));
	AssertTrue(cList.Write(&cFile));
	AssertTrue(pcDisk->Close());
	cFile.Kill();
	cList.Kill();

	cList.Init();
	AssertSize(0, cList.NumElements());

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
//
//////////////////////////////////////////////////////////////////////////
void TestLinkedListBlock(void)
{
	BeginTests();
	MemoryInit();

	TestLinkedListBlockIndexOf();
	TestLinkedListBlockWrite();

	MemoryKill();
	TestStatistics();
}

