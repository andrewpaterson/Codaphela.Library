#include "BaseLib/IntegerHelper.h"
#include "BaseLib/Numbers.h"
#include "BaseLib/Chars.h"
#include "BaseLib/SetTemplate.h"
#include "BaseLib/SetString.h"
#include "TestLib/Assert.h"
#include "WordKey.h"


typedef CSetTemplate<int> CSetInt;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetString2(void)
{
	CSetString	cSetString;
	char*		sz1;

	cSetString.Init();

	cSetString.Put("Hello");
	cSetString.Put("Michelle");
	cSetString.Put("Number");
	cSetString.Put("Michelle");

	sz1 = cSetString.Get("Michelle");
	AssertString("Michelle", sz1);

	cSetString.Kill();
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetInt2(void)
{
	CSetInt			cSetInt;
	int*			px;
	int*			pz;
	SSetIterator	sIter;
	bool			bResult;
	int				x;

	cSetInt.Init(CompareInt, true);

	x = 10;
	cSetInt.Put(&x);

	x = 5;
	cSetInt.Put(&x);

	x = 15;
	cSetInt.Put(&x);

	x = 5;
	cSetInt.Put(&x);

	bResult = cSetInt.StartIteration(&sIter, (void**)&px, NULL);
	while (bResult)
	{
		pz = cSetInt.Get(px);
		AssertInt(*px, *pz);
		bResult = cSetInt.Iterate(&sIter, (void**)&px, NULL);
	}

	cSetInt.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSet(void)
{
	BeginTests();

	TestSetString2();
	TestSetInt2();

	TestStatistics();
}

