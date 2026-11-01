#include "BaseLib/StdRandom.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "TestLib/Assert.h"
#include "ArrayTemplateTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int TestComparePtr(const void* pvArg1, const void* pvArg2)
{
	STestArrayTemplateItem*		ps1;
	STestArrayTemplateItem*		ps2;

	ps1 = (STestArrayTemplateItem*)pvArg1;
	ps2 = (STestArrayTemplateItem*)pvArg2;

	if (ps1->i1 < ps2->i1)
	{
		return -1;
	}
	if (ps1->i1 > ps2->i1)
	{
		return 1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int TestComparePtrPtr(const void* ppvArg1, const void* ppvArg2)
{
	return TestComparePtr(*((void**)ppvArg1), *((void**)ppvArg2));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayTemplatePtrSorting(void)
{
	CArrayTemplatePtr<STestArrayTemplateItem>	ap;
	STestArrayTemplateItem						s3;
	STestArrayTemplateItem						s2;
	STestArrayTemplateItem						s4;
	STestArrayTemplateItem						s1;
	STestArrayTemplateItem*						ps;

	s1.i1 = 1;
	s1.i2 = 1;
	s2.i1 = 2;
	s2.i2 = 2;
	s3.i1 = 3;
	s3.i2 = 3;
	s4.i1 = 4;
	s4.i2 = 4;

	ap.Init();
	ap.Add(&s4);
	ap.Add(&s2);
	ap.Add(&s3);
	ap.Add(&s1);

	ap.Sort(TestComparePtrPtr);

	ps = ap.GetPtr(0);
	AssertInt((int32)1, ps->i1);
	ps = ap.GetPtr(1);
	AssertInt((int32)2, ps->i1);
	ps = ap.GetPtr(2);
	AssertInt((int32)3, ps->i1);
	ps = ap.GetPtr(3);
	AssertInt((int32)4, ps->i1);

	ap.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayTemplatePtr(void)
{
	BeginTests();
	MemoryInit();

	TestArrayTemplatePtrSorting();

	MemoryKill();
	TestStatistics();
}

