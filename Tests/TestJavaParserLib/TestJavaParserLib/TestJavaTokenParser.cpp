#include "BaseLib/PointerRemapper.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TextFile.h"
#include "TestLib/Assert.h"
#include "JavaParserLib/JavaTokenParser.h"
#include "JavaTokenAssert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestJavaTokenParserEndOfFile(void)
{
	CJavaTokenParser		cTokenParser;	
	CJavaTokenMemory		cTokenMemory;
	CJavaTokenDefinitions	cTokenDefinitions;
	CJavaToken*				pcToken;
	bool					bResult;
	CLogger					cLogger;
	STextPosition*			psPos;

	cLogger.Init(NULL);
	cLogger.SetSilent(true); 
	cTokenMemory.Init();
	cTokenDefinitions.Init();
	cTokenParser.Init(&cLogger, &cTokenDefinitions, &cTokenMemory, "Test1.Java", "{\n}");

	bResult = cTokenParser.Parse(true);
	AssertTrue(bResult);

	pcToken = cTokenParser.GetFirstToken();
	pcToken = AssertSeparator(&cTokenDefinitions, pcToken, JS_CurlyBracketLeft);
	pcToken = AssertSeparator(&cTokenDefinitions, pcToken, JS_CurlyBracketRight);
	AssertNull(pcToken);

	pcToken = cTokenParser.GetFirstToken();
	psPos = pcToken->GetPosition();
	AssertString("{\n}", psPos->szPos);

	pcToken = pcToken->GetNext();
	psPos = pcToken->GetPosition();
	AssertString("}", psPos->szPos);

	cTokenParser.Kill();	
	cTokenMemory.Kill();
	cTokenMemory.Kill();
	cTokenDefinitions.Kill();
	cLogger.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestJavaTokenParserStartAndEndWithComment(void)
{
	CJavaTokenParser		cTokenParser;	
	CJavaTokenMemory		cTokenMemory;
	CJavaTokenDefinitions	cTokenDefinitions;
	CJavaToken*				pcToken;
	TRISTATE				tResult;
	CLogger					cLogger;

	cLogger.Init(NULL);
	cLogger.SetSilent(true);
	cTokenDefinitions.Init();
	cTokenMemory.Init();
	cTokenParser.Init(&cLogger, &cTokenDefinitions, &cTokenMemory, "Test2.Java", "\
  // Tickable Pins Start\n\
package net.simulation.common;\n\
\n\
{\n\
  protected int x;\n\
} // Tickable Pins End\
");

	tResult = cTokenParser.Parse();
	AssertTristate(TRITRUE, tResult);

	pcToken = cTokenParser.GetFirstToken();

	pcToken = AssertKeyword(&cTokenDefinitions, pcToken, JK_package);
	pcToken = AssertIdentifier(&cTokenDefinitions, pcToken, "net");
	pcToken = AssertSeparator(&cTokenDefinitions, pcToken, JS_Dot);
	pcToken = AssertIdentifier(&cTokenDefinitions, pcToken, "simulation");
	pcToken = AssertSeparator(&cTokenDefinitions, pcToken, JS_Dot);
	pcToken = AssertIdentifier(&cTokenDefinitions, pcToken, "common");
	pcToken = AssertSeparator(&cTokenDefinitions, pcToken, JS_Semicolon);
	pcToken = AssertSeparator(&cTokenDefinitions, pcToken, JS_CurlyBracketLeft);
	pcToken = AssertKeyword(&cTokenDefinitions, pcToken, JK_protected);
	pcToken = AssertKeyword(&cTokenDefinitions, pcToken, JK_int);
	pcToken = AssertIdentifier(&cTokenDefinitions, pcToken, "x");
	pcToken = AssertSeparator(&cTokenDefinitions, pcToken, JS_Semicolon);
	pcToken = AssertSeparator(&cTokenDefinitions, pcToken, JS_CurlyBracketRight);
	AssertNull(pcToken);
	
	cTokenParser.Kill();	
	cTokenMemory.Kill();
	cTokenDefinitions.Kill();
	cLogger.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestJavaTokenParserComplexGeneric(void)
{
	CJavaTokenDefinitions	cTokenDefinitions;
	CJavaTokenParser		cTokenParser;	
	CJavaTokenMemory		cTokenMemory;
	TRISTATE				tResult;
	CChars					sz;
	CLogger					cLogger;

	cLogger.Init(NULL);
	cLogger.SetSilent(true);
	cTokenDefinitions.Init();
	cTokenMemory.Init();
	cTokenParser.Init(&cLogger, &cTokenDefinitions, &cTokenMemory, "Test3.Java", "\
  // Tickable Pins\n\
package net.simulation.common;\n\
\n\
import net.common.*;\n\
\n\
import java.util.Set; \n\
\n\
public abstract class TickablePins<\n\
	SNAPSHOT extends Snapshot,\n\
	PINS extends Pins<SNAPSHOT, PINS, ? extends IntegratedCircuit<SNAPSHOT, PINS>>,\n\
	INTEGRATED_CIRCUIT extends IntegratedCircuit<SNAPSHOT, PINS>>\n\
{\n\
  protected INTEGRATED_CIRCUIT integratedCircuit;\n\
  protected SNAPSHOT snapshot;\n\
}\
");

	tResult = cTokenParser.Parse();
	AssertTristate(TRITRUE, tResult);

	sz.Init();
	cTokenParser.PrettyPrint(&sz);

	AssertStringApproximate("\
package net.simulation.common;\n\
\n\
import net.common.*; \n\
import java.util.Set; \n\
\n\
public abstract class TickablePins<SNAPSHOT extends Snapshot, PINS extends Pins<SNAPSHOT, PINS, ? extends IntegratedCircuit<SNAPSHOT, PINS>>, INTEGRATED_CIRCUIT extends IntegratedCircuit<SNAPSHOT, PINS>>\n\
{\n\
	protected INTEGRATED_CIRCUIT integratedCircuit; \n\
	protected SNAPSHOT snapshot; \n\
}\n\
", sz.Text())

	sz.Kill();

	cTokenParser.Kill();	
	cTokenMemory.Kill();
	cTokenDefinitions.Kill();
	cLogger.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestJavaTokenParserLiterals(void)
{
	CJavaTokenDefinitions	cTokenDefinitions;
	CJavaTokenParser		cTokenParser;	
	CJavaTokenMemory		cTokenMemory;
	CJavaToken*				pcToken;
	bool					bResult;
	CLogger					cLogger;

	cLogger.Init(NULL);
	cLogger.SetSilent(true);
	cTokenDefinitions.Init();
	cTokenMemory.Init();
	cTokenParser.Init(&cLogger, &cTokenDefinitions, &cTokenMemory, "Test4.Java", "\
  public String getType()\n\
  {\n\
    long xl = 1L;\n\
    int x = 100_00;\n\
    float f = 1_000.000f;\n\
    double fl = 1.3_3e10;\n\
    char c =  '!'; \n\
	char c2 = 'A';\n\
	boolean truey = true;\n\
	boolean notTrue = !truey;\n\
	notTrue = '\\u1021' == '\\u1022';\n\
    return \"AND Gate\";\n\
  }\n");

	bResult = cTokenParser.Parse(true);
	AssertTrue(bResult);

	pcToken = cTokenParser.GetFirstToken();
	
	pcToken = AssertKeyword(&cTokenDefinitions, pcToken, JK_public);
	pcToken = AssertIdentifier(&cTokenDefinitions, pcToken, "String");
	pcToken = AssertIdentifier(&cTokenDefinitions, pcToken, "getType");
	pcToken = AssertSeparator(&cTokenDefinitions, pcToken, JS_RoundBracketLeft);
	pcToken = AssertSeparator(&cTokenDefinitions, pcToken, JS_RoundBracketRight);
	pcToken = AssertSeparator(&cTokenDefinitions, pcToken, JS_CurlyBracketLeft);
	pcToken = AssertKeyword(&cTokenDefinitions, pcToken, JK_long);
	pcToken = AssertIdentifier(&cTokenDefinitions, pcToken, "xl");
	pcToken = AssertOperator(&cTokenDefinitions, pcToken, JO_Assign);
	pcToken = AssertLiteral(&cTokenDefinitions, pcToken, 1LL);
	pcToken = AssertSeparator(&cTokenDefinitions, pcToken, JS_Semicolon);
	pcToken = AssertKeyword(&cTokenDefinitions, pcToken, JK_int);
	pcToken = AssertIdentifier(&cTokenDefinitions, pcToken, "x");
	pcToken = AssertOperator(&cTokenDefinitions, pcToken, JO_Assign);
	pcToken = AssertLiteral(&cTokenDefinitions, pcToken, 10000);
	pcToken = AssertSeparator(&cTokenDefinitions, pcToken, JS_Semicolon);
	pcToken = AssertKeyword(&cTokenDefinitions, pcToken, JK_float);
	pcToken = AssertIdentifier(&cTokenDefinitions, pcToken, "f");
	pcToken = AssertOperator(&cTokenDefinitions, pcToken, JO_Assign);
	pcToken = AssertLiteral(&cTokenDefinitions, pcToken, 1000.f);
	pcToken = AssertSeparator(&cTokenDefinitions, pcToken, JS_Semicolon);
	pcToken = AssertKeyword(&cTokenDefinitions, pcToken, JK_double);
	pcToken = AssertIdentifier(&cTokenDefinitions, pcToken, "fl");
	pcToken = AssertOperator(&cTokenDefinitions, pcToken, JO_Assign);
	pcToken = AssertLiteral(&cTokenDefinitions, pcToken, 1.33e+10);
	pcToken = AssertSeparator(&cTokenDefinitions, pcToken, JS_Semicolon);
	pcToken = AssertKeyword(&cTokenDefinitions, pcToken, JK_char);
	pcToken = AssertIdentifier(&cTokenDefinitions, pcToken, "c");
	pcToken = AssertOperator(&cTokenDefinitions, pcToken, JO_Assign);
	pcToken = AssertLiteral(&cTokenDefinitions, pcToken, '!');
	pcToken = AssertSeparator(&cTokenDefinitions, pcToken, JS_Semicolon);
	pcToken = AssertKeyword(&cTokenDefinitions, pcToken, JK_char);
	pcToken = AssertIdentifier(&cTokenDefinitions, pcToken, "c2");
	pcToken = AssertOperator(&cTokenDefinitions, pcToken, JO_Assign);
	pcToken = AssertLiteral(&cTokenDefinitions, pcToken, 'A');
	pcToken = AssertSeparator(&cTokenDefinitions, pcToken, JS_Semicolon);
	pcToken = AssertKeyword(&cTokenDefinitions, pcToken, JK_boolean);
	pcToken = AssertIdentifier(&cTokenDefinitions, pcToken, "truey");
	pcToken = AssertOperator(&cTokenDefinitions, pcToken, JO_Assign);
	pcToken = AssertLiteral(&cTokenDefinitions, pcToken, true);
	pcToken = AssertSeparator(&cTokenDefinitions, pcToken, JS_Semicolon);
	pcToken = AssertKeyword(&cTokenDefinitions, pcToken, JK_boolean);
	pcToken = AssertIdentifier(&cTokenDefinitions, pcToken, "notTrue");
	pcToken = AssertOperator(&cTokenDefinitions, pcToken, JO_Assign);
	pcToken = AssertOperator(&cTokenDefinitions, pcToken, JO_LogicalNegate);
	pcToken = AssertIdentifier(&cTokenDefinitions, pcToken, "truey");
	pcToken = AssertSeparator(&cTokenDefinitions, pcToken, JS_Semicolon);
	pcToken = AssertIdentifier(&cTokenDefinitions, pcToken, "notTrue");
	pcToken = AssertOperator(&cTokenDefinitions, pcToken, JO_Assign);
	pcToken = AssertLiteral(&cTokenDefinitions, pcToken, (char16)4129);
	pcToken = AssertOperator(&cTokenDefinitions, pcToken, JO_Equal);
	pcToken = AssertLiteral(&cTokenDefinitions, pcToken, (char16)4130);
	pcToken = AssertSeparator(&cTokenDefinitions, pcToken, JS_Semicolon);
	pcToken = AssertKeyword(&cTokenDefinitions, pcToken, JK_return);
	pcToken = AssertLiteral(&cTokenDefinitions, pcToken, "AND Gate");
	pcToken = AssertSeparator(&cTokenDefinitions, pcToken, JS_Semicolon);
	pcToken = AssertSeparator(&cTokenDefinitions, pcToken, JS_CurlyBracketRight);
	AssertNull(pcToken);

	cTokenParser.Kill();	
	cTokenMemory.Kill();
	cTokenDefinitions.Kill();
	cLogger.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestJavaTokenParserStringEscapeChars(void)
{
	CJavaTokenParser		cTokenParser;	
	CJavaTokenMemory		cTokenMemory;
	CJavaTokenDefinitions	cTokenDefinitions;
	CJavaToken*				pcToken;
	CChars					szPretty;
	bool					bResult;;
	CLogger					cLogger;

	cLogger.Init(NULL);
	cLogger.SetSilent(true);
	cTokenDefinitions.Init();
	cTokenMemory.Init();
	cTokenParser.Init(&cLogger, &cTokenDefinitions, &cTokenMemory, "Test5.Java", "return getType() + \" \\\"\" + name + \"\\\"\";");

	bResult = cTokenParser.Parse(true);
	AssertTrue(bResult);

	pcToken = cTokenParser.GetFirstToken();
	pcToken = AssertKeyword(&cTokenDefinitions, pcToken, JK_return);
	pcToken = AssertIdentifier(&cTokenDefinitions, pcToken, "getType");
	pcToken = AssertSeparator(&cTokenDefinitions, pcToken, JS_RoundBracketLeft);
	pcToken = AssertSeparator(&cTokenDefinitions, pcToken, JS_RoundBracketRight);
	pcToken = AssertOperator(&cTokenDefinitions, pcToken, JO_Plus);
	pcToken = AssertLiteral(&cTokenDefinitions, pcToken, " \"");
	pcToken = AssertOperator(&cTokenDefinitions, pcToken, JO_Plus);
	pcToken = AssertIdentifier(&cTokenDefinitions, pcToken, "name");
	pcToken = AssertOperator(&cTokenDefinitions, pcToken, JO_Plus);
	pcToken = AssertLiteral(&cTokenDefinitions, pcToken, "\"");
	pcToken = AssertSeparator(&cTokenDefinitions, pcToken, JS_Semicolon);
	AssertNull(pcToken);

	szPretty.Init();
	cTokenParser.PrettyPrint(&szPretty);
	AssertString("return getType() + \" \\\"\" + name + \"\\\"\";\n", szPretty.Text());
	szPretty.Kill();

	cTokenParser.Kill();	
	cTokenMemory.Kill();
	cTokenDefinitions.Kill();
	cLogger.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestJavaTokenParserFiles(void)
{
	CJavaTokenDefinitions	cTokenDefinitions;
	CJavaTokenParser		cTokenParser;	
	CJavaTokenMemory		cTokenMemory;
	CFileUtil				cFileUtil;
	CChars					sz;
	CTextFile				cFile;
	bool					bResult;
	CChars					szPretty;
	CLogger					cLogger;

	cLogger.Init(NULL);
	cLogger.SetSilent(true);
	sz.Init();
	cFileUtil.CurrentDirectory(&sz);
	cFileUtil.AppendToPath(&sz, "Input");
	cFileUtil.AppendToPath(&sz, "TickablePins.java");
	cFile.Init();
	bResult = cFile.Read(sz.Text());
	AssertTrue(bResult);
	cFile.PassifyNewlines();

	cTokenDefinitions.Init();
	cTokenMemory.Init();
	cTokenParser.Init(&cLogger, &cTokenDefinitions, &cTokenMemory, sz.Text(), cFile.Text());

	bResult = cTokenParser.Parse(true);
	AssertTrue(bResult);

	cFile.Kill();
	sz.Kill();

	szPretty.Init();
	cTokenParser.PrettyPrint(&szPretty);

	sz.Init();
	cFileUtil.CurrentDirectory(&sz);
	cFileUtil.AppendToPath(&sz, "Input");
	cFileUtil.AppendToPath(&sz, "TickablePins.java.expected");
	cFile.Init();
	bResult = cFile.Read(sz.Text());
	AssertTrue(bResult);
	cFile.PassifyNewlines();

	AssertStringApproximate(cFile.Text(), szPretty.Text());

	cFile.Kill();
	sz.Kill();
	szPretty.Kill();

	cTokenParser.Kill();	
	cTokenMemory.Kill();
	cTokenDefinitions.Kill();
	cLogger.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestJavaTokenParserSpecialOperators(void)
{
	CJavaTokenParser		cTokenParser;	
	CJavaTokenMemory		cTokenMemory;
	CJavaTokenDefinitions	cTokenDefinitions;
	CChars					szPretty;
	bool					bResult;;
	CLogger					cLogger;

	cLogger.Init(NULL);
	cLogger.SetSilent(true);
	cTokenDefinitions.Init();
	cTokenMemory.Init();
	cTokenParser.Init(&cLogger, &cTokenDefinitions, &cTokenMemory, "Test6.Java", "stream.forEach(System.out::println);");

	bResult = cTokenParser.Parse();
	AssertTrue(bResult);

	szPretty.Init();
	cTokenParser.PrettyPrint(&szPretty);
	AssertString("stream.forEach(System.out::println);\n", szPretty.Text());
	szPretty.Kill();

	cTokenParser.Kill();	
	cTokenMemory.Kill();

	cTokenMemory.Init();
	cTokenParser.Init(&cLogger, &cTokenDefinitions, &cTokenMemory, "Test6.Java", "stream.forEach( s-> System.out.println(s));");

	bResult = cTokenParser.Parse();
	AssertTrue(bResult);

	szPretty.Init();
	cTokenParser.PrettyPrint(&szPretty);
	AssertString("stream.forEach(s -> System.out.println(s));\n", szPretty.Text());
	szPretty.Kill();

	cTokenParser.Kill();	
	cTokenMemory.Kill();
	cTokenDefinitions.Kill();
	cLogger.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestJavaTokenParser(void)
{
	BeginTests();

	TestJavaTokenParserEndOfFile();
	TestJavaTokenParserStartAndEndWithComment();
	TestJavaTokenParserComplexGeneric();
	TestJavaTokenParserLiterals();
	TestJavaTokenParserStringEscapeChars();
	TestJavaTokenParserSpecialOperators();
	TestJavaTokenParserFiles();

	TestStatistics();
}

