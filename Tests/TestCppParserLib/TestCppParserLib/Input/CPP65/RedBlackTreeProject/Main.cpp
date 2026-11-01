#include "BaseLib/RedBlackTree.h"


struct SKeyValue
{
	int		iKey;
	int		iValue;
};


int CompareKey(const void* pvLeft, const void* pvRight)
{
	return *(int*)pvLeft - *(int*)pvRight;
}


//A bit for each check that passes.
int main()
{
	CRedBlackTree	cTree;
	SKeyValue		sElement;
	SKeyValue*		psElement;
	uint32			uiRandom;
	int				iPassed;
	int				iAdded;
	int				iPrevious;
	int				i;
	bool			bSorted;
	bool			bValid;

	cTree.Init(sizeof(SKeyValue), 0, CompareKey);

	//Add 300 pseudo random keys in 0 to 999; duplicates are not added.
	uiRandom = 12345;
	iAdded = 0;
	bValid = true;
	for (i = 0; i < 300; i++)
	{
		uiRandom = uiRandom * 1103515245 + 12345;
		sElement.iKey = (int)((uiRandom >> 8) % 1000);
		sElement.iValue = sElement.iKey * 3;
		if (cTree.Add(&sElement))
		{
			iAdded++;
		}
		if ((i % 25) == 0)
		{
			bValid &= cTree.IsValid();
		}
	}
	iPassed = 0;
	if (bValid && cTree.IsValid() && ((int)cTree.NumElements() == iAdded) && (iAdded > 250))
	{
		iPassed |= 1;
	}

	//In order, each with its value.
	bSorted = true;
	iPrevious = -1;
	for (psElement = (SKeyValue*)cTree.GetFirst(); psElement; psElement = (SKeyValue*)cTree.GetNext(psElement))
	{
		bSorted &= (psElement->iKey > iPrevious) && (psElement->iValue == psElement->iKey * 3);
		iPrevious = psElement->iKey;
	}
	if (bSorted)
	{
		iPassed |= 2;
	}

	//Remove the even keys.
	for (i = 0; i < 1000; i += 2)
	{
		if (cTree.Contains(&i))
		{
			cTree.Remove(&i);
			iAdded--;
		}
	}
	bSorted = true;
	for (psElement = (SKeyValue*)cTree.GetFirst(); psElement; psElement = (SKeyValue*)cTree.GetNext(psElement))
	{
		bSorted &= (psElement->iKey & 1) == 1;
	}
	if (bSorted && cTree.IsValid() && ((int)cTree.NumElements() == iAdded))
	{
		iPassed |= 4;
	}

	//Nearest keys.
	i = 500;
	psElement = (SKeyValue*)cTree.GetOnOrAfter(&i);
	if (psElement && (psElement->iKey > 500) && ((psElement->iKey & 1) == 1) && !cTree.Get(&i))
	{
		iPassed |= 8;
	}

	cTree.Kill();
	return iPassed;
}

