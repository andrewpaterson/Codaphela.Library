#include "BaseLib/PointerRemapper.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TextFile.h"
#include "TestLib/Assert.h"
#include "JavaParserLib/JavaTokenParserEnvironment.h"
#include "JavaParserLib/JavaSyntaxParser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestJavaSyntaxParserGenericGeneric(void)
{
	bool						bResult;
	CTokenParserEnvironment		cTokenParser;
	CJavaSyntaxParser			cSyntaxParser;
	CJavaSyntaxMemory			cSyntaxMemory;
	char						szFilename[] = "TestJavaSyntaxParserGenericGeneric.Java";
	char						szFileContents[] = "\
public final class W65C816Assembler<List<List>>\n\
{\n\
}\n\
";

	cTokenParser.Init(szFilename, szFileContents, true);
	AssertTrue(cTokenParser.Parse());

	cSyntaxMemory.Init();
	cSyntaxParser.Init(&cSyntaxMemory, cTokenParser.GetParser());

	bResult = cSyntaxParser.Parse();
	AssertTrue(bResult);
	cSyntaxParser.Dump(true);

	cSyntaxParser.Kill();
	cSyntaxMemory.Kill();
	cTokenParser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestJavaSyntaxParserGenericExtends(void)
{
	bool						bResult;
	CTokenParserEnvironment		cTokenParser;
	CJavaSyntaxParser			cSyntaxParser;
	CJavaSyntaxMemory			cSyntaxMemory;
	char						szFilename[] = "TestJavaSyntaxParserGenericExtends.Java";
	char						szFileContents[] = "\
public final class W65C816Assembler<X extends Integer>\n\
{\n\
}\n\
";

	cTokenParser.Init(szFilename, szFileContents, true);
	AssertTrue(cTokenParser.Parse());

	cSyntaxMemory.Init();
	cSyntaxParser.Init(&cSyntaxMemory, cTokenParser.GetParser());

	bResult = cSyntaxParser.Parse();
	AssertTrue(bResult);
	cSyntaxParser.Dump(true);

	cSyntaxParser.Kill();
	cSyntaxMemory.Kill();
	cTokenParser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestJavaSyntaxParserGenericWildcard(void)
{
	bool						bResult;
	CTokenParserEnvironment		cTokenParser;
	CJavaSyntaxParser			cSyntaxParser;
	CJavaSyntaxMemory			cSyntaxMemory;
	char						szFilename[] = "TestJavaSyntaxParserGenericWildcard.Java";
	char						szFileContents[] = "\
public final class W65C816Assembler<? extends Map<?, ?>>\n\
{\n\
}\n\
";

	cTokenParser.Init(szFilename, szFileContents, true);
	AssertTrue(cTokenParser.Parse());

	cSyntaxMemory.Init();
	cSyntaxParser.Init(&cSyntaxMemory, cTokenParser.GetParser());

	bResult = cSyntaxParser.Parse();
	AssertTrue(bResult);
	cSyntaxParser.Dump(true);

	cSyntaxParser.Kill();
	cSyntaxMemory.Kill();
	cTokenParser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestJavaSyntaxParserGenericComplex(void)
{
	bool						bResult;
	CTokenParserEnvironment		cTokenParser;
	CJavaSyntaxParser			cSyntaxParser;
	CJavaSyntaxMemory			cSyntaxMemory;
	char						szFilename[] = "TestJavaSyntaxParserGenericComplex.Java";
	char						szFileContents[] = "\
public class W65C816Assembler<X, Y extends List<Map<X, ? extends Integer>>, ? extends Map<?, ?>>\n\
{\n\
}\n\
";

	cTokenParser.Init(szFilename, szFileContents, true);
	AssertTrue(cTokenParser.Parse());

	cSyntaxMemory.Init();
	cSyntaxParser.Init(&cSyntaxMemory, cTokenParser.GetParser());

	bResult = cSyntaxParser.Parse();
	AssertTrue(bResult);
	cSyntaxParser.Dump(true);

	cSyntaxParser.Kill();
	cSyntaxMemory.Kill();
	cTokenParser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestJavaSyntaxParserGenericError(void)
{
	bool						bResult;
	CTokenParserEnvironment		cTokenParser;
	CJavaSyntaxParser			cSyntaxParser;
	CJavaSyntaxMemory			cSyntaxMemory;
	CChars						szOutput;
	char*						szBuffer;
	int							iLength;
	char						szFilename[] = "TestJavaSyntaxParserGenericError.Java";
	char						szFileContents[] = "\
public final class W65C816Assembler<List \n\
{\n\
}\n\
";

	cTokenParser.Init(szFilename, szFileContents, false);
	AssertTrue(cTokenParser.Parse());

	cSyntaxMemory.Init();
	cSyntaxParser.Init(&cSyntaxMemory, cTokenParser.GetParser());

	bResult = cSyntaxParser.Parse();
	AssertFalse(bResult);

	iLength = 0;
	szBuffer = cTokenParser.GetOutput(&iLength);
	szOutput.Init(szBuffer, 0, iLength);
	AssertString("ERROR: '>' or Type expected.\n\
[TestJavaSyntaxParserGenericError.Java:2]:  { \n\
                                            ^\n\
\n", szOutput.Text());
	szOutput.Kill();

	cSyntaxParser.Kill();
	cSyntaxMemory.Kill();
	cTokenParser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestJavaSyntaxParserGenerics(void)
{
	BeginTests();

	TestJavaSyntaxParserGenericGeneric();
	TestJavaSyntaxParserGenericExtends();
	TestJavaSyntaxParserGenericWildcard();
	TestJavaSyntaxParserGenericComplex();
	TestJavaSyntaxParserGenericError();

	TestStatistics();
}

