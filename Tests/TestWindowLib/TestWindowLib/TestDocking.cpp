#include <string.h>
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/XMLWriter.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/Canvas.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WindowLib/JewelTheme.h"
#include "WindowLib/TabbedContainer.h"
#include "WindowLib/DockManager.h"
#include "WindowLib/DockArea.h"
#include "WindowLib/DockSplit.h"
#include "WindowLib/DockGroup.h"
#include "WindowLib/DocumentWell.h"
#include "WindowLib/DockFloatingWindow.h"
#include "WindowLib/DockGuides.h"
#include "WindowLib/AutoHide.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"


Ptr<CFont> ReadLabelFont(void);
void ValidateLabelObjects(void);
Ptr<CCanvas> CreateTabbedPage(Ptr<CWindow> pWindow);


//Keeps what the dock manager told it.  A document named in mszKeep isn't let close.
class CTestDockListener : public CDockManagerListener
{
public:
	char			mszKeep[64];
	int				miClosing;
	int				miClosed;
	int				miActiveChanges;
	int				miLayoutChanges;
	CDockPane*		mpcActive;
	CDockPane*		mpcDocument;
	CDockManager*	mpcManager;
	Ptr<CWindow>*	mppWindow;

	void Init(CDockManager* pcManager)
	{
		mszKeep[0] = '\0';
		miClosing = 0;
		miClosed = 0;
		miActiveChanges = 0;
		miLayoutChanges = 0;
		mpcActive = NULL;
		mpcDocument = NULL;
		mpcManager = pcManager;
	}
	bool PaneClosing(CDockPane* pcPane) override
	{
		miClosing++;
		return strcmp(pcPane->GetID(), mszKeep) != 0;
	}
	void PaneClosed(CDockPane* pcPane) override
	{
		miClosed++;
	}
	void ActivePaneChanged(CDockPane* pcPane) override
	{
		miActiveChanges++;
		mpcActive = pcPane;
	}
	void DocumentSelected(CDockPane* pcPane) override
	{
		mpcDocument = pcPane;
	}
	Ptr<CDockPane> CreatePane(const char* szID) override
	{
		//Documents named in a layout are opened again.
		if (strncmp(szID, "Doc:", 4) == 0)
		{
			return mpcManager->AddPane(szID, szID + 4, NULL, CreateTabbedPage(mpcManager->GetWindow()), DPK_Document, NULL, false);
		}
		return NULL;
	}
	void LayoutChanged(void) override
	{
		miLayoutChanges++;
	}
};


//////////////////////////////////////////////////////////////////////////
//
// Presses and releases button one at x, y in pWindow, moving the pointer
// through the points in between.
//
//////////////////////////////////////////////////////////////////////////
static void DockPress(Ptr<CWindow> pWindow, int x, int y)
{
	pWindow->PointerMoved(x, y);
	pWindow->InputCommand(CIC_ActivateComponent, true);
}


static void DockMove(Ptr<CWindow> pWindow, int x, int y)
{
	pWindow->PointerMoved(x, y);
}


static void DockRelease(Ptr<CWindow> pWindow)
{
	pWindow->InputCommand(CIC_ExecuteComponent, true);
	pWindow->Layout();
}



//////////////////////////////////////////////////////////////////////////
//
// The middle of a guide shown over the main area, -1, -1 if there is none.
//
//////////////////////////////////////////////////////////////////////////
static SInt32Vec2 GuideCentre(Ptr<CDockArea> pArea, EDockTargetKind eKind, EDockSide eSide, CDockNode* pcNode, bool bOuter)
{
	Ptr<CDockGuides>	pGuides;
	SDockGuide*			psGuide;

	pGuides = pArea->GetGuides();
	if (pGuides.IsNull())
	{
		return SInt32Vec2(-1, -1);
	}
	psGuide = pGuides->FindGuide(eKind, eSide, pcNode, bOuter);
	if (psGuide == NULL)
	{
		return SInt32Vec2(-1, -1);
	}
	return SInt32Vec2(psGuide->cRect.GetLeft() + psGuide->cRect.GetWidth() / 2, psGuide->cRect.GetTop() + psGuide->cRect.GetHeight() / 2);
}


//////////////////////////////////////////////////////////////////////////
//
// A tabbed container's bar along the bottom, hidden with one tab; tabs
// taken without the removal event, inserted and moved.
//
//////////////////////////////////////////////////////////////////////////
void TestDockingTabbedContainer(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CWindowTheme>			pTheme;
		Ptr<CTabbedContainer>		pTabbed;
		Ptr<CCanvas>				pFirst;
		Ptr<CCanvas>				pSecond;
		Ptr<CCanvas>				pThird;
		Ptr<CTabComponent>			pFirstTab;
		Ptr<CTabComponent>			pSecondTab;
		Ptr<CTabComponent>			pThirdTab;
		Ptr<CComponent>				pTaken;
		int32						iBarHeight;

		cNativeFactory.Init(&gcMemoryAllocator, 200, 100, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Dock Tabs", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pTheme = CJewelTheme::CreateDarkTheme(pTestWindow, ReadLabelFont());

		pTabbed = OMalloc<CTabbedContainer>(pTestWindow, pTheme);
		pTabbed->SetTabSide(TS_Bottom);
		pTabbed->SetHideSingleTab(true);
		pTestWindow->SetContainer(pTabbed);
		pFirst = CreateTabbedPage(pTestWindow);
		pSecond = CreateTabbedPage(pTestWindow);
		pThird = CreateTabbedPage(pTestWindow);

		//One tab: no bar, the page takes everything.
		pFirstTab = pTabbed->AddTab("First", pFirst);
		pTestWindow->Layout();
		AssertFalse(pTabbed->IsBarShown());
		AssertInt(0, pFirst->GetPosition().y);
		AssertInt(100, pFirst->GetActualSize().y);
		AssertTrue(pFirstTab->GetParent().IsNotNull());

		//Two: the bar is along the bottom, under the page.
		pSecondTab = pTabbed->AddTab("Second", pSecond);
		pTestWindow->Layout();
		AssertTrue(pTabbed->IsBarShown());
		iBarHeight = pFirstTab->GetActualSize().y;
		AssertTrue(iBarHeight > 0);
		AssertInt(0, pFirst->GetPosition().y);
		AssertInt(100 - iBarHeight - 1, pFirst->GetActualSize().y);
		AssertInt(100 - iBarHeight, pFirstTab->GetPosition().y);

		//Inserted at the front.
		pThirdTab = pTabbed->InsertTab(0, "Third", pThird);
		pTestWindow->Layout();
		AssertSize(0, pTabbed->IndexOf(pThirdTab));
		AssertTrue(pThirdTab->GetPosition().x < pFirstTab->GetPosition().x);
		AssertTrue(pTabbed->GetSelected() == pFirstTab);

		//Moved to the end.
		AssertTrue(pTabbed->MoveTab(pThirdTab, 2));
		pTestWindow->Layout();
		AssertSize(2, pTabbed->IndexOf(pThirdTab));
		AssertTrue(pThirdTab->GetPosition().x > pSecondTab->GetPosition().x);
		AssertTrue(pTabbed->GetPage(2) == pThird);

		//Taken off: its page comes back, the next tab is selected.
		pTaken = pTabbed->TakeTab(pFirstTab);
		AssertTrue(pTaken == pFirst);
		AssertSize(2, pTabbed->NumTabs());
		AssertTrue(pTabbed->GetSelected() == pSecondTab);
		AssertTrue(pFirst->GetParent().IsNull());

		//Back on top.
		pTabbed->SetTabSide(TS_Top);
		pTestWindow->Layout();
		AssertInt(0, pSecondTab->GetPosition().y);
		AssertInt(iBarHeight + 1, pSecond->GetPosition().y);

		pFirstTab = NULL;
		pSecondTab = NULL;
		pThirdTab = NULL;
		pTaken = NULL;
		pFirst = NULL;
		pSecond = NULL;
		pThird = NULL;
		pTabbed = NULL;
		pTheme = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//Everything a docking test needs, made and let go by the tests.
struct SDockTest
{
	Ptr<CWindow>				pWindow;
	Ptr<CTickTestRefWindow>		pTick;
	SDataTestRefWindow			cData;
	Ptr<CWindowTheme>			pTheme;
	Ptr<CDockManager>			pManager;
	CTestDockListener			cListener;
	Ptr<CDockPane>				pExplorer;
	Ptr<CDockPane>				pProperties;
	Ptr<CDockPane>				pOutput;
	Ptr<CDockPane>				pErrors;
	Ptr<CDockPane>				pFirstDoc;
	Ptr<CDockPane>				pSecondDoc;

	void Init(CWinRefWindowFactory* pcFactory);
	void Kill(void);
};


//////////////////////////////////////////////////////////////////////////
//
// An 800 x 600 window: Explorer left of the well (200), Properties right
// (150), Output under the well (120) with Errors in its group, and two
// documents.
//
//////////////////////////////////////////////////////////////////////////
void SDockTest::Init(CWinRefWindowFactory* pcFactory)
{
	CPointer	pNull;
	SDockPlace	sPlace;

	pcFactory->Init(&gcMemoryAllocator, 800, 600, NULL);
	pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
	pWindow = OMalloc<CWindow>("Docking", pcFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
	ORoot()->Add(pWindow);
	pTheme = CJewelTheme::CreateDarkTheme(pWindow, ReadLabelFont());
	pManager = OMalloc<CDockManager>(pWindow, pTheme, Ptr<CFont>());
	ORoot()->Add(pManager);
	cListener.Init(&pManager);
	pManager->SetListener(&cListener);
	pWindow->SetContainer(pManager->GetArea());

	sPlace.BesideWell(DS_Left, 200);
	pExplorer = pManager->AddPane("Explorer", "Solution Explorer", NULL, CreateTabbedPage(pWindow), DPK_Tool, &sPlace);
	sPlace.BesideWell(DS_Right, 150);
	pProperties = pManager->AddPane("Properties", "Properties", NULL, CreateTabbedPage(pWindow), DPK_Tool, &sPlace);
	sPlace.BesideWell(DS_Bottom, 120);
	pOutput = pManager->AddPane("Output", "Output", NULL, CreateTabbedPage(pWindow), DPK_Tool, &sPlace);
	sPlace.WithPane("Output");
	pErrors = pManager->AddPane("Errors", "Error List", NULL, CreateTabbedPage(pWindow), DPK_Tool, &sPlace);
	pFirstDoc = pManager->AddPane("Doc:First.cpp", "First.cpp", NULL, CreateTabbedPage(pWindow), DPK_Document);
	pSecondDoc = pManager->AddPane("Doc:Second.cpp", "Second.cpp", NULL, CreateTabbedPage(pWindow), DPK_Document);
	pWindow->Layout();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SDockTest::Kill(void)
{
	pManager->SetListener(NULL);
	pExplorer = NULL;
	pProperties = NULL;
	pOutput = NULL;
	pErrors = NULL;
	pFirstDoc = NULL;
	pSecondDoc = NULL;
	ORoot()->Remove(pManager);
	pManager = NULL;
	pTheme = NULL;
	ORoot()->Remove(pWindow);
	pWindow = NULL;
	pTick = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Panes in their default places; the sash between Explorer and the well
// dragged; the window made wider, giving the width to the well.
//
//////////////////////////////////////////////////////////////////////////
void TestDockingDefaults(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		SDockTest				sTest;
		Ptr<CDockGroup>			pExplorerGroup;
		Ptr<CDockGroup>			pPropertiesGroup;
		Ptr<CDockGroup>			pOutputGroup;
		Ptr<CDockGroup>			pDocGroup;
		Ptr<CDockSplit>			pRoot;
		Ptr<CDockSash>			pSash;
		CInt32Rect				cBounds;
		CInt32Rect				cScreen;
		int32					iWellWidth;

		sTest.Init(&cNativeFactory);

		AssertInt(DPS_Docked, sTest.pExplorer->GetState());
		AssertInt(DPS_Docked, sTest.pFirstDoc->GetState());
		pExplorerGroup = sTest.pExplorer->GetGroup();
		pPropertiesGroup = sTest.pProperties->GetGroup();
		pOutputGroup = sTest.pOutput->GetGroup();
		pDocGroup = sTest.pFirstDoc->GetGroup();
		AssertTrue(pOutputGroup == sTest.pErrors->GetGroup());
		AssertTrue(pDocGroup == sTest.pSecondDoc->GetGroup());
		AssertSize(2, pOutputGroup->NumPanes());
		AssertFalse(pExplorerGroup->IsDocumentGroup());
		AssertTrue(pDocGroup->IsDocumentGroup());
		AssertTrue(pDocGroup->IsWithin(sTest.pManager->GetWell()));
		AssertTrue(sTest.pManager->GetActiveDocument() == sTest.pSecondDoc);

		//Explorer, the well (over Output) and Properties across the window.
		pRoot = Ptr<CDockSplit>(dynamic_cast<CDockSplit*>(&sTest.pManager->GetArea()->GetRoot()));
		AssertTrue(pRoot.IsNotNull());
		AssertTrue(pRoot->IsHorizontal());
		AssertSize(3, pRoot->NumNodes());
		AssertTrue(pRoot->GetNode(0) == pExplorerGroup);
		AssertTrue(pRoot->GetNode(2) == pPropertiesGroup);
		AssertTrue(pRoot->GetNode(1)->IsSplit());
		pExplorerGroup->GetActualBounds(&cBounds);
		AssertInt(0, cBounds.GetLeft());
		AssertInt(200, cBounds.GetWidth());
		AssertInt(600, cBounds.GetHeight());
		pPropertiesGroup->GetActualBounds(&cBounds);
		AssertInt(800 - 150, cBounds.GetLeft());
		AssertInt(150, cBounds.GetWidth());
		pOutputGroup->GetActualBounds(&cBounds);
		AssertInt(120, cBounds.GetHeight());
		AssertInt(600 - 120, cBounds.GetTop());
		AssertInt(200 + DOCK_SASH_SIZE, cBounds.GetLeft());
		sTest.pManager->GetWell()->GetActualBounds(&cBounds);
		AssertInt(800 - 200 - 150 - 2 * DOCK_SASH_SIZE, cBounds.GetWidth());
		AssertInt(600 - 120 - DOCK_SASH_SIZE, cBounds.GetHeight());

		//A tool group has its title bar; its tabs show with two panes, along the bottom.
		AssertTrue(pExplorerGroup->GetTitleBar().IsNotNull());
		AssertString("Solution Explorer", pExplorerGroup->GetTitleBar()->GetTitle());
		AssertFalse(pExplorerGroup->GetTabs()->IsBarShown());
		AssertTrue(pOutputGroup->GetTabs()->IsBarShown());
		AssertInt(TS_Bottom, pOutputGroup->GetTabs()->GetTabSide());
		AssertInt(TS_Top, pDocGroup->GetTabs()->GetTabSide());
		AssertTrue(pDocGroup->GetTitleBar().IsNull());

		//The sash right of Explorer dragged 30 pixels right.
		pSash = pRoot->GetSash(0);
		DockPress(sTest.pWindow, pSash->GetPosition().x + 1, 300);
		DockMove(sTest.pWindow, pSash->GetPosition().x + 11, 300);
		DockMove(sTest.pWindow, pSash->GetPosition().x + 31, 300);
		DockRelease(sTest.pWindow);
		pExplorerGroup->GetActualBounds(&cBounds);
		AssertInt(230, cBounds.GetWidth());
		pPropertiesGroup->GetActualBounds(&cBounds);
		AssertInt(150, cBounds.GetWidth());
		AssertInt(NC_SizeWestEast, pSash->GetCursor());

		//A wider window: the well takes it, the tools keep their widths.
		sTest.pManager->GetWell()->GetActualBounds(&cBounds);
		iWellWidth = cBounds.GetWidth();
		cScreen.Init(0, 0, 1000, 600);
		sTest.pWindow->SetScreenRectangle(&cScreen);
		sTest.pWindow->Layout();
		pExplorerGroup->GetActualBounds(&cBounds);
		AssertInt(230, cBounds.GetWidth());
		pPropertiesGroup->GetActualBounds(&cBounds);
		AssertInt(150, cBounds.GetWidth());
		AssertInt(1000 - 150, cBounds.GetLeft());
		sTest.pManager->GetWell()->GetActualBounds(&cBounds);
		AssertInt(iWellWidth + 200, cBounds.GetWidth());

		pExplorerGroup = NULL;
		pPropertiesGroup = NULL;
		pOutputGroup = NULL;
		pDocGroup = NULL;
		pRoot = NULL;
		pSash = NULL;
		sTest.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Hidden panes come back where they were; floated panes go into windows
// of their own and dock back; auto hidden panes slide out; documents
// close if the listener lets them.
//
//////////////////////////////////////////////////////////////////////////
void TestDockingHideFloatAutoHide(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		SDockTest					sTest;
		Ptr<CDockGroup>				pOutputGroup;
		Ptr<CDockFloatingWindow>	pFloating;
		Ptr<CAutoHideStrip>			pStrip;
		CInt32Rect					cBounds;
		CInt32Rect					cRect;

		sTest.Init(&cNativeFactory);
		pOutputGroup = sTest.pOutput->GetGroup();

		//Explorer hidden: the well takes its place.  Shown, it goes back on the left at its width.
		sTest.pManager->Hide(sTest.pExplorer);
		sTest.pWindow->Layout();
		AssertInt(DPS_Hidden, sTest.pExplorer->GetState());
		AssertTrue(sTest.pExplorer->GetGroup().IsNull());
		AssertTrue(sTest.pExplorer->GetContent()->GetParent().IsNull());
		sTest.pManager->GetWell()->GetActualBounds(&cBounds);
		AssertInt(0, cBounds.GetLeft());
		AssertString(DOCK_WELL_NEIGHBOUR, sTest.pExplorer->GetRestoreNeighbour());
		AssertInt(DS_Left, sTest.pExplorer->GetRestoreSide());
		sTest.pManager->Show(sTest.pExplorer);
		sTest.pWindow->Layout();
		AssertInt(DPS_Docked, sTest.pExplorer->GetState());
		sTest.pExplorer->GetGroup()->GetActualBounds(&cBounds);
		AssertInt(0, cBounds.GetLeft());
		AssertInt(200, cBounds.GetWidth());

		//Errors hidden from Output's group goes back into it, where it was.
		sTest.pManager->Hide(sTest.pErrors);
		AssertSize(1, pOutputGroup->NumPanes());
		AssertString("Output", sTest.pErrors->GetRestoreNeighbour());
		AssertInt(DS_Centre, sTest.pErrors->GetRestoreSide());
		sTest.pManager->Show(sTest.pErrors);
		AssertTrue(sTest.pErrors->GetGroup() == pOutputGroup);
		AssertSize(1, pOutputGroup->IndexOfPane(sTest.pErrors));

		//Properties floats in a window of its own, with its content.
		cRect.Init(300, 200, 500, 400);
		sTest.pManager->Float(sTest.pProperties, &cRect);
		sTest.pWindow->Layout();
		AssertInt(DPS_Floating, sTest.pProperties->GetState());
		AssertSize(1, sTest.pManager->NumFloating());
		pFloating = sTest.pManager->GetFloating(0);
		AssertTrue(pFloating->IsShown());
		AssertSize(1, sTest.pWindow->NumOwned());
		AssertTrue(sTest.pProperties->GetContent()->GetWindow() == pFloating->GetWindow());
		AssertTrue(sTest.pProperties->GetGroup()->GetWindow() == pFloating->GetWindow());
		AssertTrue(sTest.pManager->FindFloating(sTest.pProperties) == pFloating);
		AssertTrue(pFloating->IsSingleToolGroup());
		pFloating->GetWindow()->GetScreenRectangle(&cBounds);
		AssertInt(300, cBounds.GetLeft());
		AssertInt(200, cBounds.GetWidth());
		pFloating->GetWindow()->Layout();
		sTest.pProperties->GetContent()->GetActualBounds(&cBounds);
		AssertTrue(cBounds.GetWidth() > 0);
		sTest.pManager->GetWell()->GetActualBounds(&cBounds);
		AssertInt(800 - 200 - DOCK_SASH_SIZE, cBounds.GetWidth());

		//Docked back on the right; the window goes.
		sTest.pManager->Dock(sTest.pProperties);
		sTest.pWindow->Layout();
		AssertInt(DPS_Docked, sTest.pProperties->GetState());
		AssertSize(0, sTest.pManager->NumFloating());
		AssertSize(0, sTest.pWindow->NumOwned());
		AssertTrue(sTest.pProperties->GetContent()->GetWindow() == sTest.pWindow);
		sTest.pProperties->GetGroup()->GetActualBounds(&cBounds);
		AssertInt(800 - 150, cBounds.GetLeft());
		AssertFalse(pFloating->IsShown());
		pFloating = NULL;

		//Output's whole group auto hides on the bottom edge.
		sTest.pManager->AutoHideGroup(pOutputGroup);
		sTest.pWindow->Layout();
		AssertInt(DPS_AutoHidden, sTest.pOutput->GetState());
		AssertInt(DPS_AutoHidden, sTest.pErrors->GetState());
		AssertInt(DS_Bottom, sTest.pOutput->GetAutoHideSide());
		AssertInt(120, sTest.pOutput->GetAutoHideSize());
		pStrip = sTest.pManager->GetArea()->GetStrip(DS_Bottom);
		AssertSize(2, pStrip->NumTabs());
		pStrip->GetActualBounds(&cBounds);
		AssertInt(600 - AUTO_HIDE_STRIP_SIZE, cBounds.GetTop());
		sTest.pManager->GetWell()->GetActualBounds(&cBounds);
		AssertInt(600 - AUTO_HIDE_STRIP_SIZE, cBounds.GetHeight());

		//Pressing its tab slides it out over the well; pressing again slides it back.
		pStrip->GetTab(0)->GetActualBounds(&cBounds);
		DockPress(sTest.pWindow, cBounds.GetLeft() + 3, cBounds.GetTop() + 3);
		DockRelease(sTest.pWindow);
		AssertTrue(sTest.pManager->GetFlyout().IsNotNull());
		AssertTrue(sTest.pOutput->GetContent()->GetParent() == sTest.pManager->GetFlyout());
		sTest.pManager->GetFlyout()->GetActualBounds(&cBounds);
		AssertInt(120, cBounds.GetHeight());
		AssertInt(600 - AUTO_HIDE_STRIP_SIZE - 120, cBounds.GetTop());
		AssertTrue(sTest.pManager->GetActivePane() == sTest.pOutput);
		pStrip->GetTab(0)->GetActualBounds(&cBounds);
		DockPress(sTest.pWindow, cBounds.GetLeft() + 3, cBounds.GetTop() + 3);
		DockRelease(sTest.pWindow);
		AssertTrue(sTest.pManager->GetFlyout().IsNull());
		AssertTrue(sTest.pOutput->GetContent()->GetParent().IsNull());

		//The flyout's pin docks Output back where its group was; Errors follows it into the same group.
		sTest.pManager->ShowFlyout(sTest.pOutput);
		sTest.pWindow->Layout();
		sTest.pManager->GetFlyout()->GetTitleBar()->GetButton(DTB_Pin)->GetActualBounds(&cBounds);
		DockPress(sTest.pWindow, cBounds.GetLeft() + 5, cBounds.GetTop() + 5);
		DockRelease(sTest.pWindow);
		AssertInt(DPS_Docked, sTest.pOutput->GetState());
		AssertTrue(sTest.pManager->GetFlyout().IsNull());
		sTest.pManager->Dock(sTest.pErrors);
		AssertTrue(sTest.pErrors->GetGroup() == sTest.pOutput->GetGroup());
		AssertSize(0, pStrip->NumTabs());
		sTest.pWindow->Layout();
		sTest.pOutput->GetGroup()->GetActualBounds(&cBounds);
		AssertInt(120, cBounds.GetHeight());
		AssertInt(600 - 120, cBounds.GetTop());

		//Dock as Tabbed Document puts a tool in the well.
		sTest.pManager->DockAsDocument(sTest.pExplorer);
		AssertTrue(sTest.pExplorer->GetGroup() == sTest.pFirstDoc->GetGroup());

		//A document closes only if the listener lets it.
		strcpy(sTest.cListener.mszKeep, "Doc:First.cpp");
		AssertFalse(sTest.pManager->Close(sTest.pFirstDoc));
		AssertInt(DPS_Docked, sTest.pFirstDoc->GetState());
		AssertInt(1, sTest.cListener.miClosing);
		AssertTrue(sTest.pManager->Close(sTest.pSecondDoc));
		AssertInt(DPS_Removed, sTest.pSecondDoc->GetState());
		AssertTrue(sTest.pManager->GetPane("Doc:Second.cpp").IsNull());
		AssertInt(1, sTest.cListener.miClosed);

		//Reset puts everything in its default place.
		sTest.pManager->ResetLayout();
		sTest.pWindow->Layout();
		sTest.pExplorer->GetGroup()->GetActualBounds(&cBounds);
		AssertInt(0, cBounds.GetLeft());
		AssertInt(200, cBounds.GetWidth());
		AssertFalse(sTest.pExplorer->GetGroup()->IsDocumentGroup());

		pOutputGroup = NULL;
		pStrip = NULL;
		sTest.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Dragging tabs and title bars: torn off into floating windows, dropped
// on the guides, and put back by Escape.
//
//////////////////////////////////////////////////////////////////////////
void TestDockingDrag(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		SDockTest					sTest;
		Ptr<CDockGroup>				pOutputGroup;
		Ptr<CDockGroup>				pPropertiesGroup;
		Ptr<CDockGroup>				pDocGroup;
		Ptr<CTabComponent>			pTab;
		Ptr<CDockSplit>				pRoot;
		Ptr<CDockArea>				pArea;
		CInt32Rect					cBounds;
		SInt32Vec2					sGuide;
		SInt32Vec2					sStart;
		Ptr<CDockFloatingWindow>	pFloating;
		Ptr<CComponent>				pDocParent;

		sTest.Init(&cNativeFactory);
		pArea = sTest.pManager->GetArea();
		pOutputGroup = sTest.pOutput->GetGroup();

		//Errors' tab pulled off Output's group: it floats under the pointer and the guides show.
		pTab = pOutputGroup->GetTab(sTest.pErrors);
		pTab->GetActualBounds(&cBounds);
		sStart.Init(cBounds.GetLeft() + 5, cBounds.GetTop() + 5);
		DockPress(sTest.pWindow, sStart.x, sStart.y);
		DockMove(sTest.pWindow, sStart.x + 2, sStart.y - 2);
		AssertTrue(sTest.pManager->IsDragging() == false);
		DockMove(sTest.pWindow, sStart.x + 10, sStart.y - 10);
		AssertTrue(sTest.pManager->IsDragging());
		AssertInt(DPS_Floating, sTest.pErrors->GetState());
		AssertSize(1, sTest.pManager->NumFloating());
		AssertSize(1, pOutputGroup->NumPanes());
		AssertTrue(pArea->GetGuides().IsNotNull());

		//Over the left edge's marker the preview is the left of the area.
		sTest.pWindow->Layout();
		sGuide = GuideCentre(pArea, DTK_AreaEdge, DS_Left, NULL, false);
		AssertTrue(sGuide.x >= 0);
		DockMove(sTest.pWindow, sGuide.x, sGuide.y);
		AssertInt(DTK_AreaEdge, sTest.pManager->GetTarget()->eKind);
		AssertInt(DS_Left, sTest.pManager->GetTarget()->eSide);
		AssertInt(0, sTest.pManager->GetTarget()->cPreview.GetLeft());

		//Dropped: docked along the whole left edge.
		DockRelease(sTest.pWindow);
		AssertFalse(sTest.pManager->IsDragging());
		AssertInt(DPS_Docked, sTest.pErrors->GetState());
		AssertSize(0, sTest.pManager->NumFloating());
		AssertTrue(pArea->GetGuides().IsNull());
		pRoot = Ptr<CDockSplit>(dynamic_cast<CDockSplit*>(&pArea->GetRoot()));
		AssertTrue(pRoot->IsHorizontal());
		AssertTrue(pRoot->GetNode(0) == sTest.pErrors->GetGroup());
		AssertTrue(sTest.pErrors->GetContent()->GetWindow() == sTest.pWindow);
		sTest.pErrors->GetGroup()->GetActualBounds(&cBounds);
		AssertInt(0, cBounds.GetLeft());
		AssertInt(600, cBounds.GetHeight());
		AssertTrue(sTest.pManager->GetActivePane() == sTest.pErrors);

		//Explorer's title bar pulled onto the middle of Properties' compass: tabbed into its group.
		pPropertiesGroup = sTest.pProperties->GetGroup();
		sTest.pExplorer->GetGroup()->GetTitleBar()->GetActualBounds(&cBounds);
		sStart.Init(cBounds.GetLeft() + 20, cBounds.GetTop() + 5);
		DockPress(sTest.pWindow, sStart.x, sStart.y);
		DockMove(sTest.pWindow, sStart.x + 10, sStart.y + 10);
		AssertTrue(sTest.pManager->IsDragging());
		pPropertiesGroup->GetActualBounds(&cBounds);
		DockMove(sTest.pWindow, cBounds.GetLeft() + cBounds.GetWidth() / 2, cBounds.GetTop() + 50);
		sGuide = GuideCentre(pArea, DTK_Group, DS_Centre, &pPropertiesGroup, false);
		AssertTrue(sGuide.x >= 0);
		DockMove(sTest.pWindow, sGuide.x, sGuide.y);
		AssertInt(DTK_Group, sTest.pManager->GetTarget()->eKind);
		DockRelease(sTest.pWindow);
		AssertTrue(sTest.pExplorer->GetGroup() == pPropertiesGroup);
		AssertSize(2, pPropertiesGroup->NumPanes());
		AssertSize(0, sTest.pManager->NumFloating());

		//A document dragged onto the inner right arrow of the well's compass: a new tab group to the right.  The well
		//is too narrow for both tabs: the selected one is brought to the front.
		sTest.pManager->Show(sTest.pFirstDoc);
		sTest.pWindow->Layout();
		pDocGroup = sTest.pFirstDoc->GetGroup();
		AssertSize(1, pDocGroup->GetTabs()->NumShownTabs());
		AssertSize(0, pDocGroup->IndexOfPane(sTest.pFirstDoc));
		pTab = pDocGroup->GetTab(sTest.pFirstDoc);
		pTab->GetActualBounds(&cBounds);
		sStart.Init(cBounds.GetLeft() + 5, cBounds.GetTop() + 5);
		DockPress(sTest.pWindow, sStart.x, sStart.y);
		AssertTrue(pTab->IsMouseOver());
		AssertInt(FD_Select, sTest.pWindow->GetFocus()->GetDrag());
		DockMove(sTest.pWindow, sStart.x + 10, sStart.y + 10);
		AssertTrue(sTest.pManager->IsDragging());
		pDocGroup->GetActualBounds(&cBounds);
		DockMove(sTest.pWindow, cBounds.GetLeft() + cBounds.GetWidth() / 2, cBounds.GetTop() + cBounds.GetHeight() / 2);
		AssertTrue(GuideCentre(pArea, DTK_AreaEdge, DS_Left, NULL, false).x < 0);
		AssertTrue(GuideCentre(pArea, DTK_WellOuter, DS_Left, NULL, true).x < 0);
		sGuide = GuideCentre(pArea, DTK_Group, DS_Right, &pDocGroup, false);
		AssertTrue(sGuide.x >= 0);
		DockMove(sTest.pWindow, sGuide.x, sGuide.y);
		DockRelease(sTest.pWindow);
		AssertTrue(sTest.pFirstDoc->GetGroup() != pDocGroup);
		AssertTrue(sTest.pFirstDoc->GetGroup()->IsDocumentGroup());
		AssertTrue(sTest.pFirstDoc->GetGroup()->IsWithin(sTest.pManager->GetWell()));
		pDocParent = sTest.pFirstDoc->GetGroup()->GetParent();
		AssertTrue(dynamic_cast<CDockSplit*>(&pDocParent) != NULL);
		AssertTrue(((CDockSplit*)&pDocParent)->IsHorizontal());
		AssertTrue(sTest.pFirstDoc->GetGroup()->GetPosition().x > pDocGroup->GetPosition().x);

		//Escape puts a torn off tab back.
		pTab = pPropertiesGroup->GetTab(sTest.pExplorer);
		pTab->GetActualBounds(&cBounds);
		sStart.Init(cBounds.GetLeft() + 5, cBounds.GetTop() + 5);
		DockPress(sTest.pWindow, sStart.x, sStart.y);
		DockMove(sTest.pWindow, sStart.x - 100, sStart.y - 100);
		AssertTrue(sTest.pManager->IsDragging());
		AssertInt(DPS_Floating, sTest.pExplorer->GetState());
		sTest.pWindow->InputCommand(CIC_ClearSelection, true);
		AssertFalse(sTest.pManager->IsDragging());
		AssertTrue(sTest.pExplorer->GetGroup() == pPropertiesGroup);
		AssertSize(0, sTest.pManager->NumFloating());
		sTest.pWindow->Layout();

		//Dropped away from the guides it stays floating where it was let go.
		pTab = pPropertiesGroup->GetTab(sTest.pExplorer);
		pTab->GetActualBounds(&cBounds);
		sStart.Init(cBounds.GetLeft() + 5, cBounds.GetTop() + 5);
		DockPress(sTest.pWindow, sStart.x, sStart.y);
		DockMove(sTest.pWindow, sStart.x - 300, 20);
		AssertFalse(sTest.pManager->GetTarget()->IsValid());
		DockRelease(sTest.pWindow);
		AssertInt(DPS_Floating, sTest.pExplorer->GetState());
		AssertSize(1, sTest.pManager->NumFloating());
		pFloating = sTest.pManager->GetFloating(0);
		pFloating->GetRectangle(&cBounds);
		AssertInt(20 - DOCK_FRAME_BORDER - DOCK_TITLE_BAR_HEIGHT / 2, cBounds.GetTop());

		//Its title bar moves it.
		pFloating->GetWindow()->Layout();
		AssertTrue(pFloating->IsSingleToolGroup());
		sTest.pExplorer->GetGroup()->GetTitleBar()->GetActualBounds(&cBounds);
		pFloating->GetWindow()->PointerMoved(cBounds.GetLeft() + 20, cBounds.GetTop() + 5);
		pFloating->GetWindow()->InputCommand(CIC_ActivateComponent, true);
		pFloating->GetWindow()->PointerMoved(cBounds.GetLeft() + 60, cBounds.GetTop() + 45);
		AssertTrue(sTest.pManager->IsDragging());
		pFloating->GetWindow()->PointerMoved(cBounds.GetLeft() + 100, cBounds.GetTop() + 85);
		pFloating->GetWindow()->InputCommand(CIC_ExecuteComponent, true);
		AssertFalse(sTest.pManager->IsDragging());
		AssertSize(1, sTest.pManager->NumFloating());
		pFloating->GetRectangle(&cBounds);
		AssertInt(20 - DOCK_FRAME_BORDER - DOCK_TITLE_BAR_HEIGHT / 2 + 40, cBounds.GetTop());

		//Double clicking its title bar docks it back.
		sTest.pManager->ToggleFloatGroup(sTest.pExplorer->GetGroup());
		AssertInt(DPS_Docked, sTest.pExplorer->GetState());
		AssertTrue(sTest.pExplorer->GetGroup() == pPropertiesGroup);
		AssertSize(0, sTest.pManager->NumFloating());

		pOutputGroup = NULL;
		pPropertiesGroup = NULL;
		pDocGroup = NULL;
		pTab = NULL;
		pRoot = NULL;
		pArea = NULL;
		pFloating = NULL;
		pDocParent = NULL;
		sTest.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A pane's content moves to the floating window with everything of it,
// tab pages not shown included.
//
//////////////////////////////////////////////////////////////////////////
void TestDockingMoveToWindow(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		SDockTest					sTest;
		Ptr<CTabbedContainer>		pTabbed;
		Ptr<CCanvas>				pShown;
		Ptr<CCanvas>				pNotShown;
		Ptr<CDockPane>				pPane;
		Ptr<CWindow>				pFloatingWindow;
		CInt32Rect					cRect;

		sTest.Init(&cNativeFactory);
		pTabbed = OMalloc<CTabbedContainer>(sTest.pWindow, sTest.pTheme);
		pShown = CreateTabbedPage(sTest.pWindow);
		pNotShown = CreateTabbedPage(sTest.pWindow);
		pTabbed->AddTab("Shown", pShown);
		pTabbed->AddTab("Not Shown", pNotShown);
		pPane = sTest.pManager->AddPane("Nested", "Nested", NULL, pTabbed, DPK_Tool);
		AssertTrue(pNotShown->GetParent().IsNull());

		cRect.Init(100, 100, 400, 400);
		sTest.pManager->Float(pPane, &cRect);
		pFloatingWindow = sTest.pManager->GetFloating(0)->GetWindow();
		AssertTrue(pTabbed->GetWindow() == pFloatingWindow);
		AssertTrue(pShown->GetWindow() == pFloatingWindow);
		AssertTrue(pNotShown->GetWindow() == pFloatingWindow);
		AssertTrue(pTabbed->GetTab(1)->GetWindow() == pFloatingWindow);
		AssertTrue(sTest.pExplorer->GetContent()->GetWindow() == sTest.pWindow);
		AssertTrue(sTest.pManager->GetArea()->GetWindow() == sTest.pWindow);

		sTest.pManager->Dock(pPane);
		AssertTrue(pNotShown->GetWindow() == sTest.pWindow);
		AssertTrue(pShown->GetWindow() == sTest.pWindow);

		pTabbed = NULL;
		pShown = NULL;
		pNotShown = NULL;
		pPane = NULL;
		pFloatingWindow = NULL;
		sTest.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The layout written, read back and written again is the same, with a
// floating window, auto hidden and hidden panes; documents named in it
// are made by the listener.
//
//////////////////////////////////////////////////////////////////////////
void TestDockingLayoutFile(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;

	cFileUtil.MakeDir("Output" _FS_ "Docking");
	{
		SDockTest				sTest;
		CXMLWriter				cFirst;
		CXMLWriter				cSecond;
		CInt32Rect				cRect;
		CInt32Rect				cBounds;
		Ptr<CDockPane>			pThird;

		sTest.Init(&cNativeFactory);
		pThird = sTest.pManager->AddPane("Doc:Third.cpp", "Third.cpp", NULL, CreateTabbedPage(sTest.pWindow), DPK_Document);
		cRect.Init(500, 100, 760, 400);
		sTest.pManager->Float(sTest.pProperties, &cRect);
		sTest.pManager->AutoHide(sTest.pErrors);
		sTest.pManager->Hide(sTest.pOutput);
		sTest.pWindow->Layout();

		cFirst.Init();
		sTest.pManager->WriteLayout(&cFirst);
		AssertTrue(sTest.pManager->SaveLayout("Output" _FS_ "Docking" _FS_ "Layout.xml"));

		//Documents go; the layout brings them back through the listener.
		sTest.pManager->RemovePane(pThird);
		sTest.pManager->RemovePane(sTest.pSecondDoc);
		sTest.pManager->ResetLayout();
		AssertSize(0, sTest.pManager->NumFloating());
		AssertTrue(sTest.pManager->GetPane("Doc:Third.cpp").IsNull());

		AssertTrue(sTest.pManager->LoadLayout("Output" _FS_ "Docking" _FS_ "Layout.xml"));
		sTest.pWindow->Layout();
		AssertTrue(sTest.pManager->GetPane("Doc:Third.cpp").IsNotNull());
		AssertTrue(sTest.pManager->GetPane("Doc:Second.cpp").IsNotNull());
		AssertInt(DPS_Floating, sTest.pProperties->GetState());
		AssertInt(DPS_AutoHidden, sTest.pErrors->GetState());
		AssertInt(DPS_Hidden, sTest.pOutput->GetState());
		AssertSize(1, sTest.pManager->NumFloating());
		sTest.pManager->GetFloating(0)->GetRectangle(&cBounds);
		AssertInt(500, cBounds.GetLeft());
		AssertInt(300, cBounds.GetHeight());
		sTest.pExplorer->GetGroup()->GetActualBounds(&cBounds);
		AssertInt(200, cBounds.GetWidth());

		cSecond.Init();
		sTest.pManager->WriteLayout(&cSecond);
		AssertString(cFirst.GetText(), cSecond.GetText());

		//Output comes back where it was hidden from.
		sTest.pManager->Show(sTest.pOutput);
		AssertInt(DPS_Docked, sTest.pOutput->GetState());

		//A file that isn't a layout changes nothing.
		AssertFalse(sTest.pManager->LoadLayout("Output" _FS_ "Docking" _FS_ "Missing.xml"));
		AssertInt(DPS_Floating, sTest.pProperties->GetState());

		cSecond.Kill();
		cFirst.Kill();
		pThird = NULL;
		sTest.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDocking(void)
{
	BeginTests();
	ObjectsInit();

	TestDockingTabbedContainer();
	TestDockingDefaults();
	TestDockingHideFloatAutoHide();
	TestDockingDrag();
	TestDockingMoveToWindow();
	TestDockingLayoutFile();
	ValidateLabelObjects();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

