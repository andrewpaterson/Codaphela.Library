#include "BaseLib/GlobalMemory.h"
#include "BaseLib/Timer.h"
#include "BaseLib/FileUtil.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WindowLib/GridContainer.h"
#include "WindowLib/TextArea.h"
#include "WindowLib/JewelColours.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"


Ptr<CFont>		GenerateMenuFont(Ptr<CWindow> pWindow);


//////////////////////////////////////////////////////////////////////////
//
// The window's pixel at x, y as 0xRRGGBB.  The window is 24 bit RGB.
//
//////////////////////////////////////////////////////////////////////////
static uint32 GetWindowPixel(Ptr<CWindow> pWindow, int x, int y)
{
	uint8*	puiPixel;

	puiPixel = &pWindow->GetCanvas()->GetPixelData()[(y * pWindow->GetCanvas()->GetActualSize().x + x) * 3];
	return ((uint32)puiPixel[0] << 16) | ((uint32)puiPixel[1] << 8) | (uint32)puiPixel[2];
}


//////////////////////////////////////////////////////////////////////////
//
// A focussed text area's caret is drawn by the window over what it
// paints, shown and hidden every period by changing only its pixels,
// shown again when the text is edited or the caret moves.  Without focus,
// or with a period of 0, it doesn't blink.
//
//////////////////////////////////////////////////////////////////////////
void TestCaretBlinkTextArea(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	const char*				szDirectory;

	//The reference windows save each frame they present.
	szDirectory = "Output" _FS_ "CaretBlink";
	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	{
		Ptr<CWindow>		pTestWindow;
		Ptr<CGridContainer>	pGrid;
		Ptr<CTextArea>		pText;
		Ptr<CFocus>			pFocus;
		CPointer			pNull;
		CInt32Rect			cCaret;
		CInt32Rect			cMoved;
		int64				iNow;
		uint32				uiCaret;

		uiCaret = 0xCED0D6;
		cNativeFactory.Init(&gcMemoryAllocator, 120, 40, (char*)szDirectory);
		pTestWindow = OMalloc<CWindow>("Caret Window", &cNativeFactory, pNull, OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)0, (uint8)0, (uint8)0)), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFocus = pTestWindow->GetFocus();
		pText = OMalloc<CTextArea>(pTestWindow, GenerateMenuFont(pTestWindow), "Hello");
		pText->SetEditable(true);
		pText->SetAlignment(CSH_Stretch, CSV_Stretch);
		pGrid = OMalloc<CGridContainer>(pTestWindow);
		pGrid->AddComponent(pText, 0, 0, 1, 1, 1, 1);
		pTestWindow->SetContainer(pGrid);
		pTestWindow->Layout();

		//Without focus there's no caret and nothing to wake up for.
		pTestWindow->Paint();
		AssertFalse(pTestWindow->GetPaintedCaret(&cCaret));
		AssertTrue(pFocus->GetCaretBlinkWait(GetDebugTickCount()) < 0);

		//Focussed, it is drawn shown, one pixel wide at the start of the text.
		pFocus->SetFocus(pText);
		pText->SetCaretPosition(0);
		pTestWindow->Paint();
		AssertTrue(pTestWindow->GetPaintedCaret(&cCaret));
		AssertInt(1, cCaret.GetWidth());
		AssertTrue(cCaret.GetHeight() > 8);
		AssertTrue(pFocus->IsCaretOn());
		AssertInt(uiCaret, GetWindowPixel(pTestWindow, cCaret.GetLeft(), cCaret.GetTop() + 2));

		//It blinks every period: hidden puts back what was under it, without painting the window.
		iNow = GetDebugTickCount();
		AssertTrue(pFocus->GetCaretBlinkWait(iNow) > 0);
		AssertTrue(pFocus->GetCaretBlinkWait(iNow) <= FOCUS_CARET_BLINK_PERIOD);
		AssertFalse(pTestWindow->BlinkCaret(iNow + FOCUS_CARET_BLINK_PERIOD - 50));
		AssertTrue(pTestWindow->BlinkCaret(iNow + FOCUS_CARET_BLINK_PERIOD + 1));
		AssertFalse(pFocus->IsCaretOn());
		AssertInt(0, GetWindowPixel(pTestWindow, cCaret.GetLeft(), cCaret.GetTop() + 2));
		AssertTrue(pTestWindow->BlinkCaret(iNow + 2 * FOCUS_CARET_BLINK_PERIOD + 2));
		AssertTrue(pFocus->IsCaretOn());
		AssertInt(uiCaret, GetWindowPixel(pTestWindow, cCaret.GetLeft(), cCaret.GetTop() + 2));

		//Hidden, then typed into: it is shown again where it moved to.
		AssertTrue(pTestWindow->BlinkCaret(iNow + 3 * FOCUS_CARET_BLINK_PERIOD + 3));
		AssertFalse(pFocus->IsCaretOn());
		pTestWindow->InputPrintable('X', true);
		AssertTrue(pFocus->IsCaretOn());
		pTestWindow->Paint();
		AssertTrue(pTestWindow->GetPaintedCaret(&cMoved));
		AssertTrue(cMoved.GetLeft() > cCaret.GetLeft());
		AssertInt(uiCaret, GetWindowPixel(pTestWindow, cMoved.GetLeft(), cMoved.GetTop() + 2));

		//Moving the caret shows it too.
		AssertTrue(pTestWindow->BlinkCaret(GetDebugTickCount() + FOCUS_CARET_BLINK_PERIOD + 1));
		AssertFalse(pFocus->IsCaretOn());
		pText->SetCaretPosition(0);
		pTestWindow->Paint();
		AssertTrue(pFocus->IsCaretOn());

		//A period of 0 keeps it shown.
		pFocus->SetCaretBlinkPeriod(0);
		AssertTrue(pFocus->GetCaretBlinkWait(GetDebugTickCount()) < 0);
		AssertFalse(pTestWindow->BlinkCaret(GetDebugTickCount() + 10000));
		AssertTrue(pFocus->IsCaretOn());
		pFocus->SetCaretBlinkPeriod(FOCUS_CARET_BLINK_PERIOD);

		//An inactive window has no caret.
		pFocus->Deactivate();
		pTestWindow->Paint();
		AssertFalse(pTestWindow->GetPaintedCaret(&cCaret));
		AssertTrue(pFocus->GetCaretBlinkWait(GetDebugTickCount()) < 0);
		pFocus->Activate();
		pTestWindow->Paint();
		AssertTrue(pTestWindow->GetPaintedCaret(&cCaret));
		AssertTrue(pFocus->IsCaretOn());

		pFocus = NULL;
		pText = NULL;
		pGrid = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	AssertSize(2, gcObjects.NumMemoryIndexes());
	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCaretBlink(void)
{
	BeginTests();
	ObjectsInit();

	TestCaretBlinkTextArea();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}
