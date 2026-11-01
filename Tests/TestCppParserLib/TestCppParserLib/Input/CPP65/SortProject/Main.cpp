#include "BaseLib/QuickSort.h"
#include "BaseLib/BubbleSort.h"
#include "BaseLib/BinarySearch.h"


struct SRecord
{
	int		iKey;
	char	acName[6];
	short	iCheck;
};


int		gaiValues[500];
SRecord	gasRecords[60];


int CompareInt(const void* pvLeft, const void* pvRight)
{
	int	iLeft;
	int	iRight;

	iLeft = *(int*)pvLeft;
	iRight = *(int*)pvRight;
	return (iLeft < iRight) ? -1 : ((iLeft > iRight) ? 1 : 0);
}


int CompareRecord(const void* pvLeft, const void* pvRight)
{
	return ((SRecord*)pvLeft)->iKey - ((SRecord*)pvRight)->iKey;
}


void* GetValue(size iIndex)
{
	return &gaiValues[iIndex];
}


//A bit for each check that passes.
int main()
{
	unsigned	uiRandom;
	int			iPassed;
	int			i;
	int			iSum;
	int			iSorted;
	int			iKey;
	size		uiIndex;
	bool		bOrdered;
	bool		bFound;

	//QuickSort 500 pseudo random values, some negative.
	uiRandom = 99;
	iSum = 0;
	for (i = 0; i < 500; i++)
	{
		uiRandom = uiRandom * 1103515245 + 12345;
		gaiValues[i] = (int)((uiRandom >> 4) % 20000) - 10000;
		iSum += gaiValues[i];
	}
	QuickSort(CompareInt, gaiValues, sizeof(int), 500);
	bOrdered = true;
	iSorted = 0;
	for (i = 0; i < 500; i++)
	{
		iSorted += gaiValues[i];
		if (i > 0)
		{
			bOrdered &= gaiValues[i - 1] <= gaiValues[i];
		}
	}
	iPassed = 0;
	if (bOrdered && (iSorted == iSum))
	{
		iPassed |= 1;
	}

	//Each value is found, and a value between two is not.
	bFound = true;
	for (i = 0; i < 500; i += 7)
	{
		iKey = gaiValues[i];
		bFound &= BinarySearch(&iKey, 0, 499, CompareInt, &uiIndex, GetValue) && (gaiValues[uiIndex] == iKey);
	}
	iKey = 10001;
	if (bFound && !BinarySearch(&iKey, 0, 499, CompareInt, &uiIndex, GetValue) && (uiIndex == 500))
	{
		iPassed |= 2;
	}

	//BubbleSort records, keeping each record together.
	for (i = 0; i < 60; i++)
	{
		gasRecords[i].iKey = (i * 37) % 61;
		gasRecords[i].acName[0] = (char)('a' + (i % 26));
		gasRecords[i].acName[5] = (char)i;
		gasRecords[i].iCheck = (short)(gasRecords[i].iKey * 3 + i);
	}
	BubbleSort(CompareRecord, gasRecords, sizeof(SRecord), 60);
	bOrdered = true;
	for (i = 0; i < 60; i++)
	{
		bOrdered &= gasRecords[i].iCheck == gasRecords[i].iKey * 3 + gasRecords[i].acName[5];
		bOrdered &= gasRecords[i].acName[0] == (char)('a' + (gasRecords[i].acName[5] % 26));
		if (i > 0)
		{
			bOrdered &= gasRecords[i - 1].iKey < gasRecords[i].iKey;
		}
	}
	if (bOrdered)
	{
		iPassed |= 4;
	}
	return iPassed;
}

