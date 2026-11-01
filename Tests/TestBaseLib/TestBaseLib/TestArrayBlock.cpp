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
void TestArrayBlock(void)
{
	BeginTests();
	MemoryInit();

	TestArrayBlockRemoveBatch();

	MemoryKill();
	TestStatistics();
}

