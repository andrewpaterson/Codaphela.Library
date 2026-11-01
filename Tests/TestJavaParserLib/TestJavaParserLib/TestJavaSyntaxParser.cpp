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
void TestJavaSyntaxParserStuff(void)
{
	CTokenParserEnvironment		cTokenParser;
	CJavaSyntaxParser			cSyntaxParser;
	CJavaSyntaxMemory			cSyntaxMemory;
	char						szFilename[] = "TestXX.Java";
	char						szFileContents[] = "\
package net.assembler;\n\
\n\
import static net.simulation.common.TraceValue.*; \n\
import net.simulation.common.TraceValue; \n\
\n\
import java.util.ArrayList; \n\
import java.util.List; \n\
import java.util.Map; \n\
\n\
public class W65C816Assembler<X, Y extends List<Map<X, ? extends Integer>>>\n\
{\n\
  private int y; \n\
  private X x; \n\
\n\
  public W65C816Assembler()\n\
  {\n\
    TraceValue value = NotConnected;\n\
    List<Map<X, Integer>> map = new ArrayList<>();\n\
    int x = 5;\n\
  }\n\
}\n\
";

	cTokenParser.Init(szFilename, szFileContents, false);
	AssertTrue(cTokenParser.Parse());

	cSyntaxMemory.Init();
	cSyntaxParser.Init(&cSyntaxMemory, cTokenParser.GetParser());

	cSyntaxParser.Parse();
	cTokenParser.DumpLog();

	cSyntaxParser.Kill();
	cSyntaxMemory.Kill();
	cTokenParser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestJavaSyntaxParser(void)
{
	BeginTests();

	AssertSize(28, sizeof(CIdentifierArray));
	TestJavaSyntaxParserStuff();

	TestStatistics();
}

