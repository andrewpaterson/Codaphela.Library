#include "BaseLib/ConstructorCall.h"
#include "TestLib/Assert.h"
#include "ConstructorCallTestClasses.h"
#include "ConstructorsTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestConstructorVirtual(void)
{
	bool bNoVirtualTable;

	bNoVirtualTable = IsTriviallyDefaultConstructible<CConstructorCallTest1>();
	AssertTrue(bNoVirtualTable);

	bNoVirtualTable = IsTriviallyDefaultConstructible<CConstructorCallTest2>();
	AssertFalse(bNoVirtualTable);

	bNoVirtualTable = IsTriviallyDefaultConstructible<CConstructorCallTest3>();
	AssertFalse(bNoVirtualTable);

	bNoVirtualTable = IsTriviallyDefaultConstructible<CConstructorTestClass>();
	AssertFalse(bNoVirtualTable);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestConstructorPostMalloc(void)
{
	CConstructorCaller1		c1;
	CConstructorCallTest1*	pct1;
	CConstructorCaller2		c2;
	CConstructorCallTest2*	pct2;
	CConstructorCaller4		c4;
	CConstructorTestClass*	pct4;
			
	pct1 = (CConstructorCallTest1*)malloc(sizeof(CConstructorCallTest1));
	c1.PostMalloc(pct1);
	AssertString("CConstructorCallTest1", pct1->GetName());
	free(pct1);

	pct2 = (CConstructorCallTest2*)malloc(sizeof(CConstructorCallTest2));
	c2.PostMalloc(pct2);
	AssertString("CConstructorCallTest2", pct2->GetName());
	free(pct2);

	pct4 = (CConstructorTestClass*)malloc(sizeof(CConstructorTestClass));
	c4.PostMalloc(pct4);
	AssertString("CConstructorTestClass", pct4->GetName());
	free(pct4);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestConstructorCall(void)
{
	BeginTests();

	TestConstructorVirtual();
	TestConstructorPostMalloc();

	TestStatistics();
}

