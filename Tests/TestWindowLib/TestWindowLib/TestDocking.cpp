#include <string.h>
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/XMLWriter.h"
#include "BaseLib/TextFile.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/ComponentInput.h"
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
#include "WindowLib/AutoHideFlyout.h"
#include "WindowLib/MenuContainer.h"
#include "WindowLib/MenuItem.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"
#include "TickTestRefWindow.h"
#include "ScriptFramesTick.h"
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

	void Init(CWinRefWindowFactory* pcFactory, char* szImagePath = NULL);
	void Kill(void);
};


//////////////////////////////////////////////////////////////////////////
//
// An 800 x 600 window: Explorer left of the well (200), Properties right
// (150), Output under the well (120) with Errors in its group, and two
// documents.
//
//////////////////////////////////////////////////////////////////////////
void SDockTest::Init(CWinRefWindowFactory* pcFactory, char* szImagePath)
{
	CPointer	pNull;
	SDockPlace	sPlace;

	pcFactory->Init(&gcMemoryAllocator, 800, 600, szImagePath);
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
		AssertString(DOCK_WELL_NEIGHBOUR, sTest.pExplorer->GetRestore()->szNeighbour.Text());
		AssertInt(DS_Left, sTest.pExplorer->GetRestore()->eSide);
		sTest.pManager->Show(sTest.pExplorer);
		sTest.pWindow->Layout();
		AssertInt(DPS_Docked, sTest.pExplorer->GetState());
		sTest.pExplorer->GetGroup()->GetActualBounds(&cBounds);
		AssertInt(0, cBounds.GetLeft());
		AssertInt(200, cBounds.GetWidth());

		//Errors hidden from Output's group goes back into it, where it was.
		sTest.pManager->Hide(sTest.pErrors);
		AssertSize(1, pOutputGroup->NumPanes());
		AssertString("Output", sTest.pErrors->GetRestore()->szNeighbour.Text());
		AssertInt(DS_Centre, sTest.pErrors->GetRestore()->eSide);
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
// A tool group too narrow for its tabs: they keep their widths, side by
// side from its left, and the ones past its right edge are cut off.  A
// press past the edge, on the sash beside it, doesn't reach them.
//
//////////////////////////////////////////////////////////////////////////
void TestDockingNarrowToolTabs(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		SDockTest				sTest;
		SDockPlace				sPlace;
		Ptr<CDockGroup>			pGroup;
		Ptr<CTabbedContainer>	pTabs;
		Ptr<CTabComponent>		pTab;
		Ptr<CTabComponent>		pPrevious;
		Ptr<CTabComponent>		pSelected;
		Ptr<CTabComponent>		pCutOff;
		Ptr<CComponent>			pOver;
		CInt32Rect				cScreen;
		CInt32Rect				cGroup;
		CInt32Rect				cTab;
		CInt32Rect				cPrevious;
		int32					iGroupRight;
		int32					y;
		size					i;

		sTest.Init(&cNativeFactory);
		sPlace.WithPane("Output");
		sTest.pManager->AddPane("Find", "Find Results", NULL, CreateTabbedPage(sTest.pWindow), DPK_Tool, &sPlace);
		sTest.pManager->AddPane("Watch", "Watch Window", NULL, CreateTabbedPage(sTest.pWindow), DPK_Tool, &sPlace);
		pGroup = sTest.pOutput->GetGroup();
		pTabs = pGroup->GetTabs();
		AssertSize(4, pTabs->NumTabs());
		pTabs->Select((size)0);

		//The window narrowed: the well, with Output's group under it, is left 160 pixels.
		cScreen.Init(0, 0, 520, 600);
		sTest.pWindow->SetScreenRectangle(&cScreen);
		sTest.pWindow->Layout();
		pGroup->GetActualBounds(&cGroup);
		AssertInt(520 - 200 - 150 - 2 * DOCK_SASH_SIZE, cGroup.GetWidth());
		iGroupRight = cGroup.GetLeft() + cGroup.GetWidth();
		AssertTrue(pTabs->IsBarShown());

		//The tabs keep their widths and don't overlap, the first at the group's left.
		pPrevious = NULL;
		for (i = 0; i < pTabs->NumTabs(); i++)
		{
			pTab = pTabs->GetTab(i);
			pTab->GetActualBounds(&cTab);
			AssertInt(pTab->GetDesiredSize().x, cTab.GetWidth());
			if (pPrevious.IsNull())
			{
				AssertInt(cGroup.GetLeft(), cTab.GetLeft());
			}
			else
			{
				pPrevious->GetActualBounds(&cPrevious);
				AssertInt(cPrevious.GetLeft() + cPrevious.GetWidth(), cTab.GetLeft());
			}
			if ((pCutOff.IsNull()) && (cTab.GetLeft() + cTab.GetWidth() > iGroupRight))
			{
				pCutOff = pTab;
			}
			pPrevious = pTab;
		}

		//Some tab runs past the group's right edge.
		AssertTrue(pCutOff.IsNotNull());
		AssertTrue(pCutOff != pTabs->GetSelected());
		pCutOff->GetActualBounds(&cTab);
		y = cTab.GetTop() + cTab.GetHeight() / 2;

		//A press just past the edge is on the sash, not the cut off tab, which stays unselected.
		pSelected = pTabs->GetSelected();
		DockPress(sTest.pWindow, iGroupRight + 1, y);
		pOver = sTest.pWindow->GetFocus()->GetMouseOverComponent();
		AssertTrue(pOver != pCutOff);
		AssertTrue(dynamic_cast<CDockSash*>(&pOver) != NULL);
		DockRelease(sTest.pWindow);
		AssertTrue(pTabs->GetSelected() == pSelected);
		AssertFalse(pCutOff->IsSelected());

		//A press on the part of it that shows still selects it.
		if (cTab.GetLeft() < iGroupRight - 2)
		{
			DockPress(sTest.pWindow, iGroupRight - 2, y);
			AssertTrue(sTest.pWindow->GetFocus()->GetMouseOverComponent() == pCutOff);
			DockRelease(sTest.pWindow);
			AssertTrue(pTabs->GetSelected() == pCutOff);
		}

		pGroup = NULL;
		pTabs = NULL;
		pTab = NULL;
		pPrevious = NULL;
		pSelected = NULL;
		pCutOff = NULL;
		pOver = NULL;
		sTest.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// True if no split in the tree under pNode has a split of its own
// direction as a node.
//
//////////////////////////////////////////////////////////////////////////
static bool IsFlat(Ptr<CDockNode> pNode)
{
	CDockSplit*		pcSplit;
	CDocumentWell*	pcWell;
	Ptr<CDockNode>	pChild;
	CDockSplit*		pcChildSplit;
	size			i;

	pcWell = dynamic_cast<CDocumentWell*>(&pNode);
	if (pcWell)
	{
		return pcWell->GetRoot().IsNull() || IsFlat(pcWell->GetRoot());
	}
	pcSplit = dynamic_cast<CDockSplit*>(&pNode);
	if (pcSplit == NULL)
	{
		return true;
	}
	for (i = 0; i < pcSplit->NumNodes(); i++)
	{
		pChild = pcSplit->GetNode(i);
		pcChildSplit = dynamic_cast<CDockSplit*>(&pChild);
		if (pcChildSplit && (pcChildSplit->IsHorizontal() == pcSplit->IsHorizontal()))
		{
			return false;
		}
		if (!IsFlat(pChild))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The root split, NULL if the root isn't one.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CDockSplit> GetRootSplit(Ptr<CDockManager> pManager)
{
	return Ptr<CDockSplit>(dynamic_cast<CDockSplit*>(&pManager->GetArea()->GetRoot()));
}


//////////////////////////////////////////////////////////////////////////
//
// A split never holds a split of its own direction: docking along the
// area's edge, beside a group, a split left with one node, and a layout
// read with nested splits all leave one split for each direction.
//
//////////////////////////////////////////////////////////////////////////
void TestDockingSplitsMerged(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	CTextFile				cFile;

	cFileUtil.MakeDir("Output" _FS_ "Docking");
	{
		SDockTest				sTest;
		SDockTarget				sTarget;
		Ptr<CDockPane>			pLeft;
		Ptr<CDockPane>			pTop;
		Ptr<CDockPane>			pTop2;
		Ptr<CDockPane>			pBelow;
		Ptr<CDockPane>			pRight;
		Ptr<CDockSplit>			pRoot;
		Ptr<CDockGroup>			pExplorerGroup;
		SDockPlace				sPlace;

		sTest.Init(&cNativeFactory);
		sPlace.Hidden();
		pLeft = sTest.pManager->AddPane("Left", "Left", NULL, CreateTabbedPage(sTest.pWindow), DPK_Tool, &sPlace);
		pTop = sTest.pManager->AddPane("Top", "Top", NULL, CreateTabbedPage(sTest.pWindow), DPK_Tool, &sPlace);
		pTop2 = sTest.pManager->AddPane("Top2", "Top 2", NULL, CreateTabbedPage(sTest.pWindow), DPK_Tool, &sPlace);
		pBelow = sTest.pManager->AddPane("Below", "Below", NULL, CreateTabbedPage(sTest.pWindow), DPK_Tool, &sPlace);
		pRight = sTest.pManager->AddPane("Right", "Right", NULL, CreateTabbedPage(sTest.pWindow), DPK_Tool, &sPlace);
		AssertTrue(IsFlat(sTest.pManager->GetArea()->GetRoot()));

		//Along the area's left edge: the horizontal root takes it first, it isn't split again.
		sTarget.Init();
		sTarget.eKind = DTK_AreaEdge;
		sTarget.eSide = DS_Left;
		sTarget.pcArea = &sTest.pManager->GetArea();
		sTarget.cPreview.Init(0, 0, 100, 600);
		AssertTrue(sTest.pManager->DockPaneAt(pLeft, &sTarget));
		sTest.pWindow->Layout();
		pRoot = GetRootSplit(sTest.pManager);
		AssertTrue(pRoot->IsHorizontal());
		AssertSize(4, pRoot->NumNodes());
		AssertTrue(pRoot->GetNode(0) == pLeft->GetGroup());
		AssertTrue(IsFlat(pRoot));
		AssertInt(100, pRoot->GetNodeSize(0));

		//Along the top: a vertical root over the horizontal one, and the next along the top joins it.
		sTarget.eSide = DS_Top;
		sTarget.cPreview.Init(0, 0, 800, 80);
		AssertTrue(sTest.pManager->DockPaneAt(pTop, &sTarget));
		AssertTrue(sTest.pManager->DockPaneAt(pTop2, &sTarget));
		sTest.pWindow->Layout();
		pRoot = GetRootSplit(sTest.pManager);
		AssertFalse(pRoot->IsHorizontal());
		AssertSize(3, pRoot->NumNodes());
		AssertTrue(pRoot->GetNode(0) == pTop2->GetGroup());
		AssertTrue(pRoot->GetNode(1) == pTop->GetGroup());
		AssertTrue(IsFlat(pRoot));

		//Below Explorer, then right of it: a horizontal split in the vertical one beside Explorer.
		pExplorerGroup = sTest.pExplorer->GetGroup();
		sTarget.Init();
		sTarget.eKind = DTK_Group;
		sTarget.eSide = DS_Bottom;
		sTarget.pcArea = &sTest.pManager->GetArea();
		sTarget.pcNode = &pExplorerGroup;
		sTarget.cPreview.Init(0, 300, 200, 600);
		AssertTrue(sTest.pManager->DockPaneAt(pBelow, &sTarget));
		sTarget.eSide = DS_Right;
		sTarget.cPreview.Init(100, 0, 200, 300);
		AssertTrue(sTest.pManager->DockPaneAt(pRight, &sTarget));
		sTest.pWindow->Layout();
		AssertTrue(IsFlat(sTest.pManager->GetArea()->GetRoot()));
		AssertTrue(pRight->GetGroup()->GetParentSplit() == pExplorerGroup->GetParentSplit());
		AssertTrue(pRight->GetGroup()->GetParentSplit()->IsHorizontal());

		//With Below hidden its vertical split has one node, the horizontal split, which joins the horizontal one above.
		sTest.pManager->Hide(pBelow);
		sTest.pWindow->Layout();
		AssertTrue(IsFlat(sTest.pManager->GetArea()->GetRoot()));
		pRoot = pExplorerGroup->GetParentSplit();
		AssertTrue(pRoot->IsHorizontal());
		AssertTrue(pRoot == pLeft->GetGroup()->GetParentSplit());
		AssertSize(5, pRoot->NumNodes());
		AssertTrue(pRoot->GetNode(1) == pExplorerGroup);
		AssertTrue(pRoot->GetNode(2) == pRight->GetGroup());

		//A layout with a horizontal split in a horizontal split is read as one split.  Output, which it doesn't name, goes
		//under the well as it does by default.
		cFile.Init();
		cFile.mcText.Append("<DockLayout Version=\"1\" NextGroup=\"10\">\n");
		cFile.mcText.Append("  <MainWindow X=\"0\" Y=\"0\" Width=\"800\" Height=\"600\">\n");
		cFile.mcText.Append("    <Split Direction=\"Horizontal\">\n");
		cFile.mcText.Append("      <Group ID=\"1\" Kind=\"Tool\" Size=\"200\"><Pane ID=\"Explorer\"/></Group>\n");
		cFile.mcText.Append("      <Split Direction=\"Horizontal\">\n");
		cFile.mcText.Append("        <DocumentWell/>\n");
		cFile.mcText.Append("        <Group ID=\"2\" Kind=\"Tool\" Size=\"150\"><Pane ID=\"Properties\"/></Group>\n");
		cFile.mcText.Append("      </Split>\n");
		cFile.mcText.Append("    </Split>\n");
		cFile.mcText.Append("  </MainWindow>\n");
		cFile.mcText.Append("</DockLayout>\n");
		AssertTrue(cFile.Write("Output" _FS_ "Docking" _FS_ "Nested.xml"));
		cFile.Kill();
		AssertTrue(sTest.pManager->LoadLayout("Output" _FS_ "Docking" _FS_ "Nested.xml"));
		sTest.pWindow->Layout();
		pRoot = GetRootSplit(sTest.pManager);
		AssertTrue(pRoot->IsHorizontal());
		AssertSize(3, pRoot->NumNodes());
		AssertTrue(pRoot->GetNode(0) == sTest.pExplorer->GetGroup());
		AssertTrue(pRoot->GetNode(1)->ContainsDocumentWell());
		AssertTrue(IsFlat(pRoot));
		AssertTrue(pRoot->GetNode(2) == sTest.pProperties->GetGroup());
		AssertInt(200, pRoot->GetNodeSize(0));
		AssertInt(150, pRoot->GetNodeSize(2));

		pLeft = NULL;
		pTop = NULL;
		pTop2 = NULL;
		pBelow = NULL;
		pRight = NULL;
		pRoot = NULL;
		pExplorerGroup = NULL;
		sTest.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A pane's new title shows on its tab and title bar, on its auto hide tab
// and flyout, and on its floating window's title bar and title.  The dock
// colours are the theme's.
//
//////////////////////////////////////////////////////////////////////////
void TestDockingTitlesAndColours(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		SDockTest					sTest;
		Ptr<CAutoHideStrip>			pStrip;
		Ptr<CDockFloatingWindow>	pFloating;
		CInt32Rect					cRect;
		ARGB32						sColour;

		sTest.Init(&cNativeFactory);

		sTest.pManager->Activate(sTest.pExplorer);
		sTest.pExplorer->SetTitle("Explorer 2");
		AssertString("Explorer 2", sTest.pExplorer->GetGroup()->GetTitleBar()->GetTitle());
		AssertString("Explorer 2", sTest.pExplorer->GetGroup()->GetTab(sTest.pExplorer)->GetText());

		sTest.pManager->AutoHide(sTest.pExplorer);
		pStrip = sTest.pManager->GetArea()->GetStrip(DS_Left);
		sTest.pExplorer->SetTitle("Explorer 3");
		AssertString("Explorer 3", pStrip->FindTab(sTest.pExplorer)->GetText());
		sTest.pManager->ShowFlyout(sTest.pExplorer);
		sTest.pExplorer->SetTitle("Explorer 4");
		AssertString("Explorer 4", sTest.pManager->GetFlyout()->GetTitleBar()->GetTitle());
		AssertString("Explorer 4", pStrip->FindTab(sTest.pExplorer)->GetText());
		sTest.pManager->Dock(sTest.pExplorer);

		//A floating window's title is its selected pane's, on its own title bar when it has one too.
		cRect.Init(100, 100, 400, 400);
		sTest.pManager->Float(sTest.pFirstDoc, &cRect);
		pFloating = sTest.pManager->FindFloating(sTest.pFirstDoc);
		AssertString("First.cpp", pFloating->GetWindow()->GetWindowTitle());
		sTest.pFirstDoc->SetTitle("First.cpp*");
		AssertString("First.cpp*", pFloating->GetWindow()->GetWindowTitle());
		AssertString("First.cpp*", pFloating->GetTitleBar()->GetTitle());
		AssertString("First.cpp*", sTest.pFirstDoc->GetGroup()->GetTab(sTest.pFirstDoc)->GetText());

		//The colours are the theme's, and setting them sets the theme's.
		AssertInt(sTest.pTheme->GetDockColour(DC_ActiveTitleBar), sTest.pManager->GetColour(DC_ActiveTitleBar));
		sColour = Set32BitColour((uint8)1, (uint8)2, (uint8)3);
		sTest.pTheme->SetDockColour(DC_Sash, sColour);
		AssertInt(sColour, sTest.pManager->GetColour(DC_Sash));
		sTest.pManager->SetColour(DC_Guide, sColour);
		AssertInt(sColour, sTest.pTheme->GetDockColour(DC_Guide));

		pStrip = NULL;
		pFloating = NULL;
		sTest.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A tab's window position menu does what its items are tagged with,
// whatever their text says; each pane's menu acts on that pane.
//
//////////////////////////////////////////////////////////////////////////
void TestDockingMenus(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		SDockTest				sTest;
		Ptr<CDockGroup>			pGroup;
		Ptr<CMenuContainer>		pMenu;
		Ptr<CMenuContainer>		pOtherMenu;
		Ptr<CMenuItem>			pItem;
		CInt32Rect				cBounds;
		size					i;

		sTest.Init(&cNativeFactory);
		pGroup = sTest.pOutput->GetGroup();
		pMenu = pGroup->GetMenu(sTest.pErrors);
		pOtherMenu = pGroup->GetMenu(sTest.pOutput);
		AssertTrue(pMenu.IsNotNull());
		AssertTrue(pMenu != pOtherMenu);
		AssertTrue(pGroup->GetTab(sTest.pErrors)->GetContextMenu() == pMenu);

		//The Hide item, renamed, still hides Errors and only Errors.
		pItem = NULL;
		for (i = 0; i < pMenu->NumItems(); i++)
		{
			if (pMenu->GetItem(i)->GetTag() == DMI_Hide)
			{
				pItem = pMenu->GetItem(i);
			}
		}
		AssertTrue(pItem.IsNotNull());
		pItem->SetText("Verstecken");
		pGroup->OpenMenu(sTest.pErrors, SInt32Vec2(300, 200));
		sTest.pWindow->Layout();
		pItem->GetActualBounds(&cBounds);
		DockPress(sTest.pWindow, cBounds.GetLeft() + 10, cBounds.GetTop() + cBounds.GetHeight() / 2);
		DockRelease(sTest.pWindow);
		AssertInt(DPS_Hidden, sTest.pErrors->GetState());
		AssertInt(DPS_Docked, sTest.pOutput->GetState());

		//A document's menu floats it.
		pGroup = sTest.pFirstDoc->GetGroup();
		pMenu = pGroup->GetMenu(sTest.pFirstDoc);
		AssertInt(DMI_Float, pMenu->GetItem(0)->GetTag());
		pGroup->OpenMenu(sTest.pFirstDoc, SInt32Vec2(300, 200));
		sTest.pWindow->Layout();
		pMenu->GetItem(0)->GetActualBounds(&cBounds);
		DockPress(sTest.pWindow, cBounds.GetLeft() + 10, cBounds.GetTop() + cBounds.GetHeight() / 2);
		DockRelease(sTest.pWindow);
		AssertInt(DPS_Floating, sTest.pFirstDoc->GetState());
		AssertInt(DPS_Docked, sTest.pSecondDoc->GetState());

		pGroup = NULL;
		pMenu = NULL;
		pOtherMenu = NULL;
		pItem = NULL;
		sTest.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Control+F4 closes the active document and Shift+Escape hides the active
// tool window, from anywhere in the dock area.
//
//////////////////////////////////////////////////////////////////////////
void TestDockingKeys(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		SDockTest				sTest;
		CInt32Rect				cBounds;

		sTest.Init(&cNativeFactory);

		//The focus on the Second.cpp's tab, in the dock area.
		sTest.pFirstDoc->GetGroup()->GetTab(sTest.pSecondDoc)->GetActualBounds(&cBounds);
		DockPress(sTest.pWindow, cBounds.GetLeft() + 5, cBounds.GetTop() + 5);
		DockRelease(sTest.pWindow);
		AssertTrue(sTest.pManager->GetActiveDocument() == sTest.pSecondDoc);
		sTest.pWindow->InputCommand(CIC_CloseDocument, true);
		AssertInt(DPS_Removed, sTest.pSecondDoc->GetState());
		AssertInt(1, sTest.cListener.miClosed);
		AssertTrue(sTest.pManager->GetActiveDocument() == sTest.pFirstDoc);

		//Output active, the focus on its tab.
		sTest.pManager->Activate(sTest.pOutput);
		sTest.pOutput->GetGroup()->GetTab(sTest.pOutput)->GetActualBounds(&cBounds);
		DockPress(sTest.pWindow, cBounds.GetLeft() + 5, cBounds.GetTop() + 5);
		DockRelease(sTest.pWindow);
		AssertTrue(sTest.pManager->GetActivePane() == sTest.pOutput);
		sTest.pWindow->InputCommand(CIC_HideToolWindow, true);
		AssertInt(DPS_Hidden, sTest.pOutput->GetState());

		//Neither touches a document as the active pane: Shift+Escape on a document does nothing.
		sTest.pManager->Activate(sTest.pFirstDoc);
		sTest.pWindow->InputCommand(CIC_HideToolWindow, true);
		AssertInt(DPS_Docked, sTest.pFirstDoc->GetState());

		sTest.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A selected tab cut off past a narrow tool group's edge is moved to the
// front; the flyout slides back when the focus goes elsewhere; a pane
// whose neighbour is hidden comes back where the neighbour would; a drag
// that shouldn't dock (Control held) floats.
//
//////////////////////////////////////////////////////////////////////////
void TestDockingBehaviour(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		SDockTest				sTest;
		SDockPlace				sPlace;
		Ptr<CDockPane>			pFind;
		Ptr<CDockPane>			pWatch;
		Ptr<CDockPane>			pExtra;
		Ptr<CTabbedContainer>	pTabs;
		CInt32Rect				cScreen;
		CInt32Rect				cGroup;
		CInt32Rect				cBounds;
		CInt32Rect				cWell;
		SDockTarget				sTarget;
		Ptr<CDockGroup>			pOutputGroup;
		SInt32Vec2				sStart;

		sTest.Init(&cNativeFactory);

		//The last tab of a narrow tool group, selected, comes to the front where it shows.
		sPlace.WithPane("Output");
		pFind = sTest.pManager->AddPane("Find", "Find Results", NULL, CreateTabbedPage(sTest.pWindow), DPK_Tool, &sPlace);
		pWatch = sTest.pManager->AddPane("Watch", "Watch Window", NULL, CreateTabbedPage(sTest.pWindow), DPK_Tool, &sPlace);
		pOutputGroup = sTest.pOutput->GetGroup();
		pTabs = pOutputGroup->GetTabs();
		cScreen.Init(0, 0, 520, 600);
		sTest.pWindow->SetScreenRectangle(&cScreen);
		sTest.pManager->Show(sTest.pOutput);
		sTest.pWindow->Layout();
		sTest.pManager->Show(pWatch);
		sTest.pWindow->Layout();
		AssertSize(0, pOutputGroup->IndexOfPane(pWatch));
		AssertTrue(pOutputGroup->GetSelectedPane() == pWatch);
		pOutputGroup->GetActualBounds(&cGroup);
		pOutputGroup->GetTab(pWatch)->GetActualBounds(&cBounds);
		AssertInt(cGroup.GetLeft(), cBounds.GetLeft());
		AssertTrue(pOutputGroup->GetPane(0) == pWatch);
		AssertTrue(pWatch->GetGroup()->GetTabs()->GetPage(0) == pWatch->GetContent());
		cScreen.Init(0, 0, 800, 600);
		sTest.pWindow->SetScreenRectangle(&cScreen);
		sTest.pWindow->Layout();

		//The flyout slides back when a title bar elsewhere takes the focus.
		sTest.pManager->AutoHide(sTest.pExplorer);
		sTest.pManager->ShowFlyout(sTest.pExplorer);
		sTest.pWindow->Layout();
		AssertTrue(sTest.pManager->GetFlyout().IsNotNull());
		sTest.pProperties->GetGroup()->GetTitleBar()->GetActualBounds(&cBounds);
		DockPress(sTest.pWindow, cBounds.GetLeft() + 10, cBounds.GetTop() + 5);
		DockRelease(sTest.pWindow);
		AssertTrue(sTest.pManager->GetFlyout().IsNull());
		sTest.pManager->Dock(sTest.pExplorer);

		//Extra, docked into Output's group, is hidden, then the rest of the group.  Shown again it goes under the well,
		//where its neighbour would go back, not to its own default on the right.
		sPlace.BesideWell(DS_Right, 100);
		pExtra = sTest.pManager->AddPane("Extra", "Extra", NULL, CreateTabbedPage(sTest.pWindow), DPK_Tool, &sPlace);
		sTarget.Init();
		sTarget.eKind = DTK_Group;
		sTarget.eSide = DS_Centre;
		sTarget.pcArea = &sTest.pManager->GetArea();
		sTarget.pcNode = &pOutputGroup;
		AssertTrue(sTest.pManager->DockPaneAt(pExtra, &sTarget));
		AssertTrue(pExtra->GetGroup() == pOutputGroup);
		sTest.pWindow->Layout();
		sTest.pManager->Hide(pExtra);
		sTest.pManager->Hide(pFind);
		sTest.pManager->Hide(pWatch);
		sTest.pManager->Hide(sTest.pErrors);
		sTest.pManager->Hide(sTest.pOutput);
		sTest.pWindow->Layout();
		sTest.pManager->Show(pExtra);
		sTest.pWindow->Layout();
		pExtra->GetGroup()->GetActualBounds(&cBounds);
		sTest.pManager->GetWell()->GetActualBounds(&cWell);
		AssertTrue(cBounds.GetTop() >= cWell.GetTop() + cWell.GetHeight());
		AssertInt(cWell.GetLeft(), cBounds.GetLeft());
		sTest.pManager->Show(sTest.pOutput);
		AssertTrue(sTest.pOutput->GetGroup() == pExtra->GetGroup());

		//A drag that isn't to dock, as with Control held, floats where it is let go even over a guide.
		sTest.pWindow->Layout();
		pExtra->GetGroup()->GetTitleBar()->GetActualBounds(&cBounds);
		sStart.Init(cBounds.GetLeft() + 20, cBounds.GetTop() + 5);
		DockPress(sTest.pWindow, sStart.x, sStart.y);
		DockMove(sTest.pWindow, sStart.x + 10, sStart.y - 10);
		AssertTrue(sTest.pManager->IsDragging());
		sTest.pManager->SetDragNoDock(true);
		DockMove(sTest.pWindow, 8 + DOCK_GUIDE_SIZE / 2, 300);
		AssertFalse(sTest.pManager->GetTarget()->IsValid());
		AssertTrue(sTest.pManager->GetArea()->GetGuides().IsNull());
		DockRelease(sTest.pWindow);
		AssertInt(DPS_Floating, pExtra->GetState());

		pFind = NULL;
		pWatch = NULL;
		pExtra = NULL;
		pTabs = NULL;
		pOutputGroup = NULL;
		sTest.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Floating windows are kept on the screen; a layout read before the main
// window is shown puts it where it was.
//
//////////////////////////////////////////////////////////////////////////
void TestDockingScreen(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	CTextFile				cFile;

	cFileUtil.MakeDir("Output" _FS_ "Docking");
	{
		SDockTest				sTest;
		CInt32Rect				cRect;
		CInt32Rect				cScreen;

		sTest.Init(&cNativeFactory);

		//Without a screen nothing is moved.
		cRect.Init(900, 600, 1200, 900);
		sTest.pManager->Float(sTest.pProperties, &cRect);
		sTest.pManager->FindFloating(sTest.pProperties)->GetRectangle(&cRect);
		AssertInt(900, cRect.GetLeft());
		sTest.pManager->Dock(sTest.pProperties);

		//On a 1000 x 700 screen it is moved back onto it, and shrunk to fit.
		cScreen.Init(0, 0, 1000, 700);
		cNativeFactory.SetScreenArea(&cScreen);
		cRect.Init(900, 600, 1200, 900);
		sTest.pManager->Float(sTest.pProperties, &cRect);
		sTest.pManager->FindFloating(sTest.pProperties)->GetRectangle(&cRect);
		AssertInt(700, cRect.GetLeft());
		AssertInt(400, cRect.GetTop());
		AssertInt(300, cRect.GetWidth());
		sTest.pManager->Dock(sTest.pProperties);
		cRect.Init(-50, -20, 1500, 400);
		sTest.pManager->Float(sTest.pProperties, &cRect);
		sTest.pManager->FindFloating(sTest.pProperties)->GetRectangle(&cRect);
		AssertInt(0, cRect.GetLeft());
		AssertInt(0, cRect.GetTop());
		AssertInt(1000, cRect.GetWidth());
		sTest.pManager->Dock(sTest.pProperties);

		//The main window, not shown yet, goes where the layout says, kept on the screen.
		cFile.Init();
		cFile.mcText.Append("<DockLayout Version=\"1\" NextGroup=\"10\">\n");
		cFile.mcText.Append("  <MainWindow X=\"50\" Y=\"40\" Width=\"700\" Height=\"500\">\n");
		cFile.mcText.Append("    <DocumentWell/>\n");
		cFile.mcText.Append("  </MainWindow>\n");
		cFile.mcText.Append("</DockLayout>\n");
		AssertTrue(cFile.Write("Output" _FS_ "Docking" _FS_ "Window.xml"));
		cFile.Kill();
		AssertFalse(sTest.pWindow->IsShown());
		AssertTrue(sTest.pManager->LoadLayout("Output" _FS_ "Docking" _FS_ "Window.xml"));
		sTest.pWindow->GetScreenRectangle(&cRect);
		AssertInt(50, cRect.GetLeft());
		AssertInt(40, cRect.GetTop());
		AssertInt(700, cRect.GetWidth());
		AssertInt(500, cRect.GetHeight());

		sTest.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//Records how a dialog shown over a floating window left the windows.
struct SFloatingDialogCheck
{
	CWindow*	pcMain;
	CWindow*	pcFloating;
	bool		bMainEnabled;
	bool		bFloatingEnabled;
	bool		bOverFloating;
	int			iFrames;
};


//////////////////////////////////////////////////////////////////////////
//
// The dialog's first frame looks at the windows and stops it.
//
//////////////////////////////////////////////////////////////////////////
bool FloatingDialogFrames(Ptr<CWindow> pWindow, int32 iFrame, void* pvContext)
{
	SFloatingDialogCheck*	psCheck;

	psCheck = (SFloatingDialogCheck*)pvContext;
	psCheck->iFrames++;
	psCheck->bMainEnabled = psCheck->pcMain->GetNativeWindow()->IsEnabled();
	psCheck->bFloatingEnabled = psCheck->pcFloating->GetNativeWindow()->IsEnabled();
	psCheck->bOverFloating = pWindow->GetNativeWindow()->GetOwner() == psCheck->pcFloating->GetNativeWindow();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A dialog shown over a floating window is over it, and the main window
// and every floating window are disabled while it is shown.
//
//////////////////////////////////////////////////////////////////////////
void TestDockingDialogOverFloating(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "DockingDialog";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);
	{
		SDockTest					sTest;
		CInt32Rect					cRect;
		Ptr<CDockFloatingWindow>	pFloating;
		Ptr<CDockFloatingWindow>	pOther;
		Ptr<CWindow>				pDialog;
		Ptr<CScriptFramesTick>		pDialogTick;
		SFloatingDialogCheck		sCheck;
		CPointer					pNull;

		sTest.Init(&cNativeFactory, szDirectory);
		cRect.Init(100, 100, 400, 400);
		sTest.pManager->Float(sTest.pProperties, &cRect);
		cRect.Init(450, 100, 700, 400);
		sTest.pManager->Float(sTest.pOutput, &cRect);
		pFloating = sTest.pManager->FindFloating(sTest.pProperties);
		pOther = sTest.pManager->FindFloating(sTest.pOutput);

		memset(&sCheck, 0, sizeof(sCheck));
		sCheck.pcMain = &sTest.pWindow;
		sCheck.pcFloating = &pFloating->GetWindow();
		sCheck.bMainEnabled = true;
		sCheck.bFloatingEnabled = true;
		pDialogTick = OMalloc<CScriptFramesTick>(FloatingDialogFrames, &sCheck);
		pDialog = OMalloc<CWindow>("Dialog", &cNativeFactory, pDialogTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pDialog);
		AssertTrue(pDialog->ShowModal(pFloating->GetWindow()));
		AssertInt(1, sCheck.iFrames);
		AssertFalse(sCheck.bMainEnabled);
		AssertFalse(sCheck.bFloatingEnabled);
		AssertTrue(sCheck.bOverFloating);
		AssertTrue(sTest.pWindow->GetNativeWindow()->IsEnabled());
		AssertTrue(pFloating->GetWindow()->GetNativeWindow()->IsEnabled());
		AssertTrue(pOther->GetWindow()->GetNativeWindow()->IsEnabled());

		ORoot()->Remove(pDialog);
		pDialog = NULL;
		pDialogTick = NULL;
		pFloating = NULL;
		pOther = NULL;
		sTest.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
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
// A group docked along the whole bottom of the area (under Explorer and
// the well) is there again after the layout is saved and loaded.
//
//////////////////////////////////////////////////////////////////////////
void TestDockingLayoutFileAreaEdge(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;

	cFileUtil.MakeDir("Output" _FS_ "Docking");
	{
		SDockTest				sTest;
		CXMLWriter				cFirst;
		CXMLWriter				cSecond;
		SDockTarget				sTarget;
		Ptr<CDockNode>			pGroup;
		Ptr<CDockFloatingWindow>	pFloating;

		//Dragged as a drop does it: the group floats, then its floating window's root is docked.
		sTest.Init(&cNativeFactory);
		sTest.pManager->FloatGroup(sTest.pOutput->GetGroup());
		AssertSize(1, sTest.pManager->NumFloating());
		pFloating = sTest.pManager->GetFloating(0);
		pGroup = pFloating->TakeRoot();
		sTest.pManager->RemoveFloating(pFloating);
		pFloating = NULL;
		sTarget.Init();
		sTarget.eKind = DTK_AreaEdge;
		sTarget.eSide = DS_Bottom;
		sTarget.pcArea = &sTest.pManager->GetArea();
		sTarget.cPreview.Init(0, 480, 800, 120);
		AssertTrue(sTest.pManager->DockNodeAt(pGroup, &sTarget));
		sTest.pWindow->Layout();

		cFirst.Init();
		sTest.pManager->WriteLayout(&cFirst);
		AssertFalse(GetRootSplit(sTest.pManager)->IsHorizontal());
		AssertTrue(sTest.pManager->SaveLayout("Output" _FS_ "Docking" _FS_ "LayoutAreaEdge.xml"));

		sTest.pManager->ResetLayout();
		sTest.pWindow->Layout();
		AssertTrue(GetRootSplit(sTest.pManager)->IsHorizontal());

		AssertTrue(sTest.pManager->LoadLayout("Output" _FS_ "Docking" _FS_ "LayoutAreaEdge.xml"));
		sTest.pWindow->Layout();
		AssertFalse(GetRootSplit(sTest.pManager)->IsHorizontal());

		cSecond.Init();
		sTest.pManager->WriteLayout(&cSecond);
		AssertString(cFirst.GetText(), cSecond.GetText());

		cSecond.Kill();
		cFirst.Kill();
		pGroup = NULL;
		sTest.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A floating window shown before the main window has its input (a layout
// read as the application starts) gets the input when the main window
// does, and goes on sharing it.
//
//////////////////////////////////////////////////////////////////////////
void TestDockingFloatingInput(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		SDockTest				sTest;
		CComponentInput			cInput;
		Ptr<CWindow>			pFloatingWindow;

		sTest.Init(&cNativeFactory);
		AssertNull(sTest.pWindow->GetComponentInput());
		sTest.pManager->Float(sTest.pProperties);
		AssertSize(1, sTest.pManager->NumFloating());
		pFloatingWindow = sTest.pManager->GetFloating(0)->GetWindow();
		AssertNull(pFloatingWindow->GetComponentInput());

		sTest.pWindow->SetComponentInput(&cInput);
		AssertTrue(pFloatingWindow->GetComponentInput() == &cInput);
		sTest.pWindow->SetComponentInput(NULL);
		AssertNull(pFloatingWindow->GetComponentInput());

		pFloatingWindow = NULL;
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
	TestDockingNarrowToolTabs();
	TestDockingSplitsMerged();
	TestDockingTitlesAndColours();
	TestDockingKeys();
	TestDockingMenus();
	TestDockingBehaviour();
	TestDockingScreen();
	TestDockingDialogOverFloating();
	TestDockingMoveToWindow();
	TestDockingLayoutFile();
	TestDockingLayoutFileAreaEdge();
	TestDockingFloatingInput();
	ValidateLabelObjects();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

