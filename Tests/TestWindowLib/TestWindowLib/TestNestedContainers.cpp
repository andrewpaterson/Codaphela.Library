#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "StandardLib/Objects.h"
#include "SupportLib/FontReader.h"
#include "WindowLib/Window.h"
#include "WindowLib/GridContainer.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/FlowContainer.h"
#include "WindowLib/Label.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/AssertGeometric.h"
#include "TestLib/AssertFile.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"


Ptr<CFont> ReadLabelFont(void);
void ValidateLabelObjects(void);
void AssertLabelObjectsRemoved(void);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFillContainer> CreateNestedBackgroundFill(Ptr<CWindow> pWindow, ARGB32 sColour)
{
	Ptr<CFillContainer>		pFill;
	Ptr<CClearCanvasDraw>	pDraw;
	Ptr<CCanvas>			pBackground;

	//The background fills the fill container's inset area so it shows where the container's children can be drawn.
	pFill = OMalloc<CFillContainer>(pWindow);
	pDraw = OMalloc<CClearCanvasDraw>(sColour);
	pBackground = OMalloc<CCanvas>(pWindow, pDraw);
	pBackground->SetAlignment(CSH_Stretch, CSV_Stretch);
	pFill->AddComponent(pBackground);
	return pFill;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertInsideContainerBounds(CComponent* pcComponent, CContainer* pcContainer)
{
	CInt32Rect	cInner;
	CInt32Rect	cOuter;

	AssertTrue(pcComponent->GetContainerBounds(&cInner));
	AssertTrue(pcContainer->GetContainerBounds(&cOuter));
	AssertTrue(cInner.GetLeft() >= cOuter.GetLeft());
	AssertTrue(cInner.GetTop() >= cOuter.GetTop());
	AssertTrue(cInner.GetRight() <= cOuter.GetRight());
	AssertTrue(cInner.GetBottom() <= cOuter.GetBottom());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool OverflowsContainerBounds(CComponent* pcComponent, CContainer* pcContainer)
{
	CInt32Rect	cBounds;
	SInt32Vec2	sPosition;
	SInt32Vec2	sSize;

	pcContainer->GetContainerBounds(&cBounds);
	sPosition = pcComponent->GetPosition();
	sSize = pcComponent->GetActualSize();
	return (sPosition.x < cBounds.GetLeft()) || (sPosition.y < cBounds.GetTop()) ||
		   ((sPosition.x + sSize.x) > cBounds.GetRight()) || ((sPosition.y + sSize.y) > cBounds.GetBottom());
}


//////////////////////////////////////////////////////////////////////////
//
// A grid of containers, some holding containers of their own, squeezed
// into a window too small for the labels in them.  The grid's cells are
// all weighted so they share the 100 x 44 window equally, 50 x 22 each,
// and every label is 24 high.
//
//   +-----------------+-----------------+
//   | Fill            | Fill, 3 insets  |
//   |   Flow          |   "Overflowing" |
//   |   "One" "Two"   |                 |
//   |   "Three"       |                 |
//   +-----------------+-----------------+
//   | Fill                              |
//   |   Grid  "Alpha" "Beta" "Gamma"    |
//   +-----------------------------------+
//
// The flow wraps its labels onto lines that run out of the bottom of its
// cell, "Overflowing" is wider than its fill's inset area and the inner
// grid shrinks its columns below the labels' widths so they overlap each
// other and hang out of both ends of the grid.  With clipping every label
// is cut off at the edge of the container it is in.  Without clipping
// "Overflowing" is drawn over its fill's insets and the flow's cell.  The
// flow's lower lines are still hidden, by the bottom row drawn after them.
//
//////////////////////////////////////////////////////////////////////////
void TestNestedContainersLabels(char* szDirectory, bool bClipsChildren)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char*					aszFlowText[3] = { "One", "Two", "Three" };
	char*					aszGridText[3] = { "Alpha", "Beta", "Gamma" };
	int						aiInsets[4] = { 3, 3, 3, 3 };  //Top, Right, Bottom, Left.

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CClearCanvasDraw>	pBackground;
		Ptr<CTickTestRefWindow>	pTick;
		SDataTestRefWindow		cData;
		Ptr<CFont>				pFont;
		Ptr<CGridContainer>		pGrid;
		Ptr<CFillContainer>		pFlowFill;
		Ptr<CFlowContainer>		pFlow;
		Ptr<CFillContainer>		pInsetFill;
		Ptr<CFillContainer>		pGridFill;
		Ptr<CGridContainer>		pInnerGrid;
		Ptr<CLabel>				apFlowLabels[3];
		Ptr<CLabel>				pWideLabel;
		Ptr<CLabel>				apGridLabels[3];
		SInsets					sInsets;
		SInt32Vec2				sVec;
		SInt32Vec2				sRequired;
		CInt32Rect				cBounds;
		int32					iStart;
		int32					iSize;
		int32					x;
		int32					y;
		CChars					szExpectedDirectory;
		size					i;

		cNativeFactory.Init(&gcMemoryAllocator, 100, 44, szDirectory);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pBackground = OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)40, 60, 110));
		pTestWindow = OMalloc<CWindow>("Nested Containers", &cNativeFactory, pTick, pBackground, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pFont = ReadLabelFont();
		AssertTrue(pFont.IsNotNull());

		pGrid = OMalloc<CGridContainer>(pTestWindow);
		pTestWindow->SetContainer(pGrid);

		//Top left, a flow of labels inside a fill.
		pFlowFill = CreateNestedBackgroundFill(pTestWindow, Set32BitColour((uint8)110, 30, 30));
		pFlow = OMalloc<CFlowContainer>(pTestWindow);
		pFlow->SetFlowStyle(CSD_Right, CSW_Wrap, CSV_Top);
		pFlowFill->AddComponent(pFlow);
		for (i = 0; i < 3; i++)
		{
			apFlowLabels[i] = OMalloc<CLabel>(pTestWindow, pFont, aszFlowText[i]);
			apFlowLabels[i]->SetAlignment(CSH_Left, CSV_Top);
			pFlow->AddComponent(apFlowLabels[i]);
		}
		AssertTrue(pGrid->AddComponent(pFlowFill, 0, 0, 1, 1, 1, 1));

		//Top right, a label too wide for the fill it is centred in.
		pInsetFill = CreateNestedBackgroundFill(pTestWindow, Set32BitColour((uint8)30, 100, 40));
		sInsets.Init(aiInsets);
		pInsetFill->SetInsets(&sInsets);
		pWideLabel = OMalloc<CLabel>(pTestWindow, pFont, "Overflowing");
		pWideLabel->SetAlignment(CSH_Center, CSV_Center);
		pInsetFill->AddComponent(pWideLabel);
		AssertTrue(pGrid->AddComponent(pInsetFill, 1, 0, 1, 1, 1, 1));

		//Along the bottom, a grid of labels inside a fill.
		pGridFill = CreateNestedBackgroundFill(pTestWindow, Set32BitColour((uint8)90, 40, 110));
		pInnerGrid = OMalloc<CGridContainer>(pTestWindow);
		pGridFill->AddComponent(pInnerGrid);
		for (i = 0; i < 3; i++)
		{
			apGridLabels[i] = OMalloc<CLabel>(pTestWindow, pFont, aszGridText[i]);
			apGridLabels[i]->SetAlignment(CSH_Center, CSV_Center);
			AssertTrue(pInnerGrid->AddComponent(apGridLabels[i], (int)i, 0));
		}
		AssertTrue(pGrid->AddComponent(pGridFill, 0, 1, 2, 1, 1, 1));

		pGrid->SetClipsChildren(bClipsChildren);
		pFlowFill->SetClipsChildren(bClipsChildren);
		pFlow->SetClipsChildren(bClipsChildren);
		pInsetFill->SetClipsChildren(bClipsChildren);
		pGridFill->SetClipsChildren(bClipsChildren);
		pInnerGrid->SetClipsChildren(bClipsChildren);

		pTestWindow->Show();

		//Each cell of the outer grid is 50 x 22.
		AssertTrue(pGrid->GetTrackBounds(true, 0, &iStart, &iSize));
		AssertInt(0, iStart);
		AssertInt(50, iSize);
		AssertTrue(pGrid->GetTrackBounds(true, 1, &iStart, &iSize));
		AssertInt(50, iStart);
		AssertInt(50, iSize);
		AssertTrue(pGrid->GetTrackBounds(false, 0, &iStart, &iSize));
		AssertInt(0, iStart);
		AssertInt(22, iSize);
		AssertTrue(pGrid->GetTrackBounds(false, 1, &iStart, &iSize));
		AssertInt(22, iStart);
		AssertInt(22, iSize);

		//Every container sits inside its parent's container bounds whatever its children need.
		AssertInsideContainerBounds(&pFlowFill, &pGrid);
		AssertInsideContainerBounds(&pFlow, &pFlowFill);
		AssertInsideContainerBounds(&pInsetFill, &pGrid);
		AssertInsideContainerBounds(&pGridFill, &pGrid);
		AssertInsideContainerBounds(&pInnerGrid, &pGridFill);
		AssertTrue(pFlowFill->GetContainerBounds(&cBounds));
		AssertInt(0, cBounds.GetLeft());
		AssertInt(0, cBounds.GetTop());
		AssertInt(50, cBounds.GetRight());
		AssertInt(22, cBounds.GetBottom());
		AssertTrue(pInsetFill->GetContainerBounds(&cBounds));
		AssertInt(53, cBounds.GetLeft());
		AssertInt(3, cBounds.GetTop());
		AssertInt(97, cBounds.GetRight());
		AssertInt(19, cBounds.GetBottom());
		AssertTrue(pInnerGrid->GetContainerBounds(&cBounds));
		AssertInt(0, cBounds.GetLeft());
		AssertInt(22, cBounds.GetTop());
		AssertInt(100, cBounds.GetRight());
		AssertInt(44, cBounds.GetBottom());

		//The labels keep their size however little space they are given.
		for (i = 0; i < 3; i++)
		{
			sRequired = apFlowLabels[i]->GetRequiredSize();
			sVec = apFlowLabels[i]->GetActualSize();
			AssertInt32Vec2(sRequired.x, sRequired.y, &sVec);
			sRequired = apGridLabels[i]->GetRequiredSize();
			sVec = apGridLabels[i]->GetActualSize();
			AssertInt32Vec2(sRequired.x, sRequired.y, &sVec);
		}
		sRequired = pWideLabel->GetRequiredSize();
		sVec = pWideLabel->GetActualSize();
		AssertInt32Vec2(sRequired.x, sRequired.y, &sVec);

		//So they need clipping.  Two labels don't fit on a 50 wide line so each is on its own line and the last starts below
		//the cell.
		AssertTrue(OverflowsContainerBounds(&apFlowLabels[0], &pFlow));
		sVec = apFlowLabels[2]->GetPosition();
		AssertInt32Vec2(0, 48, &sVec);
		AssertTrue(pWideLabel->GetRequiredSize().x > 44);
		AssertTrue(OverflowsContainerBounds(&pWideLabel, &pInsetFill));
		AssertTrue(OverflowsContainerBounds(&apGridLabels[0], &pInnerGrid));
		AssertTrue(OverflowsContainerBounds(&apGridLabels[2], &pInnerGrid));
		sVec = apGridLabels[0]->GetPosition();
		AssertTrue(sVec.x < 0);
		AssertTrue(sVec.y < 22);

		//"Overflowing" covers the left and right insets of its fill.  Where it is clipped the fill is found instead of it.
		sVec = pWideLabel->GetPosition();
		x = 51;
		y = sVec.y + 12;
		AssertTrue(pWideLabel->IsPointIn(x, y));
		if (bClipsChildren)
		{
			AssertTrue(pTestWindow->FindComponentAt(x, y) == pInsetFill);
		}
		else
		{
			AssertTrue(pTestWindow->FindComponentAt(x, y) == pWideLabel);
		}
		AssertTrue(pTestWindow->FindComponentAt(75, y) == pWideLabel);

		ValidateLabelObjects();

		for (i = 0; i < 3; i++)
		{
			apFlowLabels[i] = NULL;
			apGridLabels[i] = NULL;
		}
		pWideLabel = NULL;
		pInnerGrid = NULL;
		pGridFill = NULL;
		pInsetFill = NULL;
		pFlow = NULL;
		pFlowFill = NULL;
		pGrid = NULL;
		pFont = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		pBackground = NULL;

		szExpectedDirectory.Init(szDirectory);
		szExpectedDirectory.Replace("Output", "Input");
		AssertDirectory(szExpectedDirectory.Text(), szDirectory);
		szExpectedDirectory.Kill();
	}

	AssertLabelObjectsRemoved();
	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNestedContainersLabelsClipped(void)
{
	TestNestedContainersLabels("Output" _FS_ "NestedContainersLabelsClipped", true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNestedContainersLabelsUnclipped(void)
{
	TestNestedContainersLabels("Output" _FS_ "NestedContainersLabelsUnclipped", false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNestedContainers(void)
{
	BeginTests();
	ObjectsInit();

	TestNestedContainersLabelsClipped();
	TestNestedContainersLabelsUnclipped();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

