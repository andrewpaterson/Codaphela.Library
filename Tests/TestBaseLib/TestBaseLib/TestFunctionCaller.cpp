#include "BaseLib/FunctionCaller.h"
#include "TestLib/Assert.h"
#include "BaseLib/Chars.h"
#include "TestFunctionCaller.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSimpleClassFunctionCall(void)
{
	CTestFunctionCaller			mcTest;
	const char*						pv;
	int							i;
	SFunctionPointer			func;

	func = GetThisCall(&CTestFunctionCaller::PrintIt);

	mcTest.Init();

	pv = "World!";
	i = 7;

	i = (((CFunctionCaller*)&mcTest)->*FunctionCast<ThisCall_Int_PointerInt>(func.thisCall))((char*)pv, i);
	AssertInt(27, i);
	AssertString("Yo! How you doing? World! 7", mcTest.msz.Text());

	mcTest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFunctionCaller(void)
{
	BeginTests();

	TestSimpleClassFunctionCall();

	TestStatistics();
}


void CTestFunctionCaller::Init(void)
{
	msz.Init("Yo! How you doing?");
}


void CTestFunctionCaller::Kill(void)
{
	msz.Kill();
}


int CTestFunctionCaller::PrintIt(const char* pv, int i)
{
	msz.Append(" ");
	msz.Append(pv);
	msz.Append(" ");
	msz.Append(i);
	return (int)msz.Length();
}

