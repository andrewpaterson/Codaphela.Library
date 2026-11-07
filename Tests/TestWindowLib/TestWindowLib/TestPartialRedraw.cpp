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
#include "WindowLib/Dialog.h"
#include "WindowLib/JewelTheme.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "WinRefLib/WinRefWindow.h"
#include "WindowLib/RoundedBoxCanvasDraw.h"
#include "WindowLib/TextArea.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertGeometric.h"
#include "BoxFillCanvasDraw.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"


Ptr<CWindowTheme> CreateTestTheme(Ptr<CFont> pFont);
Ptr<CFont> ReadLabelFont(void);


Ptr<CScrollBarImages> CreateScrollBarImages(void);
Ptr<CFont> GenerateMenuFont(Ptr<CWindow> pWindow);
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
	Ptr<CCanvas>				pSolid;  //Only made by CreatePartialWindow.
	Ptr<CCanvas>				pRounded;
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
	psWindow->pRounded = NULL;
	psWindow->pSolid = NULL;
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
// The damage window redrawing partially and checked against drawing all
// of it, with a solid fill and a transparent rounded box over the grid's
// background too.
//
//////////////////////////////////////////////////////////////////////////
static void CreatePartialWindow(SDamageWindow* psWindow, CWinRefWindowFactory* pcFactory)
{
	SInsets		sInsets;

	CreateDamageWindow(psWindow, pcFactory);
	psWindow->pSolid = OMalloc<CCanvas>(psWindow->pWindow, OMalloc<CClearCanvasDraw>(Set32BitColour(0.5f, 0.5f, 0.5f)));
	psWindow->pSolid->SetDesiredSize(12, 12);
	psWindow->pSolid->SetAlignment(CSH_Left, CSV_Top);
	sInsets.Init(1, 1);
	psWindow->pRounded = OMalloc<CCanvas>(psWindow->pWindow, OMalloc<CRoundedBoxCanvasDraw>(Set32BitColour(0, 1.0f, 1.0f), 4, &sInsets));
	psWindow->pRounded->SetTransparent(true);
	psWindow->pRounded->SetDesiredSize(14, 14);
	psWindow->pRounded->SetAlignment(CSH_Left, CSV_Top);
	psWindow->pGrid->AddComponent(psWindow->pSolid, 1, 1);
	psWindow->pGrid->AddComponent(psWindow->pRounded, 2, 1);
	psWindow->pWindow->SetPartialRedraw(true);
	psWindow->pWindow->SetVerifyRedraw(true);
}


//////////////////////////////////////////////////////////////////////////
//
// A window drawing only what changed shows exactly what drawing all of it
// would, through invalidations, a canvas growing and moving another, one
// taken out, scrolling and a popup; and only shows what changed.
//
//////////////////////////////////////////////////////////////////////////
void TestPartialRedrawMatchesFull(void)
{
	CWinRefWindowFactory	cFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "PartialRedrawMatches";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	ObjectsInit();
	cFactory.Init(&gcMemoryAllocator, 64, 48, szDirectory);
	{
		SDamageWindow			sWindow;
		Ptr<CCanvas>			pPopupContent;
		CWinRefWindow*			pcNative;
		CDamageArea*			pcPresented;
		CInt32Rect				cArea;
		CPointer				pNull;

		CreatePartialWindow(&sWindow, &cFactory);
		pcNative = (CWinRefWindow*)sWindow.pWindow->GetNativeWindow();
		pcPresented = pcNative->GetPresented();

		//The first paint draws all of it.
		sWindow.pWindow->Paint();
		AssertTrue(pcPresented->IsAll());

		//Nothing changed: nothing is drawn or shown.
		pcNative->ClearPresented();
		sWindow.pWindow->Paint();
		AssertTrue(pcPresented->IsEmpty());

		//A canvas changes colour and says so.
		pcNative->ClearPresented();
		((CBoxFillCanvasDraw*)&sWindow.pFirst->GetCanvasDraw())->SetColour(Set32BitColour(1.0f, 0.5f, 0));
		sWindow.pFirst->Invalidate();
		sWindow.pWindow->Paint();
		sWindow.pFirst->GetDrawnArea(&cArea);
		AssertFalse(pcPresented->IsAll());
		AssertTrue(pcPresented->Intersects(&cArea));
		AssertSize(0, sWindow.pWindow->GetRedrawMismatches());

		//The transparent box over the background and a solid fill.
		sWindow.pRounded->Invalidate();
		sWindow.pSolid->Invalidate();
		sWindow.pWindow->Paint();
		AssertSize(0, sWindow.pWindow->GetRedrawMismatches());

		//Growing moves the second canvas.
		sWindow.pFirst->SetDesiredSize(20, 30);
		sWindow.pWindow->Paint();
		AssertSize(0, sWindow.pWindow->GetRedrawMismatches());

		//Taken out, the background shows where it was.
		AssertTrue(sWindow.pGrid->RemoveComponent(sWindow.pSecond));
		sWindow.pWindow->Paint();
		AssertSize(0, sWindow.pWindow->GetRedrawMismatches());

		//Scrolled.
		sWindow.pScrollPane->SetScrollPosition(0, 15);
		sWindow.pWindow->Paint();
		AssertSize(0, sWindow.pWindow->GetRedrawMismatches());
		sWindow.pScrollPane->SetScrollPosition(0, 3);
		sWindow.pWindow->Paint();
		AssertSize(0, sWindow.pWindow->GetRedrawMismatches());

		//A popup over the canvases, opened and closed.
		pPopupContent = CreateBoxFill(sWindow.pWindow, Set32BitColour(1.0f, 0, 1.0f), 30, 20);
		sWindow.pWindow->OpenPopup(pPopupContent, SInt32Vec2(10, 5), pNull);
		sWindow.pWindow->Paint();
		AssertSize(0, sWindow.pWindow->GetRedrawMismatches());
		AssertTrue(sWindow.pWindow->ClosePopup(pPopupContent));
		sWindow.pWindow->Paint();
		AssertSize(0, sWindow.pWindow->GetRedrawMismatches());

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
// A canvas that changes without being invalidated keeps showing what it
// last drew in a partial paint; checking against drawing all of the
// window finds it.
//
//////////////////////////////////////////////////////////////////////////
void TestPartialRedrawVerifyFindsStale(void)
{
	CWinRefWindowFactory	cFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "PartialRedrawStale";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	ObjectsInit();
	cFactory.Init(&gcMemoryAllocator, 64, 48, szDirectory);
	{
		SDamageWindow			sWindow;
		SLogConfig				sLogConfig;

		CreatePartialWindow(&sWindow, &cFactory);
		sWindow.pWindow->Paint();
		AssertSize(0, sWindow.pWindow->GetRedrawMismatches());

		//The first changes colour without saying so; something else changing makes the window paint.
		((CBoxFillCanvasDraw*)&sWindow.pFirst->GetCanvasDraw())->SetColour(Set32BitColour(1.0f, 0.5f, 0));
		sWindow.pFixed->Invalidate();
		sLogConfig = gcLogger.SetSilent();
		sWindow.pWindow->Paint();
		gcLogger.SetConfig(&sLogConfig);
		AssertSize(1, sWindow.pWindow->GetRedrawMismatches());

		//Checking drew all of the window, so it is right again.
		sWindow.pFixed->Invalidate();
		sWindow.pWindow->Paint();
		AssertSize(1, sWindow.pWindow->GetRedrawMismatches());

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
// A dialog draws partially, and is verified, as the window it belongs to.
//
//////////////////////////////////////////////////////////////////////////
void TestPartialRedrawDialogLikeOwner(void)
{
	CWinRefWindowFactory	cFactory;

	ObjectsInit();
	cFactory.Init(&gcMemoryAllocator, 300, 200, NULL);
	{
		Ptr<CWindow>		pOwner;
		Ptr<CWindowTheme>	pTheme;
		SDialogLook			sLook;
		Ptr<CDialog>		pFull;
		Ptr<CDialog>		pPartial;
		CPointer			pNull;

		pOwner = OMalloc<CWindow>("Dialog Owner", &cFactory, pNull, OMalloc<CClearCanvasDraw>(Set32BitColour(0.0f, 0, 0)), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pOwner);
		pTheme = CJewelTheme::CreateDarkTheme(pOwner, GenerateMenuFont(pOwner));
		sLook.pcTheme = &pTheme;
		sLook.pcDimFont = &pTheme->GetFont();

		pFull = OMalloc<CDialog>(pOwner, &sLook, "Full");
		ORoot()->Add(pFull);
		AssertFalse(pFull->GetWindow()->IsPartialRedraw());

		pOwner->SetPartialRedraw(true);
		pOwner->SetVerifyRedraw(true);
		pPartial = OMalloc<CDialog>(pOwner, &sLook, "Partial");
		ORoot()->Add(pPartial);
		AssertTrue(pPartial->GetWindow()->IsPartialRedraw());
		AssertTrue(pPartial->GetWindow()->IsVerifyRedraw());

		ORoot()->Remove(pPartial);
		ORoot()->Remove(pFull);
		pPartial = NULL;
		pFull = NULL;
		pTheme = NULL;
		ORoot()->Remove(pOwner);
		pOwner = NULL;
	}
	AssertTrue(ORoot()->RemoveAll());
	cFactory.Kill();
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// The window's pixel at x, y as its bytes.
//
//////////////////////////////////////////////////////////////////////////
static uint32 GetWindowPixel(Ptr<CWindow> pWindow, int32 x, int32 y)
{
	Ptr<CImage>	pImage;
	uint8*		puiData;
	size		uiPixelBytes;
	size		uiOffset;
	uint32		uiPixel;

	pImage = pWindow->GetCanvas()->GetImageOrNull();
	puiData = (uint8*)pImage->GetData();
	uiPixelBytes = pImage->GetByteSize() / ((size)pImage->GetWidth() * (size)pImage->GetHeight());
	uiOffset = ((size)y * (size)pImage->GetWidth() + (size)x) * uiPixelBytes;
	uiPixel = 0;
	memcpy(&uiPixel, &puiData[uiOffset], uiPixelBytes < sizeof(uint32) ? uiPixelBytes : sizeof(uint32));
	return uiPixel;
}


//////////////////////////////////////////////////////////////////////////
//
// A window with one 20 by 10 canvas at its top left, filled by pDraw,
// drawing only what changed.  Not checked against drawing all of it: the
// tests change the fill without invalidating all of the canvas.
//
//////////////////////////////////////////////////////////////////////////
static void CreatePartCanvasWindow(SDamageWindow* psWindow, CWinRefWindowFactory* pcFactory, Ptr<CBoxFillCanvasDraw> pDraw, bool bTransparent)
{
	Ptr<CFillContainer>	pFill;

	psWindow->pTick = OMalloc<CTickTestRefWindow>(&psWindow->cData, 1);
	psWindow->pWindow = OMalloc<CWindow>("Part", pcFactory, psWindow->pTick, OMalloc<CClearCanvasDraw>(Set32BitColour(0.0f, 0, 1.0f)), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
	ORoot()->Add(psWindow->pWindow);
	pFill = OMalloc<CFillContainer>(psWindow->pWindow);
	psWindow->pWindow->SetContainer(pFill);
	psWindow->pFirst = OMalloc<CCanvas>(psWindow->pWindow, pDraw);
	psWindow->pFirst->SetDesiredSize(20, 10);
	psWindow->pFirst->SetAlignment(CSH_Left, CSV_Top);
	psWindow->pFirst->SetTransparent(bTransparent);
	pFill->AddComponent(psWindow->pFirst);
	psWindow->pWindow->SetPartialRedraw(true);
	psWindow->pWindow->Paint();
}


//////////////////////////////////////////////////////////////////////////
//
// A canvas invalidated in part draws again, and shows again, only that
// part, if its draw can; a draw that can't draws all of it.  A
// transparent canvas's part is cleared before it is drawn, and
// invalidating all of a canvas takes over from its parts.
//
//////////////////////////////////////////////////////////////////////////
void TestPartialRedrawCanvasPart(void)
{
	CWinRefWindowFactory	cFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "PartialRedrawPart";
	ARGB32					sRed;
	ARGB32					sGreen;
	ARGB32					sTranslucent;

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);
	sRed = Set32BitColour(1.0f, 0, 0);
	sGreen = Set32BitColour(0, 1.0f, 0);
	sTranslucent = Set32BitColour(1.0f, 0, 0, 0.5f);

	ObjectsInit();
	cFactory.Init(&gcMemoryAllocator, 40, 20, szDirectory);
	{
		SDamageWindow			sWindow;
		Ptr<CBoxFillCanvasDraw>	pDraw;
		CWinRefWindow*			pcNative;
		CInt32Rect				cLeft;
		CInt32Rect				cPresented;
		uint32					uiRed;
		uint32					uiGreen;
		uint32					uiOverBlue;

		pDraw = OMalloc<CBoxFillCanvasDraw>(sRed);
		CreatePartCanvasWindow(&sWindow, &cFactory, pDraw, false);
		pcNative = (CWinRefWindow*)sWindow.pWindow->GetNativeWindow();
		uiRed = GetWindowPixel(sWindow.pWindow, 15, 5);

		//The left half draws green again; the right half keeps the red it drew before.
		pDraw->SetColour(sGreen);
		cLeft.Init(0, 0, 10, 10);
		pcNative->ClearPresented();
		sWindow.pFirst->InvalidateArea(&cLeft);
		sWindow.pWindow->Paint();
		uiGreen = GetWindowPixel(sWindow.pWindow, 5, 5);
		AssertTrue(uiGreen != uiRed);
		AssertInt(uiRed, GetWindowPixel(sWindow.pWindow, 15, 5));
		pcNative->GetPresented()->GetBounds(&cPresented);
		AssertTrue(cPresented.Equals(&cLeft));

		//A draw that can't draw part draws all of it.
		pDraw->SetColour(sRed);
		pDraw->SetCanDrawPart(false);
		sWindow.pFirst->InvalidateArea(&cLeft);
		sWindow.pWindow->Paint();
		AssertInt(uiRed, GetWindowPixel(sWindow.pWindow, 5, 5));
		AssertInt(uiRed, GetWindowPixel(sWindow.pWindow, 15, 5));

		//All of it invalidated after part of it: all of it draws.
		pDraw->SetCanDrawPart(true);
		pDraw->SetColour(sGreen);
		sWindow.pFirst->InvalidateArea(&cLeft);
		sWindow.pFirst->Invalidate();
		sWindow.pWindow->Paint();
		AssertInt(uiGreen, GetWindowPixel(sWindow.pWindow, 15, 5));

		pDraw = NULL;
		KillDamageWindow(&sWindow);

		//A translucent fill over the window's blue: the part drawn again is cleared first, or it would be drawn over itself.
		pDraw = OMalloc<CBoxFillCanvasDraw>(sTranslucent);
		CreatePartCanvasWindow(&sWindow, &cFactory, pDraw, true);
		uiOverBlue = GetWindowPixel(sWindow.pWindow, 5, 5);
		sWindow.pFirst->InvalidateArea(&cLeft);
		sWindow.pWindow->Paint();
		AssertInt(uiOverBlue, GetWindowPixel(sWindow.pWindow, 5, 5));
		AssertInt(uiOverBlue, GetWindowPixel(sWindow.pWindow, 15, 5));

		pDraw = NULL;
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
// A text area's selection moved from the keys is drawn by the next
// partial paint (it wasn't drawn until something else was), as are its
// edits, each as drawing all of the window would.
//
//////////////////////////////////////////////////////////////////////////
void TestPartialRedrawTextArea(void)
{
	CWinRefWindowFactory	cFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "PartialRedrawTextArea";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	ObjectsInit();
	cFactory.Init(&gcMemoryAllocator, 96, 32, szDirectory);
	{
		Ptr<CWindow>			pWindow;
		Ptr<CTickTestRefWindow>	pTick;
		SDataTestRefWindow		cData;
		Ptr<CFillContainer>		pFill;
		Ptr<CTextArea>			pTextArea;
		SInt32Vec2				sPosition;
		uint32					uiUnselected;

		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pWindow = OMalloc<CWindow>("Text", &cFactory, pTick, OMalloc<CClearCanvasDraw>(Set32BitColour(0.0f, 0, 0)), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);
		pFill = OMalloc<CFillContainer>(pWindow);
		pWindow->SetContainer(pFill);
		pTextArea = OMalloc<CTextArea>(pWindow, CreateTestTheme(Ptr<CFont>()), ReadLabelFont(), "abc def");
		pTextArea->SetMarkColours(Set32BitColour(1.0f, 0, 0), Set32BitColour(1.0f, 1.0f, 1.0f));
		pFill->AddComponent(pTextArea);
		pWindow->SetPartialRedraw(true);
		pWindow->SetVerifyRedraw(true);

		//As a release build, the window keeps its layout between paints rather than checking it by laying out again.
		pWindow->SetCheckLayoutCache(false);
		pWindow->Paint();

		//Above the glyphs of the first character, in the selection when it is selected.
		sPosition = pTextArea->GetPosition();
		uiUnselected = GetWindowPixel(pWindow, sPosition.x + 1, sPosition.y + 1);

		pTextArea->SelectRight();
		pTextArea->SelectRight();
		pWindow->Paint();
		AssertTrue(uiUnselected != GetWindowPixel(pWindow, sPosition.x + 1, sPosition.y + 1));
		AssertSize(0, pWindow->GetRedrawMismatches());

		pTextArea->ClearSelection();
		pWindow->Paint();
		AssertInt(uiUnselected, GetWindowPixel(pWindow, sPosition.x + 1, sPosition.y + 1));
		AssertSize(0, pWindow->GetRedrawMismatches());

		pTextArea->SelectDocumentHome();
		pWindow->Paint();
		AssertTrue(uiUnselected != GetWindowPixel(pWindow, sPosition.x + 1, sPosition.y + 1));
		AssertSize(0, pWindow->GetRedrawMismatches());

		//The edit, not from input, narrows the text: the text area is laid out to its new width.
		pTextArea->Printable('x', true);
		pWindow->Paint();
		AssertInt(uiUnselected, GetWindowPixel(pWindow, pTextArea->GetPosition().x + 1, sPosition.y + 1));
		AssertInt(pTextArea->GetTextSize().x, pTextArea->GetActualSize().x);
		AssertSize(0, pWindow->GetRedrawMismatches());

		pTextArea = NULL;
		pFill = NULL;
		ORoot()->Remove(pWindow);
		pWindow = NULL;
		pTick = NULL;
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
	TestPartialRedrawMatchesFull();
	TestPartialRedrawVerifyFindsStale();
	TestPartialRedrawDialogLikeOwner();
	TestPartialRedrawCanvasPart();
	TestPartialRedrawTextArea();

	TestStatistics();
}

