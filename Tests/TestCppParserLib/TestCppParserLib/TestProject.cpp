#include "BaseLib/PointerRemapper.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "CppParserLib/Project.h"
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
//
//////////////////////////////////////////////////////////////////////////
void TestProject(void)
{
	BeginTests();

	TestNoInclude();
	TestOneInclude();
	TestReinclude();

	TestStatistics();
}

