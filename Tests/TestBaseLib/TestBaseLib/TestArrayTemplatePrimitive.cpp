#include "BaseLib/ArrayFloat32.h"
#include "BaseLib/ArrayFloat64.h"
#include "BaseLib/ArrayChar.h"
#include "BaseLib/ArrayInt64.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayTemplatePrimitive(void)
{
	CArrayChar		aChars;
	CArrayLargeChar	aLargeChars;
	CArrayFloat32	aFloats;
	CArrayFloat64	aDoubles;
	CArrayInt64		aLongs;
	size				iIndex;

	BeginTests();

	aChars.Init();
	aChars.Add('A');
	AssertChar('A', aChars.GetValue(0));
	aChars.Kill();

	aLargeChars.Init();
	aLargeChars.AddList(0, 'C', 'A', 'B', 0);
	AssertSize(3, aLargeChars.NumElements());
	aLargeChars.QuickSort();
	AssertChar('A', aLargeChars.GetValue(0));
	AssertChar('C', aLargeChars.GetValue(2));
	AssertSize(1, aLargeChars.Find('B'));
	AssertTrue(aLargeChars.Find('Z') == ARRAY_LARGE_ELEMENT_NOT_FOUND);
	aLargeChars.GrowToAtLeastNumElements(70000, true, 0);
	aLargeChars.SetValue(69999, 'Z');
	AssertLSize(70000, aLargeChars.NumElements());
	AssertLSize(69999, aLargeChars.Find('Z'));
	aLargeChars.Kill();

	aFloats.Init();
	aFloats.InsertAt(5.1f, 0);
	AssertFloat(5.1f, aFloats.GetValue(0), 2);
	AssertSize(1, aFloats.AllocatedElements());
	AssertSize(1, aFloats.ChunkSize());
	AssertSize(1, aFloats.NumElements());
	aFloats.Kill();

	aDoubles.Init();
	aDoubles.Add(0.0);
	aDoubles.Add(0.1);
	aDoubles.Add(0.2);
	aDoubles.Add(0.3);
	aDoubles.Add(0.4);
	AssertTrue(aDoubles.IsSorted());
	aDoubles.InsertAt(0.5, 2);
	AssertFalse(aDoubles.IsSorted());
	aDoubles.Kill();

	aLongs.Init();
	aLongs.AddList(0LL, 4LL, 0x3122345482773411LL, 2LL, 1LL, 0LL);
	iIndex = aLongs.Find(0x3122345482773411LL);
	AssertSize(1, iIndex);
	aLongs.Kill();

	TestStatistics();
}

