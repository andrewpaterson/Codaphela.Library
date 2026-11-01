#include "BaseLib/ArrayVoidPtr.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayVoidPtrGet(void)
{
	CArrayVoidPtr	cArray;
	int				iValue;
	int*			pi;

	iValue = 55;

	cArray.Init();
	cArray.Add(&iValue);

	pi = NULL;
	cArray.Get(0, (void**)&pi);
	AssertInt(55, *pi);

	pi = (int*)cArray.GetPtr(0);
	AssertInt(55, *pi);

	cArray.Kill();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayVoidPtrFind(void)
{
	CArrayVoidPtr	cArray;
	int				aiValue[5];

	aiValue[0] = 55;
	aiValue[1] = 66;
	aiValue[2] = 77;
	aiValue[3] = 88;
	aiValue[4] = 99;

	cArray.Init();
	cArray.Add(&aiValue[4]);
	cArray.Add(&aiValue[3]);
	cArray.Add(&aiValue[2]);
	cArray.Add(&aiValue[1]);
	cArray.Add(&aiValue[0]);

	AssertInt(55, *((int*)cArray.GetPtr(0)));

	cArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayVoidPtr(void)
{
	BeginTests();

	TestArrayVoidPtrGet();
	TestArrayVoidPtrFind();

	TestStatistics();
}

