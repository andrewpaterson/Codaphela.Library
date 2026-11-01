#include "BaseLib/Constructors.h"
#include "TestLib/Assert.h"
#include "ConstructorsTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestConstructorsAdd(void)
{
	CConstructors			cConstructors;
	CConstructorTestClass*	pcClass;
	CConstructorTestSuper2*	pcSuper2;

	cConstructors.Init();

	cConstructors.Add<CConstructorTestClass>("ConstructorClass");
	cConstructors.Add<CConstructorTestSuper2>("ConstructorSuper2");
	pcClass = (CConstructorTestClass*)cConstructors.Construct("ConstructorClass", &gcSystemAllocator);
	AssertString("CConstructorTestClass", pcClass->GetName());
	AssertInt(2, pcClass->Method2());
	AssertInt(3, pcClass->Method1());
	AssertInt(0, pcClass->i);
	AssertInt(0, pcClass->j);
	AssertInt(0, pcClass->k);

	pcSuper2 = (CConstructorTestSuper2*)cConstructors.Construct("ConstructorSuper2", &gcSystemAllocator);
	AssertString("CConstructorTestSuper2", pcSuper2->GetName());
	AssertInt(2, pcSuper2->Method2());
	AssertInt(0, pcSuper2->j);

	gcSystemAllocator.Free(pcClass);
	gcSystemAllocator.Free(pcSuper2);

	cConstructors.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestConstructors(void)
{
	BeginTests();

	TestConstructorsAdd();

	TestStatistics();
}

