#include "BaseLib/TemporaryMemory.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTemporaryMemorySize(void)
{
	CTemporaryMemory	cTemp;
	void*				pv;
	void*				pvOld;

	cTemp.Init();
	AssertNull(cTemp.Get());
	pv = cTemp.Size(10);
	AssertNotNull(cTemp.Get());
	pvOld = pv;
	pv = cTemp.Size(9);
	AssertPointer(pvOld, pv);
	pvOld = pv;
	pv = cTemp.Size((lsize)1024 * 1024);
	AssertFalse(pvOld == pv);

	cTemp.Kill();
	AssertNull(cTemp.Get());

	cTemp.Init();
	pv = cTemp.Size(10);
	AssertNotNull(pv);
	cTemp.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTemporaryMemory(void)
{
	BeginTests();

	TestTemporaryMemorySize();

	TestStatistics();
}

