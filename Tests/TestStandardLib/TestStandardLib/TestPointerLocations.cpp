#include "BaseLib/GlobalMemory.h"
#include "BaseLib/StdRandom.h"
#include "BaseLib/ArrayTemplate.h"
#include "StandardLib/PointerLocations.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// A pointer that differs from its neighbours only in bits the table's
// hash looks at least: they collide.
//
//////////////////////////////////////////////////////////////////////////
void* TestPointerLocationsPointer(size uiIndex)
{
	return (void*)(ptr)(0x10000 + uiIndex * 0x10);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPointerLocationsSimple(void)
{
	CPointerLocations			cLocations;
	SPointerLocationsIterator	sIter;
	ptr							uiLocation;
	void*						pvA;
	void*						pvB;

	pvA = TestPointerLocationsPointer(1);
	pvB = TestPointerLocationsPointer(2);

	cLocations.Init();
	AssertSize(0, cLocations.NumLocations());
	AssertFalse(cLocations.StartIteration(&sIter, pvA, &uiLocation));
	AssertFalse(cLocations.Remove(pvA, 0));

	//A pointer may have many locations, the same one more than once too.
	cLocations.Add(pvA, 7);
	cLocations.Add(pvA, 3);
	cLocations.Add(pvB, 7);
	cLocations.Add(pvA, 3);
	AssertSize(4, cLocations.NumLocations());
	AssertSize(3, cLocations.NumLocations(pvA));
	AssertSize(1, cLocations.NumLocations(pvB));
	AssertTrue(cLocations.Contains(pvA, 7));
	AssertTrue(cLocations.Contains(pvA, 3));
	AssertFalse(cLocations.Contains(pvB, 3));

	AssertTrue(cLocations.Remove(pvA, 3));
	AssertSize(2, cLocations.NumLocations(pvA));
	AssertTrue(cLocations.Contains(pvA, 3));
	AssertFalse(cLocations.Remove(pvB, 3));
	AssertTrue(cLocations.Remove(pvA, 3));
	AssertFalse(cLocations.Contains(pvA, 3));
	AssertTrue(cLocations.Contains(pvA, 7));
	AssertTrue(cLocations.Contains(pvB, 7));

	AssertTrue(cLocations.StartIteration(&sIter, pvA, &uiLocation));
	AssertInt(7, (int)uiLocation);
	AssertFalse(cLocations.Iterate(&sIter, &uiLocation));

	cLocations.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A hollow object's locations become those of the object loaded for it;
// an array's slots after one removed move down.
//
//////////////////////////////////////////////////////////////////////////
void TestPointerLocationsMoving(void)
{
	CPointerLocations	cLocations;
	void*				pvOld;
	void*				pvNew;
	void*				pvOther;

	pvOld = TestPointerLocationsPointer(1);
	pvNew = TestPointerLocationsPointer(2);
	pvOther = TestPointerLocationsPointer(3);

	cLocations.Init();
	cLocations.Add(pvOld, 0);
	cLocations.Add(pvOther, 1);
	cLocations.Add(pvOld, 2);
	cLocations.Add(pvOther, 3);

	AssertSize(2, cLocations.MovePointer(pvOld, pvNew));
	AssertSize(0, cLocations.NumLocations(pvOld));
	AssertTrue(cLocations.Contains(pvNew, 0));
	AssertTrue(cLocations.Contains(pvNew, 2));
	AssertSize(4, cLocations.NumLocations());

	//Slot 1 removed: slots 2 and 3 become 1 and 2.
	AssertTrue(cLocations.Remove(pvOther, 1));
	cLocations.MoveLocations(2, -1);
	AssertTrue(cLocations.Contains(pvNew, 0));
	AssertTrue(cLocations.Contains(pvNew, 1));
	AssertTrue(cLocations.Contains(pvOther, 2));
	AssertSize(3, cLocations.NumLocations());

	cLocations.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Random adds and removes of colliding pointers, checked against a list:
// growing and shifting slots back on remove must lose nothing.  With few
// locations the table stays at its smallest, and pointers whose home
// slots are at its end probe around to its start.
//
//////////////////////////////////////////////////////////////////////////
void TestPointerLocationsRandom(size uiFirstPointer, size uiLastPointer, size uiMaxLocations)
{
	CPointerLocations					cLocations;
	CArrayTemplate<SPointerLocation>	asExpected;
	SPointerLocation*					psExpected;
	SPointerLocation					sLocation;
	CRandom								cRandom;
	size								i;
	size								uiStep;
	size								uiIndex;
	size								uiWrong;
	size								uiNotRemoved;

	uiNotRemoved = 0;
	cLocations.Init();
	asExpected.Init();
	cRandom.Init(7);
	for (uiStep = 0; uiStep < 3000; uiStep++)
	{
		if ((asExpected.NumElements() == 0) || ((asExpected.NumElements() < uiMaxLocations) && (cRandom.Next(0, 2) != 0)))
		{
			sLocation.pvPointer = TestPointerLocationsPointer(cRandom.Next((int)uiFirstPointer, (int)uiLastPointer));
			sLocation.uiLocation = (ptr)cRandom.Next(0, 5);
			cLocations.Add(sLocation.pvPointer, sLocation.uiLocation);
			asExpected.Add(&sLocation);
		}
		else
		{
			uiIndex = cRandom.Next(0, (int)asExpected.NumElements() - 1);
			psExpected = asExpected.Get(uiIndex);
			if (!cLocations.Remove(psExpected->pvPointer, psExpected->uiLocation))
			{
				uiNotRemoved++;
			}
			asExpected.RemoveAt(uiIndex, false);
		}
	}

	AssertSize(0, uiNotRemoved);
	AssertSize(asExpected.NumElements(), cLocations.NumLocations());
	uiWrong = 0;
	for (i = 0; i < asExpected.NumElements(); i++)
	{
		psExpected = asExpected.Get(i);
		if (!cLocations.Contains(psExpected->pvPointer, psExpected->uiLocation))
		{
			uiWrong++;
		}
	}
	AssertSize(0, uiWrong);

	asExpected.Kill();
	cRandom.Kill();
	cLocations.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPointerLocations(void)
{
	BeginTests();
	MemoryInit();

	TestPointerLocationsSimple();
	TestPointerLocationsMoving();
	TestPointerLocationsRandom(1, 40, 3000);
	TestPointerLocationsRandom(5, 10, 7);  //Home slots 13, 14, 15, 0, 1 and 2 of 16.

	MemoryKill();
	TestStatistics();
}

