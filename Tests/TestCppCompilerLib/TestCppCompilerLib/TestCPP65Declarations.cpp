#include <stdio.h>
#include <stdlib.h>
#include "BaseLib/FileUtil.h"
#include "CppCompilerLib/CPP65Builder.h"
#include "CppCompilerLib/CPPCompileStatistics.h"
#include "TestLib/Assert.h"
#include "TestCPP65Context.h"


//////////////////////////////////////////////////////////////////////////
//
// Builds the DeclarationsProject for eTarget, a unit at a time: First.cpp
// indexes Shapes.h, Shapes.cpp defines the classes' functions, Named.cpp
// names every class and is compiled twice, Last.cpp has main.
//
//////////////////////////////////////////////////////////////////////////
static bool TestCPP65DeclarationsBuild(CCPP65Builder* pcBuilder, ECPPTarget eTarget)
{
	static const char*	aszUnits[] = { "Input/CPP65/DeclarationsProject/First.cpp", "Input/CPP65/DeclarationsProject/Shapes.cpp", "Input/CPP65/DeclarationsProject/Named.cpp", "Input/CPP65/DeclarationsProject/Last.cpp", NULL };
	CChars				szLibc65;
	CFileUtil			cFileUtil;
	bool				bResult;

	pcBuilder->Init();
	pcBuilder->SetTarget(eTarget);
	TestCPP65AddIncludeDirectories(pcBuilder);
	bResult = pcBuilder->AddStartUp();
	if (eTarget == CPPT_W65C816)
	{
		szLibc65.Init(".." _FS_ ".." _FS_ ".." _FS_ ".." _FS_ "HCP65" _FS_ "Libraries" _FS_ "Libc65");
		cFileUtil.FullPath(&szLibc65);
		pcBuilder->AddMathTables(szLibc65.Text());
		szLibc65.Kill();
	}
	TestCPP65AddUnits(pcBuilder, aszUnits, eTarget == CPPT_65C64);
	bResult &= pcBuilder->Link();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// What compiling the DeclarationsProject counts, on both processors.  The
// counts are exact, and are the work the compiler does outside function
// bodies that grows with the headers a unit includes, not with what it
// uses of them (Libraries/CppCompilerLib/DeclarationsSpeedPlan.txt):
//
//   class layouts               each class a unit laid out (LayoutClass);
//   class layouts where defined those of them laid out only to check
//                               override and final (CheckVirtSpecifiers);
//   named instance nodes        the syntax nodes looked at for an instance
//                               written out under the class that names it;
//   quiet trials                which the layouts make.
//
// A count that rises says a change made every unit pay for classes it
// doesn't use; look at why before the number here is raised.  One that
// falls is lowered here.  The instance Named.cpp writes out is warned of
// once, though the unit is compiled twice.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65DeclarationsCounts(void)
{
	CCPP65Builder			cBuilder;
	CCPPCompileStatistics*	pcStatistics;
	STestCPP65Run			sRun;
	ECPPTarget				eTarget;
	int						i;
	bool					bBuilt;

	for (i = 0; i < 2; i++)
	{
		eTarget = (i == 0) ? CPPT_65C64 : CPPT_W65C816;
		bBuilt = TestCPP65DeclarationsBuild(&cBuilder, eTarget);
		AssertString("", cBuilder.GetErrors());
		AssertTrue(bBuilt);
		AssertSize(1, cBuilder.NumCompiledAgain());
		AssertString("Named.cpp (21, 2): Warning: CHolderTemplate<CGauge> has the name CGaugeHolder: use the name, which is a type of its own [CHolderTemplate]\n", cBuilder.GetWarnings());

		pcStatistics = cBuilder.GetStatistics();
		if (getenv("TEST_CPP65_DECLARATIONS_COUNTS"))
		{
			//The counts as they are, to put here when a change has moved them.
			fprintf(stderr, "COUNTS %s: %d layouts, %d where defined, %d nodes, %d quiet trials\n", (i == 0) ? "65C64" : "W65C816", (int)pcStatistics->GetCount(CPPCC_ClassLayouts), (int)pcStatistics->GetCount(CPPCC_ClassLayoutsWhereDefined), (int)pcStatistics->GetCount(CPPCC_NamedInstanceNodes), (int)pcStatistics->GetCount(CPPCC_QuietTrials));
		}
		AssertSize(24, (size)pcStatistics->GetCount(CPPCC_ClassLayouts));
		AssertSize(0, (size)pcStatistics->GetCount(CPPCC_ClassLayoutsWhereDefined));
		AssertSize(283, (size)pcStatistics->GetCount(CPPCC_NamedInstanceNodes));
		AssertSize(160, (size)pcStatistics->GetCount(CPPCC_QuietTrials));

		if (bBuilt && (eTarget == CPPT_65C64))
		{
			TestCPP65BeginBuilt(&sRun, &cBuilder);
			AssertTrue(TestCPP65Run(&sRun, 1000000) < 1000000);
			AssertInt(9 + 2 + 1 + 1, (int32)TestCPP65MPU(&sRun)->GetA());
			TestCPP65End(&sRun);
		}
		cBuilder.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Builds szSource as the one unit of a program and gives what the compiler
// said of it.
//
//////////////////////////////////////////////////////////////////////////
static bool TestCPP65DeclarationsSource(const char* szSource, CChars* pszErrors)
{
	CCPP65Builder	cBuilder;
	bool			bBuilt;

	cBuilder.Init();
	TestCPP65AddIncludeDirectories(&cBuilder);
	bBuilt = cBuilder.AddStartUp();
	bBuilt &= cBuilder.AddSource("Unit.cpp", szSource);
	bBuilt &= cBuilder.Link();
	pszErrors->Init(cBuilder.GetErrors());
	cBuilder.Kill();
	return bBuilt;
}


//////////////////////////////////////////////////////////////////////////
//
// Where a class's override that overrides nothing is reported.  A class
// written in the unit's own file is laid out where it is defined, used or
// not, and is refused there.  A header's class (WrongOverride.h) is laid
// out by the unit that first needs it whole and refused there; a unit that
// only names it doesn't lay it out, as every unit would otherwise lay out
// every class its headers mention (CCPPUnit::CheckVirtSpecifiers).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65DeclarationsHeaderClass(void)
{
	CChars	szErrors;
	bool	bBuilt;

	//Only named.
	bBuilt = TestCPP65DeclarationsSource("#include \"DeclarationsProject/WrongOverride.h\"\nint Named(CWrong* pcWrong) { return pcWrong ? 1 : 0; }\nint main() { return Named(0); }\n", &szErrors);
	AssertString("", szErrors.Text());
	AssertTrue(bBuilt);
	szErrors.Kill();

	//Needed whole.
	bBuilt = TestCPP65DeclarationsSource("#include \"DeclarationsProject/WrongOverride.h\"\nint main() { CWrong cWrong; return cWrong.Value(); }\n", &szErrors);
	AssertString("WrongOverride.h (16, 6): The function is declared override but overrides no virtual function of a base class [Other]\n", szErrors.Text());
	AssertFalse(bBuilt);
	szErrors.Kill();

	//Written in the unit's own file, and not used.
	bBuilt = TestCPP65DeclarationsSource("class CRight { public: virtual int Value(void) { return 1; } };\nclass CWrong : public CRight { public: int Other(void) override { return 2; } };\nint main() { return 0; }\n", &szErrors);
	AssertString("Unit.cpp (2, 44): The function is declared override but overrides no virtual function of a base class [Other]\n", szErrors.Text());
	AssertFalse(bBuilt);
	szErrors.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A header's class is laid out when a unit first needs it whole, so what
// asks a class about its members must lay it out first.  Assigning a base
// to a class that takes the base's operator= by a using-declaration
// (AssignBase.h) is the first thing this unit does with the class, and
// calls that operator: CCPPUnit::HasOperator read the class's members
// before it had any, found no operator and refused the assignment ("The
// object cannot be copied to that class"), which stopped SupportLib's
// ShapeSubtraction.cpp compiling.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65DeclarationsHeaderAssignment(void)
{
	CCPP65Builder	cBuilder;
	STestCPP65Run	sRun;
	bool			bBuilt;

	cBuilder.Init();
	TestCPP65AddIncludeDirectories(&cBuilder);
	bBuilt = cBuilder.AddStartUp();
	bBuilt &= cBuilder.AddSource("Unit.cpp",
		"#include \"DeclarationsProject/AssignBase.h\"\n"
		"int Assign(CAssignDerived* pcTo, CAssignBase* pcFrom) { *pcTo = *pcFrom; return pcTo->miValue; }\n"
		"int main() { CAssignDerived cTo; CAssignBase cFrom; cFrom.miValue = 4; return Assign(&cTo, &cFrom); }\n");
	bBuilt &= cBuilder.Link();
	AssertString("", cBuilder.GetErrors());
	AssertTrue(bBuilt);
	if (bBuilt)
	{
		TestCPP65BeginBuilt(&sRun, &cBuilder);
		AssertTrue(TestCPP65Run(&sRun, 1000000) < 1000000);
		AssertInt(5, (int32)TestCPP65MPU(&sRun)->GetA());
		TestCPP65End(&sRun);
	}
	cBuilder.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65Declarations(void)
{
	BeginTests();

	TestCPP65DeclarationsCounts();
	TestCPP65DeclarationsHeaderClass();
	TestCPP65DeclarationsHeaderAssignment();

	TestStatistics();
}
