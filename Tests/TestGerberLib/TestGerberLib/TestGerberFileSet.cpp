#include "BaseLib/LogToMemory.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/NumberControl.h"
#include "GerberLib/GerberFileSets.h"
#include "GerberLib/GerberInterpreter.h"
#include "GerberLib/GerberOutlineBounds.h"
#include "TestLib/Assert.h"


TRISTATE ParseGerberText(char* szText, CGerberCommands* pcCommands);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertGerberLayer(CGerberFileSet* pcSet, EGerberBoardLayer eLayer, EGerberBoardSide eSide, const char* szFileName)
{
	CGerberLayerFile*	pcLayer;

	pcLayer = pcSet->GetLayer(eLayer, eSide);
	AssertNotNull(pcLayer);
	if (pcLayer)
	{
		AssertString(szFileName, pcLayer->GetFileName());
		AssertTrue(pcLayer->GetCommands()->NumCommands() > 0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberFileSetLoad(void)
{
	CGerberFileSet		cSet;
	CGerberDrillFile*	pcDrill;
	size				i;

	cSet.Init();
	AssertTrue(cSet.Load("Input" _FS_ "KiCad" _FS_ "Clock Doubler"));
	AssertString("Clock Doubler", cSet.GetName());
	AssertString("Input" _FS_ "KiCad" _FS_ "Clock Doubler", cSet.GetDirectory());

	//KiCad's two layer fab output: nine Gerber layers and plated and non-plated drills.
	AssertSize(9, cSet.NumLayers());
	AssertSize(2, cSet.NumDrills());
	AssertString("Clock Doubler-B_Cu.gbl", cSet.GetLayer((size)0)->GetFileName());
	AssertNull(cSet.GetLayer((size)9));

	AssertGerberLayer(&cSet, GBL_Copper, GBS_Top, "Clock Doubler-F_Cu.gtl");
	AssertGerberLayer(&cSet, GBL_Copper, GBS_Bottom, "Clock Doubler-B_Cu.gbl");
	AssertGerberLayer(&cSet, GBL_SolderMask, GBS_Top, "Clock Doubler-F_Mask.gts");
	AssertGerberLayer(&cSet, GBL_SolderMask, GBS_Bottom, "Clock Doubler-B_Mask.gbs");
	AssertGerberLayer(&cSet, GBL_Paste, GBS_Top, "Clock Doubler-F_Paste.gtp");
	AssertGerberLayer(&cSet, GBL_Paste, GBS_Bottom, "Clock Doubler-B_Paste.gbp");
	AssertGerberLayer(&cSet, GBL_Silk, GBS_Top, "Clock Doubler-F_Silkscreen.gto");
	AssertGerberLayer(&cSet, GBL_Silk, GBS_Bottom, "Clock Doubler-B_Silkscreen.gbo");
	AssertGerberLayer(&cSet, GBL_Outline, GBS_Both, "Clock Doubler-Edge_Cuts.gm1");
	AssertNull(cSet.GetLayer(GBL_Copper, GBS_Internal));

	pcDrill = cSet.GetDrill(GDP_Plated);
	AssertNotNull(pcDrill);
	AssertString("Clock Doubler-PTH.drl", pcDrill->GetFileName());
	AssertTrue(pcDrill->GetCommands()->NumCommands() > 0);
	pcDrill = cSet.GetDrill(GDP_NonPlated);
	AssertNotNull(pcDrill);
	AssertString("Clock Doubler-NPTH.drl", pcDrill->GetFileName());
	AssertNull(cSet.GetDrill(GDP_Unknown));

	for (i = 0; i < cSet.NumLayers(); i++)
	{
		AssertTrue(cSet.GetLayer(i)->GetLayer() != GBL_Unknown);
		AssertTrue(cSet.GetLayer(i)->GetSide() != GBS_Unknown);
	}

	cSet.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberFileSetNameFromDirectory(void)
{
	CGerberFileSet	cSet;

	//The set is named after the directory, not the file names.  The Adder 8bit files are called "Added 8bit".
	cSet.Init();
	AssertTrue(cSet.Load("Input" _FS_ "KiCad" _FS_ "Adder 8bit" _FS_));
	AssertString("Adder 8bit", cSet.GetName());
	AssertString("Input" _FS_ "KiCad" _FS_ "Adder 8bit", cSet.GetDirectory());
	AssertGerberLayer(&cSet, GBL_Copper, GBS_Top, "Added 8bit-F_Cu.gtl");
	cSet.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberFileSetsAddDirectories(void)
{
	CGerberFileSets		cSets;
	CGerberFileSet*		pcSet;
	size				i;
	const char*			aszNames[] = {	"Adder 16bit", "Adder 8bit", "Address Buffer 24bit", "Clock Doubler", "Clock Hold Select", "Clock Transition",
										"Counter 16bit", "Main Address Buffer", "Main Data Buffer", "Selector 1-of-4 4bit", "State Reader 8bit" };

	cSets.Init();
	AssertTrue(cSets.AddDirectories("Input" _FS_ "KiCad"));
	AssertSize(11, cSets.NumSets());

	for (i = 0; i < cSets.NumSets(); i++)
	{
		pcSet = cSets.Get(i);
		AssertString(aszNames[i], pcSet->GetName());
		AssertSize(9, pcSet->NumLayers());
		AssertSize(2, pcSet->NumDrills());
		AssertNotNull(pcSet->GetLayer(GBL_Copper, GBS_Top));
		AssertNotNull(pcSet->GetLayer(GBL_Copper, GBS_Bottom));
		AssertNotNull(pcSet->GetLayer(GBL_Outline, GBS_Both));
		AssertNotNull(pcSet->GetDrill(GDP_Plated));
		AssertNotNull(pcSet->GetDrill(GDP_NonPlated));
	}
	AssertNull(cSets.Get((size)11));

	pcSet = cSets.Get("Main Data Buffer");
	AssertNotNull(pcSet);
	AssertString("Main Data Buffer", pcSet->GetName());
	AssertNull(cSets.Get("Big Board"));

	cSets.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberFileSetsAddErrors(void)
{
	CGerberFileSets		cSets;
	CLogToMemory		cMemoryLog;
	char				szLogText[1000];

	cSets.Init();
	AssertNotNull(cSets.Add("Input" _FS_ "KiCad" _FS_ "Clock Doubler"));

	//Two sets can't have the same name.
	cMemoryLog.Start(true);
	AssertNull(cSets.Add("Input" _FS_ "KiCad" _FS_ "Clock Doubler"));
	cMemoryLog.Stop(szLogText, 1000);
	AssertTrue(strstr(szLogText, "A file set named [Clock Doubler] has already been added.") != NULL);
	AssertSize(1, cSets.NumSets());

	//A directory with no Gerber files.
	cMemoryLog.Start(true);
	AssertNull(cSets.Add("Input" _FS_ "Missing Board"));
	cMemoryLog.Stop(szLogText, 1000);
	AssertTrue(strstr(szLogText, "Missing Board") != NULL);
	AssertSize(1, cSets.NumSets());

	cSets.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool GetGerberOutlineBounds(char* szText, SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum)
{
	CGerberCommands			cCommands;
	CGerberInterpreter		cInterpreter;
	CGerberOutlineBounds	cBounds;
	CFloat64Rect			cRect;
	bool					bResult;

	cCommands.Init();
	AssertTritrue(ParseGerberText(szText, &cCommands));
	cBounds.Init();
	cInterpreter.Init(&cCommands, &cBounds);
	AssertTrue(cInterpreter.Interpret());
	cInterpreter.Kill();
	bResult = cBounds.GetBounds(&cRect);
	*psMinimum = cRect.msTopLeft;
	*psMaximum = cRect.msBottomRight;
	cBounds.Kill();
	cCommands.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberOutlineBounds(void)
{
	SFloat64Vec2	sMinimum;
	SFloat64Vec2	sMaximum;

	//A one inch square with a semicircle bulging up from the top edge.  The arc's top is beyond its end points and the
	//0.1 inch stroke is not included.
	AssertTrue(GetGerberOutlineBounds(
		"%FSLAX24Y24*%\n"
		"%MOIN*%\n"
		"%ADD10C,0.1*%\n"
		"D10*\n"
		"G01*\n"
		"X0Y0D02*\n"
		"X10000Y0D01*\n"
		"X10000Y10000D01*\n"
		"X7500Y10000D01*\n"
		"G75*\n"
		"G03*\n"
		"X2500Y10000I-2500J0D01*\n"
		"G01*\n"
		"X0Y10000D01*\n"
		"X0Y0D01*\n"
		"M02*\n", &sMinimum, &sMaximum));
	AssertDouble(0.0, sMinimum.x, 9);
	AssertDouble(0.0, sMinimum.y, 9);
	AssertDouble(25.4, sMaximum.x, 9);
	AssertDouble(31.75, sMaximum.y, 9);

	//Clockwise the same arc dips into the square, so the top is the straight edge.
	AssertTrue(GetGerberOutlineBounds(
		"%FSLAX24Y24*%\n"
		"%MOIN*%\n"
		"%ADD10C,0.1*%\n"
		"D10*\n"
		"G01*\n"
		"X0Y0D02*\n"
		"X10000Y0D01*\n"
		"X10000Y10000D01*\n"
		"X7500Y10000D01*\n"
		"G75*\n"
		"G02*\n"
		"X2500Y10000I-2500J0D01*\n"
		"G01*\n"
		"X0Y10000D01*\n"
		"X0Y0D01*\n"
		"M02*\n", &sMinimum, &sMaximum));
	AssertDouble(25.4, sMaximum.x, 9);
	AssertDouble(25.4, sMaximum.y, 9);

	//Nothing drawn, no bounds.
	AssertFalse(GetGerberOutlineBounds(
		"%FSLAX46Y46*%\n"
		"%MOMM*%\n"
		"M02*\n", &sMinimum, &sMaximum));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberFileSetOutlineBounds(void)
{
	CGerberFileSets		cSets;
	CGerberFileSet*		pcSet;
	size				i;

	//Centre line bounds of each board's edge cuts, worked out from gerbonara's lines and arcs.
	const char*			aszNames[] = {	"Adder 16bit", "Adder 8bit", "Address Buffer 24bit", "Clock Doubler", "Clock Hold Select", "Clock Transition",
										"Counter 16bit", "Main Address Buffer", "Main Data Buffer", "Selector 1-of-4 4bit", "State Reader 8bit" };
	float64				afBounds[][4] = {	{-1.778000, -71.193733, 17.018000, 2.616200},
											{-1.778003, -40.713733, 17.018003, 2.616200},
											{-1.778000, -71.193733, 17.018000, 2.616200},
											{-1.778000, -30.553733, 17.018003, 2.616200},
											{-1.905000, -53.975000, 24.765000, 5.715000},
											{-1.905000, -18.415000, 24.765000, 5.715000},
											{-1.778000, -50.873733, 17.018000, 2.616200},
											{-1.778003, -71.193733, 17.018003, 2.616200},
											{-1.778003, -40.713733, 17.018003, 2.616200},
											{-1.778000, -40.713733, 17.018000, 2.616200},
											{-1.778003, -30.553733, 17.018003, 2.616200} };

	cSets.Init();
	AssertTrue(cSets.AddDirectories("Input" _FS_ "KiCad"));
	AssertSize(11, cSets.NumSets());

	for (i = 0; i < cSets.NumSets(); i++)
	{
		pcSet = cSets.Get(i);
		AssertString(aszNames[i], pcSet->GetName());
		AssertTrue(pcSet->HasOutlineBounds());
		AssertDouble(afBounds[i][0], pcSet->GetOutlineBounds()->msTopLeft.x, 6);
		AssertDouble(afBounds[i][1], pcSet->GetOutlineBounds()->msTopLeft.y, 6);
		AssertDouble(afBounds[i][2], pcSet->GetOutlineBounds()->msBottomRight.x, 6);
		AssertDouble(afBounds[i][3], pcSet->GetOutlineBounds()->msBottomRight.y, 6);
	}

	//The Clock Doubler is 18.796 by 33.17 mm.  Its top right corner arc ends 3nm right of the straight edge.
	pcSet = cSets.Get("Clock Doubler");
	AssertDouble(18.796003, pcSet->GetOutlineBounds()->GetWidth(), 6);
	AssertDouble(33.169933, pcSet->GetOutlineBounds()->GetHeight(), 6);

	cSets.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberFileSet(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();

	TestGerberFileSetLoad();
	TestGerberFileSetNameFromDirectory();
	TestGerberFileSetsAddDirectories();
	TestGerberFileSetsAddErrors();
	TestGerberOutlineBounds();
	TestGerberFileSetOutlineBounds();

	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

