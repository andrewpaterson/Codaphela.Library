#include "BaseLib/LogToMemory.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/NumberControl.h"
#include "BaseLib/FileUtil.h"
#include "GerberLib/GerberPanel.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void IncludePlacedPoint(CGerberPanelPlacement* pcPlacement, float64 x, float64 y, SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum)
{
	SFloat64Vec2	sPoint;
	SFloat64Vec2	sPlaced;

	sPoint.Init(x, y);
	pcPlacement->Place(&sPoint, &sPlaced);
	if (sPlaced.x < psMinimum->x) psMinimum->x = sPlaced.x;
	if (sPlaced.y < psMinimum->y) psMinimum->y = sPlaced.y;
	if (sPlaced.x > psMaximum->x) psMaximum->x = sPlaced.x;
	if (sPlaced.y > psMaximum->y) psMaximum->y = sPlaced.y;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertGerberPanel(CGerberPanel* pcPanel, CGerberFileSets* pcSets, float64 fGap, float64 fEdge)
{
	size					i;
	size					j;
	CGerberPanelPlacement*	pcPlacement;
	CGerberPanelPlacement*	pcOther;
	CGerberFileSet*			pcSet;
	SFloat64Vec2			sMinimum;
	SFloat64Vec2			sMaximum;
	CFloat64Rect*			pcBounds;
	float64					fTolerance;

	fTolerance = 0.0000001;
	AssertSize(pcSets->NumSets(), pcPanel->NumPlacements());
	for (i = 0; i < pcSets->NumSets(); i++)
	{
		pcSet = pcSets->Get(i);
		pcPlacement = pcPanel->GetPlacement(pcSet);
		AssertNotNull(pcPlacement);
		if (pcPlacement == NULL)
		{
			continue;
		}

		//The board's real outline corners land inside its rounded slot, which is at most a micrometre bigger each side.
		sMinimum.Init(1e30, 1e30);
		sMaximum.Init(-1e30, -1e30);
		pcBounds = pcSet->GetOutlineBounds();
		IncludePlacedPoint(pcPlacement, pcBounds->msTopLeft.x, pcBounds->msTopLeft.y, &sMinimum, &sMaximum);
		IncludePlacedPoint(pcPlacement, pcBounds->msBottomRight.x, pcBounds->msTopLeft.y, &sMinimum, &sMaximum);
		IncludePlacedPoint(pcPlacement, pcBounds->msTopLeft.x, pcBounds->msBottomRight.y, &sMinimum, &sMaximum);
		IncludePlacedPoint(pcPlacement, pcBounds->msBottomRight.x, pcBounds->msBottomRight.y, &sMinimum, &sMaximum);
		AssertTrue(sMinimum.x >= pcPlacement->mcBounds.msTopLeft.x - fTolerance);
		AssertTrue(sMinimum.y >= pcPlacement->mcBounds.msTopLeft.y - fTolerance);
		AssertTrue(sMaximum.x <= pcPlacement->mcBounds.msBottomRight.x + fTolerance);
		AssertTrue(sMaximum.y <= pcPlacement->mcBounds.msBottomRight.y + fTolerance);
		AssertTrue(sMinimum.x - pcPlacement->mcBounds.msTopLeft.x < 0.001 + fTolerance);
		AssertTrue(sMinimum.y - pcPlacement->mcBounds.msTopLeft.y < 0.001 + fTolerance);
		AssertTrue(pcPlacement->mcBounds.msBottomRight.x - sMaximum.x < 0.001 + fTolerance);
		AssertTrue(pcPlacement->mcBounds.msBottomRight.y - sMaximum.y < 0.001 + fTolerance);

		//A turned board swaps its width and height.
		if (pcPlacement->mbRotated)
		{
			AssertDouble(pcBounds->GetHeight(), sMaximum.x - sMinimum.x, 6);
			AssertDouble(pcBounds->GetWidth(), sMaximum.y - sMinimum.y, 6);
		}
		else
		{
			AssertDouble(pcBounds->GetWidth(), sMaximum.x - sMinimum.x, 6);
			AssertDouble(pcBounds->GetHeight(), sMaximum.y - sMinimum.y, 6);
		}

		//Clear of the panel's sides.
		AssertTrue(pcPlacement->mcBounds.msTopLeft.x >= fEdge - fTolerance);
		AssertTrue(pcPlacement->mcBounds.msTopLeft.y >= fEdge - fTolerance);
		AssertTrue(pcPlacement->mcBounds.msBottomRight.x <= pcPanel->GetSize()->x - fEdge + fTolerance);
		AssertTrue(pcPlacement->mcBounds.msBottomRight.y <= pcPanel->GetSize()->y - fEdge + fTolerance);
	}

	//At least a router bit between every two boards.
	for (i = 0; i < pcPanel->NumPlacements(); i++)
	{
		pcPlacement = pcPanel->GetPlacement(i);
		for (j = i + 1; j < pcPanel->NumPlacements(); j++)
		{
			pcOther = pcPanel->GetPlacement(j);
			AssertTrue((pcPlacement->mcBounds.msBottomRight.x + fGap <= pcOther->mcBounds.msTopLeft.x + fTolerance) ||
					   (pcOther->mcBounds.msBottomRight.x + fGap <= pcPlacement->mcBounds.msTopLeft.x + fTolerance) ||
					   (pcPlacement->mcBounds.msBottomRight.y + fGap <= pcOther->mcBounds.msTopLeft.y + fTolerance) ||
					   (pcOther->mcBounds.msBottomRight.y + fGap <= pcPlacement->mcBounds.msTopLeft.y + fTolerance));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberPanelTwoBoards(void)
{
	CGerberFileSets			cSets;
	CGerberPanel			cPanel;
	CGerberPanelPlacement*	pcPlacement;
	CGerberFileSet*			pcDoubler;
	CGerberFileSet*			pcTransition;

	cSets.Init();
	pcDoubler = cSets.Add("Input" _FS_ "KiCad" _FS_ "Clock Doubler");
	pcTransition = cSets.Add("Input" _FS_ "KiCad" _FS_ "Clock Transition");
	AssertNotNull(pcDoubler);
	AssertNotNull(pcTransition);

	//Rounded outwards the Clock Doubler is 18.797 x 33.171mm (-1.778, -30.554 to 17.019, 2.617) and the Clock
	//Transition 26.670 x 24.130mm (-1.905, -18.415 to 24.765, 5.715).  Side by side they are 18.797 + 1.6 + 26.670 wide.
	cPanel.Init(1.6, 1.6, false);
	AssertTrue(cPanel.Pack(&cSets));
	AssertGerberPanel(&cPanel, &cSets, 1.6, 1.6);
	AssertDouble(1.6 + 26.670 + 1.6 + 18.797 + 1.6, cPanel.GetSize()->x, 6);
	AssertDouble(1.6 + 33.171 + 1.6, cPanel.GetSize()->y, 6);

	//The taller board leads the line.
	pcPlacement = cPanel.GetPlacement(pcDoubler);
	AssertFalse(pcPlacement->mbRotated);
	AssertDouble(1.6, pcPlacement->mcBounds.msTopLeft.x, 6);
	AssertDouble(1.6, pcPlacement->mcBounds.msTopLeft.y, 6);
	AssertDouble(1.6 + 1.778, pcPlacement->msOffset.x, 6);
	AssertDouble(1.6 + 30.554, pcPlacement->msOffset.y, 6);

	pcPlacement = cPanel.GetPlacement(pcTransition);
	AssertFalse(pcPlacement->mbRotated);
	AssertDouble(1.6 + 18.797 + 1.6, pcPlacement->mcBounds.msTopLeft.x, 6);
	AssertDouble(1.6, pcPlacement->mcBounds.msTopLeft.y, 6);
	AssertDouble(1.6 + 18.797 + 1.6 + 1.905, pcPlacement->msOffset.x, 6);
	AssertDouble(1.6 + 18.415, pcPlacement->msOffset.y, 6);
	cPanel.Kill();

	cSets.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberPanelAllBoards(void)
{
	CGerberFileSets	cSets;
	CGerberPanel	cPanel;
	float64			fArea;
	float64			fRotatedArea;

	cSets.Init();
	AssertTrue(cSets.AddDirectories("Input" _FS_ "KiCad"));
	AssertSize(11, cSets.NumSets());

	cPanel.Init(1.6, 1.6, false);
	AssertTrue(cPanel.Pack(&cSets));
	AssertGerberPanel(&cPanel, &cSets, 1.6, 1.6);
	fArea = cPanel.GetSize()->x * cPanel.GetSize()->y;
	cPanel.Kill();

	//Turning boards never makes the panel bigger.
	cPanel.Init(1.6, 1.6, true);
	AssertTrue(cPanel.Pack(&cSets));
	AssertGerberPanel(&cPanel, &cSets, 1.6, 1.6);
	fRotatedArea = cPanel.GetSize()->x * cPanel.GetSize()->y;
	AssertTrue(fRotatedArea <= fArea + 0.0001);
	cPanel.Kill();

	//A wider gap and a 5mm rail.
	cPanel.Init(2.0, 5.0, true);
	AssertTrue(cPanel.Pack(&cSets));
	AssertGerberPanel(&cPanel, &cSets, 2.0, 5.0);
	cPanel.Kill();

	cSets.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberPanelNoOutline(void)
{
	CGerberFileSets		cSets;
	CGerberPanel		cPanel;
	CFileUtil			cFileUtil;
	CLogToMemory		cMemoryLog;
	char				szLogText[1000];

	//A board with only a copper layer has no edge cuts to pack by.
	cFileUtil.RemoveDir("Output" _FS_ "NoOutline");
	AssertTrue(cFileUtil.TouchDir("Output" _FS_ "NoOutline"));
	AssertTrue(cFileUtil.Copy("Input" _FS_ "KiCad" _FS_ "Clock Doubler" _FS_ "Clock Doubler-F_Cu.gtl", "Output" _FS_ "NoOutline" _FS_ "NoOutline-F_Cu.gtl"));

	cSets.Init();
	AssertNotNull(cSets.Add("Output" _FS_ "NoOutline"));
	AssertFalse(cSets.Get("NoOutline")->HasOutlineBounds());

	cPanel.Init();
	cMemoryLog.Start(true);
	AssertFalse(cPanel.Pack(&cSets));
	cMemoryLog.Stop(szLogText, 1000);
	AssertTrue(strstr(szLogText, "Board [NoOutline] has no edge cuts outline.") != NULL);
	AssertSize(0, cPanel.NumPlacements());
	cPanel.Kill();

	cSets.Kill();
	cFileUtil.RemoveDir("Output");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberPanel(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();

	TestGerberPanelTwoBoards();
	TestGerberPanelAllBoards();
	TestGerberPanelNoOutline();

	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

