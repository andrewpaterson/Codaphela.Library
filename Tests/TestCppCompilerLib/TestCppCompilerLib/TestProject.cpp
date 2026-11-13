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
// An include directory added without its sub directories finds a header
// in one when a unit names it ("Sub/Deep.h"), as a C++ compiler's -I
// does, and the same header for every unit.  A name that isn't there, or
// leaves the directory, is not found.
//
//////////////////////////////////////////////////////////////////////////
void TestIncludeDirectorySubDirectory(void)
{
	CFileUtil			cFileUtil;
	CProject			cProject;
	CLibrary*			pcLibrary;
	CConfig*			pcConfig;
	CTranslationUnit*	pcTU1;
	CTranslationUnit*	pcTU2;
	CChars				szErrors;
	CChars				szDeep;
	bool				bResult;

	TypesInit();
	TypeConverterInit();
	NumberInit();

	cFileUtil.RemoveDir("Output" _FS_ "IncludeSub");
	cFileUtil.TouchDir("Output" _FS_ "IncludeSub" _FS_ "Base" _FS_ "Sub");
	cFileUtil.TouchDir("Output" _FS_ "IncludeSub" _FS_ "Unit");
	AssertTrue(cFileUtil.IsDirectory("Output" _FS_ "IncludeSub" _FS_ "Base" _FS_ "Sub"));
	AssertTrue(cFileUtil.IsDirectory("Output" _FS_ "IncludeSub" _FS_ "Unit"));
	WriteProjectFile("Output" _FS_ "IncludeSub" _FS_ "Base" _FS_ "Top.h", "int giTop;\n");
	WriteProjectFile("Output" _FS_ "IncludeSub" _FS_ "Base" _FS_ "Sub" _FS_ "Deep.h", "#pragma once\nint giDeep;\n");
	WriteProjectFile("Output" _FS_ "IncludeSub" _FS_ "Outside.h", "int giOutside;\n");
	WriteProjectFile("Output" _FS_ "IncludeSub" _FS_ "Unit" _FS_ "Unit1.cpp", "#include \"Top.h\"\n#include \"Sub/Deep.h\"\n");
	WriteProjectFile("Output" _FS_ "IncludeSub" _FS_ "Unit" _FS_ "Unit2.cpp", "#include <Sub/Deep.h>\n");
	WriteProjectFile("Output" _FS_ "IncludeSub" _FS_ "Unit" _FS_ "Missing.cpp", "#include \"Sub/Missing.h\"\n");
	WriteProjectFile("Output" _FS_ "IncludeSub" _FS_ "Unit" _FS_ "Leaves.cpp", "#include \"Sub/../../Outside.h\"\n");

	cProject.Init(true);
	AssertTrue(cProject.AddIncludeDirectory("Output" _FS_ "IncludeSub" _FS_ "Base", false, false));
	pcLibrary = cProject.AddLibrary("Test", "Output" _FS_ "IncludeSub" _FS_ "Unit", false);
	pcTU1 = pcLibrary->AddTranslationUnit("Unit1.cpp", true);
	pcTU2 = pcLibrary->AddTranslationUnit("Unit2.cpp", true);
	pcConfig = pcLibrary->AddConfiguration("Debug");
	pcConfig->AddDefines("_DEBUG");

	szErrors.Init();
	bResult = cProject.Process("Debug", &szErrors);
	AssertString("", szErrors.Text());
	AssertTrue(bResult);
	szErrors.Kill();
	AssertString("\
     Unit1.cpp (Included)\n\
         Top.h (Included)\n\
         Deep.h (Included)\n", pcTU1->GetLogs()->szIncludesLog.Text());
	AssertString("\
     Unit2.cpp (Included)\n\
         Deep.h (Included)\n", pcTU2->GetLogs()->szIncludesLog.Text());
	szDeep.Init("Output" _FS_ "IncludeSub" _FS_ "Base" _FS_ "Sub" _FS_ "Deep.h");
	cFileUtil.FullPath(&szDeep);
	AssertNotNull(cProject.GetIncludeFile(szDeep.Text()));
	szDeep.Kill();
	cProject.Kill();

	cProject.Init(true);
	AssertTrue(cProject.AddIncludeDirectory("Output" _FS_ "IncludeSub" _FS_ "Base", false, false));
	pcLibrary = cProject.AddLibrary("Test", "Output" _FS_ "IncludeSub" _FS_ "Unit", false);
	pcLibrary->AddTranslationUnit("Missing.cpp", true);
	pcConfig = pcLibrary->AddConfiguration("Debug");
	pcConfig->AddDefines("_DEBUG");
	szErrors.Init();
	AssertFalse(cProject.Process("Debug", &szErrors));
	AssertTrue(szErrors.Contains("Sub/Missing.h"));
	szErrors.Kill();
	cProject.Kill();

	cProject.Init(true);
	AssertTrue(cProject.AddIncludeDirectory("Output" _FS_ "IncludeSub" _FS_ "Base", false, false));
	pcLibrary = cProject.AddLibrary("Test", "Output" _FS_ "IncludeSub" _FS_ "Unit", false);
	pcLibrary->AddTranslationUnit("Leaves.cpp", true);
	pcConfig = pcLibrary->AddConfiguration("Debug");
	pcConfig->AddDefines("_DEBUG");
	szErrors.Init();
	AssertFalse(cProject.Process("Debug", &szErrors));
	szErrors.Kill();
	cProject.Kill();

	cFileUtil.RemoveDir("Output" _FS_ "IncludeSub");

	NumberKill();
	TypeConverterKill();
	TypesKill();
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
	TestIncludeDirectorySubDirectory();

	TestStatistics();
}

