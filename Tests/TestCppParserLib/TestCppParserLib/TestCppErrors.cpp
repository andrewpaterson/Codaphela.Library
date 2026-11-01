#include "CppParserLib/CPPErrors.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCppErrorsAdd(void)
{
	CCPPErrors	cErrors;

	cErrors.Init();
	AssertSize(0, cErrors.NumErrors());
	AssertString("", cErrors.Text());

	cErrors.Add("File.cpp", 2, 6, 17, "Expected ';'.", "x");
	cErrors.Add("Other.h", 0, -1, -1, "Bad line.");
	cErrors.Add("File.cpp", -1, -1, -1, "Unterminated conditional directive.");
	cErrors.Add(NULL, -1, -1, -1, "Directory missing.");

	AssertSize(4, cErrors.NumErrors());
	AssertString("File.cpp (3, 7): Expected ';'. [x]", cErrors.Get(0));
	AssertString("Other.h (1): Bad line.", cErrors.Get(1));
	AssertString("File.cpp: Unterminated conditional directive.", cErrors.Get(2));
	AssertString("Directory missing.", cErrors.Get(3));
	AssertString("File.cpp", cErrors.GetFile(0));
	AssertString("File.cpp", cErrors.GetFile(2));
	AssertNull(cErrors.GetFile(3));
	AssertInt(2, cErrors.GetLine(0));
	AssertInt(6, cErrors.GetColumn(0));
	AssertInt(17, cErrors.GetToken(0));
	AssertInt(-1, cErrors.GetToken(1));
	AssertString("File.cpp (3, 7): Expected ';'. [x]\nOther.h (1): Bad line.\nFile.cpp: Unterminated conditional directive.\nDirectory missing.\n", cErrors.Text());

	cErrors.Truncate(1);
	AssertSize(1, cErrors.NumErrors());
	AssertString("File.cpp (3, 7): Expected ';'. [x]\n", cErrors.Text());

	cErrors.Clear();
	AssertSize(0, cErrors.NumErrors());
	cErrors.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCppErrorsAppend(void)
{
	CCPPErrors	cFirst;
	CCPPErrors	cSecond;

	cFirst.Init();
	cSecond.Init();
	cFirst.Add("A.cpp", 0, 0, 1, "One.");
	cSecond.Add("B.cpp", 4, 2, 9, "Two.", "y");
	cSecond.AppendTo(&cFirst);

	AssertSize(2, cFirst.NumErrors());
	AssertString("B.cpp (5, 3): Two. [y]", cFirst.Get(1));
	AssertString("B.cpp", cFirst.GetFile(1));
	AssertInt(9, cFirst.GetToken(1));
	AssertSize(1, cSecond.NumErrors());
	cSecond.Kill();
	cFirst.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCppErrors(void)
{
	BeginTests();

	TestCppErrorsAdd();
	TestCppErrorsAppend();

	TestStatistics();
}
