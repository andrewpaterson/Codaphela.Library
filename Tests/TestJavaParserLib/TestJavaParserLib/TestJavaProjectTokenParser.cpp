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
void PrettyPrintFile(char* szFilename)
{
	BeginTests();

	CJavaTokenDefinitions	cTokenDefinitions;
	CJavaTokenParser		cTokenParser;
	CJavaTokenMemory		cTokenMemory;
	CTextFile				cFile;
	bool					bResult;
	CChars					szPretty;
	CLogger					cLogger;

	cLogger.Init();
	cFile.Init();
	bResult = cFile.Read(szFilename);
	AssertTrue(bResult);
	cFile.PassifyNewlines();

	cTokenDefinitions.Init();
	cTokenMemory.Init();
	cTokenParser.Init(&cLogger, &cTokenDefinitions, &cTokenMemory, szFilename, cFile.Text());

	bResult = cTokenParser.Parse(true);
	AssertTrue(bResult);

	cFile.Kill();

	szPretty.Init(" /////////////// ");
	szPretty.Append(szFilename);
	szPretty.Append(" /////////////// ");
	
	szPretty.AppendNewLine();
	cTokenParser.PrettyPrint(&szPretty);

	szPretty.Dump();
	szPretty.Kill();

	cTokenParser.Kill();
	cTokenMemory.Kill();
	cTokenDefinitions.Kill();
	cLogger.Kill();

	TestStatistics();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestJavaProjectTokenParser(void)
{
	BeginTests();

	CFileUtil				cFileUtil;
	CChars					sz;
	CArrayChars				aszJavaFilenames;
	size					i;
	CChars*					szFilename;

	sz.Init("D:" _FS_ "Work" _FS_ "658-Computer" _FS_ "logi65816" _FS_ "src");

	aszJavaFilenames.Init();
	cFileUtil.FindFilesWithExtension(sz.Text(), "java", &aszJavaFilenames, true);
	sz.Kill();

	for (i = 0; i < aszJavaFilenames.NumElements(); i++)
	{
		szFilename = aszJavaFilenames.Get(i);
		PrettyPrintFile(szFilename->Text());
	}
	aszJavaFilenames.Kill();

	TestStatistics();
}

