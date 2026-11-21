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
		AssertSize(56, (size)pcStatistics->GetCount(CPPCC_ClassLayouts));
		AssertSize(41, (size)pcStatistics->GetCount(CPPCC_ClassLayoutsWhereDefined));
		AssertSize(283, (size)pcStatistics->GetCount(CPPCC_NamedInstanceNodes));
		AssertSize(210, (size)pcStatistics->GetCount(CPPCC_QuietTrials));

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
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65Declarations(void)
{
	BeginTests();

	TestCPP65DeclarationsCounts();

	TestStatistics();
}
