#include "BaseLib/TextParser.h"
#include "BaseLib/Numbers.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestSkipWhitespaceWithCStyleComments(void)
{
	CTextParser		cParser;
	char			szIdentifier[1 KB];
	size			iLength;
	TRISTATE		/* / * Hello */ tResult;

	cParser.Init("  /* /* Hello */  X");
	cParser.SkipWhitespace();
	tResult = cParser.GetIdentifier(szIdentifier, &iLength, false);
	AssertTristate(TRITRUE, tResult);
	AssertString("X", szIdentifier);
	cParser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestSkipWhitespaceWithCppStyleComments(void)
{
	CTextParser		cParser;
	char			szIdentifier[1 KB];
	size			iLength;
	TRISTATE		tResult;  //

	cParser.Init("  //\n  //\n  X");
	cParser.SkipWhitespace();
	tResult = cParser.GetIdentifier(szIdentifier, &iLength, false);
	AssertTristate(TRITRUE, tResult);
	AssertString("X", szIdentifier);
	cParser.Kill();
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestSkipComments(void)
{
	BeginTests();

	TestSkipWhitespaceWithCppStyleComments();
	TestSkipWhitespaceWithCStyleComments();

	TestStatistics();
}

