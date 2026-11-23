#include "BaseLib/SystemAllocator.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Chars.h"
#include "BaseLib/QueueTemplate.h"
#include "TestLib/Assert.h"


//An allocator that gives memory from the system's until told to fail: then every Malloc and Realloc gives NULL (a
//failed Realloc leaves its memory allocated, as realloc does).  It counts what is allocated, so a test can see that
//nothing leaked.
class CTestFailingAllocator : public CMallocator
{
public:
	bool	mbFail;
	int32	miAllocations;
	int32	miFailures;

	void Init(void)
	{
		mbFail = false;
		miAllocations = 0;
		miFailures = 0;
	}

	void Kill(void) override
	{
	}

	void* Malloc(lsize uiSize) override
	{
		void*	pv;

		if (mbFail)
		{
			miFailures++;
			return NULL;
		}
		pv = gcSystemAllocator.Malloc(uiSize);
		if (pv)
		{
			miAllocations++;
		}
		return pv;
	}

	void* Realloc(void* pv, lsize uiSize) override
	{
		if (mbFail)
		{
			miFailures++;
			return NULL;
		}
		if (pv == NULL)
		{
			return Malloc(uiSize);
		}
		return gcSystemAllocator.Realloc(pv, uiSize);
	}

	bool Free(void* pv) override
	{
		if (pv)
		{
			miAllocations--;
		}
		return gcSystemAllocator.Free(pv);
	}

	const char*	ClassName(void) override { return "CTestFailingAllocator"; }
	size		ClassSize(void) override { return sizeof(CTestFailingAllocator); }
	bool		IsLocal(void) const override { return false; }
	size		SizeOffset(void) override { return gcSystemAllocator.SizeOffset(); }
	const char*	ShortName(void) override { return "Failing"; }
};


//An element with a virtual function table, which the arrays give it as it is added.
class CTestGrowVirtual
{
public:
	int32	miValue;

	virtual int32 Value(void) { return miValue; }
};


//////////////////////////////////////////////////////////////////////////
//
// The array holds 0 to iCount - 1.
//
//////////////////////////////////////////////////////////////////////////
static bool TestGrowFailsHolds(CArrayTemplate<int32>* paValues, int32 iCount)
{
	int32	i;

	if (paValues->NumElements() != (size)iCount)
	{
		return false;
	}
	for (i = 0; i < iCount; i++)
	{
		if (*paValues->Get((size)i) != i)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An array whose memory is full when the allocator starts failing: each
// way of growing it fails, gives NULL (or not found, or false), and leaves
// it as it was.
//
//////////////////////////////////////////////////////////////////////////
void TestArrayGrowFailsBlock(void)
{
	CTestFailingAllocator	cAlloc;
	CArrayTemplate<int32>	aValues;
	CArrayTemplate<int32>	aOther;
	int32					i;
	int32					iValue;
	int32					aiBlock[3];
	size					uiIndex;

	cAlloc.Init();
	aValues.Init(&cAlloc);
	aOther.Init();
	for (i = 0; i < 3; i++)
	{
		aiBlock[i] = 100 + i;
		aOther.Add(&aiBlock[i]);
	}

	//Filled to exactly what it has room for, so that the next element needs more memory.
	i = 0;
	do
	{
		*aValues.Add() = i;
		i++;
	}
	while (aValues.NumElements() < aValues.AllocatedElements());
	cAlloc.mbFail = true;

	AssertNull(aValues.Add());
	AssertTrue(TestGrowFailsHolds(&aValues, i));
	iValue = -1;
	AssertNull(aValues.Add(&iValue));
	aValues.Push(&iValue);
	AssertNull(aValues.PushCopy());
	AssertNull(aValues.AddGetIndex(&uiIndex));
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, uiIndex);
	AssertNull(aValues.InsertAt(0));
	AssertNull(aValues.InsertAt(&iValue, 1));
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, aValues.AddNum(5));
	AssertNull(aValues.InsertNumAt(2, 1));
	AssertNull(aValues.InsertArrayAfterEnd(&aOther));
	AssertNull(aValues.InsertArrayBeforeStart(&aOther));
	AssertNull(aValues.InsertArrayAt(&aOther, 1));
	AssertNull(aValues.InsertBlockAfterEnd(aiBlock, 3));
	AssertNull(aValues.InsertBlockBeforeStart(aiBlock, 3));
	AssertNull(aValues.InsertBlockAt(aiBlock, 3, 1));
	AssertNull(aValues.GrowToAtLeastNumElements((size)i + 10));
	AssertFalse(aValues.SafeSet((size)i + 3, &iValue));
	AssertTrue(TestGrowFailsHolds(&aValues, i));
	AssertTrue(cAlloc.miFailures > 10);

	//Memory again: it grows as before.
	cAlloc.mbFail = false;
	*aValues.Add() = i;
	AssertTrue(TestGrowFailsHolds(&aValues, i + 1));

	aOther.Kill();
	aValues.Kill();
	AssertInt(0, cAlloc.miAllocations);
}


//////////////////////////////////////////////////////////////////////////
//
// A shrink whose realloc fails keeps the memory it has and the elements
// it keeps.
//
//////////////////////////////////////////////////////////////////////////
void TestArrayGrowFailsShrink(void)
{
	CTestFailingAllocator	cAlloc;
	CArrayTemplate<int32>	aValues;
	int32					i;

	cAlloc.Init();
	aValues.Init(&cAlloc);
	for (i = 0; i < 100; i++)
	{
		*aValues.Add() = i;
	}
	cAlloc.mbFail = true;
	aValues.SetUsedElements(10);
	AssertTrue(TestGrowFailsHolds(&aValues, 10));
	cAlloc.mbFail = false;
	aValues.Kill();
	AssertInt(0, cAlloc.miAllocations);
}


//////////////////////////////////////////////////////////////////////////
//
// Copy into an array that can't grow leaves it as it was.
//
//////////////////////////////////////////////////////////////////////////
void TestArrayGrowFailsCopy(void)
{
	CTestFailingAllocator	cAlloc;
	CArrayTemplate<int32>	aValues;
	CArrayTemplate<int32>	aSource;
	int32					i;

	cAlloc.Init();
	aValues.Init(&cAlloc);
	aSource.Init();
	for (i = 0; i < 4; i++)
	{
		*aValues.Add() = i;
	}
	for (i = 0; i < 200; i++)
	{
		*aSource.Add() = -i;
	}
	cAlloc.mbFail = true;
	aValues.Copy(&aSource);
	AssertTrue(TestGrowFailsHolds(&aValues, 4));
	cAlloc.mbFail = false;

	aSource.Kill();
	aValues.Kill();
	AssertInt(0, cAlloc.miAllocations);
}


//////////////////////////////////////////////////////////////////////////
//
// An element with a virtual function table: a failed Add gives NULL (no
// table is put at NULL), and InsertNumAt gives the first element it made.
//
//////////////////////////////////////////////////////////////////////////
void TestArrayGrowFailsVirtual(void)
{
	CTestFailingAllocator				cAlloc;
	CArrayTemplate<CTestGrowVirtual>	aValues;
	CTestGrowVirtual*					pcElement;

	cAlloc.Init();
	aValues.Init(&cAlloc);
	do
	{
		pcElement = aValues.Add();
		pcElement->miValue = (int32)aValues.NumElements();
	}
	while (aValues.NumElements() < aValues.AllocatedElements());
	cAlloc.mbFail = true;
	AssertNull(aValues.Add());
	AssertNull(aValues.InsertNumAt(3, 0));
	cAlloc.mbFail = false;

	pcElement = aValues.InsertNumAt(3, 1);
	AssertPointer(aValues.Get(1), pcElement);
	pcElement->miValue = 42;
	AssertInt(42, aValues.Get(1)->Value());
	AssertInt(1, aValues.Get(0)->Value());

	aValues.Kill();
	AssertInt(0, cAlloc.miAllocations);
}


//////////////////////////////////////////////////////////////////////////
//
// A string that can't grow is as it was.
//
//////////////////////////////////////////////////////////////////////////
void TestArrayGrowFailsChars(void)
{
	CTestFailingAllocator	cAlloc;
	CChars					sz;

	cAlloc.Init();
	sz.Init(&cAlloc);
	sz.Append("Hello");
	cAlloc.mbFail = true;
	sz.Append(" world, which is longer than any chunk the string has room for already, by a long way");
	AssertString("Hello", sz.Text());
	sz.Insert(0, "Say this, which is also longer than any room the string has, by a long way: ");
	AssertString("Hello", sz.Text());
	sz.Insert(0, '-', 200);
	AssertString("Hello", sz.Text());
	AssertSize(0, sz.Replace("l", "LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL"));
	AssertString("Hello", sz.Text());
	cAlloc.mbFail = false;
	sz.Append(" world");
	AssertString("Hello world", sz.Text());
	sz.Kill();
	AssertInt(0, cAlloc.miAllocations);
}


//////////////////////////////////////////////////////////////////////////
//
// A queue that can't grow keeps what it had.
//
//////////////////////////////////////////////////////////////////////////
void TestArrayGrowFailsQueue(void)
{
	CTestFailingAllocator	cAlloc;
	CQueueTemplate<int32>	cQueue;
	int32					i;
	bool					bResult;

	cAlloc.Init();
	cQueue.Init(&cAlloc);
	for (i = 0; i < 50; i++)
	{
		*cQueue.Push() = i;
	}
	for (i = 0; i < 20; i++)
	{
		cQueue.Pop();
	}
	cAlloc.mbFail = true;
	for (i = 0; i < 1000; i++)
	{
		if (cQueue.Push() == NULL)
		{
			break;
		}
		*cQueue.Back() = 50 + i;
	}
	AssertTrue(i < 1000);
	bResult = true;
	for (i = 0; i < (int32)cQueue.NumElements(); i++)
	{
		bResult &= (*cQueue.Get((size)i) == 20 + i);
	}
	AssertTrue(bResult);
	cAlloc.mbFail = false;
	cQueue.Kill();
	AssertInt(0, cAlloc.miAllocations);
}


struct STestGrowBig
{
	uint8	auiBytes[1000];
};


//////////////////////////////////////////////////////////////////////////
//
// Where size is 16 bits an array (counted in size) holds at most MAX_SIZE
// bytes: an Add past that, or past the memory free (which on the W65C816
// may come first), gives NULL and the array keeps what it had.
//
//////////////////////////////////////////////////////////////////////////
void TestArrayGrowFailsMaxSize(void)
{
#if defined(SIZE_16)
	CArrayTemplate<STestGrowBig>	aBig;
	STestGrowBig*					ps;
	int32							i;
	int32							iFilled;
	bool							bResult;

	aBig.Init();
	for (i = 0; i < 100; i++)
	{
		ps = aBig.Add();
		if (ps == NULL)
		{
			break;
		}
		ps->auiBytes[0] = (uint8)i;
		ps->auiBytes[999] = (uint8)(i + 1);
	}
	AssertTrue(i < 100);
	AssertTrue(i > 0);
	AssertSize((size)i, aBig.NumElements());
	iFilled = i;
	bResult = true;
	for (i = 0; i < iFilled; i++)
	{
		bResult &= (aBig.Get((size)i)->auiBytes[0] == (uint8)i);
		bResult &= (aBig.Get((size)i)->auiBytes[999] == (uint8)(i + 1));
	}
	AssertTrue(bResult);
	aBig.Kill();
#endif // SIZE_16
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayGrowFails(void)
{
	BeginTests();

	TestArrayGrowFailsBlock();
	TestArrayGrowFailsShrink();
	TestArrayGrowFailsCopy();
	TestArrayGrowFailsVirtual();
	TestArrayGrowFailsChars();
	TestArrayGrowFailsQueue();
	TestArrayGrowFailsMaxSize();

	TestStatistics();
}

