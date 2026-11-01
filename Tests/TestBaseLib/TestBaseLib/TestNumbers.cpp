#include "BaseLib/Numbers.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNumbers(void)
{
	BeginTests();

	float	fTiny;

	fTiny = SMALL_NUMBER/2.0f;

	AssertTrue(FloatEqual(10.f, 10.f));
	AssertFalse(FloatEqual(10.f, 11.f));
	AssertFalse(FloatEqual(11.f, 10.f));

	AssertTrue(FloatEqual(10, 10-fTiny));
	AssertTrue(FloatEqual(10, 10+fTiny));
	AssertTrue(FloatEqual(10-fTiny, 10));
	AssertTrue(FloatEqual(10+fTiny, 10));

	AssertTrue(FloatGreaterThanOrEqual(10, 10));
	AssertTrue(FloatGreaterThanOrEqual(11, 10));
	AssertFalse(FloatGreaterThanOrEqual(10, 11));
	AssertTrue(FloatGreaterThanOrEqual(10-fTiny, 10));
	AssertTrue(FloatGreaterThanOrEqual(10, 10+fTiny));

	AssertTrue(FloatLessThanOrEqual(10, 10));
	AssertTrue(FloatLessThanOrEqual(10, 11));
	AssertFalse(FloatLessThanOrEqual(11, 10));
	AssertTrue(FloatLessThanOrEqual(10+fTiny, 10));
	AssertTrue(FloatLessThanOrEqual(10, 10-fTiny));

	AssertFalse(FloatGreaterThan(10, 10));
	AssertFalse(FloatGreaterThan(10, 11));
	AssertTrue(FloatGreaterThan(11, 10));
	AssertFalse(FloatGreaterThan(10+fTiny, 10));
	AssertFalse(FloatGreaterThan(10, 10-fTiny));

	AssertFalse(FloatLessThan(10, 10));
	AssertFalse(FloatLessThan(11, 10));
	AssertTrue(FloatLessThan(10, 11));
	AssertFalse(FloatLessThan(10-fTiny, 10));
	AssertFalse(FloatLessThan(10, 10+fTiny));

	TestStatistics();
}

