#include "BaseLib/GlobalMemory.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestReferences(void)
{
	BeginTests();

	int&& x = 5;
	int y = x;
	AssertInt(5, y);
	AssertInt(5, x);
	Pass();

	x = 6;
	AssertInt(5, y);
	AssertInt(6, x);
	Pass();

	int b = 7;
	int& a = b;
	int& c = a;
	AssertInt(7, a);
	AssertInt(7, b);
	AssertInt(7, c);
	Pass();

	c = 8;
	AssertInt(8, a);
	AssertInt(8, b);
	AssertInt(8, c);
	Pass();

	b = 9;
	AssertInt(9, a);
	AssertInt(9, b);
	AssertInt(9, c);
	Pass();

	y = 3;
	x = y;
	AssertInt(3, x);
	AssertInt(3, y);
	Pass();

	int&& m = 2;
	AssertInt(2, m);
	m = 4;
	AssertInt(4, m);
	Pass();

	a = m;
	x = b;
	AssertInt(4, a);
	AssertInt(4, x);
	Pass();

	int* p = NULL;
	x = 6;
	p = &x;
	a = *p;
	AssertInt(6, *p);
	AssertInt(6, x);
	AssertInt(6, a);
	Pass();

	a = 7;
	p = &a;
	x = *p;
	AssertInt(7, *p);
	AssertInt(7, x);
	AssertInt(7, a);

	TestStatistics();
}

