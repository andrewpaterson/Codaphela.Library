#include "BaseLib/LeakDetector.h"

#include "TestLib/Assert.h"

void TestJavaTokenParser(void);
void TestSkipComments(void);
void TestJavaParserGetInteger(void);
void TestJavaParserGetFloat(void);
void TestJavaParserGetCharacter(void);
void TestJavaParserGetString(void);
void TestJavaParserPrintFloat(void);
void TestJavaProjectTokenParser(void);
void TestJavaSyntaxParserImports(void);
void TestJavaSyntaxParserGenerics(void);
void TestJavaSyntaxParserClassVariablesSimpleDeclaration(void);
void TestJavaSyntaxParserClassVariables(void);
void TestJavaSyntaxParser(void);


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int __cdecl main(void)
{
	BreakOnAllocation(0);
	
	int	iTotalTestsFailed;

	InitTotalStatistics();
	NumberInit();

	TestJavaParserPrintFloat();
	TestSkipComments();
	TestJavaParserGetInteger();
	TestJavaParserGetCharacter();
	TestJavaParserGetString();
	TestJavaParserGetFloat();
	TestJavaTokenParser();
	//TestJavaProjectTokenParser();
	TestJavaSyntaxParserImports();
	TestJavaSyntaxParserGenerics();
	TestJavaSyntaxParserClassVariables();
	//TestJavaSyntaxParser();

	iTotalTestsFailed = TestTotalStatistics();

	NumberKill();
	LeakDetection();
	return iTotalTestsFailed;
}

