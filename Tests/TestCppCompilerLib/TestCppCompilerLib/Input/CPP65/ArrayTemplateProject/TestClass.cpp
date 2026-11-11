#include "TestClass.h"


int CTestClass::siMade = 0;


CTestClass::CTestClass()
{
	miValue = -1;
	miSquare = 1;
	siMade++;
}


void CTestClass::Set(int iValue)
{
	miValue = iValue;
	miSquare = (short)(iValue * iValue);
}


int CTestClass::Describe(void)
{
	return miValue + miSquare;
}


//Instantiates CArrayTemplate<CTestClass>::Get here too.
int SumDescriptions(CArrayTestClass* pacTests)
{
	int		iTotal;
	size	i;

	iTotal = 0;
	for (i = 0; i < pacTests->NumElements(); i++)
	{
		iTotal += pacTests->Get(i)->Describe();
	}
	return iTotal;
}
