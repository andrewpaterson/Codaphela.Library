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
// Recycle empties the pad but keeps up to the given bytes of its chunks,
// which the Adds after use before allocating more.
//
//////////////////////////////////////////////////////////////////////////
void TestScratchPadRecycle(void)
{
	CScratchPad		cPad;
	void*			apv[4];
	void*			pv;
	size			i;
	size			uiChunk;

	uiChunk = 100 + sizeof(SSPNode);
	cPad.Init(uiChunk);
	for (i = 0; i < 4; i++)
	{
		apv[i] = cPad.Add(100);
		AssertNotNull(apv[i]);
	}
	AssertSize(4 * uiChunk, cPad.GetMemorySize());

	//Two chunks are kept, the other two freed.
	cPad.Recycle(2 * uiChunk);
	AssertSize(0, cPad.GetMemorySize());
	AssertSize(0, cPad.GetUsedSize());

	//The kept chunks are used again (the last kept first), then a new one is allocated.
	pv = cPad.Add(100);
	AssertTrue((pv == apv[0]) || (pv == apv[1]));
	pv = cPad.Add(100);
	AssertTrue((pv == apv[0]) || (pv == apv[1]));
	AssertSize(2 * uiChunk, cPad.GetMemorySize());
	AssertNotNull(cPad.Add(100));
	AssertSize(3 * uiChunk, cPad.GetMemorySize());
	AssertSize(3 * uiChunk, cPad.GetUsedSize());

	//Nothing kept.
	cPad.Recycle(0);
	AssertSize(0, cPad.GetMemorySize());
	AssertNotNull(cPad.Add(100));
	AssertSize(uiChunk, cPad.GetMemorySize());

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
	TestScratchPadRecycle();

	TestStatistics();
}


