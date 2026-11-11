#include "CppCompilerLib/IRParser.h"
#include "CppCompilerLib/IRImports.h"
#include "TestLib/Assert.h"


#define TEST_IR_IMPORTS_MAX_RESUMES	8


//What the imports resumed (CIRImports::SetResume), in order.
struct STestIRImportsResumes
{
	void*	apvUnits[TEST_IR_IMPORTS_MAX_RESUMES];
	bool	abStale[TEST_IR_IMPORTS_MAX_RESUMES];
	size	uiNum;
};


//////////////////////////////////////////////////////////////////////////
//
// The resume function: records the unit.
//
//////////////////////////////////////////////////////////////////////////
static void TestIRImportsResume(void* pvContext, void* pvUnit, bool bStale)
{
	STestIRImportsResumes*	psResumes;

	psResumes = (STestIRImportsResumes*)pvContext;
	if (psResumes->uiNum < TEST_IR_IMPORTS_MAX_RESUMES)
	{
		psResumes->apvUnits[psResumes->uiNum] = pvUnit;
		psResumes->abStale[psResumes->uiNum] = bStale;
	}
	psResumes->uiNum++;
}


//////////////////////////////////////////////////////////////////////////
//
// Imports with no turns taken, resuming into psResumes.
//
//////////////////////////////////////////////////////////////////////////
static void TestIRImportsBegin(CIRImports* pcImports, STestIRImportsResumes* psResumes)
{
	psResumes->uiNum = 0;
	pcImports->Init(16);
	pcImports->SetResume(TestIRImportsResume, psResumes);
	pcImports->ResetTurns();
}


//////////////////////////////////////////////////////////////////////////
//
// Exports a small function called szName (as a unit in its turn does).
//
//////////////////////////////////////////////////////////////////////////
static void TestIRImportsExport(CIRImports* pcImports, const char* szName)
{
	CIRModule	cModule;
	CCPPErrors	cErrors;
	CIRParser	cParser;
	CChars		szText;

	szText.Init("space default 4\n\nfunction void @");
	szText.Append(szName);
	szText.Append("(i32 %i)\n{\nentry:\n\tret\n}\n");
	cErrors.Init();
	cModule.Init();
	cParser.Init(&cModule, &cErrors);
	AssertTrue(cParser.Parse(szText.Text()));
	cParser.Kill();
	AssertTrue(pcImports->Export(&cModule, cModule.GetFunction(0)));
	AssertNotNull(pcImports->Find(szName));
	cModule.Kill();
	cErrors.Kill();
	szText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A unit that arrives in its turn goes on; one that arrives before it is
// parked and resumed when the turn before it ends (not before).
//
//////////////////////////////////////////////////////////////////////////
void TestIRImportsArriveInOrder(void)
{
	CIRImports				cImports;
	STestIRImportsResumes	sResumes;
	int						aiUnits[3];

	TestIRImportsBegin(&cImports, &sResumes);

	AssertInt(IRA_Turn, cImports.Arrive(0, &aiUnits[0], NULL));
	AssertInt(IRA_Parked, cImports.Arrive(2, &aiUnits[2], NULL));
	AssertInt(IRA_Parked, cImports.Arrive(1, &aiUnits[1], NULL));
	AssertSize(0, sResumes.uiNum);

	cImports.EndTurn(0);
	AssertSize(1, sResumes.uiNum);
	AssertPointer(&aiUnits[1], sResumes.apvUnits[0]);
	AssertFalse(sResumes.abStale[0]);

	//Ending a turn again does nothing.
	cImports.EndTurn(0);
	AssertSize(1, sResumes.uiNum);

	cImports.EndTurn(1);
	AssertSize(2, sResumes.uiNum);
	AssertPointer(&aiUnits[2], sResumes.apvUnits[1]);
	AssertFalse(sResumes.abStale[1]);

	cImports.EndTurn(2);
	AssertSize(2, sResumes.uiNum);
	AssertSize(0, cImports.NumStale());
	cImports.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A parked unit watching names (it looked for them and didn't find them)
// is resumed stale when one is exported, not when another is, and not
// again in its turn.  A unit arriving after one was exported is stale at
// once.
//
//////////////////////////////////////////////////////////////////////////
void TestIRImportsArriveStale(void)
{
	CIRImports				cImports;
	STestIRImportsResumes	sResumes;
	int						aiUnits[3];
	CArrayChars				aszWatched;
	CArrayChars				aszEarly;

	TestIRImportsBegin(&cImports, &sResumes);
	aszWatched.Init();
	aszWatched.Add("Watched");
	aszWatched.Add("AlsoWatched");
	aszEarly.Init();
	aszEarly.Add("Early");

	AssertInt(IRA_Turn, cImports.Arrive(0, &aiUnits[0], NULL));
	AssertInt(IRA_Parked, cImports.Arrive(1, &aiUnits[1], &aszWatched));

	//Unit 0, in its turn, exports.
	TestIRImportsExport(&cImports, "Other");
	AssertSize(0, sResumes.uiNum);
	TestIRImportsExport(&cImports, "AlsoWatched");
	AssertSize(1, sResumes.uiNum);
	AssertPointer(&aiUnits[1], sResumes.apvUnits[0]);
	AssertTrue(sResumes.abStale[0]);
	AssertSize(1, cImports.NumStale());

	//Exporting another it watched, or its turn coming, doesn't resume it again.
	TestIRImportsExport(&cImports, "Watched");
	cImports.EndTurn(0);
	AssertSize(1, sResumes.uiNum);

	//Done again (it is resumed stale, so generated again), it arrives in its turn.
	AssertInt(IRA_Turn, cImports.Arrive(1, &aiUnits[1], &aszWatched));
	TestIRImportsExport(&cImports, "Early");

	//Exported before unit 2 arrived: it is stale at once, not parked (nothing resumes it).
	AssertInt(IRA_Stale, cImports.Arrive(2, &aiUnits[2], &aszEarly));
	AssertSize(2, cImports.NumStale());
	cImports.EndTurn(1);
	AssertSize(1, sResumes.uiNum);
	AssertInt(IRA_Turn, cImports.Arrive(2, &aiUnits[2], &aszEarly));
	cImports.EndTurn(2);

	aszEarly.Kill();
	aszWatched.Kill();
	cImports.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A parked unit resumed in its turn isn't resumed again (stale) when a
// name it watched is exported after.
//
//////////////////////////////////////////////////////////////////////////
void TestIRImportsResumedOnce(void)
{
	CIRImports				cImports;
	STestIRImportsResumes	sResumes;
	int						aiUnits[2];
	CArrayChars				aszWatched;

	TestIRImportsBegin(&cImports, &sResumes);
	aszWatched.Init();
	aszWatched.Add("Late");

	AssertInt(IRA_Turn, cImports.Arrive(0, &aiUnits[0], NULL));
	AssertInt(IRA_Parked, cImports.Arrive(1, &aiUnits[1], &aszWatched));
	cImports.EndTurn(0);
	AssertSize(1, sResumes.uiNum);
	AssertFalse(sResumes.abStale[0]);

	//Unit 1, in its turn, exports what it watched: it isn't parked any more.
	TestIRImportsExport(&cImports, "Late");
	AssertSize(1, sResumes.uiNum);
	AssertSize(0, cImports.NumStale());

	//In its turn it finds it missed it (it is generated again).
	AssertTrue(cImports.Missed(&aszWatched));
	AssertSize(1, cImports.NumMissed());
	cImports.EndTurn(1);

	aszWatched.Kill();
	cImports.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Missed: a name looked for and not found that has been exported since;
// with SetForceMisses each turn misses once.
//
//////////////////////////////////////////////////////////////////////////
void TestIRImportsMissed(void)
{
	CIRImports				cImports;
	STestIRImportsResumes	sResumes;
	int						iUnit;
	CArrayChars				aszNotFound;

	TestIRImportsBegin(&cImports, &sResumes);
	aszNotFound.Init();
	aszNotFound.Add("NotYet");

	AssertInt(IRA_Turn, cImports.Arrive(0, &iUnit, &aszNotFound));
	AssertFalse(cImports.Missed(&aszNotFound));
	TestIRImportsExport(&cImports, "NotYet");
	cImports.EndTurn(0);

	AssertInt(IRA_Turn, cImports.Arrive(1, &iUnit, NULL));
	AssertTrue(cImports.Missed(&aszNotFound));
	cImports.EndTurn(1);

	cImports.SetForceMisses(true);
	AssertInt(IRA_Turn, cImports.Arrive(2, &iUnit, NULL));
	aszNotFound.ReInit();
	AssertTrue(cImports.Missed(&aszNotFound));
	AssertFalse(cImports.Missed(&aszNotFound));
	cImports.EndTurn(2);
	AssertSize(2, cImports.NumMissed());

	aszNotFound.Kill();
	cImports.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIRImportsTurns(void)
{
	BeginTests();

	TestIRImportsArriveInOrder();
	TestIRImportsArriveStale();
	TestIRImportsResumedOnce();
	TestIRImportsMissed();

	TestStatistics();
}
