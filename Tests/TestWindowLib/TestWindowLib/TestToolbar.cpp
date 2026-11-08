#include "BaseLib/GlobalMemory.h"
#include "BaseLib/StringHelper.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/GridContainer.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/Toolbar.h"
#include "WindowLib/CommandSet.h"
#include "WindowLib/JewelTheme.h"
#include "WindowLib/Label.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"


Ptr<CFont> GenerateMenuFont(Ptr<CWindow> pWindow);


//An application's commands for the toolbar tests: log when they run.
class CTestToolbarCommands
{
public:
	CChars*		mpszLog;

	void Build(void) { mpszLog->Append("Build;"); }
	void Run(void) { mpszLog->Append("Run;"); }
};


//////////////////////////////////////////////////////////////////////////
//
// A toolbar along the top of a window: the inset, a command's icon
// button, a separator, a command's caption button, a stretch and a label
// at the far end.  Each is its own size, centred across the bar, which is
// the theme's thickness high and as wide as the window.
//
//////////////////////////////////////////////////////////////////////////
void TestToolbarLayout(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>			pWindow;
		Ptr<CWindowTheme>		pTheme;
		Ptr<CGridContainer>		pMain;
		Ptr<CToolbar>			pToolbar;
		Ptr<CCommandSet>		pCommands;
		Ptr<CButton>			pRun;
		Ptr<CButton>			pBuild;
		Ptr<CLabel>				pBranch;
		CTestToolbarCommands	cCommands;
		CPointer				pNull;
		CChars					szLog;
		CInt32Rect				cRun;
		CInt32Rect				cBuild;
		CInt32Rect				cBranch;
		CInt32Rect				cBar;
		int32					iThickness;

		szLog.Init();
		cCommands.mpszLog = &szLog;
		cNativeFactory.Init(&gcMemoryAllocator, 400, 100, NULL);
		pWindow = OMalloc<CWindow>("Toolbar Window", &cNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);
		pTheme = CJewelTheme::CreateDarkTheme(pWindow, GenerateMenuFont(pWindow));
		iThickness = pTheme->GetMetric(TM_ToolbarThickness);

		pCommands = OMalloc<CCommandSet>();
		pCommands->Add("Run", "Run", "F5", pTheme->GetImage(TI_NextArrow), &cCommands, &CTestToolbarCommands::Run);
		pCommands->Add("Build", "Build", "Shift+Control+B", Ptr<CImageCel>(), &cCommands, &CTestToolbarCommands::Build);

		pToolbar = OMalloc<CToolbar>(pWindow, pTheme, false);
		AssertFalse(pToolbar->IsVertical());
		pRun = pToolbar->AddCommand(pCommands->Find("Run"));
		pToolbar->AddSeparator();
		pBuild = pToolbar->AddCommand(pCommands->Find("Build"));
		pToolbar->AddStretch();
		AssertTrue(pToolbar->IsStretched());
		pBranch = OMalloc<CLabel>(pWindow, pTheme->GetFont(), "main");
		pToolbar->Append(pBranch);

		pMain = OMalloc<CGridContainer>(pWindow);
		pMain->AddComponent(pToolbar, 0, 0, 1, 1, 1, 0);
		pMain->AddComponent(OMalloc<CFillContainer>(pWindow), 0, 1, 1, 1, 1, 1);  //The window's content under it.
		pWindow->SetContainer(pMain);
		pWindow->Layout();

		pToolbar->GetActualBounds(&cBar);
		AssertInt(0, cBar.GetLeft());
		AssertInt(0, cBar.GetTop());
		AssertInt(400, cBar.GetWidth());
		AssertInt(iThickness, cBar.GetHeight());

		//The icon button is flat, after the inset; the caption button after the separator's space.
		AssertTrue(pRun->GetIcon() == pTheme->GetImage(TI_NextArrow));
		AssertTrue(StrEmpty(pRun->GetCaption()));
		AssertString("Build", pBuild->GetCaption());
		pRun->GetActualBounds(&cRun);
		pBuild->GetActualBounds(&cBuild);
		AssertInt(pTheme->GetMetric(TM_ToolbarInset), cRun.GetLeft());
		AssertInt(cRun.GetRight() + pTheme->GetMetric(TM_ToolbarSeparatorSpace), cBuild.GetLeft());
		AssertInt((iThickness - cRun.GetHeight()) / 2, cRun.GetTop());
		AssertInt((iThickness - cBuild.GetHeight()) / 2, cBuild.GetTop());

		//After the stretch: at the far end, the inset from it.
		pBranch->GetActualBounds(&cBranch);
		AssertInt(400 - pTheme->GetMetric(TM_ToolbarInset), cBranch.GetRight());

		//Its buttons run their commands.
		pRun->Click();
		pBuild->Click();
		AssertString("Run;Build;", szLog.Text());

		pBranch = NULL;
		pBuild = NULL;
		pRun = NULL;
		pToolbar = NULL;
		pMain = NULL;
		pCommands = NULL;
		pTheme = NULL;
		ORoot()->Remove(pWindow);
		pWindow = NULL;
		szLog.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A column of tool buttons down a window's side, as IntelliJ's tool
// window stripes: the theme's thickness wide, the buttons from the top
// after the inset, the space left over after the last.
//
//////////////////////////////////////////////////////////////////////////
void TestToolbarVertical(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>			pWindow;
		Ptr<CWindowTheme>		pTheme;
		Ptr<CGridContainer>		pMain;
		Ptr<CToolbar>			pStripe;
		Ptr<CCommandSet>		pCommands;
		Ptr<CButton>			pFirst;
		Ptr<CButton>			pSecond;
		CTestToolbarCommands	cCommands;
		CPointer				pNull;
		CChars					szLog;
		CInt32Rect				cFirst;
		CInt32Rect				cSecond;
		CInt32Rect				cStripe;

		szLog.Init();
		cCommands.mpszLog = &szLog;
		cNativeFactory.Init(&gcMemoryAllocator, 200, 300, NULL);
		pWindow = OMalloc<CWindow>("Stripe Window", &cNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);
		pTheme = CJewelTheme::CreateDarkTheme(pWindow, GenerateMenuFont(pWindow));
		pTheme->SetMetric(TM_ToolbarThickness, 40);

		pCommands = OMalloc<CCommandSet>();
		pCommands->Add("Run", "Run", "", pTheme->GetImage(TI_NextArrow), &cCommands, &CTestToolbarCommands::Run);
		pCommands->Add("Back", "Back", "", pTheme->GetImage(TI_PreviousArrow), &cCommands, &CTestToolbarCommands::Run);

		pStripe = OMalloc<CToolbar>(pWindow, pTheme, true);
		AssertTrue(pStripe->IsVertical());
		pFirst = pStripe->AddCommand(pCommands->Find("Run"));
		pStripe->AddSeparator();
		pSecond = pStripe->AddCommand(pCommands->Find("Back"));
		AssertFalse(pStripe->IsStretched());

		pMain = OMalloc<CGridContainer>(pWindow);
		pMain->AddComponent(pStripe, 0, 0, 1, 1, 0, 1);
		pMain->AddComponent(OMalloc<CFillContainer>(pWindow), 1, 0, 1, 1, 1, 1);  //The window's content beside it.
		pWindow->SetContainer(pMain);
		pWindow->Layout();

		pStripe->GetActualBounds(&cStripe);
		AssertInt(40, cStripe.GetWidth());
		AssertInt(300, cStripe.GetHeight());
		pFirst->GetActualBounds(&cFirst);
		pSecond->GetActualBounds(&cSecond);
		AssertInt(pTheme->GetMetric(TM_ToolbarInset), cFirst.GetTop());
		AssertInt(cFirst.GetBottom() + pTheme->GetMetric(TM_ToolbarSeparatorSpace), cSecond.GetTop());
		AssertInt((40 - cFirst.GetWidth()) / 2, cFirst.GetLeft());

		pSecond = NULL;
		pFirst = NULL;
		pStripe = NULL;
		pMain = NULL;
		pCommands = NULL;
		pTheme = NULL;
		ORoot()->Remove(pWindow);
		pWindow = NULL;
		szLog.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A command's button has its command's caption and keys as its tool tip,
// as a menu item shows them.  A button with a caption only has its
// command's tip if the command has keys.  A tip set on the button is its
// own.
//
//////////////////////////////////////////////////////////////////////////
void TestToolbarCommandToolTips(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>			pWindow;
		Ptr<CWindowTheme>		pTheme;
		Ptr<CToolbar>			pToolbar;
		Ptr<CCommandSet>		pCommands;
		Ptr<CButton>			pRun;
		Ptr<CButton>			pBuild;
		Ptr<CButton>			pClean;
		Ptr<CButton>			pPlain;
		CTestToolbarCommands	cCommands;
		CPointer				pNull;
		CChars					szLog;
		CChars					szTip;

		szLog.Init();
		cCommands.mpszLog = &szLog;
		cNativeFactory.Init(&gcMemoryAllocator, 400, 100, NULL);
		pWindow = OMalloc<CWindow>("Toolbar Window", &cNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);
		pTheme = CJewelTheme::CreateDarkTheme(pWindow, GenerateMenuFont(pWindow));

		pCommands = OMalloc<CCommandSet>();
		pCommands->Add("Run", "Run", "", pTheme->GetImage(TI_NextArrow), &cCommands, &CTestToolbarCommands::Run);
		pCommands->Add("Build", "Build Solution", "Shift+Control+B", Ptr<CImageCel>(), &cCommands, &CTestToolbarCommands::Build);
		pCommands->Add("Clean", "Clean", "", Ptr<CImageCel>(), &cCommands, &CTestToolbarCommands::Build);

		pToolbar = OMalloc<CToolbar>(pWindow, pTheme, false);
		pRun = pToolbar->AddCommand(pCommands->Find("Run"));
		pBuild = pToolbar->AddCommand(pCommands->Find("Build"));
		pClean = pToolbar->AddCommand(pCommands->Find("Clean"));
		pPlain = OMalloc<CButton>(pWindow, pTheme, "Plain");
		pToolbar->Append(pPlain);

		//An icon button: its command's caption.
		szTip.Init();
		AssertTrue(pRun->GetToolTip(&szTip));
		AssertString("Run", szTip.Text());

		//A caption button whose command has keys: the caption and the keys.
		szTip.Clear();
		AssertTrue(pBuild->GetToolTip(&szTip));
		AssertString("Build Solution (Ctrl+Shift+B)", szTip.Text());

		//A caption button whose command has no keys, and a button without a command: none.
		szTip.Clear();
		AssertFalse(pClean->GetToolTip(&szTip));
		AssertFalse(pPlain->GetToolTip(&szTip));
		AssertString("", szTip.Text());

		//Its own tip wins.
		pRun->SetToolTip("Run the program");
		AssertTrue(pRun->GetToolTip(&szTip));
		AssertString("Run the program", szTip.Text());
		pRun->SetToolTip(NULL);
		szTip.Clear();
		AssertTrue(pRun->GetToolTip(&szTip));
		AssertString("Run", szTip.Text());
		szTip.Kill();

		pPlain = NULL;
		pClean = NULL;
		pBuild = NULL;
		pRun = NULL;
		pToolbar = NULL;
		pCommands = NULL;
		pTheme = NULL;
		ORoot()->Remove(pWindow);
		pWindow = NULL;
		szLog.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestToolbar(void)
{
	BeginTests();
	ObjectsInit();

	TestToolbarLayout();
	TestToolbarVertical();
	TestToolbarCommandToolTips();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

