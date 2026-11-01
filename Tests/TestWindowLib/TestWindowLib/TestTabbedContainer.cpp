#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/Canvas.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WindowLib/JewelTheme.h"
#include "WindowLib/TabbedContainer.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"


Ptr<CFont> ReadLabelFont(void);


class CTestTabbedContainerListener : public CTabbedContainerListener
{
CONSTRUCTABLE(CTestTabbedContainerListener);
public:
	CTabComponent*	mpcDoubleClicked;
	int				miDoubleClicks;

	void Init(void)
	{
		mpcDoubleClicked = NULL;
		miDoubleClicks = 0;
	}

	void TabDoubleClicked(CUnknown* pcSource, void* pvContext) override
	{
		mpcDoubleClicked = (CTabComponent*)pcSource;
		miDoubleClicks++;
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CCanvas> CreateTabbedPage(Ptr<CWindow> pWindow)
{
	Ptr<CCanvas>	pPage;

	pPage = OMalloc<CCanvas>(pWindow, OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)0x1E, 0x1F, 0x22)));
	pPage->SetAlignment(CSH_Stretch, CSV_Stretch);
	return pPage;
}


//////////////////////////////////////////////////////////////////////////
//
// The window is 200x100.  The tabs are on a bar along the top and the
// selected tab's page fills the rest.
//
//////////////////////////////////////////////////////////////////////////
void TestTabbedContainerTabs(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CWindowTheme>			pTheme;
		Ptr<CTabbedContainer>		pTabbed;
		Ptr<CCanvas>				pInput;
		Ptr<CCanvas>				pEditor;
		Ptr<CTabComponent>			pInputTab;
		Ptr<CTabComponent>			pEditorTab;
		Ptr<CTabComponent>			pFileTab;
		Ptr<CCanvas>				pFile;
		int32						iBarHeight;
		int32						iEditorWidth;
		CTestTabbedContainerListener	cListener;

		cNativeFactory.Init(&gcMemoryAllocator, 200, 100, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Tabbed Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pTheme = CJewelTheme::CreateDarkTheme(pTestWindow, ReadLabelFont());
		pTabbed = OMalloc<CTabbedContainer>(pTestWindow, pTheme);
		pTestWindow->SetContainer(pTabbed);
		pInput = CreateTabbedPage(pTestWindow);
		pEditor = CreateTabbedPage(pTestWindow);
		pFile = CreateTabbedPage(pTestWindow);
		pInputTab = pTabbed->AddTab("Input", pInput);
		pEditorTab = pTabbed->AddTab("Editor", pEditor);
		pFileTab = pTabbed->AddTab("File", pFile, pTheme->GetTabClose(), true);
		AssertSize(3, pTabbed->NumTabs());
		AssertFalse(pInputTab->IsClosable());
		AssertTrue(pInputTab->GetIcon().IsNull());
		AssertTrue(pFileTab->IsClosable());
		AssertTrue(pFileTab->GetIcon().IsNotNull());
		AssertString("File", pFileTab->GetText());

		//The first tab added is selected.  Its page is under the bar and the separator line.
		pTestWindow->Layout();
		AssertTrue(pTabbed->GetSelected() == pInputTab);
		AssertTrue(pInputTab->IsSelected());
		AssertFalse(pEditorTab->IsSelected());
		iBarHeight = pInputTab->GetActualSize().y;
		AssertTrue(iBarHeight > 0);
		AssertInt(0, pInputTab->GetPosition().y);
		AssertInt(iBarHeight, pFileTab->GetActualSize().y);
		AssertTrue(pEditorTab->GetPosition().x > pInputTab->GetPosition().x);
		AssertTrue(pFileTab->GetPosition().x > pEditorTab->GetPosition().x);
		AssertTrue(pInput->GetParent().IsNotNull());
		AssertTrue(pEditor->GetParent().IsNull());
		AssertInt(iBarHeight + 1, pInput->GetPosition().y);
		AssertInt(200, pInput->GetActualSize().x);
		AssertInt(100 - iBarHeight - 1, pInput->GetActualSize().y);

		//The icon is left of the name, the cross right of it.
		AssertTrue(pFileTab->GetIcon()->GetPosition().x < pFileTab->GetClose()->GetPosition().x);

		//Pressing a tab shows its page.
		pTestWindow->PointerMoved(pEditorTab->GetPosition().x + 2, 2);
		AssertTrue(pEditorTab->IsMouseOver());
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		pTestWindow->Layout();
		AssertTrue(pTabbed->GetSelected() == pEditorTab);
		AssertFalse(pInputTab->IsSelected());
		AssertTrue(pEditorTab->IsSelected());
		AssertTrue(pInput->GetParent().IsNull());
		AssertTrue(pEditor->GetParent().IsNotNull());
		AssertInt(iBarHeight + 1, pEditor->GetPosition().y);

		//Pressing the file tab, but not its cross, selects it.
		pTestWindow->PointerMoved(pFileTab->GetPosition().x + 2, 2);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertTrue(pTabbed->GetSelected() == pFileTab);

		//Pressing and releasing a tab twice quickly is a double click, which tells the listeners.  The cross isn't.
		cListener.Init();
		AssertTrue(pTabbed->AddListener(&cListener));
		pTestWindow->Layout();
		pTestWindow->PointerMoved(pInputTab->GetPosition().x + 10, 2);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertInt(0, cListener.miDoubleClicks);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertInt(1, cListener.miDoubleClicks);
		AssertTrue(cListener.mpcDoubleClicked == &pInputTab);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertInt(1, cListener.miDoubleClicks);

		//Renaming a tab resizes it.
		iEditorWidth = pEditorTab->GetActualSize().x;
		pEditorTab->SetText("Editor 2");
		pTestWindow->Layout();
		AssertString("Editor 2", pEditorTab->GetText());
		AssertTrue(pEditorTab->GetActualSize().x > iEditorWidth);
		pEditorTab->SetText("Editor");
		pTestWindow->Layout();
		AssertInt(iEditorWidth, pEditorTab->GetActualSize().x);

		pTestWindow->PointerMoved(pFileTab->GetPosition().x + 2, 2);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertTrue(pTabbed->GetSelected() == pFileTab);

		//Its cross closes it and the tab before it, the last now, is selected.
		pTestWindow->Layout();
		pTestWindow->PointerMoved(pFileTab->GetClose()->GetPosition().x + 8, pFileTab->GetClose()->GetPosition().y + 8);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertSize(2, pTabbed->NumTabs());
		AssertTrue(pTabbed->GetSelected() == pEditorTab);
		AssertTrue(pFile->GetParent().IsNull());
		AssertTrue(pFileTab->GetParent().IsNull());
		AssertInt(1, cListener.miDoubleClicks);
		AssertTrue(pTabbed->RemoveListener(&cListener));

		//A tab added after a closed one goes after the others.
		pFileTab = pTabbed->AddTab("Again", pFile, NULL, true);
		pTestWindow->Layout();
		AssertTrue(pFileTab->GetPosition().x > pEditorTab->GetPosition().x);

		//And back by index.
		pTabbed->Select((size)0);
		AssertTrue(pTabbed->GetSelected() == pInputTab);

		pInputTab = NULL;
		pEditorTab = NULL;
		pFileTab = NULL;
		pInput = NULL;
		pEditor = NULL;
		pFile = NULL;
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTabbedContainer(void)
{
	BeginTests();
	ObjectsInit();

	TestTabbedContainerTabs();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}
