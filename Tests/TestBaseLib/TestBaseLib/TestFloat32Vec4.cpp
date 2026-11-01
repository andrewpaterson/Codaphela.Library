#include "BaseLib/Float32Vec4.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFloat32Vec4Add(void)
{
	SFloat32Vec4 sfa;
	SFloat32Vec4 sfb;

	sfa.Init(1, 2, 3, 4);
	sfb.Init(4, 3, 2, 1);

	sfa += sfb;

	AssertFloat(5, sfa.x, 13);
	AssertFloat(5, sfa.y, 13);
	AssertFloat(5, sfa.z, 13);
	AssertFloat(5, sfa.w, 13);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFloat32Vec4Print(void)
{
	SFloat32Vec4	sf;
	CChars			sz;

	sf.Init(345.342359f, 0.0002345f, 53.0f, -90234534.34234234f);
	sz.Init();
	sf.Print(&sz);
	AssertString("[      345.34,        0.00,       53.00,-90234536.00]", sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFloat32Vec4Magnitude(void)
{
	SFloat32Vec4	sf;

	sf.Init(345.0f, 432.0f, 1789.0f, 400.f);
	AssertFloat(1914.7244873046875f, sf.Magnitude(), 13);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFloat32Vec4(void)
{
	BeginTests();
	NumberInit();

	TestFloat32Vec4Add();
	TestFloat32Vec4Print();
	TestFloat32Vec4Magnitude();

	NumberKill();
	TestStatistics();
}

