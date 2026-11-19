#include "BaseLib/GlobalMemory.h"
#include "StandardLib/IndexedObjects.h"
#include "TestLib/Assert.h"


//The table only keeps the pointers, so any address will do for an object.
static CBaseObject* TestIndexedObject(OIndex oi)
{
	return (CBaseObject*)(ptr)(0x10000 + (oi & 0xFFFFF) * 16);
}


//////////////////////////////////////////////////////////////////////////
//
// Objects put, found and taken out by index, across pages (256 indexes)
// and directories (65536).
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedObjectsAddGetRemove(void)
{
	CIndexedObjects		cIndexed;
	OIndex				oi;

	cIndexed.Init();
	for (oi = 1; oi <= 600; oi++)
	{
		AssertTrue(cIndexed.Add(oi, TestIndexedObject(oi)));
	}
	AssertTrue(cIndexed.Add(70000, TestIndexedObject(70000)));
	AssertSize(601, cIndexed.NumIndexed());

	for (oi = 1; oi <= 600; oi++)
	{
		AssertPointer(TestIndexedObject(oi), cIndexed.Get(oi));
	}
	AssertPointer(TestIndexedObject(70000), cIndexed.Get(70000));
	AssertNull(cIndexed.Get(0));
	AssertNull(cIndexed.Get(601));
	AssertNull(cIndexed.Get(69999));
	AssertNull(cIndexed.Get(1000000));
	AssertFalse(cIndexed.Has(601));
	AssertTrue(cIndexed.Has(256));

	//Put again where there is one: in its place, not another.
	AssertTrue(cIndexed.Add(256, TestIndexedObject(1)));
	AssertPointer(TestIndexedObject(1), cIndexed.Get(256));
	AssertSize(601, cIndexed.NumIndexed());

	for (oi = 1; oi <= 600; oi += 2)
	{
		AssertTrue(cIndexed.Remove(oi));
	}
	AssertFalse(cIndexed.Remove(1));
	AssertFalse(cIndexed.Remove(601));
	AssertFalse(cIndexed.Remove(5000000));
	AssertSize(301, cIndexed.NumIndexed());
	for (oi = 1; oi <= 600; oi++)
	{
		AssertBool((oi % 2) == 0, cIndexed.Has(oi));
	}

	AssertTrue(cIndexed.Remove(70000));
	AssertNull(cIndexed.Get(70000));
	AssertSize(300, cIndexed.NumIndexed());
	cIndexed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Iterated in index order over the gaps between pages and directories,
// and on past what is taken out as it goes.
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedObjectsIteration(void)
{
	CIndexedObjects		cIndexed;
	SIndexesIterator	sIter;
	OIndex				aoi[] = { 1, 2, 255, 256, 257, 70000, 70001, 1000000 };
	size				uiNum;
	size				i;
	OIndex				oi;
	CBaseObject*		pcObject;

	cIndexed.Init();
	AssertLong(INVALID_O_INDEX, cIndexed.StartIteration(&sIter));
	AssertNull(cIndexed.StartIterationObject(&sIter));

	uiNum = sizeof(aoi) / sizeof(OIndex);
	for (i = uiNum; i > 0; i--)
	{
		cIndexed.Add(aoi[i - 1], TestIndexedObject(aoi[i - 1]));
	}

	i = 0;
	oi = cIndexed.StartIteration(&sIter);
	while (oi != INVALID_O_INDEX)
	{
		AssertLong(aoi[i], oi);
		i++;
		oi = cIndexed.Iterate(&sIter);
	}
	AssertSize(uiNum, i);

	i = 0;
	pcObject = cIndexed.StartIterationObject(&sIter);
	while (pcObject != NULL)
	{
		AssertPointer(TestIndexedObject(aoi[i]), pcObject);
		i++;
		pcObject = cIndexed.IterateObject(&sIter);
	}
	AssertSize(uiNum, i);

	//Each taken out as it is reached and another put behind the iteration (not reached), and one put ahead of it
	//(reached last).
	i = 0;
	oi = cIndexed.StartIteration(&sIter);
	cIndexed.Add(2000000, TestIndexedObject(2000000));
	while (oi != INVALID_O_INDEX)
	{
		AssertLong((i < uiNum) ? aoi[i] : 2000000, oi);
		AssertTrue(cIndexed.Remove(oi));
		AssertTrue(cIndexed.Add(oi - 1, TestIndexedObject(oi)));
		i++;
		oi = cIndexed.Iterate(&sIter);
	}
	AssertSize(uiNum + 1, i);
	AssertSize(uiNum + 1, cIndexed.NumIndexed());
	cIndexed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A page that empties stays (made and freed over and over at the start of
// a page, an object doesn't make and free the page each time) until
// another page empties; a page that goes is kept for the next one needed.
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedObjectsPages(void)
{
	CIndexedObjects		cIndexed;
	CChars				sz;

	cIndexed.Init();
	cIndexed.Add(1, TestIndexedObject(1));
	cIndexed.Add(300, TestIndexedObject(300));
	cIndexed.Remove(300);

	sz.Init();
	cIndexed.Print(&sz);
	AssertString("Objects [1] in pages of [256]\n  [0..255]: 1\n  [256..511]: 0\n", sz.Text());
	sz.Kill();

	cIndexed.Add(301, TestIndexedObject(301));
	cIndexed.Remove(301);
	cIndexed.Add(600, TestIndexedObject(600));
	cIndexed.Remove(600);
	sz.Init();
	cIndexed.Print(&sz);
	AssertString("Objects [1] in pages of [256]\n  [0..255]: 1\n  [512..767]: 0\n", sz.Text());
	sz.Kill();

	cIndexed.Add(66000, TestIndexedObject(66000));
	AssertPointer(TestIndexedObject(66000), cIndexed.Get(66000));
	AssertPointer(TestIndexedObject(1), cIndexed.Get(1));
	AssertSize(2, cIndexed.NumIndexed());
	cIndexed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// What can't be kept: no object, a negative index.
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedObjectsRefused(void)
{
	CIndexedObjects		cIndexed;

	cIndexed.Init();
	AssertFalse(cIndexed.Add(5, NULL));
	AssertFalse(cIndexed.Add(INVALID_O_INDEX, TestIndexedObject(1)));
	AssertFalse(cIndexed.Add(-70000, TestIndexedObject(1)));
	AssertNull(cIndexed.Get(INVALID_O_INDEX));
	AssertFalse(cIndexed.Remove(INVALID_O_INDEX));
	AssertSize(0, cIndexed.NumIndexed());
	cIndexed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexedObjects(void)
{
	BeginTests();
	MemoryInit();

	TestIndexedObjectsAddGetRemove();
	TestIndexedObjectsIteration();
	TestIndexedObjectsPages();
	TestIndexedObjectsRefused();

	MemoryKill();
	TestStatistics();
}
