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
//
//////////////////////////////////////////////////////////////////////////
void TestArrayBlock(void)
{
	BeginTests();
	MemoryInit();

	TestArrayBlockRemoveBatch();
	TestArrayBlockFindInSorted<CArrayBlock, size>();
	TestArrayBlockFindInSorted<CArrayLargeBlock, lsize>();

	MemoryKill();
	TestStatistics();
}

