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
void TestJavaSyntaxParserImport(void)
{
	CTokenParserEnvironment		cTokenParser;
	CJavaSyntaxParser			cSyntaxParser;
	CJavaSyntaxMemory			cSyntaxMemory;
	CChars						sz;
	char						szFilename[] = "TestJavaSyntaxParserImport.Java";
	char						szFileContents[] = "\
package net.assembler;\n\
\n\
import static net.simulation.common.TraceValue.*; \n\
import net.simulation.common.TraceValue; \n\
\n\
import java.util.ArrayList; \n\
import java.util.List; \n\
import java.util.Map; \n\
";

	cTokenParser.Init(szFilename, szFileContents, true);
	AssertTrue(cTokenParser.Parse());

	cSyntaxMemory.Init();
	cSyntaxParser.Init(&cSyntaxMemory, cTokenParser.GetParser());

	cSyntaxParser.Parse();
	sz.Init();
	cSyntaxParser.TypePrint(&sz);
	AssertString("\
File: TestJavaSyntaxParserImport.Java\n\
  Package: net.assembler\n\
  Import: net.simulation.common.TraceValue\n\
  Import: net.simulation.common.TraceValue\n\
  Import: java.util.ArrayList\n\
  Import: java.util.List\n\
  Import: java.util.Map\n", sz.Text());
	sz.Kill();

	cSyntaxParser.Kill();
	cSyntaxMemory.Kill();
	cTokenParser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestJavaSyntaxParserPackageError(void)
{
	CTokenParserEnvironment		cTokenParser;
	CJavaSyntaxParser			cSyntaxParser;
	CJavaSyntaxMemory			cSyntaxMemory;
	CChars						szOutput;
	bool						bResult;

	cTokenParser.Init(NULL, "package;", false);
	AssertTrue(cTokenParser.Parse());
	cSyntaxMemory.Init();
	cSyntaxParser.Init(&cSyntaxMemory, cTokenParser.GetParser());
	bResult = cSyntaxParser.Parse();
	AssertFalse(bResult);

	AssertString("ERROR: Identifier expected.\n\
package;\n\
       ^", cTokenParser.GetOutput(&szOutput));
	szOutput.Kill();
	cSyntaxParser.Kill();
	cSyntaxMemory.Kill();
	cTokenParser.Kill();


	cTokenParser.Init(NULL, "package net.;", false);
	AssertTrue(cTokenParser.Parse());
	cSyntaxMemory.Init();
	cSyntaxParser.Init(&cSyntaxMemory, cTokenParser.GetParser());
	bResult = cSyntaxParser.Parse();
	AssertFalse(bResult);

	AssertString("ERROR: Identifier expected.\n\
package net.;\n\
            ^", cTokenParser.GetOutput(&szOutput));
	szOutput.Kill();
	cSyntaxParser.Kill();
	cSyntaxMemory.Kill();
	cTokenParser.Kill();


	cTokenParser.Init(NULL, "package net.;", false);
	AssertTrue(cTokenParser.Parse());
	cSyntaxMemory.Init();
	cSyntaxParser.Init(&cSyntaxMemory, cTokenParser.GetParser());
	bResult = cSyntaxParser.Parse();
	AssertFalse(bResult);

	AssertString("ERROR: Identifier expected.\n\
package net.;\n\
            ^", cTokenParser.GetOutput(&szOutput));
	szOutput.Kill();
	cSyntaxParser.Kill();
	cSyntaxMemory.Kill();
	cTokenParser.Kill();


	cTokenParser.Init(NULL, "package net.3.parse;", false);
	AssertTrue(cTokenParser.Parse());
	cSyntaxMemory.Init();
	cSyntaxParser.Init(&cSyntaxMemory, cTokenParser.GetParser());
	bResult = cSyntaxParser.Parse();
	AssertFalse(bResult);

	AssertString("ERROR: Identifier expected.\n\
package net.3.parse;\n\
            ^", cTokenParser.GetOutput(&szOutput));
	szOutput.Kill();
	cSyntaxParser.Kill();
	cSyntaxMemory.Kill();
	cTokenParser.Kill();


	cTokenParser.Init(NULL, "package net", false);
	AssertTrue(cTokenParser.Parse());
	cSyntaxMemory.Init();
	cSyntaxParser.Init(&cSyntaxMemory, cTokenParser.GetParser());
	bResult = cSyntaxParser.Parse();
	AssertFalse(bResult);

	AssertString("ERROR: '.' or ';' expected.\n\
package net\n\
           ^", cTokenParser.GetOutput(&szOutput));
	szOutput.Kill();
	cSyntaxParser.Kill();
	cSyntaxMemory.Kill();
	cTokenParser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestJavaSyntaxParserImportError(void)
{
	CTokenParserEnvironment		cTokenParser;
	CJavaSyntaxParser			cSyntaxParser;
	CJavaSyntaxMemory			cSyntaxMemory;
	CChars						szOutput;
	bool						bResult;

	cTokenParser.Init(NULL, "import static net.simulation.common.TraceValue.*", false);
	AssertTrue(cTokenParser.Parse());
	cSyntaxMemory.Init();
	cSyntaxParser.Init(&cSyntaxMemory, cTokenParser.GetParser());
	bResult = cSyntaxParser.Parse();
	AssertFalse(bResult);

	AssertString("ERROR: ';' expected.\n\
import static net.simulation.common.TraceValue.*\n\
                                                ^", 
		cTokenParser.GetOutput(&szOutput));
	szOutput.Kill();
	cSyntaxParser.Kill();
	cSyntaxMemory.Kill();
	cTokenParser.Kill();


	cTokenParser.Init(NULL, "import static.simulation.common.TraceValue.*;", false);
	AssertTrue(cTokenParser.Parse());
	cSyntaxMemory.Init();
	cSyntaxParser.Init(&cSyntaxMemory, cTokenParser.GetParser());
	bResult = cSyntaxParser.Parse();
	AssertFalse(bResult);

	AssertString("ERROR: Identifier expected.\n\
import static.simulation.common.TraceValue.*;\n\
             ^",
		cTokenParser.GetOutput(&szOutput));
	szOutput.Kill();
	cSyntaxParser.Kill();
	cSyntaxMemory.Kill();
	cTokenParser.Kill();


	cTokenParser.Init(NULL, "import static ;", false);
	//cTokenParser.Init(NULL, "import static net.simulation.common.TraceValue.*", false);
	AssertTrue(cTokenParser.Parse());
	cSyntaxMemory.Init();
	cSyntaxParser.Init(&cSyntaxMemory, cTokenParser.GetParser());
	bResult = cSyntaxParser.Parse();
	AssertFalse(bResult);

	AssertString("ERROR: Identifier expected.\n\
import static ;\n\
              ^",
		cTokenParser.GetOutput(&szOutput));
	szOutput.Kill();
	cSyntaxParser.Kill();
	cSyntaxMemory.Kill();
	cTokenParser.Kill();


	cTokenParser.Init(NULL, "import static *;", false);
	//cTokenParser.Init(NULL, "import static net.simulation.common.TraceValue.*", false);
	AssertTrue(cTokenParser.Parse());
	cSyntaxMemory.Init();
	cSyntaxParser.Init(&cSyntaxMemory, cTokenParser.GetParser());
	bResult = cSyntaxParser.Parse();
	AssertFalse(bResult);

	AssertString("ERROR: Identifier expected.\n\
import static *;\n\
              ^",
		cTokenParser.GetOutput(&szOutput));
	szOutput.Kill();
	cSyntaxParser.Kill();
	cSyntaxMemory.Kill();
	cTokenParser.Kill();


	cTokenParser.Init(NULL, "import net.simulation.;", false);
	AssertTrue(cTokenParser.Parse());
	cSyntaxMemory.Init();
	cSyntaxParser.Init(&cSyntaxMemory, cTokenParser.GetParser());
	bResult = cSyntaxParser.Parse();
	AssertFalse(bResult);

	AssertString("ERROR: Identifier expected.\n\
import net.simulation.;\n\
                      ^",
		cTokenParser.GetOutput(&szOutput));
	szOutput.Kill();
	cSyntaxParser.Kill();
	cSyntaxMemory.Kill();
	cTokenParser.Kill();


	cTokenParser.Init(NULL, "import", false);
	AssertTrue(cTokenParser.Parse());
	cSyntaxMemory.Init();
	cSyntaxParser.Init(&cSyntaxMemory, cTokenParser.GetParser());
	bResult = cSyntaxParser.Parse();
	AssertFalse(bResult);

	AssertString("ERROR: Identifier expected.\n\
import\n\
      ^",
		cTokenParser.GetOutput(&szOutput));
	szOutput.Kill();
	cSyntaxParser.Kill();
	cSyntaxMemory.Kill();
	cTokenParser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestJavaSyntaxParserImports(void)
{
	BeginTests();

	TestJavaSyntaxParserImport();
	TestJavaSyntaxParserPackageError();
	TestJavaSyntaxParserImportError();

	TestStatistics();
}

