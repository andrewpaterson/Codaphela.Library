#include "BaseLib/GlobalMemory.h"
#include "BaseLib/ArrayBlock.h"
#include "TestLib/Assert.h"
#include "ArrayTemplateTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayBlockRemoveBatch(void)
{
	CArrayBlock		cArray;
	size			i;

	cArray.Init(1);
	for (i = 0; i < 6; i++)
	{
		cArray.Add((char*)&i);
	}

	cArray.RemoveBatch(1, 1, 3, 1);

	AssertSize(3, cArray.NumElements());

	AssertChar(0, *((char*)cArray.Get(0)));
	AssertChar(2, *((char*)cArray.Get(1)));
	AssertChar(4, *((char*)cArray.Get(2)));
	cArray.Kill();


	cArray.Init(1);
	for (i = 0; i < 6; i++)
	{
		cArray.Add((char*)&i);
	}

	cArray.RemoveBatch(0, 2, 2, 1);

	AssertSize(2, cArray.NumElements());

	AssertChar(2, *((char*)cArray.Get(0)));
	AssertChar(5, *((char*)cArray.Get(1)));
	cArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CompareArrayBlockKey(const void* pvLeft, const void* pvRight)
{
	uint16	uiLeft;
	uint16	uiRight;

	memcpy(&uiLeft, pvLeft, sizeof(uint16));
	memcpy(&uiRight, pvRight, sizeof(uint16));
	if (uiLeft < uiRight)
	{
		return -1;
	}
	else if (uiLeft > uiRight)
	{
		return 1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class B, class S>
class CTestSearchArrayBlock : public B
{
public:
	bool Search(void* pvData, S iLeft, S iRight, DataCompare fCompare, S* piIndex)
	{
		return B::BinarySearch(pvData, iLeft, iRight, fCompare, piIndex);
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class B, class S>
void TestArrayBlockFindInSorted(void)
{
	CTestSearchArrayBlock<B, S>	cArray;
	size						auiElementSizes[] = { 2, 3, 8, 13 };
	S							iSize;
	S							iCount;
	S							i;
	uint16						uiKey;
	char						acElement[16];
	S							iIndex;
	bool						bFound;
	S							iLeft;
	S							iRight;
	S							iExpected;

	for (iSize = 0; iSize < 4; iSize++)
	{
		for (iCount = 0; iCount <= 33; iCount++)
		{
			cArray.Init(auiElementSizes[iSize]);
			memset(acElement, 0, 16);
			for (i = 0; i < iCount; i++)
			{
				uiKey = (uint16)(i * 2 + 1);
				memcpy(acElement, &uiKey, sizeof(uint16));
				acElement[2] = (char)i;
				cArray.Add(acElement);
			}

			//Every present key (odd) is found at its index and every absent key (even) gives its insertion index.
			for (uiKey = 0; uiKey <= iCount * 2 + 1; uiKey++)
			{
				memcpy(acElement, &uiKey, sizeof(uint16));
				iIndex = 999;
				bFound = cArray.FindInSorted(acElement, CompareArrayBlockKey, &iIndex);
				AssertBool((uiKey & 1) && (uiKey < iCount * 2), bFound);
				AssertSize((size)(uiKey / 2), (size)iIndex);
			}

			//A search of part of the array only looks between its left and right indices.
			if (iCount >= 4)
			{
				iLeft = 1;
				iRight = iCount - 2;
				for (uiKey = 0; uiKey <= iCount * 2 + 1; uiKey++)
				{
					memcpy(acElement, &uiKey, sizeof(uint16));
					bFound = cArray.Search(acElement, iLeft, iRight, CompareArrayBlockKey, &iIndex);
					AssertBool((uiKey & 1) && ((S)(uiKey / 2) >= iLeft) && ((S)(uiKey / 2) <= iRight), bFound);
					iExpected = uiKey / 2;
					if (iExpected < iLeft)
					{
						iExpected = iLeft;
					}
					else if (iExpected > iRight + 1)
					{
						iExpected = iRight + 1;
					}
					AssertSize((size)iExpected, (size)iIndex);
				}
			}
			cArray.Kill();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// TakeFrom moves the elements without copying them, and leaves the
// source an empty array that can be added to and must be killed.
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void TestArrayBlockTakeFrom(void)
{
	M		cSource;
	M		cDest;
	void*	pvData;

	cSource.Init(&gcSystemAllocator, sizeof(int32), 4);
	*(int32*)cSource.Add() = 11;
	*(int32*)cSource.Add() = 22;
	*(int32*)cSource.Add() = 33;
	pvData = cSource.GetData();

	cDest.TakeFrom(&cSource);
	AssertPointer(pvData, cDest.GetData());
	AssertTrue(cDest.NumElements() == 3);
	AssertTrue(cDest.ChunkSize() == 4);
	AssertInt((int32)22, *(int32*)cDest.Get(1));

	AssertNull(cSource.GetData());
	AssertTrue(cSource.IsEmpty());
	AssertTrue(cSource.ChunkSize() == 4);

	//Neither is tied to the other any more.
	*(int32*)cSource.Add() = 44;
	AssertTrue(cSource.NumElements() == 1);
	AssertInt((int32)44, *(int32*)cSource.Get(0));
	AssertTrue(cDest.NumElements() == 3);
	*(int32*)cDest.Add() = 55;
	AssertInt((int32)11, *(int32*)cDest.Get(0));
	AssertInt((int32)55, *(int32*)cDest.Get(3));

	cSource.Kill();
	cDest.Kill();

	//An empty source gives an empty array.
	cSource.Init(sizeof(int32));
	cDest.TakeFrom(&cSource);
	AssertTrue(cDest.IsEmpty());
	AssertNull(cDest.GetData());
	cSource.Kill();
	cDest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayBlock(void)
{
	BeginTests();
	MemoryInit();

	TestArrayBlockRemoveBatch();
	TestArrayBlockFindInSorted<CArrayBlock, size>();
	TestArrayBlockFindInSorted<CArrayLargeBlock, lsize>();
	TestArrayBlockTakeFrom<CArrayBlock>();
	TestArrayBlockTakeFrom<CArrayLargeBlock>();

	MemoryKill();
	TestStatistics();
}

