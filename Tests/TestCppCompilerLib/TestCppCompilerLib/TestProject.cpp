#include "BaseLib/PointerRemapper.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/FileUtil.h"
#include "CppCompilerLib/Project.h"
#include "CppCompilerLib/VisualStudioProject.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNoInclude(void)
{
	TypesInit();
	TypeConverterInit();
	NumberInit();

	CProject			cProject;
	CLibrary*			pcLibrary;
	CConfig*			pcConfig;
	CTranslationUnit*	pcTU3;

	cProject.Init(true);

	pcLibrary = cProject.AddLibrary("Test", ".", false);
	pcTU3 = pcLibrary->AddTranslationUnit("TestUnit3.cpp", true);
	pcConfig = pcLibrary->AddConfiguration("Debug");
	pcConfig->AddDefines("_WIN32;_DEBUG;_LIB;_CRT_SECURE_NO_DEPRECATE");

	cProject.Process("Debug");

	AssertString("     TestUnit3.cpp (Included)\n", pcTU3->GetLogs()->szIncludesLog.Text());

	cProject.Kill();

	NumberKill();
	TypeConverterKill();
	TypesKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestOneInclude(void)
{
	TypesInit();
	TypeConverterInit();
	NumberInit();

	CProject			cProject;
	CLibrary* pcLibrary;
	CConfig* pcConfig;
	CTranslationUnit* pcTU1;

	cProject.Init(true);

	pcLibrary = cProject.AddLibrary("Test", ".", false);
	pcLibrary->AddHeaderFile("TestHeader2.h");
	pcTU1 = pcLibrary->AddTranslationUnit("TestUnit4.cpp", true);
	pcConfig = pcLibrary->AddConfiguration("Debug");
	pcConfig->AddDefines("_WIN32;_DEBUG;_LIB;_CRT_SECURE_NO_DEPRECATE");

	cProject.Process("Debug");

	AssertString("\
     TestUnit4.cpp (Included)\n\
         TestHeader2.h (Included)\n", pcTU1->GetLogs()->szIncludesLog.Text());

	cProject.Kill();

	NumberKill();
	TypeConverterKill();
	TypesKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestReinclude(void)
{
	TypesInit();
	TypeConverterInit();
	NumberInit();

	CProject			cProject;
	CLibrary*			pcLibrary;
	CConfig*			pcConfig;
	CTranslationUnit*	pcTU1;
	CTranslationUnit*	pcTU2;

	cProject.Init(true);

	pcLibrary = cProject.AddLibrary("Test", ".", false);
	pcLibrary->AddHeaderFile("TestHeader.h");
	pcTU1 = pcLibrary->AddTranslationUnit("TestUnit1.cpp", true);
	pcTU2 = pcLibrary->AddTranslationUnit("TestUnit2.cpp", true);
	pcLibrary->AddHeaderFile("TestHeader2.h");
	pcConfig = pcLibrary->AddConfiguration("Debug");
	pcConfig->AddDefines("_WIN32;_DEBUG;_LIB;_CRT_SECURE_NO_DEPRECATE");

	cProject.Process("Debug");

	AssertString("\
     TestUnit1.cpp (Included)\n\
         TestHeader.h (Included)\n\
             TestHeader2.h (Included)\n\
         TestHeader.h (Included)\n\
         TestHeader.h (Included)\n", pcTU1->GetLogs()->szIncludesLog.Text());

		AssertString("\
     TestUnit2.cpp (Included)\n\
         TestHeader.h (Included)\n\
             TestHeader2.h (Included)\n\
         TestHeader.h (Included)\n\
         TestHeader.h (Included)\n\
", pcTU2->GetLogs()->szIncludesLog.Text());

	cProject.Kill();

	NumberKill();
	TypeConverterKill();
	TypesKill();
}


//////////////////////////////////////////////////////////////////////////
//
// The Post file's text (see TestWritePostFile).
//
//////////////////////////////////////////////////////////////////////////
static void AssertPostFile(const char* szExpected, const char* szFilename)
{
	CTextFile	cFile;

	cFile.Init();
	AssertTrue(cFile.Read(szFilename));
	AssertString(szExpected, cFile.Text());
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// With bWritePostFile a unit's preprocessed text is written to Post\<unit>.  Processed again (as CCPP65Builder
// does, without emptying Post) it replaces a longer file that was there.
//
//////////////////////////////////////////////////////////////////////////
void TestWritePostFile(void)
{
	TypesInit();
	TypeConverterInit();
	NumberInit();

	CProject			cProject;
	CLibrary*			pcLibrary;
	CConfig*			pcConfig;
	CTranslationUnit*	pcTU;
	CTextFile			cFile;
	CFileUtil			cFileUtil;
	const char*			szExpected;

	//TestHeader2.h's declaration, then TestUnit4.cpp without its comment.
	szExpected = "extern float gfYayForFloat;\nvoid TestUnit4(void)\n{\n};\n";

	cProject.Init(false, true);
	pcLibrary = cProject.AddLibrary("Test", ".", false);
	pcLibrary->AddHeaderFile("TestHeader2.h");
	pcTU = pcLibrary->AddTranslationUnit("TestUnit4.cpp", true);
	pcConfig = pcLibrary->AddConfiguration("Debug");
	pcConfig->AddDefines("_WIN32;_DEBUG;_LIB;_CRT_SECURE_NO_DEPRECATE");
	AssertTrue(cProject.Process("Debug"));
	AssertPostFile(szExpected, "Post" _FS_ "TestUnit4.cpp");

	cFile.Init("A stale Post file, longer than the unit's preprocessed text, from an earlier run.  Its end must not be left behind when the file is written again.\n");
	AssertTrue(cFile.Write("Post" _FS_ "TestUnit4.cpp"));
	cFile.Kill();
	AssertTrue(cProject.ProcessUnit(pcTU, "Debug"));
	AssertPostFile(szExpected, "Post" _FS_ "TestUnit4.cpp");

	cProject.Kill();
	AssertTrue(cFileUtil.Delete("Post" _FS_ "TestUnit4.cpp"));

	NumberKill();
	TypeConverterKill();
	TypesKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Without bWritePostFile no Post file is written.
//
//////////////////////////////////////////////////////////////////////////
void TestNoPostFile(void)
{
	TypesInit();
	TypeConverterInit();
	NumberInit();

	CProject			cProject;
	CLibrary*			pcLibrary;
	CConfig*			pcConfig;
	CFileUtil			cFileUtil;

	cFileUtil.Delete("Post" _FS_ "TestUnit3.cpp");

	cProject.Init(false);
	pcLibrary = cProject.AddLibrary("Test", ".", false);
	pcLibrary->AddTranslationUnit("TestUnit3.cpp", true);
	pcConfig = pcLibrary->AddConfiguration("Debug");
	pcConfig->AddDefines("_WIN32;_DEBUG;_LIB;_CRT_SECURE_NO_DEPRECATE");
	AssertTrue(cProject.Process("Debug"));
	cProject.Kill();

	AssertFalse(cFileUtil.Exists("Post" _FS_ "TestUnit3.cpp"));

	NumberKill();
	TypeConverterKill();
	TypesKill();
}


//////////////////////////////////////////////////////////////////////////
//
// An include directory that doesn't exist is reported, has no files, and
// is searched and killed like the others.
//
//////////////////////////////////////////////////////////////////////////
void TestMissingIncludeDirectory(void)
{
	TypesInit();
	TypeConverterInit();
	NumberInit();

	CProject			cProject;
	CLibrary*			pcLibrary;
	CConfig*			pcConfig;
	CTranslationUnit*	pcTU1;

	cProject.Init(true);

	AssertFalse(cProject.AddIncludeDirectory("NoSuchDirectory", false, false));
	pcLibrary = cProject.AddLibrary("Test", ".", false);
	pcLibrary->AddHeaderFile("TestHeader2.h");
	pcTU1 = pcLibrary->AddTranslationUnit("TestUnit4.cpp", true);
	pcConfig = pcLibrary->AddConfiguration("Debug");
	pcConfig->AddDefines("_WIN32;_DEBUG;_LIB;_CRT_SECURE_NO_DEPRECATE");

	AssertTrue(cProject.Process("Debug"));
	AssertString("\
     TestUnit4.cpp (Included)\n\
         TestHeader2.h (Included)\n", pcTU1->GetLogs()->szIncludesLog.Text());

	cProject.Kill();

	NumberKill();
	TypeConverterKill();
	TypesKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Writes szText to szPath.
//
//////////////////////////////////////////////////////////////////////////
static void WriteProjectFile(const char* szPath, const char* szText)
{
	CTextFile	cFile;

	cFile.Init(szText);
	AssertTrue(cFile.Write(szPath));
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A Visual Studio project's sources are named as the files are, whatever
// case the project wrote them in, a directory at a time: sub\unit.cpp is
// Sub/Unit.cpp, not Other/Unit.cpp.
//
//////////////////////////////////////////////////////////////////////////
void TestVisualStudioProjectCase(void)
{
	CFileUtil				cFileUtil;
	CVisualStudioProject	cProject;
	CChars					szExpected;

	cFileUtil.RemoveDir("Output" _FS_ "VSProject");
	AssertTrue(cFileUtil.TouchDir("Output" _FS_ "VSProject" _FS_ "Sub"));
	AssertTrue(cFileUtil.TouchDir("Output" _FS_ "VSProject" _FS_ "Other"));
	WriteProjectFile("Output" _FS_ "VSProject" _FS_ "Sub" _FS_ "Unit.cpp", "int x;\n");
	WriteProjectFile("Output" _FS_ "VSProject" _FS_ "Other" _FS_ "Unit.cpp", "int y;\n");
	WriteProjectFile("Output" _FS_ "VSProject" _FS_ "Main.cpp", "int z;\n");
	WriteProjectFile("Output" _FS_ "VSProject" _FS_ "Test.vcxproj", "<ItemGroup>\n<ClCompile Include=\"sub\\unit.cpp\" />\n<ClCompile Include=\"MAIN.cpp\" />\n<ClCompile Include=\"Missing.cpp\" />\n</ItemGroup>\n");

	cProject.Init();
	AssertTrue(cProject.Read("Output" _FS_ "VSProject" _FS_ "Test.vcxproj"));
	AssertSize(3, cProject.NumSources());
	szExpected.Init("Sub" _FS_ "Unit.cpp");
	AssertString(szExpected.Text(), cProject.GetSource(0));
	szExpected.Kill();
	AssertString("Main.cpp", cProject.GetSource(1));
	AssertString("Missing.cpp", cProject.GetSource(2));
	cProject.Kill();
	cFileUtil.RemoveDir("Output" _FS_ "VSProject");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestProject(void)
{
	BeginTests();

	TestNoInclude();
	TestOneInclude();
	TestReinclude();
	TestWritePostFile();
	TestNoPostFile();
	TestMissingIncludeDirectory();
	TestVisualStudioProjectCase();

	TestStatistics();
}

