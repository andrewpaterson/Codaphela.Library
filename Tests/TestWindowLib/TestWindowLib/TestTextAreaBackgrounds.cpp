#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/TextArea.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/AssertFile.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"


Ptr<CFontFamily> CreateMonoTextAreaFamily(float32 fDefaultPixelSize);
void AssertLabelObjectsRemoved(void);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertTextAreaBackground(ETextAreaBackground eType, size uiStart, size uiEnd, STextAreaBackground* psBackground)
{
	AssertNotNull(psBackground);
	AssertInt(eType, psBackground->eType);
	AssertSize(uiStart, psBackground->uiStart);
	AssertSize(uiEnd, psBackground->uiEnd);
}


//////////////////////////////////////////////////////////////////////////
//
// Draws a tiled text area of szText in Cascadia Mono with the backgrounds
// fAddBackgrounds adds, and compares the frame with the expected one.
//
//////////////////////////////////////////////////////////////////////////
void AssertTextAreaBackgroundsDrawn(char* szDirectory, const char* szText, void(*fAddBackgrounds)(Ptr<CTextArea> pTextArea))
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);
	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CFillContainer>		pFill;
		Ptr<CClearCanvasDraw>	pBackground;
		Ptr<CTextArea>			pTextArea;
		Ptr<CTickTestRefWindow>	pTick;
		SDataTestRefWindow		cData;
		CChars					szExpectedDirectory;

		cNativeFactory.Init(&gcMemoryAllocator, 320, 140, szDirectory);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pBackground = OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)30, 31, 34));
		pTestWindow = OMalloc<CWindow>("Text Area Window", &cNativeFactory, pTick, pBackground, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);

		pTextArea = OMalloc<CTextArea>(pTestWindow, CreateMonoTextAreaFamily(15.0f), szText);
		pTextArea->SetTiledText(true);
		pTextArea->SetColour(Set32BitColour((uint8)0xBC, (uint8)0xBE, (uint8)0xC4));
		fAddBackgrounds(pTextArea);
		pFill->AddComponent(pTextArea);

		pTestWindow->Show();

		pTextArea = NULL;
		pFill = NULL;
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
// Line backgrounds that skip a gap, ranges across lines, boundaries above
// a line and below the last, and the selection over them.
//
//////////////////////////////////////////////////////////////////////////
void AddCodeBackgrounds(Ptr<CTextArea> pTextArea)
{
	AssertTrue(pTextArea->SetLineGap(1, pTextArea->GetFont()->GetLineHeight()));
	AssertTrue(pTextArea->AddLineBackground(0, 3, Set32BitColour((uint8)0x29, (uint8)0x44, (uint8)0x36)));
	AssertTrue(pTextArea->AddBackground(13, 21, Set32BitColour((uint8)0x38, (uint8)0x6B, (uint8)0x4E)));		//aiValues
	AssertTrue(pTextArea->AddBackground(41, 67, Set32BitColour((uint8)0x5A, (uint8)0x3E, (uint8)0x8C)));		//int iTotal = 0; to return
	AssertTrue(pTextArea->AddLineBoundary(4, 2, Set32BitColour((uint8)0x80, (uint8)0x80, (uint8)0x80)));
	AssertTrue(pTextArea->AddLineBoundary(5, 2, Set32BitColour((uint8)0xC0, (uint8)0x40, (uint8)0x40)));
	AssertSize(5, pTextArea->NumBackgrounds());
	pTextArea->Select(4, 7);		//Sum
}


//////////////////////////////////////////////////////////////////////////
//
// Text with no characters still has a line: a background over it and
// boundaries above and below it.  Stretched, the text area is as wide as
// the window; otherwise it would have no width.
//
//////////////////////////////////////////////////////////////////////////
void AddEmptyBackgrounds(Ptr<CTextArea> pTextArea)
{
	pTextArea->SetAlignment(CSH_Stretch, CSV_Top);
	AssertTrue(pTextArea->AddLineBackground(0, 1, Set32BitColour((uint8)0x29, (uint8)0x44, (uint8)0x36)));
	AssertTrue(pTextArea->AddLineBoundary(0, 2, Set32BitColour((uint8)0x80, (uint8)0x80, (uint8)0x80)));
	AssertTrue(pTextArea->AddLineBoundary(1, 2, Set32BitColour((uint8)0xC0, (uint8)0x40, (uint8)0x40)));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaBackgroundsDraw(void)
{
	char	szCode[] = "Output" _FS_ "TextAreaBackgrounds";
	char	szEmpty[] = "Output" _FS_ "TextAreaBackgroundsEmpty";

	AssertTextAreaBackgroundsDrawn(szCode, "int Sum(int* aiValues, int iCount)\n{\n    int iTotal = 0;\n    return iTotal;\n}", AddCodeBackgrounds);
	AssertTextAreaBackgroundsDrawn(szEmpty, "", AddEmptyBackgrounds);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaBackgroundsAdd(void)
{
	CWinRefWindowFactory	cNativeFactory;

	cNativeFactory.Init(&gcMemoryAllocator, 200, 100, NULL);
	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CTextArea>			pTextArea;
		Ptr<CTickTestRefWindow>	pTick;
		SDataTestRefWindow		cData;
		CPointer				pNull;

		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Text Area Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pTextArea = OMalloc<CTextArea>(pTestWindow, CreateMonoTextAreaFamily(15.0f), "aaa\nbbb");
		pTestWindow->SetContainer(pTextArea);

		//Out of the text is refused; nothing at all is accepted and ignored.
		AssertFalse(pTextArea->AddBackground(5, 4, 0));
		AssertFalse(pTextArea->AddBackground(0, 8, 0));
		AssertTrue(pTextArea->AddBackground(2, 2, 0));
		AssertFalse(pTextArea->AddLineBackground(1, 0, 0));
		AssertFalse(pTextArea->AddLineBackground(0, 3, 0));
		AssertTrue(pTextArea->AddLineBackground(1, 1, 0));
		AssertFalse(pTextArea->AddLineBoundary(3, 1, 0));
		AssertFalse(pTextArea->AddLineBoundary(0, 0, 0));
		AssertSize(0, pTextArea->NumBackgrounds());

		AssertTrue(pTextArea->AddBackground(0, 7, 0));
		AssertTrue(pTextArea->AddLineBackground(0, 2, 0));
		AssertTrue(pTextArea->AddLineBoundary(2, 1, 0));
		AssertSize(3, pTextArea->NumBackgrounds());
		AssertTextAreaBackground(TAB_Range, 0, 7, pTextArea->GetBackground(0));
		AssertTextAreaBackground(TAB_Lines, 0, 2, pTextArea->GetBackground(1));
		AssertTextAreaBackground(TAB_Boundary, 2, 2, pTextArea->GetBackground(2));
		AssertNull(pTextArea->GetBackground(3));
		pTestWindow->Layout();

		pTextArea->ClearBackgrounds();
		AssertSize(0, pTextArea->NumBackgrounds());

		//Setting the text removes them.
		AssertTrue(pTextArea->AddBackground(0, 1, 0));
		pTextArea->SetText("ccc");
		AssertSize(0, pTextArea->NumBackgrounds());

		//Text with no characters has a line.
		pTextArea->SetText("");
		AssertTrue(pTextArea->AddLineBackground(0, 1, 0));
		AssertTrue(pTextArea->AddLineBoundary(1, 1, 0));
		AssertFalse(pTextArea->AddLineBoundary(2, 1, 0));

		pTextArea = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
	}
	AssertLabelObjectsRemoved();
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Ranges move with the text and grow with text put strictly inside them.
// Line backgrounds and boundaries stay with their lines as gaps do.
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaBackgroundsEdit(void)
{
	CWinRefWindowFactory	cNativeFactory;

	cNativeFactory.Init(&gcMemoryAllocator, 200, 100, NULL);
	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CTextArea>			pTextArea;
		Ptr<CTickTestRefWindow>	pTick;
		SDataTestRefWindow		cData;
		CPointer				pNull;

		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Text Area Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		//Lines: aaa 0-2, bbb 4-6, ccc 8-10, ddd 12-14.
		pTextArea = OMalloc<CTextArea>(pTestWindow, CreateMonoTextAreaFamily(15.0f), "aaa\nbbb\nccc\nddd");
		pTestWindow->SetContainer(pTextArea);
		AssertTrue(pTextArea->AddBackground(5, 10, 0));
		AssertTrue(pTextArea->AddLineBackground(1, 3, 0));
		AssertTrue(pTextArea->AddLineBoundary(2, 1, 0));

		//Inside the range it grows; no lines are put in.
		pTextArea->SetCaretPosition(6);
		pTextArea->Paste("XY");
		AssertString("aaa\nbbXYb\nccc\nddd", pTextArea->GetText());
		AssertTextAreaBackground(TAB_Range, 5, 12, pTextArea->GetBackground(0));
		AssertTextAreaBackground(TAB_Lines, 1, 3, pTextArea->GetBackground(1));
		AssertTextAreaBackground(TAB_Boundary, 2, 2, pTextArea->GetBackground(2));

		//At the range's start and end it doesn't.
		pTextArea->SetCaretPosition(5);
		pTextArea->Paste("<");
		pTextArea->SetCaretPosition(13);
		pTextArea->Paste(">");
		AssertString("aaa\nb<bXYb\ncc>c\nddd", pTextArea->GetText());
		AssertTextAreaBackground(TAB_Range, 6, 13, pTextArea->GetBackground(0));

		//A line put in at the start of line 1 pushes the lines down.
		pTextArea->SetCaretPosition(4);
		pTextArea->Paste("Z\n");
		AssertString("aaa\nZ\nb<bXYb\ncc>c\nddd", pTextArea->GetText());
		AssertTextAreaBackground(TAB_Range, 8, 15, pTextArea->GetBackground(0));
		AssertTextAreaBackground(TAB_Lines, 2, 4, pTextArea->GetBackground(1));
		AssertTextAreaBackground(TAB_Boundary, 3, 3, pTextArea->GetBackground(2));

		//A line put in inside the line background grows it.
		pTextArea->SetCaretPosition(9);
		pTextArea->Paste("\n");
		AssertString("aaa\nZ\nb<b\nXYb\ncc>c\nddd", pTextArea->GetText());
		AssertTextAreaBackground(TAB_Range, 8, 16, pTextArea->GetBackground(0));
		AssertTextAreaBackground(TAB_Lines, 2, 5, pTextArea->GetBackground(1));
		AssertTextAreaBackground(TAB_Boundary, 4, 4, pTextArea->GetBackground(2));

		//Deleting line 1 whole moves everything back up.
		pTextArea->Select(4, 6);
		pTextArea->Delete();
		AssertString("aaa\nb<b\nXYb\ncc>c\nddd", pTextArea->GetText());
		AssertTextAreaBackground(TAB_Range, 6, 14, pTextArea->GetBackground(0));
		AssertTextAreaBackground(TAB_Lines, 1, 4, pTextArea->GetBackground(1));
		AssertTextAreaBackground(TAB_Boundary, 3, 3, pTextArea->GetBackground(2));

		//Joining line 2 onto line 1 shrinks the line background, and the boundary above line 3 stays above it.
		pTextArea->SetCaretPosition(7);
		pTextArea->Delete();
		AssertString("aaa\nb<bXYb\ncc>c\nddd", pTextArea->GetText());
		AssertTextAreaBackground(TAB_Range, 6, 13, pTextArea->GetBackground(0));
		AssertTextAreaBackground(TAB_Lines, 1, 3, pTextArea->GetBackground(1));
		AssertTextAreaBackground(TAB_Boundary, 2, 2, pTextArea->GetBackground(2));

		//Deleting the lines a line background is on removes it, and the range in them goes too.
		pTextArea->Select(4, 16);
		pTextArea->Delete();
		AssertString("aaa\nddd", pTextArea->GetText());
		AssertSize(1, pTextArea->NumBackgrounds());
		AssertTextAreaBackground(TAB_Boundary, 1, 1, pTextArea->GetBackground(0));

		//A boundary below the last line moves down with lines put in at the end.
		AssertTrue(pTextArea->AddLineBoundary(2, 1, 0));
		pTextArea->SetCaretPosition(7);
		pTextArea->Paste("\neee");
		AssertTextAreaBackground(TAB_Boundary, 1, 1, pTextArea->GetBackground(0));
		AssertTextAreaBackground(TAB_Boundary, 3, 3, pTextArea->GetBackground(1));
		pTestWindow->Layout();

		pTextArea = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
	}
	AssertLabelObjectsRemoved();
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaBackgrounds(void)
{
	BeginTests();
	ObjectsInit();

	TestTextAreaBackgroundsDraw();
	TestTextAreaBackgroundsAdd();
	TestTextAreaBackgroundsEdit();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}
