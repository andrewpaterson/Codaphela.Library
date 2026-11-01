#include "BaseLib/IntegerHelper.h"
#include "BaseLib/LinkedListTemplate.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/GlobalMemory.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLinkedListTemplateIndexOf(void)
{
	CLinkedListTemplate<int>	cList;
	int*						pi1;
	int*						pi2;
	int*						pi3;

	pi1 = NULL;
	pi2 = NULL;
	pi3 = NULL;

	cList.Init();
	pi1 = cList.InsertAfterTail();
	*pi1 = 3;
	pi2 = cList.InsertAfterTail();
	*pi2 = 2;
	pi3 = cList.InsertAfterTail();
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
void TestLinkedListTemplateByteSize(void)
{
	CLinkedListTemplate<int>	cList;

	cList.Init();
	AssertSize(0, cList.ByteSize());
	*cList.InsertAfterTail() = 1;
	*cList.InsertAfterTail() = 2;
	*cList.InsertAfterTail() = 3;
	AssertSize(3 * (sizeof(SLLNode) + sizeof(int)), cList.ByteSize());
	cList.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLinkedListTemplateWrite(void)
{
	CLinkedListTemplate<int64>	cList;
	CFileBasic					cFile;
	CDiskFile*					pcDisk;
	CFileUtil					cFileUtil;
	char						szDirectory[] = "Output" _FS_ "LinkedListTemplate";
	char						szFile[] = "Output" _FS_ "LinkedListTemplate" _FS_ "Test.DAT";
	bool						b;
	int64*						plli;

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.TouchDir(szDirectory);

	cList.Init();
	(*cList.Add()) = 3498489579023845LL;
	(*cList.Add()) = 9837459843598451LL;
	(*cList.Add()) = 6409377732684332LL;
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

	plli = cList.GetHead();
	AssertLong(3498489579023845LL, *plli);
	plli = cList.GetNext(plli);
	AssertLong(9837459843598451LL, *plli);
	plli = cList.GetNext(plli);
	AssertLong(6409377732684332LL, *plli);
	plli = cList.GetNext(plli);
	AssertNull(plli);
	cList.Kill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertLinkedListString(CLinkedListTemplate<char>* pcList, const char* szExpected)
{
	CChars	sz;
	char*	pc;


	sz.Init();
	pc = pcList->GetHead();
	while (pc)
	{
		sz.Append(*pc);
		pc = pcList->GetNext(pc);
	}

	AssertString(szExpected, sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLinkedListTemplateMoveTo(void)
{
	CLinkedListTemplate<char>	cList;
	int							i;
	char*						pc;

	cList.Init();

	for (i = 0; i < 26; i++)
	{
		(*cList.Add()) = 'A' + (char)i;
	}
	AssertLinkedListString(&cList, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

	pc = cList.GetTail();
	cList.MoveToHead(pc);
	AssertLinkedListString(&cList, "ZABCDEFGHIJKLMNOPQRSTUVWXY");

	pc = cList.GetTail();
	pc = cList.GetPrev(pc);
	cList.MoveToHead(pc);
	AssertLinkedListString(&cList, "XZABCDEFGHIJKLMNOPQRSTUVWY");

	pc = cList.GetHead();
	cList.MoveToHead(pc);
	AssertLinkedListString(&cList, "XZABCDEFGHIJKLMNOPQRSTUVWY");

	pc = cList.GetHead();
	pc = cList.GetNext(pc);
	cList.MoveToHead(pc);
	AssertLinkedListString(&cList, "ZXABCDEFGHIJKLMNOPQRSTUVWY");

	pc = cList.GetHead();
	pc = cList.GetNext(pc);
	pc = cList.GetNext(pc);
	cList.MoveToTTail(pc);
	AssertLinkedListString(&cList, "ZXBCDEFGHIJKLMNOPQRSTUVWYA");

	pc = cList.GetHead();
	cList.MoveToTTail(pc);
	AssertLinkedListString(&cList, "XBCDEFGHIJKLMNOPQRSTUVWYAZ");

	pc = cList.GetTail();
	pc = cList.GetPrev(pc);
	cList.MoveToTTail(pc);
	AssertLinkedListString(&cList, "XBCDEFGHIJKLMNOPQRSTUVWYZA");

	pc = cList.GetTail();
	cList.MoveToTTail(pc);
	AssertLinkedListString(&cList, "XBCDEFGHIJKLMNOPQRSTUVWYZA");

	for (i = 0; i < 11; i++)
	{
		pc = cList.GetTail();
		pc = cList.GetPrev(pc);
		cList.Remove(pc);

		pc = cList.GetHead();
		pc = cList.GetNext(pc);
		cList.Remove(pc);
	}
	AssertLinkedListString(&cList, "XMNA");

	pc = cList.GetTail();
	cList.Remove(pc);
	pc = cList.GetHead();
	cList.Remove(pc);
	AssertLinkedListString(&cList, "MN");

	pc = cList.GetTail();
	cList.MoveToHead(pc);
	AssertLinkedListString(&cList, "NM");

	pc = cList.GetHead();
	cList.MoveToTTail(pc);
	AssertLinkedListString(&cList, "MN");

	pc = cList.GetHead();
	cList.Remove(pc);
	AssertLinkedListString(&cList, "N");

	pc = cList.GetTail();
	cList.MoveToHead(pc);
	AssertLinkedListString(&cList, "N");

	pc = cList.GetHead();
	cList.MoveToTTail(pc);
	AssertLinkedListString(&cList, "N");

	cList.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLinkedListTemplate(void)
{
	BeginTests();
	MemoryInit();

	TestLinkedListTemplateIndexOf();
	TestLinkedListTemplateByteSize();
	TestLinkedListTemplateWrite();
	TestLinkedListTemplateMoveTo();

	MemoryKill();
	TestStatistics();
}

