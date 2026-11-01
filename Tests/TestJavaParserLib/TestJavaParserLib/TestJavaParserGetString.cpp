#include "BaseLib/TextParser.h"
#include "BaseLib/Numbers.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
TRISTATE ParseString(char* sz, void* szDest, size* piLength, uint16* piWidth)
{
	CTextParser		cParser;
	TRISTATE		tResult;

	cParser.Init(sz);
	tResult = cParser.GetStringLiteral(szDest, 4 KB, true, piLength, piWidth, false);
	cParser.Kill();

	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void AssertZtring(TRISTATE tResult, char* szActual, char* szExpected, size iLengthActual, size iLengthExpected, int iWidthActual, int iWidthExpected)
{
	AssertTristate(TRITRUE, tResult);
	AssertString(szExpected, szActual);
	AssertSize(iLengthExpected, iLengthActual);
	AssertInt(iWidthExpected, iWidthActual);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestJavaParserGetStringSizeofChar16(void)
{
	AssertSize(2, sizeof(char16));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestJavaParserGetStringSimple(void)
{
	TRISTATE		tResult;
	uint16			iWidth;
	size			iLength;
	char			sz[4 KB];

	tResult = ParseString("\"H\"", sz, &iLength, &iWidth);
	AssertZtring(tResult, sz, "H", iLength, 1, iWidth, 1);

	tResult = ParseString("\"He\"", sz, &iLength, &iWidth);
	AssertZtring(tResult, sz, "He", iLength, 2, iWidth, 1);

	tResult = ParseString("\"Hello World\"", sz, &iLength, &iWidth);
	AssertZtring(tResult, sz, "Hello World", iLength, 11, iWidth, 1);

	tResult = ParseString("\"Hello\\u0020World\"", sz, &iLength, &iWidth);
	AssertZtring(tResult, sz, "H", iLength, 11, iWidth, 2);
	AssertMemory("H\0e\0l\0l\0o\0 \0W\0o\0r\0l\0d\0\0\0", sz, 24);

	tResult = ParseString("\"AND Gate\";\n", sz, &iLength, &iWidth);
	AssertZtring(tResult, sz, "AND Gate", iLength, 8, iWidth, 1);
	
	Pass();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestJavaParserGetStringEscaped(void)
{
	TRISTATE	tResult;
	uint16		iWidth;
	size		iLength;
	char		sz[4 KB];

	tResult = ParseString("\"\"", sz, &iLength, &iWidth);
	AssertZtring(tResult, sz, "", iLength, 0, iWidth, 1);

	tResult = ParseString("\"\\\"\"", sz, &iLength, &iWidth);
	AssertZtring(tResult, sz, "\"", iLength, 1, iWidth, 1);

	Pass();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestJavaParserGetString(void)
{
	BeginTests();

	TestJavaParserGetStringSizeofChar16();
	TestJavaParserGetStringSimple();
	TestJavaParserGetStringEscaped();

	TestStatistics();

}