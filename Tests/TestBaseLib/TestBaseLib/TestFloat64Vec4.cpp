#include "BaseLib/Float64Vec4.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFloat64Vec4Add(void)
{
	SFloat64Vec4 sfa;
	SFloat64Vec4 sfb;

	sfa.Init(1, 2, 3, 4);
	sfb.Init(4, 3, 2, 1);

	sfa += sfb;

	AssertDouble(5, sfa.x, 13);
	AssertDouble(5, sfa.y, 13);
	AssertDouble(5, sfa.z, 13);
	AssertDouble(5, sfa.w, 13);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFloat64Vec4Print(void)
{
	SFloat64Vec4	sf;
	CChars			sz;

	sf.Init(345.342359, 0.0002345, 53.0, -90234534.34234234);
	sz.Init();
	sf.Print(&sz);
	AssertString("[      345.34,        0.00,       53.00,-90234534.34]", sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFloat64Vec4Magnitude(void)
{
	SFloat64Vec4	sf;

	sf.Init(345.0, 464.0, 1789.0, 400);
	AssertDouble(1922.1971803121551, sf.Magnitude(), 13);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFloat64Vec4(void)
{
	BeginTests();
	NumberInit();

	TestFloat64Vec4Add();
	TestFloat64Vec4Print();
	TestFloat64Vec4Magnitude();

	NumberKill();
	TestStatistics();
}

