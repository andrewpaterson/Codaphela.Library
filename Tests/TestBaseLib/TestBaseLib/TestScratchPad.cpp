#include "BaseLib/ScratchPad.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestScratchPadAdd(void)
{
	CScratchPad		cPad;
	void*			pvData;

	cPad.Init(10 + sizeof(SSPNode));

	pvData = cPad.Add(10);
	AssertNotNull(pvData);
#ifdef BUILD_64
	AssertSize(26, cPad.GetMemorySize());
	AssertSize(26, cPad.GetUsedSize());
#else
	AssertSize(18, cPad.GetMemorySize());
	AssertSize(18, cPad.GetUsedSize());
#endif // BUILD_64

	pvData = cPad.Add(9);
	AssertNotNull(pvData);
#ifdef BUILD_64
	AssertSize(52, cPad.GetMemorySize());
	AssertSize(51, cPad.GetUsedSize());
#else
	AssertSize(36, cPad.GetMemorySize());
	AssertSize(35, cPad.GetUsedSize());
#endif // BUILD_64

	pvData = cPad.Add(5);
	AssertNotNull(pvData);
#ifdef BUILD_64
	AssertSize(78, cPad.GetMemorySize());
	AssertSize(72, cPad.GetUsedSize());
#else
	AssertSize(54, cPad.GetMemorySize());
	AssertSize(48, cPad.GetUsedSize());
#endif // BUILD_64

	pvData = cPad.Add(5);
	AssertNotNull(pvData);
#ifdef BUILD_64
	AssertSize(104, cPad.GetMemorySize());
	AssertSize(93, cPad.GetUsedSize());
#else
	AssertSize(72, cPad.GetMemorySize());
	AssertSize(61, cPad.GetUsedSize());
#endif // BUILD_64

	pvData = cPad.Add(11);
	AssertNotNull(pvData);
#ifdef BUILD_64
	AssertSize(131, cPad.GetMemorySize());
	AssertSize(120, cPad.GetUsedSize());
#else
	AssertSize(91, cPad.GetMemorySize());
	AssertSize(80, cPad.GetUsedSize());
#endif // BUILD_64
	
	cPad.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestScratchPadRollback(void)
{
	CScratchPad		cPad;
	
	cPad.Init(100);

	cPad.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestScratchPad(void)
{
	BeginTests();

	TestScratchPadAdd();
	TestScratchPadRollback();

	TestStatistics();
}


