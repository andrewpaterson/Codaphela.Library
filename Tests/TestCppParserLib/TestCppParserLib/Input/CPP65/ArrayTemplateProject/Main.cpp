#include "TestClass.h"


int main()
{
	CArrayTestClass	acTests;
	CTestClass*		pcTest;
	int				i;

	acTests.Init();
	for (i = 1; i <= 4; i++)
	{
		pcTest = acTests.Add();
		pcTest->Set(i);
	}
	acTests.Add();

	//Made by the constructor: 5.  Values + squares: (1+1) + (2+4) + (3+9) + (4+16) + (-1+1) = 40.  [2] is 3.
	if ((CTestClass::siMade != 5) || (acTests.NumElements() != 5) || (acTests.ElementSize() != sizeof(CTestClass)))
	{
		return -1;
	}
	return SumDescriptions(&acTests) * 100 + acTests[2].miValue * 10 + acTests.Get(4)->miSquare;
}
