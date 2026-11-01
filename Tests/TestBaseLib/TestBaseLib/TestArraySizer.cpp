#include "BaseLib/ArraySizer.h"
#include "BaseLib/ArrayBlock.h"
#include "BaseLib/DebugOutput.h"
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/MapLongLong.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArraySizer(void)
{
	BeginTests();

	CMapLongLong	miiExpected;
	lsize		iElements;
	lsize		iChunkSize;
	int64*		piExpectedChunkSize;
	size		iCount;

	miiExpected.Init(true);
	miiExpected.Put(0, 1);
	miiExpected.Put(1, 1);
	miiExpected.Put(2, 2);
	miiExpected.Put(4, 4);
	miiExpected.Put(8, 8);
	miiExpected.Put(16, 16);
	miiExpected.Put(32, 16);
	miiExpected.Put(48, 16);
	miiExpected.Put(64, 32);
	miiExpected.Put(96, 32);
	miiExpected.Put(128, 32);
	miiExpected.Put(160, 32);
	miiExpected.Put(192, 48);
	miiExpected.Put(240, 48);
	miiExpected.Put(288, 72);
	miiExpected.Put(360, 72);
	miiExpected.Put(432, 108);
	miiExpected.Put(540, 108);
	miiExpected.Put(648, 162);
	miiExpected.Put(810, 162);
	miiExpected.Put(972, 243);
	miiExpected.Put(1215, 243);
	miiExpected.Put(1458, 486);
	miiExpected.Put(1944, 486);
	miiExpected.Put(2430, 486);
	miiExpected.Put(2916, 729);
	miiExpected.Put(3645, 729);
	miiExpected.Put(4374, 1458);
	miiExpected.Put(5832, 1458);
	miiExpected.Put(7290, 1458);
	miiExpected.Put(8748, 2187);
	miiExpected.Put(10935, 2187);
	miiExpected.Put(13122, 4374);
	miiExpected.Put(17496, 4374);
	miiExpected.Put(21870, 4374);
	miiExpected.Put(26244, 6561);
	miiExpected.Put(32805, 6561);
	miiExpected.Put(39366, 13122);
	miiExpected.Put(52488, 13122);
	miiExpected.Put(65610, 21870);
	miiExpected.Put(87480, 21870);
	miiExpected.Put(109350, 36450);
	miiExpected.Put(145800, 36450);
	miiExpected.Put(182250, 60750);
	miiExpected.Put(243000, 60750);
	miiExpected.Put(303750, 60750);
	miiExpected.Put(364500, 60750);

	iCount = 0;
	;
	iChunkSize = 1;
	for (iElements = 0; iElements < 400000; iElements++)
	{
		if (iElements % iChunkSize == 0)
		{
			iChunkSize = CalculateChunkSize(iElements);
			AssertLSize(0, iElements % iChunkSize);

			piExpectedChunkSize = miiExpected.Get((int64)iElements);
			if (piExpectedChunkSize)
			{
				AssertLSize((lsize)*piExpectedChunkSize, iChunkSize);
				iCount++;
			}
		}
	}
	AssertSize(miiExpected.NumElements(), iCount);
	miiExpected.Kill();

	TestStatistics();
}

