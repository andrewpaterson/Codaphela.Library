#include <string.h>
#include "BaseLib/IntegerHelper.h"
#include "SortedTestObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSortedTestObject::Init(int iValue, const char* szThreeChars)
{
	memcpy(mszThreeChars, szThreeChars, 3);
	mszThreeChars[3] = 0;
	miValue = iValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CompareSortedTestObject(const void* arg1, const void* arg2)
{
	CSortedTestObject*	ps1;
	CSortedTestObject*	ps2;

	ps1 = (CSortedTestObject*)arg1;
	ps2 = (CSortedTestObject*)arg2;

	return CompareInt(&ps1->miValue, &ps2->miValue);
}

