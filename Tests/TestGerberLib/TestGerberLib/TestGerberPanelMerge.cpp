#include <math.h>
#include "BaseLib/LogToMemory.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/NumberControl.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/StringHelper.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/ImageWriter.h"
#include "SupportLib/ImageReader.h"
#include "GerberLib/GerberLayerMerger.h"
#include "GerberLib/ExcellonDrillMerger.h"
#include "GerberLib/GerberPanelFiles.h"
#include "GerberLib/GerberParser.h"
#include "GerberLib/GerberInterpreter.h"
#include "GerberLib/GerberVectoriser.h"
#include "GerberLib/ExcellonParser.h"
#include "GerberLib/ExcellonInterpreter.h"
#include "GerberLib/GerberBoardRenderer.h"
#include "TestLib/Assert.h"


TRISTATE ParseGerberText(char* szText, CGerberCommands* pcCommands);
TRISTATE ParseExcellonText(char* szText, CExcellonCommands* pcCommands);
void VectoriseGerberCommands(CGerberCommands* pcCommands, CGerberVectorImage* pcImage);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
class CPanelHoleListener : public CExcellonInterpreterListener
{
public:
	CArrayFloat64Vec2	masHoles;
	CArrayFloat64Vec2	masDiameters;  //x is the diameter.

	void Init(void)
	{
		masHoles.Init();
		masDiameters.Init();
	}

	void Kill(void)
	{
		masDiameters.Kill();
		masHoles.Kill();
	}

	void Hole(CExcellonCommandToolDefine* pcTool, SGerberPoint* psPoint)
	{
		masHoles.Add()->Init(psPoint->cX.DoubleValue(), psPoint->cY.DoubleValue());
		masDiameters.Add()->Init(pcTool->GetDiameter()->DoubleValue(), 0);
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 VectorImageArea(CGerberVectorImage* pcImage)
{
	size	i;
	float64	fArea;

	fArea = 0;
	for (i = 0; i < pcImage->NumObjects(); i++)
	{
		fArea += pcImage->GetObject(i)->Area();
	}
	return fArea;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CountMergedCommands(CGerberCommands* pcCommands, size* puiDefines, size* puiMacros, CArrayChars* paszMacroNames, uint32* puiMaxDCode)
{
	size			i;
	CGerberCommand*	pcCommand;
	uint32			uiDCode;

	for (i = 0; i < pcCommands->NumCommands(); i++)
	{
		pcCommand = pcCommands->GetCommand(i);
		if (pcCommand->IsApertureDefine())
		{
			(*puiDefines)++;
			uiDCode = ((CGerberCommandApertureDefine*)pcCommand)->GetDCode();
			if (uiDCode > *puiMaxDCode)
			{
				*puiMaxDCode = uiDCode;
			}
		}
		else if (pcCommand->IsApertureMacro())
		{
			(*puiMacros)++;
			if (paszMacroNames)
			{
				paszMacroNames->Add(((CGerberCommandApertureMacro*)pcCommand)->NameText());
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberLayerMergeTwoBoards(void)
{
	CGerberFileSets			cSets;
	CGerberPanel			cPanel;
	CGerberLayerMerger		cMerger;
	CChars					szText;
	CGerberCommands			cCommands;
	CGerberVectorImage		cMerged;
	CGerberVectorImage		cBoard;
	CGerberPanelPlacement*	pcPlacement;
	SFloat64Vec2			sMinimum;
	SFloat64Vec2			sMaximum;
	SFloat64Vec2			sBoardMinimum;
	SFloat64Vec2			sBoardMaximum;
	size					uiObjects;
	float64					fArea;
	size					uiDefines;
	size					uiMacros;
	size					uiBoardDefines;
	size					uiBoardMacros;
	uint32					uiMaxDCode;
	CArrayChars				aszMacros;
	size					i;
	bool					bFirst;

	cSets.Init();
	AssertNotNull(cSets.Add("Input" _FS_ "KiCad" _FS_ "Clock Doubler"));
	AssertNotNull(cSets.Add("Input" _FS_ "KiCad" _FS_ "Clock Transition"));
	cPanel.Init(1.6, 1.6, false);
	AssertTrue(cPanel.Pack(&cSets));

	cMerger.Init(&cPanel);
	szText.Init();
	AssertTrue(cMerger.Merge(GBL_Copper, GBS_Top, &szText));
	cMerger.Kill();

	//One header and end, the panel's attributes and each board's name.
	AssertTrue(szText.StartsWith("%TF.GenerationSoftware,Codaphela,GerberLib*%\n%TF.FileFunction,Copper,L1,Top*%\n%TF.FilePolarity,Positive*%\n%TF.Part,Array*%\n%FSLAX46Y46*%\n%MOMM*%\n"));
	AssertTrue(szText.EndsWith("M02*\n"));
	AssertTrue(szText.Contains("\nG04 Board Clock Doubler*\n%TD*%\n"));
	AssertTrue(szText.Contains("\nG04 Board Clock Transition*\n%TD*%\n"));
	AssertSize(1, szText.Occurrences("%FSLAX46Y46*%"));
	AssertSize(1, szText.Occurrences("M02*"));
	AssertFalse(szText.Contains("CreationDate"));

	cCommands.Init();
	AssertTritrue(ParseGerberText(szText.Text(), &cCommands));
	cMerged.Init();
	VectoriseGerberCommands(&cCommands, &cMerged);

	//The same objects, moved.
	uiObjects = 0;
	fArea = 0;
	uiBoardDefines = 0;
	uiBoardMacros = 0;
	uiMaxDCode = 0;
	bFirst = true;
	for (i = 0; i < cPanel.NumPlacements(); i++)
	{
		pcPlacement = cPanel.GetPlacement(i);
		AssertFalse(pcPlacement->mbRotated);
		cBoard.Init();
		VectoriseGerberCommands(pcPlacement->mpcSet->GetLayer(GBL_Copper, GBS_Top)->GetCommands(), &cBoard);
		uiObjects += cBoard.NumObjects();
		fArea += VectorImageArea(&cBoard);
		AssertTrue(cBoard.GetBounds(&sBoardMinimum, &sBoardMaximum));
		sBoardMinimum.x += pcPlacement->msOffset.x;
		sBoardMinimum.y += pcPlacement->msOffset.y;
		sBoardMaximum.x += pcPlacement->msOffset.x;
		sBoardMaximum.y += pcPlacement->msOffset.y;
		if (bFirst)
		{
			sMinimum = sBoardMinimum;
			sMaximum = sBoardMaximum;
			bFirst = false;
		}
		sMinimum.x = fmin(sMinimum.x, sBoardMinimum.x);
		sMinimum.y = fmin(sMinimum.y, sBoardMinimum.y);
		sMaximum.x = fmax(sMaximum.x, sBoardMaximum.x);
		sMaximum.y = fmax(sMaximum.y, sBoardMaximum.y);
		cBoard.Kill();
		CountMergedCommands(pcPlacement->mpcSet->GetLayer(GBL_Copper, GBS_Top)->GetCommands(), &uiBoardDefines, &uiBoardMacros, NULL, &uiMaxDCode);
	}
	AssertSize(uiObjects, cMerged.NumObjects());
	AssertDouble(fArea, VectorImageArea(&cMerged), 6);
	AssertTrue(cMerged.GetBounds(&sBoardMinimum, &sBoardMaximum));
	AssertDouble(sMinimum.x, sBoardMinimum.x, 6);
	AssertDouble(sMinimum.y, sBoardMinimum.y, 6);
	AssertDouble(sMaximum.x, sBoardMaximum.x, 6);
	AssertDouble(sMaximum.y, sBoardMaximum.y, 6);

	//Every aperture is renumbered from D10 and the boards' identical RoundRect macro is written once.
	uiDefines = 0;
	uiMacros = 0;
	uiMaxDCode = 0;
	aszMacros.Init();
	CountMergedCommands(&cCommands, &uiDefines, &uiMacros, &aszMacros, &uiMaxDCode);
	AssertSize(uiBoardDefines, uiDefines);
	AssertInt((int)(9 + uiDefines), (int)uiMaxDCode);
	AssertSize(2, uiBoardMacros);
	AssertSize(1, uiMacros);
	AssertString("RoundRect", aszMacros.Get(0)->Text());
	aszMacros.Kill();

	cMerged.Kill();
	cCommands.Kill();
	szText.Kill();
	cPanel.Kill();
	cSets.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberLayerMergeRotated(void)
{
	CGerberFileSets			cSets;
	CGerberPanel			cPanel;
	CGerberLayerMerger		cMerger;
	CChars					szText;
	CGerberCommands			cCommands;
	CGerberCommands*		pcOriginal;
	CGerberVectorImage		cMerged;
	CGerberVectorImage		cBoard;
	CGerberPanelPlacement*	pcPlacement;
	SFloat64Vec2			sMinimum;
	SFloat64Vec2			sMaximum;
	SFloat64Vec2			sBoardMinimum;
	SFloat64Vec2			sBoardMaximum;
	CArrayChars				aszMacros;
	size					uiDefines;
	size					uiMacros;
	uint32					uiMaxDCode;
	size					i;
	size					j;
	CGerberCommandApertureDefine*	pcOldDefine;
	CGerberCommandApertureDefine*	pcNewDefine;
	size					uiRectangles;

	//The Adder 8bit has arcs, regions and RoundRect pads.  It is turned here whatever the packer chose.
	cSets.Init();
	AssertNotNull(cSets.Add("Input" _FS_ "KiCad" _FS_ "Adder 8bit"));
	cPanel.Init(1.6, 1.6, false);
	AssertTrue(cPanel.Pack(&cSets));
	pcPlacement = cPanel.GetPlacement((size)0);
	pcPlacement->mbRotated = true;

	cMerger.Init(&cPanel);
	szText.Init();
	AssertTrue(cMerger.Merge(GBL_Copper, GBS_Top, &szText));
	cMerger.Kill();
	AssertTrue(szText.Contains("\nG04 Board Adder 8bit, rotated 90 degrees*\n"));

	cCommands.Init();
	AssertTritrue(ParseGerberText(szText.Text(), &cCommands));
	cMerged.Init();
	VectoriseGerberCommands(&cCommands, &cMerged);
	pcOriginal = pcPlacement->mpcSet->GetLayer(GBL_Copper, GBS_Top)->GetCommands();
	cBoard.Init();
	VectoriseGerberCommands(pcOriginal, &cBoard);

	//Turning keeps every object's area.
	AssertSize(cBoard.NumObjects(), cMerged.NumObjects());
	for (i = 0; i < cBoard.NumObjects(); i++)
	{
		AssertDouble(cBoard.GetObject(i)->Area(), cMerged.GetObject(i)->Area(), 6);
	}

	//(x, y) -> (-y, x) then moved.
	AssertTrue(cBoard.GetBounds(&sBoardMinimum, &sBoardMaximum));
	AssertTrue(cMerged.GetBounds(&sMinimum, &sMaximum));
	AssertDouble(-sBoardMaximum.y + pcPlacement->msOffset.x, sMinimum.x, 6);
	AssertDouble(sBoardMinimum.x + pcPlacement->msOffset.y, sMinimum.y, 6);
	AssertDouble(-sBoardMinimum.y + pcPlacement->msOffset.x, sMaximum.x, 6);
	AssertDouble(sBoardMaximum.x + pcPlacement->msOffset.y, sMaximum.y, 6);

	//The macro is a rotated copy and rectangles swap their sizes.
	uiDefines = 0;
	uiMacros = 0;
	uiMaxDCode = 0;
	aszMacros.Init();
	CountMergedCommands(&cCommands, &uiDefines, &uiMacros, &aszMacros, &uiMaxDCode);
	AssertSize(1, uiMacros);
	AssertString("RoundRectR90", aszMacros.Get(0)->Text());
	aszMacros.Kill();

	uiRectangles = 0;
	j = 0;
	for (i = 0; i < pcOriginal->NumCommands(); i++)
	{
		if (!pcOriginal->GetCommand(i)->IsApertureDefine())
		{
			continue;
		}
		pcOldDefine = (CGerberCommandApertureDefine*)pcOriginal->GetCommand(i);
		while (!cCommands.GetCommand(j)->IsApertureDefine())
		{
			j++;
		}
		pcNewDefine = (CGerberCommandApertureDefine*)cCommands.GetCommand(j);
		j++;
		if (StringCompare(pcOldDefine->TemplateText(), "R") == 0)
		{
			AssertString("R", pcNewDefine->TemplateText());
			AssertTrue(pcOldDefine->GetParameter(0)->Equals(pcNewDefine->GetParameter(1)));
			AssertTrue(pcOldDefine->GetParameter(1)->Equals(pcNewDefine->GetParameter(0)));
			uiRectangles++;
		}
		else if (StringCompare(pcOldDefine->TemplateText(), "RoundRect") == 0)
		{
			AssertString("RoundRectR90", pcNewDefine->TemplateText());
		}
	}
	AssertTrue(uiRectangles > 0);

	cBoard.Kill();
	cMerged.Kill();
	cCommands.Kill();
	szText.Kill();
	cPanel.Kill();
	cSets.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestExcellonDrillMerge(void)
{
	CGerberFileSets			cSets;
	CGerberPanel			cPanel;
	CExcellonDrillMerger	cMerger;
	CChars					szText;
	CExcellonCommands		cCommands;
	CExcellonInterpreter	cInterpreter;
	CPanelHoleListener		cListener;
	CGerberPanelPlacement*	pcAdder;
	SFloat64Vec2			sHole;
	SFloat64Vec2			sPlaced;
	SFloat64Vec2			asBites[2];
	CNumber					cDiameter;
	CArrayChars				aszFunction;
	size					i;
	bool					bFound;

	//The Adder 16bit's drills are in inches and the Clock Doubler's the same sizes in millimetres, so their via and
	//component drills become one tool each.
	cSets.Init();
	AssertNotNull(cSets.Add("Input" _FS_ "KiCad" _FS_ "Adder 16bit"));
	AssertNotNull(cSets.Add("Input" _FS_ "KiCad" _FS_ "Clock Doubler"));
	cPanel.Init(1.6, 1.6, true);
	AssertTrue(cPanel.Pack(&cSets));

	cMerger.Init(&cPanel);
	AssertTrue(cMerger.AddBoards(GDP_Plated));
	AssertSize(2, cMerger.NumTools());
	asBites[0].Init(10.0, 20.0);
	asBites[1].Init(10.8, 20.0);
	cDiameter.Init("0.5");
	aszFunction.Init();
	aszFunction.Add("NonPlated");
	aszFunction.Add("NPTH");
	cMerger.AddHoles(&cDiameter, &aszFunction, asBites, 2);
	aszFunction.Kill();
	AssertSize(3, cMerger.NumTools());
	szText.Init();
	AssertTrue(cMerger.Write(&szText));
	cMerger.Kill();

	AssertTrue(szText.StartsWith("M48\n; DRILL file {Codaphela GerberLib} panel\n; FORMAT={-:-/ absolute / metric / decimal}\n; #@! TF.GenerationSoftware,Codaphela,GerberLib\n; #@! TF.FileFunction,Plated,1,2,PTH\nFMAT,2\nMETRIC\n"));
	AssertFalse(szText.Contains("INCH"));
	//Smallest drill first.
	AssertTrue(szText.Contains("\n; #@! TA.AperFunction,Plated,PTH,ViaDrill\nT1C0.39878\n; #@! TA.AperFunction,NonPlated,NPTH\nT2C0.5\n; #@! TA.AperFunction,Plated,PTH,ComponentDrill\nT3C1.00076\n%\n"));
	AssertTrue(szText.EndsWith("T0\nM30\n"));

	cCommands.Init();
	AssertTritrue(ParseExcellonText(szText.Text(), &cCommands));
	cListener.Init();
	cInterpreter.Init(&cCommands, &cListener);
	AssertTrue(cInterpreter.Interpret());
	cInterpreter.Kill();
	AssertSize(186 + 2, cListener.masHoles.NumElements());

	//The Adder's first hole is at 1.27, -64.77 mm on the board.
	pcAdder = cPanel.GetPlacement(cSets.Get("Adder 16bit"));
	AssertNotNull(pcAdder);
	sHole.Init(1.27, -64.77);
	pcAdder->Place(&sHole, &sPlaced);
	bFound = false;
	for (i = 0; i < cListener.masHoles.NumElements(); i++)
	{
		if ((fabs(cListener.masHoles.Get(i)->x - sPlaced.x) < 0.0000001) && (fabs(cListener.masHoles.Get(i)->y - sPlaced.y) < 0.0000001))
		{
			AssertDouble(0.39878, cListener.masDiameters.Get(i)->x, 9);
			bFound = true;
		}
	}
	AssertTrue(bFound);
	cListener.Kill();

	cCommands.Kill();
	szText.Kill();
	cPanel.Kill();
	cSets.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberPanelTabsTwoBoards(void)
{
	CGerberFileSets			cSets;
	CGerberPanel			cPanel;
	CGerberPanelFiles		cFiles;
	CGerberPanelTabs*		pcTabs;
	CGerberPanelEdgeCuts*	pcEdgeCuts;
	CGerberPanelOutlines*	pcOutlines;
	SGerberPanelTab*		psTab;
	CArrayFloat64Vec2		asHoles;
	CGerberPanelLoop*		pcLoop;
	float64					fArea;
	CChars					szText;
	CGerberCommands			cCommands;
	CGerberVectorImage		cImage;
	int						i;

	//Side by side, the Clock Doubler on the left.  The edges face each other from y = 1.600267 to 25.73, so one tab
	//goes in the middle, where both edges are straight.
	cSets.Init();
	AssertNotNull(cSets.Add("Input" _FS_ "KiCad" _FS_ "Clock Doubler"));
	AssertNotNull(cSets.Add("Input" _FS_ "KiCad" _FS_ "Clock Transition"));
	cPanel.Init(1.6, 1.6, false);
	AssertTrue(cPanel.Pack(&cSets));
	cFiles.Init(&cPanel);
	AssertTrue(cFiles.Calculate());

	pcTabs = cFiles.GetTabs();
	AssertSize(1, pcTabs->NumTabs());
	AssertSize(0, pcTabs->NumMoved());
	AssertSize(0, pcTabs->NumDropped());
	psTab = pcTabs->GetTab(0);
	AssertSize(0, psTab->uiBoardA);
	AssertSize(1, psTab->uiBoardB);
	AssertTrue(psTab->bAcrossX);
	AssertDouble(1.6 + 18.796003, psTab->fEdgeA, 6);
	AssertDouble(1.6 + 18.797 + 1.6, psTab->fEdgeB, 6);
	AssertDouble(11.165133, psTab->fStart, 6);
	AssertDouble(16.165133, psTab->fEnd, 6);

	//Six 0.5mm holes 0.8mm apart along each edge, just outside it in the tab.
	asHoles.Init();
	pcTabs->GetMouseBites(&asHoles);
	AssertSize(12, asHoles.NumElements());
	for (i = 0; i < 6; i++)
	{
		AssertDouble(psTab->fEdgeA + 0.25, asHoles.Get(i * 2)->x, 9);
		AssertDouble(psTab->fEdgeB - 0.25, asHoles.Get(i * 2 + 1)->x, 9);
		AssertDouble(13.665133 + (i - 2.5) * 0.8, asHoles.Get(i * 2)->y, 6);
	}
	asHoles.Kill();

	//One outline round both boards and the tab, counterclockwise.
	pcOutlines = cFiles.GetOutlines();
	pcEdgeCuts = cFiles.GetEdgeCuts();
	AssertSize(1, pcEdgeCuts->NumLoops());
	pcLoop = pcEdgeCuts->GetLoop(0);
	AssertTrue(pcLoop->IsClosed(0.000001));
	fArea = pcOutlines->GetOuterLoop(0)->Area() + pcOutlines->GetOuterLoop(1)->Area() + 5.0 * (psTab->fEdgeB - psTab->fEdgeA);
	AssertDouble(fArea, pcLoop->Area(), 4);

	//The written edge cuts draw that outline.
	szText.Init();
	AssertTrue(pcEdgeCuts->Write(&szText));
	AssertTrue(szText.Contains("%TF.FileFunction,Profile,NP*%\n"));
	AssertTrue(szText.Contains("%TF.Part,Array*%\n"));
	cCommands.Init();
	AssertTritrue(ParseGerberText(szText.Text(), &cCommands));
	cImage.Init();
	VectoriseGerberCommands(&cCommands, &cImage);
	AssertSize(pcLoop->masSegments.NumElements(), cImage.NumObjects());
	cImage.Kill();
	cCommands.Kill();
	szText.Kill();

	cFiles.Kill();
	cPanel.Kill();
	cSets.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberPanelTabsNotch(void)
{
	CGerberFileSets			cSets;
	CGerberPanel			cPanel;
	CGerberPanelFiles		cFiles;
	CGerberPanelTabs*		pcTabs;
	SGerberPanelTab*		psTab;
	CFloat64Rect			cBelow;
	CFloat64Rect			cAbove;
	float64					fNotch;

	//One board above the other; their 18.8mm ends face each other and each has a notch in the middle.  The end is one
	//edge, so it gets one tab, beside the notch rather than one either side of it.
	cSets.Init();
	AssertNotNull(cSets.Add("Input" _FS_ "KiCad" _FS_ "Clock Doubler"));
	AssertNotNull(cSets.Add("Input" _FS_ "KiCad" _FS_ "State Reader 8bit"));
	cPanel.Init(1.6, 1.6, false);
	AssertTrue(cPanel.Pack(&cSets));
	cFiles.Init(&cPanel);
	AssertTrue(cFiles.Calculate());

	pcTabs = cFiles.GetTabs();
	AssertSize(1, pcTabs->NumTabs());
	AssertSize(1, pcTabs->NumMoved());
	AssertSize(0, pcTabs->NumDropped());
	psTab = pcTabs->GetTab(0);
	AssertFalse(psTab->bAcrossX);
	AssertDouble(5.0, psTab->fEnd - psTab->fStart, 9);

	//The notch is in the middle of the end; the tab is clear of it but on the end.
	cFiles.GetOutlines()->GetOuterLoop(psTab->uiBoardA)->GetBounds(&cBelow);
	cFiles.GetOutlines()->GetOuterLoop(psTab->uiBoardB)->GetBounds(&cAbove);
	//Lined up, apart from the packer rounding each board out to the micrometre.
	AssertTrue(fabs(cBelow.msTopLeft.x - cAbove.msTopLeft.x) < 0.01);
	fNotch = (cBelow.msTopLeft.x + cBelow.msBottomRight.x) / 2;
	AssertTrue((psTab->fEnd < fNotch - 0.889) || (psTab->fStart > fNotch + 0.889));
	AssertTrue(psTab->fStart > cBelow.msTopLeft.x);
	AssertTrue(psTab->fEnd < cBelow.msBottomRight.x);

	//So the two outlines join into one.
	AssertSize(1, cFiles.GetEdgeCuts()->NumLoops());

	cFiles.Kill();
	cPanel.Kill();
	cSets.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberPanelTabsNotJoined(void)
{
	CGerberFileSets			cSets;
	CGerberPanel			cPanel;
	CGerberPanelOutlines	cOutlines;
	CGerberPanelTabs		cTabs;
	CLogToMemory			cMemoryLog;
	char					szLogText[1000];

	//50mm tabs fit on no edge so the boards would fall apart.
	cSets.Init();
	AssertNotNull(cSets.Add("Input" _FS_ "KiCad" _FS_ "Clock Doubler"));
	AssertNotNull(cSets.Add("Input" _FS_ "KiCad" _FS_ "Clock Transition"));
	cPanel.Init(1.6, 1.6, false);
	AssertTrue(cPanel.Pack(&cSets));
	cOutlines.Init();
	AssertTrue(cOutlines.Read(&cPanel));
	cTabs.Init(50.0);
	cMemoryLog.Start(true);
	AssertFalse(cTabs.Calculate(&cPanel, &cOutlines));
	cMemoryLog.Stop(szLogText, 1000);
	AssertTrue(strstr(szLogText, "] isn't joined to the rest of the panel by any tab.") != NULL);
	AssertSize(0, cTabs.NumTabs());
	cTabs.Kill();
	cOutlines.Kill();
	cPanel.Kill();
	cSets.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CountRenderedColour(Ptr<CImage> pcImage, int r, int g, int b, int iTolerance)
{
	uint8*	puiPixels;
	size	i;
	size	uiNumPixels;
	size	uiCount;

	puiPixels = (uint8*)pcImage->GetData();
	uiNumPixels = (size)pcImage->GetWidth() * pcImage->GetHeight();
	uiCount = 0;
	for (i = 0; i < uiNumPixels; i++)
	{
		if ((abs(puiPixels[i * 3] - r) <= iTolerance) && (abs(puiPixels[i * 3 + 1] - g) <= iTolerance) && (abs(puiPixels[i * 3 + 2] - b) <= iTolerance))
		{
			uiCount++;
		}
	}
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertRenderedColour(Ptr<CImage> pcImage, int x, int y, int r, int g, int b)
{
	uint8*	puiPixel;

	puiPixel = &((uint8*)pcImage->GetData())[((size)y * pcImage->GetWidth() + x) * 3];
	AssertInt(r, puiPixel[0]);
	AssertInt(g, puiPixel[1]);
	AssertInt(b, puiPixel[2]);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertRenderedImage(const char* szExpected, Ptr<CImage> pcActual)
{
	Ptr<CImage>	pcExpected;
	uint8*		puiExpected;
	uint8*		puiActual;
	size		i;
	size		uiNumPixels;
	size		uiDifferent;

	//Anti-aliased edges may come out a little differently with another compiler, so a few pixels may differ.
	pcExpected = ReadImage((char*)szExpected);
	AssertTrue(pcExpected.IsNotNull());
	if (pcExpected.IsNull())
	{
		return;
	}
	AssertInt((int)pcExpected->GetWidth(), (int)pcActual->GetWidth());
	AssertInt((int)pcExpected->GetHeight(), (int)pcActual->GetHeight());
	AssertSize(pcExpected->GetByteSize(), pcActual->GetByteSize());
	if ((pcExpected->GetWidth() != pcActual->GetWidth()) || (pcExpected->GetHeight() != pcActual->GetHeight()) || (pcExpected->GetByteSize() != pcActual->GetByteSize()))
	{
		pcExpected = NULL;
		return;
	}

	puiExpected = (uint8*)pcExpected->GetData();
	puiActual = (uint8*)pcActual->GetData();
	uiNumPixels = (size)pcActual->GetWidth() * pcActual->GetHeight();
	uiDifferent = 0;
	for (i = 0; i < uiNumPixels; i++)
	{
		if ((abs(puiExpected[i * 3] - puiActual[i * 3]) > 8) || (abs(puiExpected[i * 3 + 1] - puiActual[i * 3 + 1]) > 8) || (abs(puiExpected[i * 3 + 2] - puiActual[i * 3 + 2]) > 8))
		{
			uiDifferent++;
		}
	}
	AssertTrue(uiDifferent <= uiNumPixels / 1000);
	pcExpected = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberPanelFilesAllBoards(void)
{
	CGerberFileSets			cSets;
	CGerberPanel			cPanel;
	CGerberPanelFiles		cFiles;
	CGerberPanelEdgeCuts*	pcEdgeCuts;
	CGerberPanelOutlines*	pcOutlines;
	CGerberPanelTabs*		pcTabs;
	CFileUtil				cFileUtil;
	CGerberFileSet			cPanelSet;
	CGerberLayerFile*		pcLayer;
	CGerberDrillFile*		pcDrill;
	CGerberVectorImage		cImage;
	CExcellonInterpreter	cInterpreter;
	CPanelHoleListener		cListener;
	float64					fArea;
	float64					fLoops;
	size					i;
	size					uiObjects;
	size					uiOutside;
	CGerberBoardRenderer	cRenderer;
	Ptr<CImage>				pcFront;
	Ptr<CImage>				pcBack;
	CArrayFloat64Vec2		asBites;
	CFloat64Rect*			pcBounds;
	int						x;
	int						y;

	cSets.Init();
	AssertTrue(cSets.AddDirectories("Input" _FS_ "KiCad"));
	AssertSize(11, cSets.NumSets());
	cPanel.Init(1.6, 1.6, true);
	AssertTrue(cPanel.Pack(&cSets));
	cFiles.Init(&cPanel);
	AssertTrue(cFiles.Calculate());

	pcOutlines = cFiles.GetOutlines();
	pcTabs = cFiles.GetTabs();
	pcEdgeCuts = cFiles.GetEdgeCuts();
	AssertSize(0, pcTabs->NumDropped());

	//The boards have no cut outs, so the first tab to reach each board joins outlines and every other tab splits off
	//a piece of channel: tabs - boards + 2 outlines.
	AssertTrue(pcTabs->NumTabs() >= 10);
	AssertSize(pcTabs->NumTabs() - 11 + 2, pcEdgeCuts->NumLoops());
	fArea = 0;
	for (i = 0; i < pcOutlines->NumBoards(); i++)
	{
		AssertSize(1, pcOutlines->NumBoardLoops(i));
		fArea += pcOutlines->GetOuterLoop(i)->Area();
	}
	for (i = 0; i < pcTabs->NumTabs(); i++)
	{
		fArea += 5.0 * (pcTabs->GetTab(i)->fEdgeB - pcTabs->GetTab(i)->fEdgeA);
	}
	fLoops = 0;
	uiOutside = 0;
	for (i = 0; i < pcEdgeCuts->NumLoops(); i++)
	{
		AssertTrue(pcEdgeCuts->GetLoop(i)->IsClosed(0.000001));
		fLoops += pcEdgeCuts->GetLoop(i)->Area();
		if (pcEdgeCuts->GetLoop(i)->Area() > 0)
		{
			uiOutside++;
		}
	}
	//One outside turning counterclockwise; the channel pieces turn clockwise.
	AssertSize(1, uiOutside);
	AssertTrue(fabs(fLoops - fArea) < 0.0001 * fArea);

	//Write the files and read them back as a board.
	cFileUtil.RemoveDir("Output" _FS_ "Panel");
	AssertTrue(cFileUtil.TouchDir("Output" _FS_ "Panel"));
	AssertTrue(cFiles.Write("Output" _FS_ "Panel", "Panel"));
	AssertSize(11, cFiles.NumFiles());
	AssertString("Panel-B_Cu.gbl", cFiles.GetFile(0));
	AssertString("Panel-Edge_Cuts.gm1", cFiles.GetFile(8));
	AssertString("Panel-NPTH.drl", cFiles.GetFile(10));

	cPanelSet.Init();
	AssertTrue(cPanelSet.Load("Output" _FS_ "Panel"));
	AssertSize(9, cPanelSet.NumLayers());
	AssertSize(2, cPanelSet.NumDrills());
	AssertTrue(cPanelSet.HasOutlineBounds());
	AssertTrue(cPanelSet.GetOutlineBounds()->msTopLeft.x >= 1.6 - 0.000001);
	AssertTrue(cPanelSet.GetOutlineBounds()->msBottomRight.x <= cPanel.GetSize()->x - 1.6 + 0.000001);

	pcLayer = cPanelSet.GetLayer(GBL_Copper, GBS_Top);
	cImage.Init();
	VectoriseGerberCommands(pcLayer->GetCommands(), &cImage);
	uiObjects = 0;
	for (i = 0; i < cSets.NumSets(); i++)
	{
		CGerberVectorImage	cBoard;

		cBoard.Init();
		VectoriseGerberCommands(cSets.Get(i)->GetLayer(GBL_Copper, GBS_Top)->GetCommands(), &cBoard);
		uiObjects += cBoard.NumObjects();
		cBoard.Kill();
	}
	AssertSize(uiObjects, cImage.NumObjects());
	cImage.Kill();

	//The non-plated drills are the mouse bites: two rows of six for each tab.
	pcDrill = cPanelSet.GetDrill(GDP_NonPlated);
	AssertNotNull(pcDrill);
	cListener.Init();
	cInterpreter.Init(pcDrill->GetCommands(), &cListener);
	AssertTrue(cInterpreter.Interpret());
	cInterpreter.Kill();
	AssertSize(12 * pcTabs->NumTabs(), cListener.masHoles.NumElements());
	cListener.Kill();

	//Pictures of the front and the back, left in Output/Panel to look at.  At 20 pixels per millimetre with a 2mm margin.
	cRenderer.Init(20.0, 2.0);
	pcFront = cRenderer.Render(&cPanelSet, GBS_Top);
	pcBack = cRenderer.Render(&cPanelSet, GBS_Bottom);
	cRenderer.Kill();
	AssertTrue(pcFront.IsNotNull());
	AssertTrue(pcBack.IsNotNull());
	pcBounds = cPanelSet.GetOutlineBounds();
	AssertInt((int)ceil((pcBounds->GetWidth() + 4) * 20), (int)pcFront->GetWidth());
	AssertInt((int)ceil((pcBounds->GetHeight() + 4) * 20), (int)pcFront->GetHeight());
	AssertInt((int)pcFront->GetWidth(), (int)pcBack->GetWidth());
	AssertTrue(WriteImage(pcFront, "Output" _FS_ "Panel" _FS_ "Panel-Front.png"));
	AssertTrue(WriteImage(pcBack, "Output" _FS_ "Panel" _FS_ "Panel-Back.png"));
	AssertRenderedImage("Input" _FS_ "Panel" _FS_ "Panel-Front.png", pcFront);
	AssertRenderedImage("Input" _FS_ "Panel" _FS_ "Panel-Back.png", pcBack);

	//A mouse bite shows the bench through on both sides; the back is mirrored.
	asBites.Init();
	pcTabs->GetMouseBites(&asBites);
	x = (int)floor((asBites.Get(0)->x - (pcBounds->msTopLeft.x - 2)) * 20);
	y = (int)floor(((pcBounds->msBottomRight.y + 2) - asBites.Get(0)->y) * 20);
	AssertRenderedColour(pcFront, x, y, 30, 30, 34);
	AssertRenderedColour(pcBack, pcBack->GetWidth() - 1 - x, y, 30, 30, 34);
	asBites.Kill();

	//Gold pads on both sides; silkscreen only on the front as the boards have none on the back.
	AssertTrue(CountRenderedColour(pcFront, 212, 175, 55, 6) > 1000);
	AssertTrue(CountRenderedColour(pcBack, 212, 175, 55, 6) > 1000);
	AssertTrue(CountRenderedColour(pcFront, 245, 245, 240, 6) > 1000);
	AssertSize(0, CountRenderedColour(pcBack, 245, 245, 240, 6));
	pcFront = NULL;
	pcBack = NULL;

	cPanelSet.Kill();

	cFiles.Kill();
	cPanel.Kill();
	cSets.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberPanelMerge(void)
{
	TypeConverterInit();
	NumberInit();
	DataIOInit();
	ObjectsInit();
	BeginTests();

	TestGerberLayerMergeTwoBoards();
	TestGerberLayerMergeRotated();
	TestExcellonDrillMerge();
	TestGerberPanelTabsTwoBoards();
	TestGerberPanelTabsNotch();
	TestGerberPanelTabsNotJoined();
	TestGerberPanelFilesAllBoards();

	TestStatistics();
	ObjectsKill(false);
	DataIOKill();
	NumberKill();
	TypeConverterKill();
}

