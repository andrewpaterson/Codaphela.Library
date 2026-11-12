#include <string.h>
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/Timer.h"
#include "BaseLib/ArrayChars.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/GridContainer.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "WinRefLib/WinRefWindow.h"
#include "TestLib/Assert.h"
#include "BoxFillCanvasDraw.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"


//////////////////////////////////////////////////////////////////////////
//
// A window cleared to one colour with a box filled canvas in it, drawing
// only what changed.  Shown, it ticks iTicks times and stops.
//
//////////////////////////////////////////////////////////////////////////
struct SScreenWindow
{
	SDataTestRefWindow		cData;
	Ptr<CTickTestRefWindow>	pTick;
	Ptr<CWindow>			pWindow;
	Ptr<CGridContainer>		pGrid;
	Ptr<CCanvas>			pBox;

	void Init(const char* szTitle, CWinRefWindowFactory* pcFactory, ARGB32 sClear, int32 iTicks)
	{
		pTick = OMalloc<CTickTestRefWindow>(&cData, iTicks);
		pWindow = OMalloc<CWindow>(szTitle, pcFactory, pTick, OMalloc<CClearCanvasDraw>(sClear), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);
		pWindow->SetPartialRedraw(true);
		pGrid = OMalloc<CGridContainer>(pWindow);
		pWindow->SetContainer(pGrid);
		pBox = OMalloc<CCanvas>(pWindow, OMalloc<CBoxFillCanvasDraw>(Set32BitColour(1.0f, 1.0f, 0)));
		pBox->SetDesiredSize(20, 10);
		pBox->SetAlignment(CSH_Left, CSV_Top);
		pGrid->AddComponent(pBox, 0, 0);
	}

	void Kill(void)
	{
		pBox = NULL;
		pGrid = NULL;
		ORoot()->Remove(pWindow);
		pWindow = NULL;
		pTick = NULL;
	}

	CWinRefWindow* GetNative(void)
	{
		return (CWinRefWindow*)pWindow->GetNativeWindow();
	}
};


//////////////////////////////////////////////////////////////////////////
//
// Checked, a present that shows what was drawn passes; one that leaves
// some of what was drawn unshown is logged and counted once, and the
// window shows its canvas again.
//
//////////////////////////////////////////////////////////////////////////
void TestRefWindowScreenCheck(void)
{
	CWinRefWindowFactory	cFactory;
	SLogConfig				sLogConfig;

	ObjectsInit();
	cFactory.Init(&gcMemoryAllocator, 64, 48, NULL);
	cFactory.SetCheckPresents(true);
	AssertTrue(cFactory.GetScreen()->IsWatching());
	{
		SScreenWindow	sWindow;
		Ptr<CImage>		pImage;

		sWindow.Init("Checked", &cFactory, Set32BitColour(0, 0, 1.0f), 1);
		sWindow.pWindow->Paint();
		AssertInt(64, sWindow.GetNative()->GetShown()->GetWidth());
		AssertInt(48, sWindow.GetNative()->GetShown()->GetHeight());
		sWindow.pWindow->Paint();
		sWindow.pBox->Invalidate();
		sWindow.pWindow->Paint();
		sWindow.pBox->SetDesiredSize(30, 20);
		sWindow.pWindow->Paint();
		AssertSize(0, cFactory.GetScreen()->NumMismatches());

		//Drawn but not presented: the paint after finds nothing changed and shows nothing.
		pImage = sWindow.pWindow->GetCanvas()->GetImageOrNull();
		((uint8*)pImage->GetData())[0] ^= 0xFF;
		pImage = NULL;
		sLogConfig = gcLogger.SetSilent();
		sWindow.pWindow->Paint();
		gcLogger.SetConfig(&sLogConfig);
		AssertSize(1, cFactory.GetScreen()->NumMismatches());
		sWindow.pWindow->Paint();
		AssertSize(1, cFactory.GetScreen()->NumMismatches());

		sWindow.Kill();
	}
	AssertTrue(ORoot()->RemoveAll());
	cFactory.Kill();
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// The window is shown (it ticks once), then painted again: its box
// invalidated, nothing changed, its box invalidated again.
//
//////////////////////////////////////////////////////////////////////////
static void HashWindowPresents(const char* szFileName, EWinRefHashArea eArea)
{
	CWinRefWindowFactory	cFactory;
	CInt32Rect				cScreen;

	ObjectsInit();
	cFactory.Init(&gcMemoryAllocator, 64, 48, NULL);
	cScreen.Init(0, 0, 80, 60);
	cFactory.SetScreenArea(&cScreen);
	AssertTrue(cFactory.SetHashPresents(szFileName, eArea));
	{
		SScreenWindow	sWindow;

		sWindow.Init("Hashed", &cFactory, Set32BitColour(0, 0, 1.0f), 1);
		AssertTrue(sWindow.pWindow->Show());
		sWindow.pBox->Invalidate();
		sWindow.pWindow->Paint();
		sWindow.pWindow->Paint();
		sWindow.pBox->Invalidate();
		sWindow.pWindow->Paint();
		sWindow.Kill();
	}
	AssertTrue(ORoot()->RemoveAll());
	cFactory.Kill();
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// The hash file's lines, each split at its spaces: the present, the
// window's title and the hash.
//
//////////////////////////////////////////////////////////////////////////
static void ReadHashLines(const char* szFileName, CArrayChars* paszLines)
{
	CTextFile	cFile;
	CChars		szText;

	cFile.Init();
	AssertTrue(cFile.Read(szFileName));
	szText.Init(cFile.Text());
	paszLines->Init();
	szText.SplitLines(paszLines);
	if ((paszLines->NumElements() > 0) && (paszLines->Get(paszLines->NumElements() - 1)->Empty()))
	{
		paszLines->RemoveTail();
	}
	szText.Kill();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertHashLine(CChars* pszLine, const char* szTitle, CChars* pszHash)
{
	CArrayChars	aszParts;

	aszParts.Init();
	pszLine->Split(&aszParts, ' ');
	AssertSize(3, aszParts.NumElements());
	AssertString(szTitle, aszParts.GetText(1));
	AssertInt(16, aszParts.Get(2)->Length());
	pszHash->Init(aszParts.Get(2));
	aszParts.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A line a present that showed something, flushed as it is written.  The
// same presents hash the same.  Hashing what was presented, a present
// of the box hashes other than one of all the window; hashing the
// screen, both are the same picture and hash the same.
//
//////////////////////////////////////////////////////////////////////////
void TestRefWindowScreenHash(void)
{
	CFileUtil		cFileUtil;
	char			szDirectory[] = "Output" _FS_ "ScreenHash";
	char			szPresented[] = "Output" _FS_ "ScreenHash" _FS_ "Presented.txt";
	char			szAgain[] = "Output" _FS_ "ScreenHash" _FS_ "PresentedAgain.txt";
	char			szScreen[] = "Output" _FS_ "ScreenHash" _FS_ "Screen.txt";
	CArrayChars		aszLines;
	CArrayChars		aszAgain;
	CChars			szAll;
	CChars			szBox;
	CChars			szSecondBox;
	size			i;

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	HashWindowPresents(szPresented, WRHA_Presented);
	HashWindowPresents(szAgain, WRHA_Presented);
	HashWindowPresents(szScreen, WRHA_Screen);

	//The first paint shows all of the window; the paint that changed nothing shows nothing.
	ReadHashLines(szPresented, &aszLines);
	AssertSize(3, aszLines.NumElements());
	AssertHashLine(aszLines.Get(0), "[Hashed]", &szAll);
	AssertHashLine(aszLines.Get(1), "[Hashed]", &szBox);
	AssertHashLine(aszLines.Get(2), "[Hashed]", &szSecondBox);
	AssertFalse(szAll.Equals(&szBox));
	AssertString(szBox.Text(), szSecondBox.Text());
	szSecondBox.Kill();
	szBox.Kill();
	szAll.Kill();

	ReadHashLines(szAgain, &aszAgain);
	AssertSize(aszLines.NumElements(), aszAgain.NumElements());
	for (i = 0; i < aszLines.NumElements(); i++)
	{
		AssertString(aszLines.GetText(i), aszAgain.GetText(i));
	}
	aszAgain.Kill();
	aszLines.Kill();

	ReadHashLines(szScreen, &aszLines);
	AssertSize(3, aszLines.NumElements());
	AssertHashLine(aszLines.Get(0), "[Hashed]", &szAll);
	AssertHashLine(aszLines.Get(1), "[Hashed]", &szBox);
	AssertString(szAll.Text(), szBox.Text());
	szBox.Kill();
	szAll.Kill();
	aszLines.Kill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
// The screen's pixel at x, y is the window's at its place in it.
//
//////////////////////////////////////////////////////////////////////////
static void AssertScreenShows(CWinRefScreen* pcScreen, int32 x, int32 y, CWinRefWindow* pcWindow, int32 iWindowX, int32 iWindowY)
{
	CWinRefPixels*	pcShown;

	pcScreen->Compose();
	pcShown = pcWindow->GetShown();
	AssertSize(pcShown->GetPixelBytes(), pcScreen->GetPixels()->GetPixelBytes());
	AssertInt(0, memcmp(pcScreen->GetPixels()->GetPixel(x, y), pcShown->GetPixel(iWindowX, iWindowY), pcShown->GetPixelBytes()));
}


//////////////////////////////////////////////////////////////////////////
//
// A tool window floats over its owner on the screen, even when its owner
// shows all of itself again, until the owner is raised over it.  A
// window closed is taken off the screen.
//
//////////////////////////////////////////////////////////////////////////
void TestRefWindowScreenStacking(void)
{
	CWinRefWindowFactory	cFactory;
	CInt32Rect				cScreen;
	CInt32Rect				cTool;

	ObjectsInit();
	cFactory.Init(&gcMemoryAllocator, 64, 48, NULL);
	cScreen.Init(0, 0, 64, 48);
	cFactory.SetScreenArea(&cScreen);
	cFactory.SetCheckPresents(true);
	{
		SScreenWindow	sMain;
		Ptr<CWindow>	pTool;
		CPointer		pNull;
		CWinRefWindow*	pcTool;

		sMain.Init("Main", &cFactory, Set32BitColour(1.0f, 0, 0), 1);
		pTool = OMalloc<CWindow>("Tool", &cFactory, pNull, OMalloc<CClearCanvasDraw>(Set32BitColour(0, 1.0f, 0)), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTool);
		cTool.Init(30, 20, 50, 35);
		AssertTrue(pTool->ShowOwned(sMain.pWindow, &cTool));
		AssertTrue(sMain.pWindow->Show());
		pcTool = (CWinRefWindow*)pTool->GetNativeWindow();

		AssertScreenShows(cFactory.GetScreen(), 2, 30, sMain.GetNative(), 2, 30);
		AssertScreenShows(cFactory.GetScreen(), 35, 25, pcTool, 5, 5);

		sMain.pWindow->Invalidate();
		sMain.pWindow->Paint();
		AssertScreenShows(cFactory.GetScreen(), 35, 25, pcTool, 5, 5);

		sMain.pWindow->Raise();
		AssertScreenShows(cFactory.GetScreen(), 35, 25, sMain.GetNative(), 35, 25);

		pTool->Raise();
		AssertScreenShows(cFactory.GetScreen(), 35, 25, pcTool, 5, 5);
		pTool->CloseOwned();
		AssertScreenShows(cFactory.GetScreen(), 35, 25, sMain.GetNative(), 35, 25);
		AssertSize(0, cFactory.GetScreen()->NumMismatches());

		ORoot()->Remove(pTool);
		pTool = NULL;
		sMain.Kill();
	}
	AssertTrue(ORoot()->RemoveAll());
	cFactory.Kill();
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// How long a window shown ticking iTicks times takes, in milliseconds.
//
//////////////////////////////////////////////////////////////////////////
static int64 TimeShow(CWinRefWindowFactory* pcFactory, int32 iTicks)
{
	int64	iStart;

	ObjectsInit();
	{
		SScreenWindow	sWindow;

		sWindow.Init("Waiting", pcFactory, Set32BitColour(0, 0, 1.0f), iTicks);
		iStart = GetDebugTickCount();
		AssertTrue(sWindow.pWindow->Show());
		iStart = GetDebugTickCount() - iStart;
		AssertInt(iTicks, sWindow.cData.iTick);
		sWindow.Kill();
	}
	AssertTrue(ORoot()->RemoveAll());
	ObjectsFlush();
	ObjectsKill();
	return iStart;
}


//////////////////////////////////////////////////////////////////////////
//
// Waiting for events, a window with nothing due sleeps no longer than
// the longest wait between ticks; it never waits for ever.  Not waiting,
// it ticks as fast as it can.
//
//////////////////////////////////////////////////////////////////////////
void TestRefWindowScreenWait(void)
{
	CWinRefWindowFactory	cFactory;
	int64					iTime;

	cFactory.Init(&gcMemoryAllocator, 64, 48, NULL);
	AssertLong(-1, cFactory.GetMaxWait());

	cFactory.SetWaitForEvents(true, 20);
	AssertLong(20, cFactory.GetMaxWait());
	iTime = TimeShow(&cFactory, 5);
	AssertTrue(iTime >= 4 * 20);
	AssertTrue(iTime < 2000);

	cFactory.SetWaitForEvents(false);
	AssertLong(-1, cFactory.GetMaxWait());
	iTime = TimeShow(&cFactory, 5);
	AssertTrue(iTime < 4 * 20);

	cFactory.SetWaitForEvents(true);
	AssertLong(WIN_REF_MAX_WAIT, cFactory.GetMaxWait());
	cFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// With an image path, every interval'th frame is saved.
//
//////////////////////////////////////////////////////////////////////////
void TestRefWindowScreenFrameInterval(void)
{
	CWinRefWindowFactory	cFactory;

	cFactory.Init(&gcMemoryAllocator, 64, 48, NULL);
	AssertTrue(cFactory.IsFrameSaved(0));
	AssertTrue(cFactory.IsFrameSaved(1));
	cFactory.SetFrameInterval(3);
	AssertTrue(cFactory.IsFrameSaved(0));
	AssertFalse(cFactory.IsFrameSaved(1));
	AssertFalse(cFactory.IsFrameSaved(2));
	AssertTrue(cFactory.IsFrameSaved(3));
	cFactory.SetFrameInterval(0);
	AssertTrue(cFactory.IsFrameSaved(1));
	cFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRefWindowScreen(void)
{
	BeginTests();

	TestRefWindowScreenCheck();
	TestRefWindowScreenHash();
	TestRefWindowScreenStacking();
	TestRefWindowScreenWait();
	TestRefWindowScreenFrameInterval();

	TestStatistics();
}

