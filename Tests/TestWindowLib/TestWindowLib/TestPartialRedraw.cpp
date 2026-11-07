#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/GridContainer.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WindowLib/ScrollPane.h"
#include "WindowLib/WindowTheme.h"
#include "WindowLib/DamageArea.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertGeometric.h"
#include "BoxFillCanvasDraw.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"


Ptr<CScrollBarImages> CreateScrollBarImages(void);
Ptr<CWindowTheme> CreateScrollPaneTheme(Ptr<CScrollBarImages> pImages);


//////////////////////////////////////////////////////////////////////////
//
// A rectangle, right and bottom exclusive.
//
//////////////////////////////////////////////////////////////////////////
static CInt32Rect* SetRect(CInt32Rect* pcRect, int32 iLeft, int32 iTop, int32 iRight, int32 iBottom)
{
	pcRect->Init(iLeft, iTop, iRight, iBottom);
	return pcRect;
}


//////////////////////////////////////////////////////////////////////////
//
// Rectangles inside one already there add nothing, one covering others
// replaces them, more than the most kept merge into their bounds.
//
//////////////////////////////////////////////////////////////////////////
void TestDamageArea(void)
{
	CDamageArea	cDamage;
	CInt32Rect	cRect;
	CInt32Rect	cBounds;
	int32		i;

	cDamage.Init();
	AssertTrue(cDamage.IsEmpty());
	AssertFalse(cDamage.IsAll());

	//Empty rectangles add nothing.
	cDamage.Add(SetRect(&cRect, 5, 5, 5, 10));
	cDamage.Add(SetRect(&cRect, 5, 5, 10, 5));
	AssertTrue(cDamage.IsEmpty());

	cDamage.Add(SetRect(&cRect, 0, 0, 10, 10));
	cDamage.Add(SetRect(&cRect, 2, 2, 8, 8));
	AssertSize(1, cDamage.NumRectangles());
	cDamage.Add(SetRect(&cRect, 20, 0, 30, 10));
	AssertSize(2, cDamage.NumRectangles());
	cDamage.Add(SetRect(&cRect, -5, -5, 15, 15));
	AssertSize(2, cDamage.NumRectangles());
	AssertTrue(cDamage.GetRectangle(1)->Equals(SetRect(&cRect, -5, -5, 15, 15)));

	AssertTrue(cDamage.Intersects(SetRect(&cRect, 14, 14, 16, 16)));
	AssertFalse(cDamage.Intersects(SetRect(&cRect, 15, 0, 20, 10)));
	AssertFalse(cDamage.Intersects(SetRect(&cRect, 0, 0, 0, 0)));

	//Too many merge into their bounds.
	cDamage.Clear();
	for (i = 0; i < DAMAGE_AREA_MAX_RECTANGLES; i++)
	{
		cDamage.Add(SetRect(&cRect, i * 10, 0, i * 10 + 5, 5));
	}
	AssertSize(DAMAGE_AREA_MAX_RECTANGLES, cDamage.NumRectangles());
	cDamage.Add(SetRect(&cRect, 0, 100, 5, 105));
	AssertSize(1, cDamage.NumRectangles());
	cDamage.GetBounds(&cBounds);
	AssertTrue(cBounds.Equals(SetRect(&cRect, 0, 0, (DAMAGE_AREA_MAX_RECTANGLES - 1) * 10 + 5, 105)));

	//All of the window.
	cDamage.AddAll();
	AssertTrue(cDamage.IsAll());
	AssertFalse(cDamage.IsEmpty());
	AssertTrue(cDamage.Intersects(SetRect(&cRect, 1000, 1000, 1001, 1001)));
	cDamage.Add(SetRect(&cRect, 0, 0, 10, 10));
	AssertTrue(cDamage.IsAll());
	cDamage.Clear();
	AssertTrue(cDamage.IsEmpty());

	cDamage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A window of box filled canvases in a grid: two in the first column
// (pFirst over pSecond), one beside them that never moves (pFixed), and a
// scroll pane holding a tall canvas (pScrolled).
//
//////////////////////////////////////////////////////////////////////////
struct SDamageWindow
{
	Ptr<CWindow>				pWindow;
	Ptr<CTickTestRefWindow>		pTick;
	Ptr<CGridContainer>			pGrid;
	Ptr<CCanvas>				pFirst;
	Ptr<CCanvas>				pSecond;
	Ptr<CCanvas>				pFixed;
	Ptr<CScrollPane>			pScrollPane;
	Ptr<CCanvas>				pScrolled;
	SDataTestRefWindow			cData;
};


static Ptr<CCanvas> CreateBoxFill(Ptr<CWindow> pWindow, ARGB32 sColour, int32 iWidth, int32 iHeight)
{
	Ptr<CCanvas>	pCanvas;

	pCanvas = OMalloc<CCanvas>(pWindow, OMalloc<CBoxFillCanvasDraw>(sColour));
	pCanvas->SetDesiredSize(iWidth, iHeight);
	pCanvas->SetAlignment(CSH_Left, CSV_Top);
	return pCanvas;
}


static void CreateDamageWindow(SDamageWindow* psWindow, CWinRefWindowFactory* pcFactory)
{
	CPointer	pNull;

	psWindow->pTick = OMalloc<CTickTestRefWindow>(&psWindow->cData, 1);
	psWindow->pWindow = OMalloc<CWindow>("Damage", pcFactory, psWindow->pTick, OMalloc<CClearCanvasDraw>(Set32BitColour(0.0f, 0, 0)), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
	ORoot()->Add(psWindow->pWindow);

	psWindow->pGrid = OMalloc<CGridContainer>(psWindow->pWindow);
	psWindow->pWindow->SetContainer(psWindow->pGrid);
	psWindow->pFirst = CreateBoxFill(psWindow->pWindow, Set32BitColour(1.0f, 0, 0), 20, 10);
	psWindow->pSecond = CreateBoxFill(psWindow->pWindow, Set32BitColour(0, 1.0f, 0), 20, 10);
	psWindow->pFixed = CreateBoxFill(psWindow->pWindow, Set32BitColour(0, 0, 1.0f), 16, 16);
	psWindow->pScrollPane = OMalloc<CScrollPane>(psWindow->pWindow, CreateScrollPaneTheme(CreateScrollBarImages()));
	psWindow->pScrollPane->SetDesiredSize(24, 20);
	psWindow->pScrolled = CreateBoxFill(psWindow->pWindow, Set32BitColour(1.0f, 1.0f, 0), 24, 60);
	psWindow->pScrollPane->SetContent(psWindow->pScrolled);
	psWindow->pGrid->AddComponent(psWindow->pFirst, 0, 0);
	psWindow->pGrid->AddComponent(psWindow->pSecond, 0, 1);
	psWindow->pGrid->AddComponent(psWindow->pFixed, 1, 0);
	psWindow->pGrid->AddComponent(psWindow->pScrollPane, 2, 0);
}


static void KillDamageWindow(SDamageWindow* psWindow)
{
	psWindow->pScrolled = NULL;
	psWindow->pScrollPane = NULL;
	psWindow->pFixed = NULL;
	psWindow->pSecond = NULL;
	psWindow->pFirst = NULL;
	psWindow->pGrid = NULL;
	ORoot()->Remove(psWindow->pWindow);
	psWindow->pWindow = NULL;
	psWindow->pTick = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// pCanvas was drawn in pcBefore at the paint before last.  If it is still
// there, it mustn't have been damaged.  pcBefore becomes where it is now.
//
//////////////////////////////////////////////////////////////////////////
static void AssertUndamagedIfStill(CDamageArea* pcDamage, Ptr<CCanvas> pCanvas, CInt32Rect* pcBefore)
{
	CInt32Rect	cNow;

	pCanvas->GetDrawnArea(&cNow);
	if (cNow.Equals(pcBefore))
	{
		AssertFalse(pcDamage->Intersects(&cNow));
	}
	pcBefore->Init(&cNow);
}


//////////////////////////////////////////////////////////////////////////
//
// What a paint finds changed: nothing when nothing did; a canvas's area
// when it is invalidated; the old and new areas of canvases that moved;
// where a canvas was when it is taken out; a scrolled canvas's area; and
// a popup's area when it opens and closes.  Nothing else.
//
//////////////////////////////////////////////////////////////////////////
void TestPartialRedrawDamage(void)
{
	CWinRefWindowFactory	cFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "PartialRedrawDamage";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	ObjectsInit();
	cFactory.Init(&gcMemoryAllocator, 64, 48, szDirectory);
	{
		SDamageWindow		sWindow;
		CDamageArea*		pcDamage;
		CInt32Rect			cFirst;
		CInt32Rect			cSecond;
		CInt32Rect			cFixed;
		CInt32Rect			cScrolled;
		CInt32Rect			cArea;
		CInt32Rect			cBounds;
		Ptr<CCanvas>		pPopupContent;
		CPointer			pNull;

		CreateDamageWindow(&sWindow, &cFactory);

		//Everything appears at the first paint; nothing has changed at the second.
		sWindow.pWindow->Paint();
		pcDamage = sWindow.pWindow->GetPaintedDamage();
		AssertFalse(pcDamage->IsEmpty());
		sWindow.pWindow->Paint();
		AssertTrue(pcDamage->IsEmpty());

		sWindow.pFirst->GetDrawnArea(&cFirst);
		sWindow.pSecond->GetDrawnArea(&cSecond);
		sWindow.pFixed->GetDrawnArea(&cFixed);
		sWindow.pScrolled->GetDrawnArea(&cScrolled);
		AssertInt(20, cFirst.GetWidth());
		AssertInt(10, cFirst.GetHeight());
		AssertInt(20, cSecond.GetWidth());
		AssertTrue(cSecond.GetTop() >= cFirst.GetBottom());
		AssertInt(16, cFixed.GetWidth());
		AssertInt(20, cScrolled.GetHeight());

		//An invalidated canvas.
		sWindow.pFirst->Invalidate();
		sWindow.pWindow->Paint();
		AssertSize(1, pcDamage->NumRectangles());
		AssertTrue(pcDamage->GetRectangle(0)->Equals(&cFirst));

		//The first grows taller than its row, so the second moves down.
		sWindow.pFirst->SetDesiredSize(20, 30);
		sWindow.pWindow->Paint();
		AssertTrue(pcDamage->Intersects(&cFirst));
		sWindow.pFirst->GetDrawnArea(&cArea);
		AssertInt(30, cArea.GetHeight());
		AssertTrue(pcDamage->Intersects(&cArea));
		AssertTrue(pcDamage->Intersects(&cSecond));
		sWindow.pSecond->GetDrawnArea(&cArea);
		AssertFalse(cArea.Equals(&cSecond));
		AssertTrue(pcDamage->Intersects(&cArea));
		AssertUndamagedIfStill(pcDamage, sWindow.pFixed, &cFixed);
		AssertUndamagedIfStill(pcDamage, sWindow.pScrolled, &cScrolled);

		//The second is taken out.
		sWindow.pSecond->GetDrawnArea(&cSecond);
		AssertTrue(sWindow.pGrid->RemoveComponent(sWindow.pSecond));
		sWindow.pWindow->Paint();
		AssertTrue(pcDamage->Intersects(&cSecond));
		AssertUndamagedIfStill(pcDamage, sWindow.pFixed, &cFixed);
		sWindow.pSecond->GetDrawnArea(&cArea);
		AssertInt(0, cArea.GetWidth());

		//Scrolled, the canvas shows another part of itself in the same place.
		sWindow.pScrollPane->SetScrollPosition(0, 15);
		sWindow.pWindow->Paint();
		AssertTrue(pcDamage->Intersects(&cScrolled));
		AssertUndamagedIfStill(pcDamage, sWindow.pFixed, &cFixed);
		AssertUndamagedIfStill(pcDamage, sWindow.pFirst, &cFirst);

		//A popup, where it is while it is open.
		pPopupContent = CreateBoxFill(sWindow.pWindow, Set32BitColour(1.0f, 0, 1.0f), 12, 8);
		sWindow.pWindow->OpenPopup(pPopupContent, SInt32Vec2(40, 30), pNull);
		sWindow.pWindow->Paint();
		pPopupContent->GetDrawnArea(&cArea);
		AssertInt(12, cArea.GetWidth());
		AssertTrue(pcDamage->Intersects(&cArea));
		AssertUndamagedIfStill(pcDamage, sWindow.pFixed, &cFixed);
		AssertTrue(sWindow.pWindow->ClosePopup(pPopupContent));
		sWindow.pWindow->Paint();
		AssertTrue(pcDamage->Intersects(&cArea));
		AssertUndamagedIfStill(pcDamage, sWindow.pFixed, &cFixed);

		//The window invalidated is all of it.
		sWindow.pWindow->Invalidate();
		sWindow.pWindow->Paint();
		AssertTrue(pcDamage->IsAll());

		pPopupContent = NULL;
		KillDamageWindow(&sWindow);
	}
	AssertTrue(ORoot()->RemoveAll());
	cFactory.Kill();
	ObjectsFlush();
	ObjectsKill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPartialRedraw(void)
{
	BeginTests();

	TestDamageArea();
	TestPartialRedrawDamage();

	TestStatistics();
}

