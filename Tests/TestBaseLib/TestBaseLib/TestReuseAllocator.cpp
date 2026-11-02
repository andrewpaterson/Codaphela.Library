#include "BaseLib/ReuseAllocator.h"
#include "BaseLib/SystemAllocator.h"
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayInt.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// Freed, the block is kept and handed back by the next Malloc; Realloc never makes it smaller.
//
//////////////////////////////////////////////////////////////////////////
void TestReuseAllocatorKeepsBlock(void)
{
	CReuseAllocator	cReuse;
	void*			pv1;
	void*			pv2;

	cReuse.Init(0);
	AssertSize(0, (size)cReuse.BlockSize());

	pv1 = cReuse.Malloc(100);
	AssertNotNull(pv1);
	AssertSize(100, (size)cReuse.BlockSize());
	AssertTrue(cReuse.Free(pv1));
	AssertSize(100, (size)cReuse.BlockSize());

	pv2 = cReuse.Malloc(50);
	AssertPointer(pv1, pv2);
	AssertSize(100, (size)cReuse.BlockSize());

	pv2 = cReuse.Realloc(pv2, 20);
	AssertPointer(pv1, pv2);
	AssertSize(100, (size)cReuse.BlockSize());

	pv2 = cReuse.Realloc(pv2, 300);
	AssertNotNull(pv2);
	AssertSize(300, (size)cReuse.BlockSize());
	AssertTrue(cReuse.Free(pv2));

	pv1 = cReuse.Realloc(NULL, 200);
	AssertPointer(pv2, pv1);
	AssertSize(300, (size)cReuse.BlockSize());
	AssertTrue(cReuse.Free(pv1));

	AssertFalse(cReuse.Free(NULL));

	cReuse.Kill();
	AssertSize(0, (size)cReuse.BlockSize());
}


//////////////////////////////////////////////////////////////////////////
//
// A second block, wanted while the kept one is in use, is the system's.
//
//////////////////////////////////////////////////////////////////////////
void TestReuseAllocatorSecondBlock(void)
{
	CReuseAllocator	cReuse;
	void*			pv1;
	void*			pv2;

	cReuse.Init(0);

	pv1 = cReuse.Malloc(100);
	pv2 = cReuse.Malloc(100);
	AssertNotNull(pv2);
	AssertTrue(pv1 != pv2);
	AssertSize(100, (size)cReuse.BlockSize());

	pv2 = cReuse.Realloc(pv2, 1000);
	AssertNotNull(pv2);
	AssertSize(100, (size)cReuse.BlockSize());
	AssertTrue(cReuse.Free(pv2));

	AssertTrue(cReuse.Free(pv1));
	AssertPointer(pv1, cReuse.Malloc(10));

	cReuse.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A block bigger than the keep limit is not kept: freed when it is freed, shrunk when reallocated to fit.
//
//////////////////////////////////////////////////////////////////////////
void TestReuseAllocatorKeepLimit(void)
{
	CReuseAllocator	cReuse;
	void*			pv;

	cReuse.Init(1000);

	pv = cReuse.Malloc(1000);
	cReuse.Free(pv);
	AssertSize(1000, (size)cReuse.BlockSize());

	pv = cReuse.Malloc(2000);
	AssertSize(2000, (size)cReuse.BlockSize());
	cReuse.Free(pv);
	AssertSize(0, (size)cReuse.BlockSize());

	pv = cReuse.Malloc(2000);
	pv = cReuse.Realloc(pv, 1500);
	AssertSize(2000, (size)cReuse.BlockSize());
	pv = cReuse.Realloc(pv, 500);
	AssertNotNull(pv);
	AssertSize(500, (size)cReuse.BlockSize());
	cReuse.Free(pv);
	AssertSize(500, (size)cReuse.BlockSize());

	cReuse.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A CChars made with the allocator is never faked empty: emptied and filled again it stays in the same memory.
//
//////////////////////////////////////////////////////////////////////////
void TestReuseAllocatorChars(void)
{
	CReuseAllocator	cReuse;
	CChars			sz;
	char*			szFirst;
	lsize			uiBlockSize;

	cReuse.Init(0);

	sz.Init(&cReuse);
	AssertSize(0, sz.Length());
	AssertTrue(sz.Empty());
	AssertString("", sz.Text());
	szFirst = sz.Text();
	AssertTrue(cReuse.BlockSize() > 0);

	sz.Append("Hello World, a line of assembly");
	AssertString("Hello World, a line of assembly", sz.Text());
	szFirst = sz.Text();
	uiBlockSize = cReuse.BlockSize();

	sz.Clear();
	AssertSize(0, sz.Length());
	AssertString("", sz.Text());
	AssertPointer(szFirst, sz.Text());
	AssertSize((size)uiBlockSize, (size)cReuse.BlockSize());

	sz.Append("Hi");
	AssertString("Hi", sz.Text());
	AssertPointer(szFirst, sz.Text());
	AssertSize((size)uiBlockSize, (size)cReuse.BlockSize());

	sz.SetLength(0);
	AssertString("", sz.Text());
	AssertPointer(szFirst, sz.Text());
	sz.Set("");
	AssertString("", sz.Text());
	AssertPointer(szFirst, sz.Text());

	sz.Append(' ', 500);
	AssertSize(500, sz.Length());
	AssertTrue(cReuse.BlockSize() > 500);
	uiBlockSize = cReuse.BlockSize();
	szFirst = sz.Text();

	sz.Clear();
	sz.Append("lda #1\n");
	AssertString("lda #1\n", sz.Text());
	AssertPointer(szFirst, sz.Text());
	AssertSize((size)uiBlockSize, (size)cReuse.BlockSize());

	//Killed, the memory is kept for the next string.
	sz.Kill();
	AssertSize((size)uiBlockSize, (size)cReuse.BlockSize());
	sz.Init(&cReuse);
	sz.Append("sta $10\n");
	AssertString("sta $10\n", sz.Text());
	AssertPointer(szFirst, sz.Text());
	sz.Kill();

	cReuse.Kill();
	AssertSize(0, (size)cReuse.BlockSize());
}


//////////////////////////////////////////////////////////////////////////
//
// One ridiculously long line is not kept.
//
//////////////////////////////////////////////////////////////////////////
void TestReuseAllocatorCharsKeepLimit(void)
{
	CReuseAllocator	cReuse;
	CChars			sz;

	cReuse.Init(64);

	sz.Init(&cReuse);
	sz.Append("short");
	AssertTrue(cReuse.BlockSize() <= 64);

	sz.Append('x', 200);
	AssertSize(205, sz.Length());
	AssertTrue(cReuse.BlockSize() > 64);

	sz.Clear();
	AssertString("", sz.Text());
	AssertTrue(cReuse.BlockSize() <= 64);

	sz.Append("again");
	AssertString("again", sz.Text());
	sz.Kill();

	cReuse.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The system allocator's CChars are faked empty as before.
//
//////////////////////////////////////////////////////////////////////////
void TestReuseAllocatorCharsSystem(void)
{
	CChars	sz1;
	CChars	sz2;

	sz1.Init();
	sz2.Init(&gcSystemAllocator);
	AssertPointer(sz1.Text(), sz2.Text());

	sz2.Append("text");
	sz2.Clear();
	AssertPointer(sz1.Text(), sz2.Text());

	sz2.Kill();
	sz1.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// An array killed and made again with the allocator gets the same memory.
//
//////////////////////////////////////////////////////////////////////////
void TestReuseAllocatorArray(void)
{
	CReuseAllocator	cReuse;
	CArrayInt		ai;
	int*			piFirst;
	int				i;

	cReuse.Init(0);

	ai.Init(&cReuse);
	for (i = 0; i < 100; i++)
	{
		ai.Add(i);
	}
	piFirst = ai.GetData();
	ai.Kill();

	ai.Init(&cReuse);
	for (i = 0; i < 10; i++)
	{
		ai.Add(i * 2);
	}
	AssertPointer(piFirst, ai.GetData());
	AssertSize(10, ai.NumElements());
	AssertInt(18, ai.GetValue(9));
	ai.Kill();

	cReuse.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestReuseAllocator(void)
{
	BeginTests();

	TestReuseAllocatorKeepsBlock();
	TestReuseAllocatorSecondBlock();
	TestReuseAllocatorKeepLimit();
	TestReuseAllocatorChars();
	TestReuseAllocatorCharsKeepLimit();
	TestReuseAllocatorCharsSystem();
	TestReuseAllocatorArray();

	TestStatistics();
}
