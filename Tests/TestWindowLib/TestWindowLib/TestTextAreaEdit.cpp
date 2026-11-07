#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Objects.h"
#include "SupportLib/GlyphListFontDraw.h"
#include "WindowLib/Window.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/ScrollPane.h"
#include "WindowLib/JewelTheme.h"
#include "WindowLib/TextArea.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"


Ptr<CWindowTheme> CreateTestTheme(Ptr<CFont> pFont);


Ptr<CFont> ReadLabelFont(void);
int32 TextAreaWidth(Ptr<CFont> pFont, const char* szText);
Ptr<CFontFamily> CreateMonoTextAreaFamily(float32 fDefaultPixelSize);
void LayoutTextAreaText(Ptr<CTextLayout> pLayout, Ptr<CGlyphListFontDraw> pDraw, Ptr<CFont> pFont, const char* szText);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertTextAreaSelection(size uiExpectedStart, size uiExpectedEnd, Ptr<CTextArea> pTextArea)
{
	size	uiStart;
	size	uiEnd;

	AssertTrue(pTextArea->GetSelection(&uiStart, &uiEnd));
	AssertSize(uiExpectedStart, uiStart);
	AssertSize(uiExpectedEnd, uiEnd);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertTextAreaNoSelection(Ptr<CTextArea> pTextArea)
{
	size	uiStart;
	size	uiEnd;

	AssertFalse(pTextArea->GetSelection(&uiStart, &uiEnd));
}


//////////////////////////////////////////////////////////////////////////
//
// A layout has a stop for each character, line feeds too, and one after
// the last, and knows its lines' stops.
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaEditLayoutStops(void)
{
	{
		Ptr<CFont>					pFont;
		Ptr<CTextLayout>			pLayout;
		Ptr<CGlyphListFontDraw>		pDraw;
		STextLayoutStop*			psStop;
		STextLayoutLine*			psLine;

		pFont = ReadLabelFont();
		pLayout = OMalloc<CTextLayout>();
		pDraw = OMalloc<CGlyphListFontDraw>();

		LayoutTextAreaText(pLayout, pDraw, pFont, "ab\ncd");
		AssertSize(6, pLayout->NumStops());
		AssertSize(2, pLayout->NumLines());
		psStop = pLayout->GetStop(1);
		AssertInt(TextAreaWidth(pFont, "a"), psStop->x);
		AssertInt(TextAreaWidth(pFont, "b"), psStop->iWidth);
		psStop = pLayout->GetStop(2);
		AssertInt(TextAreaWidth(pFont, "ab"), psStop->x);
		AssertInt(0, psStop->iWidth);
		AssertSize(0, psStop->uiLine);
		psStop = pLayout->GetStop(3);
		AssertInt(0, psStop->x);
		AssertSize(1, psStop->uiLine);
		psStop = pLayout->GetStop(5);
		AssertInt(TextAreaWidth(pFont, "cd"), psStop->x);
		AssertSize(1, psStop->uiLine);
		psLine = pLayout->GetLine(0);
		AssertInt(0, psLine->iTop);
		AssertInt(24, psLine->iHeight);
		AssertSize(0, psLine->uiFirstStop);
		AssertSize(3, psLine->uiNumStops);
		psLine = pLayout->GetLine(1);
		AssertInt(24, psLine->iTop);
		AssertSize(3, psLine->uiFirstStop);
		AssertSize(3, psLine->uiNumStops);

		//The space before a wrapped word stays on the line above.
		pLayout->SetWidth(TextAreaWidth(pFont, "aaa bbb"));
		LayoutTextAreaText(pLayout, pDraw, pFont, "aaa bbb ccc");
		AssertSize(12, pLayout->NumStops());
		AssertSize(2, pLayout->NumLines());
		psStop = pLayout->GetStop(7);
		AssertInt(TextAreaWidth(pFont, "aaa bbb"), psStop->x);
		AssertSize(0, psStop->uiLine);
		psStop = pLayout->GetStop(8);
		AssertInt(0, psStop->x);
		AssertSize(1, psStop->uiLine);
		AssertSize(8, pLayout->GetLine(0)->uiNumStops);
		AssertSize(4, pLayout->GetLine(1)->uiNumStops);

		//An empty text still has a line and a stop.
		LayoutTextAreaText(pLayout, pDraw, pFont, "");
		AssertSize(1, pLayout->NumStops());
		AssertSize(1, pLayout->NumLines());

		pDraw = NULL;
		pLayout = NULL;
		pFont = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The caret moves by characters, words, lines and rows.  Up and down keep
// the caret's x.
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaEditNavigation(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CFillContainer>			pFill;
		Ptr<CFont>					pFont;
		Ptr<CTextArea>				pTextArea;

		cNativeFactory.Init(&gcMemoryAllocator, 300, 200, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Text Area Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pFont = ReadLabelFont();
		pTextArea = OMalloc<CTextArea>(pTestWindow, CreateTestTheme(Ptr<CFont>()), pFont, "  abc def\nxy");
		pTextArea->SetAlignment(CSH_Stretch, CSV_Stretch);
		pFill->AddComponent(pTextArea);
		pTestWindow->Layout();

		AssertSize(0, pTextArea->GetCaretPosition());
		pTextArea->Right();
		AssertSize(1, pTextArea->GetCaretPosition());
		pTextArea->End();
		AssertSize(9, pTextArea->GetCaretPosition());

		//Home goes to the first character that isn't a space, then the start of the line, and back.
		pTextArea->Home();
		AssertSize(2, pTextArea->GetCaretPosition());
		pTextArea->Home();
		AssertSize(0, pTextArea->GetCaretPosition());
		pTextArea->Home();
		AssertSize(2, pTextArea->GetCaretPosition());

		pTextArea->WordRight();
		AssertSize(5, pTextArea->GetCaretPosition());
		pTextArea->WordRight();
		AssertSize(6, pTextArea->GetCaretPosition());
		pTextArea->WordLeft();
		AssertSize(5, pTextArea->GetCaretPosition());
		pTextArea->WordLeft();
		AssertSize(2, pTextArea->GetCaretPosition());
		pTextArea->DocumentEnd();
		AssertSize(12, pTextArea->GetCaretPosition());
		pTextArea->DocumentHome();
		AssertSize(0, pTextArea->GetCaretPosition());

		//Down onto a shorter line goes to its end; up goes back to the same x.
		pTextArea->SetCaretPosition(5);
		pTextArea->Down();
		AssertSize(12, pTextArea->GetCaretPosition());
		pTextArea->Up();
		AssertSize(5, pTextArea->GetCaretPosition());
		pTextArea->Up();
		AssertSize(0, pTextArea->GetCaretPosition());
		pTextArea->Down();
		pTextArea->Down();
		AssertSize(12, pTextArea->GetCaretPosition());

		//Characters of UTF8 are stepped over whole.
		pTextArea->SetText("a\xC3\xA9\xE2\x82\xAC" "b");
		pTextArea->Right();
		AssertSize(1, pTextArea->GetCaretPosition());
		pTextArea->Right();
		AssertSize(3, pTextArea->GetCaretPosition());
		pTextArea->Right();
		AssertSize(6, pTextArea->GetCaretPosition());
		pTextArea->Right();
		AssertSize(7, pTextArea->GetCaretPosition());
		pTextArea->Right();
		AssertSize(7, pTextArea->GetCaretPosition());
		pTextArea->Left();
		pTextArea->Left();
		AssertSize(3, pTextArea->GetCaretPosition());
		pTextArea->Left();
		AssertSize(1, pTextArea->GetCaretPosition());
		pTextArea->SetCaretPosition(6);
		pTextArea->Backspace();
		AssertString("a\xC3\xA9" "b", pTextArea->GetText());
		AssertSize(3, pTextArea->GetCaretPosition());

		//A carriage return and line feed are one character, and the end of the line is before them.
		pTextArea->SetText("ab\r\ncd");
		pTextArea->End();
		AssertSize(2, pTextArea->GetCaretPosition());
		pTextArea->Right();
		AssertSize(4, pTextArea->GetCaretPosition());
		pTextArea->Left();
		AssertSize(2, pTextArea->GetCaretPosition());
		pTextArea->Down();
		AssertSize(6, pTextArea->GetCaretPosition());

		//The pointer finds the nearest gap.
		pTextArea->SetText("ab\ncd");
		pTestWindow->Layout();
		AssertSize(0, pTextArea->FindPositionAt(-5, -5));
		AssertSize(2, pTextArea->FindPositionAt(1000, 5));
		AssertSize(3, pTextArea->FindPositionAt(0, 30));
		AssertSize(5, pTextArea->FindPositionAt(1000, 1000));
		AssertSize(1, pTextArea->FindPositionAt(TextAreaWidth(pFont, "a") + 1, 5));

		pTextArea = NULL;
		pFont = NULL;
		pFill = NULL;
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
// Home, end, up and down work on the rows a wrapped line is in.
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaEditWrapped(void)
{
	CWinRefWindowFactory	cNativeFactory;
	Ptr<CFont>				pFont;
	int32					iWidth;

	pFont = ReadLabelFont();
	iWidth = TextAreaWidth(pFont, "aaa bbb");

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CFillContainer>			pFill;
		Ptr<CTextArea>				pTextArea;
		CInt32Rect					cCaret;

		cNativeFactory.Init(&gcMemoryAllocator, iWidth, 200, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Text Area Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pTextArea = OMalloc<CTextArea>(pTestWindow, CreateTestTheme(Ptr<CFont>()), pFont, "aaa bbb ccc");
		pTextArea->SetWordWrap(true);
		pTextArea->SetAlignment(CSH_Stretch, CSV_Stretch);
		pFill->AddComponent(pTextArea);
		pTestWindow->Layout();
		AssertInt(48, pTextArea->GetTextSize().y);

		//End goes to the end of the row, after its space; the caret is drawn on the row, not the next.
		pTextArea->End();
		AssertSize(8, pTextArea->GetCaretPosition());
		pTextArea->GetCaretBounds(&cCaret);
		AssertInt(0, cCaret.GetTop());
		AssertInt(iWidth - 1, cCaret.GetLeft());

		//Again goes to the end of the line.
		pTextArea->End();
		AssertSize(11, pTextArea->GetCaretPosition());
		pTextArea->GetCaretBounds(&cCaret);
		AssertInt(24, cCaret.GetTop());
		AssertInt(TextAreaWidth(pFont, "ccc"), cCaret.GetLeft());

		//Home goes to the start of the row, then the start of the line.
		pTextArea->Home();
		AssertSize(8, pTextArea->GetCaretPosition());
		pTextArea->GetCaretBounds(&cCaret);
		AssertInt(24, cCaret.GetTop());
		AssertInt(0, cCaret.GetLeft());
		pTextArea->Home();
		AssertSize(0, pTextArea->GetCaretPosition());

		//Up and down go between the rows.
		pTextArea->SetCaretPosition(1);
		pTextArea->Down();
		AssertSize(pTextArea->FindPositionAt(TextAreaWidth(pFont, "a"), 30), pTextArea->GetCaretPosition());
		pTextArea->GetCaretBounds(&cCaret);
		AssertInt(24, cCaret.GetTop());
		pTextArea->Up();
		AssertSize(1, pTextArea->GetCaretPosition());

		//Past the end of a wrapped row is its end.
		AssertSize(8, pTextArea->FindPositionAt(1000, 5));
		AssertSize(8, pTextArea->FindPositionAt(0, 30));

		pTextArea = NULL;
		pFill = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
	}

	pFont = NULL;
	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Typing, deleting, tabs, lines and the clipboard functions.
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaEditModification(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CFillContainer>			pFill;
		Ptr<CTextArea>				pTextArea;
		CChars						sz;

		cNativeFactory.Init(&gcMemoryAllocator, 300, 200, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Text Area Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pTextArea = OMalloc<CTextArea>(pTestWindow, CreateTestTheme(Ptr<CFont>()), ReadLabelFont(), "abc");
		pTextArea->SetAlignment(CSH_Stretch, CSV_Stretch);
		pFill->AddComponent(pTextArea);
		pTestWindow->Layout();

		//Typing inserts, or overwrites.
		pTextArea->DocumentEnd();
		pTextArea->Printable('d', true);
		AssertString("abcd", pTextArea->GetText());
		pTextArea->Left();
		pTextArea->Printable('X', false);
		AssertString("abcX", pTextArea->GetText());
		AssertSize(4, pTextArea->GetCaretPosition());
		pTextArea->Printable('Y', false);
		AssertString("abcXY", pTextArea->GetText());

		//Tab inserts spaces to the next stop.
		pTextArea->Enter();
		AssertString("abcXY\n", pTextArea->GetText());
		pTextArea->Tab(true);
		pTextArea->Printable('y', true);
		pTextArea->Tab(true);
		AssertString("abcXY\n    y   ", pTextArea->GetText());
		AssertSize(14, pTextArea->GetCaretPosition());

		//Tab indents the lines of a selection over more than one, and back tab removes it.
		pTextArea->SetText("one\ntwo\nthree");
		pTextArea->Select(0, 6);
		pTextArea->Tab(true);
		AssertString("    one\n    two\nthree", pTextArea->GetText());
		AssertTextAreaSelection(0, 14, pTextArea);
		pTextArea->BackTab();
		AssertString("one\ntwo\nthree", pTextArea->GetText());
		AssertTextAreaSelection(0, 6, pTextArea);
		pTextArea->Select(4, 8);
		pTextArea->Tab(true);
		AssertString("one\n    two\nthree", pTextArea->GetText());
		pTextArea->SetCaretPosition(5);
		pTextArea->BackTab();
		AssertString("one\ntwo\nthree", pTextArea->GetText());

		//A selection is replaced.
		pTextArea->Select(4, 7);
		pTextArea->Printable('2', true);
		AssertString("one\n2\nthree", pTextArea->GetText());
		AssertTextAreaNoSelection(pTextArea);

		//Pasted carriage returns become line feeds.
		pTextArea->SetText("xy");
		pTextArea->SetCaretPosition(1);
		pTextArea->Paste((char*)"a\r\nb\rc");
		AssertString("xa\nb\ncy", pTextArea->GetText());
		AssertSize(6, pTextArea->GetCaretPosition());

		//Copy and cut the selection.
		pTextArea->SetText("hello world");
		pTextArea->Select(0, 5);
		sz.Init();
		pTextArea->Copy(&sz);
		AssertString("hello", sz.Text());
		sz.Kill();
		sz.Init();
		pTextArea->Cut(&sz);
		AssertString("hello", sz.Text());
		AssertString(" world", pTextArea->GetText());
		AssertSize(0, pTextArea->GetCaretPosition());
		sz.Kill();

		//Without a selection copy does nothing and cut cuts the line.
		pTextArea->SetText("l1\nl2\nl3");
		pTextArea->SetCaretPosition(4);
		sz.Init();
		pTextArea->Copy(&sz);
		AssertString("", sz.Text());
		pTextArea->Cut(&sz);
		AssertString("l2\n", sz.Text());
		AssertString("l1\nl3", pTextArea->GetText());
		AssertSize(4, pTextArea->GetCaretPosition());
		sz.Kill();

		//Duplicate a line or the selection.
		pTextArea->SetText("ab\ncd");
		pTextArea->SetCaretPosition(1);
		pTextArea->Duplicate();
		AssertString("ab\nab\ncd", pTextArea->GetText());
		AssertSize(4, pTextArea->GetCaretPosition());
		pTextArea->Select(6, 8);
		pTextArea->Duplicate();
		AssertString("ab\nab\ncdcd", pTextArea->GetText());
		AssertTextAreaSelection(8, 10, pTextArea);

		//Delete lines.
		pTextArea->SetText("l1\nl2\nl3");
		pTextArea->SetCaretPosition(4);
		pTextArea->DeleteLine();
		AssertString("l1\nl3", pTextArea->GetText());
		AssertSize(4, pTextArea->GetCaretPosition());
		pTextArea->DocumentEnd();
		pTextArea->DeleteLine();
		AssertString("l1", pTextArea->GetText());
		AssertSize(2, pTextArea->GetCaretPosition());
		pTextArea->SetText("l1\nl2\nl3");
		pTextArea->Select(1, 6);
		pTextArea->DeleteLine();
		AssertString("l3", pTextArea->GetText());

		//Delete the rest of a word.
		pTextArea->SetText("foo bar");
		pTextArea->DocumentEnd();
		pTextArea->DeleteWordRemainingLeft();
		AssertString("foo ", pTextArea->GetText());
		AssertSize(4, pTextArea->GetCaretPosition());
		pTextArea->DocumentHome();
		pTextArea->DeleteWordRemainingRight();
		AssertString(" ", pTextArea->GetText());
		AssertSize(0, pTextArea->GetCaretPosition());

		//New lines before and after the caret's line.
		pTextArea->SetText("ab");
		pTextArea->SetCaretPosition(1);
		pTextArea->HomeEnter();
		AssertString("\nab", pTextArea->GetText());
		AssertSize(0, pTextArea->GetCaretPosition());
		pTextArea->SetText("ab\ncd");
		pTextArea->EndEnter();
		AssertString("ab\n\ncd", pTextArea->GetText());
		AssertSize(3, pTextArea->GetCaretPosition());

		//Delete and backspace remove the selection, or a character.
		pTextArea->SetText("abcd");
		pTextArea->Select(1, 3);
		pTextArea->Delete();
		AssertString("ad", pTextArea->GetText());
		pTextArea->Delete();
		AssertString("a", pTextArea->GetText());
		pTextArea->Backspace();
		AssertString("", pTextArea->GetText());
		pTextArea->Backspace();
		pTextArea->Delete();
		AssertString("", pTextArea->GetText());

		//Select words and everything.
		pTextArea->SetText("foo bar_baz qux");
		pTextArea->SetCaretPosition(6);
		pTextArea->SelectWholeWord();
		AssertTextAreaSelection(4, 11, pTextArea);
		pTextArea->SelectAll();
		AssertTextAreaSelection(0, 15, pTextArea);
		pTextArea->SetCaretPosition(4);
		pTextArea->SelectRight();
		pTextArea->SelectRight();
		AssertTextAreaSelection(4, 6, pTextArea);
		pTextArea->SelectWordLeft();
		AssertTextAreaNoSelection(pTextArea);
		pTextArea->SelectWordLeft();
		AssertTextAreaSelection(3, 4, pTextArea);
		pTextArea->SelectDocumentEnd();
		AssertTextAreaSelection(4, 15, pTextArea);
		pTextArea->Right();
		AssertTextAreaNoSelection(pTextArea);

		//A text area that isn't editable can still be selected and copied.
		pTextArea->SetEditable(false);
		pTextArea->SelectAll();
		pTextArea->Printable('z', true);
		pTextArea->Delete();
		pTextArea->Paste((char*)"x");
		AssertString("foo bar_baz qux", pTextArea->GetText());
		sz.Init();
		pTextArea->Cut(&sz);
		AssertString("foo bar_baz qux", sz.Text());
		AssertString("foo bar_baz qux", pTextArea->GetText());
		sz.Kill();
		AssertFalse(pTextArea->WantsEnter());

		//Cut and cut line copy what they would cut: the selection, or without one the caret's line.
		pTextArea->SetText("one\ntwo\n");
		pTextArea->SetCaretPosition(5);
		sz.Init();
		pTextArea->Cut(&sz);
		AssertString("two\n", sz.Text());
		sz.Clear();
		pTextArea->CutLine(&sz);
		AssertString("two\n", sz.Text());
		sz.Clear();
		pTextArea->Select(0, 2);
		pTextArea->CutLine(&sz);
		AssertString("on", sz.Text());
		AssertString("one\ntwo\n", pTextArea->GetText());
		sz.Kill();

		pTextArea = NULL;
		pFill = NULL;
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
// Spans grow with text typed inside or at their end, move with text
// before them and go when their text does.
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaEditSpans(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CFillContainer>			pFill;
		Ptr<CTextArea>				pTextArea;
		ARGB32						sRed;
		ARGB32						sPlain;

		cNativeFactory.Init(&gcMemoryAllocator, 300, 200, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Text Area Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pTextArea = OMalloc<CTextArea>(pTestWindow, CreateTestTheme(Ptr<CFont>()), ReadLabelFont(), "abc");
		pTextArea->SetAlignment(CSH_Stretch, CSV_Stretch);
		pFill->AddComponent(pTextArea);
		pTestWindow->Layout();

		sRed = Set32BitColour((uint8)255, 0, 0);
		sPlain = pTextArea->GetColour();
		AssertTrue(pTextArea->AddSpan(1, 1, sRed));

		//Typed after the b it is red too.
		pTextArea->SetCaretPosition(2);
		pTextArea->Printable('x', true);
		AssertString("abxc", pTextArea->GetText());
		AssertTrue(sRed == pTextArea->GetColourAt(1));
		AssertTrue(sRed == pTextArea->GetColourAt(2));
		AssertTrue(sPlain == pTextArea->GetColourAt(3));

		//Typed before the span it moves the span.
		pTextArea->SetCaretPosition(1);
		pTextArea->Printable('y', true);
		AssertString("aybxc", pTextArea->GetText());
		AssertTrue(sPlain == pTextArea->GetColourAt(1));
		AssertTrue(sRed == pTextArea->GetColourAt(2));
		AssertTrue(sRed == pTextArea->GetColourAt(3));

		//Deleting part shrinks it, deleting all removes it.
		pTextArea->Select(3, 5);
		pTextArea->Delete();
		AssertString("ayb", pTextArea->GetText());
		AssertSize(1, pTextArea->NumSpans());
		AssertTrue(sRed == pTextArea->GetColourAt(2));
		pTextArea->Backspace();
		AssertString("ay", pTextArea->GetText());
		AssertSize(0, pTextArea->NumSpans());

		pTextArea = NULL;
		pFill = NULL;
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
// In a scroll pane the caret is kept in view, a page is the view's height
// and pressing places the caret.
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaEditScrollPane(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CWindowTheme>			pTheme;
		Ptr<CScrollPane>			pScroll;
		Ptr<CTextArea>				pTextArea;
		CChars						szText;
		int32						i;
		CInt32Rect					cCaret;
		int32						iCaretTop;
		int32						iLeft;

		cNativeFactory.Init(&gcMemoryAllocator, 200, 100, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Text Area Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pTheme = CJewelTheme::CreateDarkTheme(pTestWindow, ReadLabelFont());
		pScroll = OMalloc<CScrollPane>(pTestWindow, pTheme);
		pTestWindow->SetContainer(pScroll);

		//Twenty lines of "line N", 480 pixels high in a view 100 high.
		szText.Init();
		for (i = 0; i < 20; i++)
		{
			if (i > 0)
			{
				szText.Append('\n');
			}
			szText.Append("line ");
			szText.Append(i);
		}
		pTextArea = OMalloc<CTextArea>(pTestWindow, CreateTestTheme(Ptr<CFont>()), ReadLabelFont(), szText.Text());
		szText.Kill();
		pScroll->SetContent(pTextArea);
		pTestWindow->Layout();
		AssertInt(0, pScroll->GetScrollPosition().y);
		AssertInt(380, pScroll->GetMaximumScrollPosition().y);

		//The end of the text is scrolled into view.
		pTextArea->DocumentEnd();
		pTestWindow->Layout();
		AssertInt(380, pScroll->GetScrollPosition().y);
		pTextArea->GetCaretBounds(&cCaret);
		iCaretTop = pTextArea->GetPosition().y + cCaret.GetTop();
		AssertTrue(iCaretTop >= 0);
		AssertTrue(iCaretTop + cCaret.GetHeight() <= 100);

		pTextArea->DocumentHome();
		pTestWindow->Layout();
		AssertInt(0, pScroll->GetScrollPosition().y);

		//Down past the view scrolls just enough.
		pTextArea->Down();
		pTextArea->Down();
		pTextArea->Down();
		pTextArea->Down();
		pTestWindow->Layout();
		AssertInt(5 * 24 - 100, pScroll->GetScrollPosition().y);

		//A page down moves the caret about a view's height and the view as far, so the caret stays in its place.
		pTextArea->DocumentHome();
		pTestWindow->Layout();
		pTextArea->PageDown();
		pTestWindow->Layout();
		AssertSize(28, pTextArea->GetCaretPosition());
		AssertInt(96, pScroll->GetScrollPosition().y);
		pTextArea->PageUp();
		pTestWindow->Layout();
		AssertSize(0, pTextArea->GetCaretPosition());
		AssertInt(0, pScroll->GetScrollPosition().y);
		pTextArea->SelectPageDown();
		pTestWindow->Layout();
		AssertTextAreaSelection(0, 28, pTextArea);

		//Line down scrolls without moving the caret.
		pTextArea->DocumentHome();
		pTestWindow->Layout();
		pTextArea->LineDown();
		pTestWindow->Layout();
		AssertInt(24, pScroll->GetScrollPosition().y);
		AssertSize(0, pTextArea->GetCaretPosition());

		//Pressing on the third visible row, line 3, places the caret there.  Dragging selects.
		pTestWindow->GetFocus()->SetFocus(pTextArea);
		AssertTrue(pTextArea->IsFocussed());
		iLeft = pTextArea->GetPosition().x + 1;
		pTestWindow->PointerMoved(iLeft, 2 * 24 + 5);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		AssertSize(3 * 7, pTextArea->GetCaretPosition());
		pTestWindow->PointerMoved(iLeft, 3 * 24 + 5);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertTextAreaSelection(3 * 7, 4 * 7, pTextArea);
		AssertSize(4 * 7, pTextArea->GetCaretPosition());

		pTextArea = NULL;
		pScroll = NULL;
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
//////////////////////////////////////////////////////////////////////////
//
// A fixed width font as tiles: the caret, the pointer and the text's size
// go by columns, tabs go to tab stops and a character the font hasn't got
// takes a column.
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaEditTiles(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CFillContainer>			pFill;
		Ptr<CFontFamily>			pFamily;
		Ptr<CTextArea>				pTextArea;
		Ptr<CFont>					pFont;
		CInt32Rect					cCaret;
		SInt32Vec2					sSize;
		int32						iCell;
		int32						iLine;

		cNativeFactory.Init(&gcMemoryAllocator, 300, 200, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Text Area Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pFamily = CreateMonoTextAreaFamily(15.0f);
		pTextArea = OMalloc<CTextArea>(pTestWindow, CreateTestTheme(Ptr<CFont>()), pFamily, "a\tb\ncd");
		pTextArea->SetAlignment(CSH_Stretch, CSV_Stretch);
		pFill->AddComponent(pTextArea);
		pTestWindow->Layout();

		pFont = pTextArea->GetFont();
		AssertTrue(pFont->IsFixedWidth());
		iCell = (int32)pFont->GetAverageWidth();
		iLine = pFont->GetLineHeight();

		//As sprites too the tab goes to column 4: b is at x 4 cells and the text is 5 columns wide.  Mid line it goes
		//to the next stop.
		pTextArea->SetCaretPosition(2);
		pTextArea->GetCaretBounds(&cCaret);
		AssertInt(4 * iCell, cCaret.GetLeft());
		AssertInt(5 * iCell, pTextArea->GetTextSize().x);
		AssertInt(4 * iCell, pTextArea->GetTabPixels());
		pTextArea->SetText("abcde\tf");
		pTextArea->SetCaretPosition(6);
		pTextArea->GetCaretBounds(&cCaret);
		AssertInt(8 * iCell, cCaret.GetLeft());
		pTextArea->SetTabWidth(2);
		pTextArea->GetCaretBounds(&cCaret);
		AssertInt(6 * iCell, cCaret.GetLeft());
		pTextArea->SetTabWidth(4);
		pTextArea->SetText("a\tb\ncd");

		//Tiles are only used when they are asked for.
		AssertFalse(pTextArea->IsDrawnAsTiles());
		pTextArea->SetTiledText(true);
		AssertTrue(pTextArea->IsTiledText());
		AssertTrue(pTextArea->IsDrawnAsTiles());

		//The tab goes to column 4: b is at x 4 cells, the text is 5 columns wide and 2 lines high.
		pTextArea->SetCaretPosition(2);
		pTextArea->GetCaretBounds(&cCaret);
		AssertInt(4 * iCell, cCaret.GetLeft());
		AssertInt(0, cCaret.GetTop());
		AssertInt(iLine, cCaret.GetHeight());
		pTextArea->SetCaretPosition(3);
		pTextArea->GetCaretBounds(&cCaret);
		AssertInt(5 * iCell, cCaret.GetLeft());
		sSize = pTextArea->GetTextSize();
		AssertInt(5 * iCell, sSize.x);
		AssertInt(2 * iLine, sSize.y);

		//The pointer finds the gap nearest it; over the tab is either side of it.
		AssertSize(2, pTextArea->FindPositionAt(4 * iCell + 1, 1));
		AssertSize(1, pTextArea->FindPositionAt(iCell + 2, 1));
		AssertSize(5, pTextArea->FindPositionAt(iCell + 1, iLine + 1));
		AssertSize(6, pTextArea->FindPositionAt(1000, iLine + 1));

		//A bigger tab width moves b along.
		pTextArea->SetTabWidth(8);
		pTextArea->SetCaretPosition(2);
		pTextArea->GetCaretBounds(&cCaret);
		AssertInt(8 * iCell, cCaret.GetLeft());
		pTextArea->SetTabWidth(4);

		//Down and up keep the column across a tab.
		pTextArea->SetText("\tx\nabcdefgh");
		pTextArea->SetCaretPosition(1);
		pTextArea->Down();
		AssertSize(3 + 4, pTextArea->GetCaretPosition());
		pTextArea->Right();
		pTextArea->Up();
		AssertSize(2, pTextArea->GetCaretPosition());

		//A character the font hasn't got is a column, like the others.
		pTextArea->SetText("\xC3\xA9" "ab");
		pTextArea->SetCaretPosition(2);
		pTextArea->GetCaretBounds(&cCaret);
		AssertInt(iCell, cCaret.GetLeft());
		AssertInt(3 * iCell, pTextArea->GetTextSize().x);

		//Editing as tiles.
		pTextArea->SetText("ab");
		pTextArea->DocumentEnd();
		pTextArea->Printable('c', true);
		pTextArea->Enter();
		pTextArea->Printable('d', true);
		AssertString("abc\nd", pTextArea->GetText());
		pTextArea->GetCaretBounds(&cCaret);
		AssertInt(iCell, cCaret.GetLeft());
		AssertInt(iLine, cCaret.GetTop());
		AssertInt(3 * iCell, pTextArea->GetTextSize().x);
		AssertInt(2 * iLine, pTextArea->GetTextSize().y);
		pTextArea->Backspace();
		pTextArea->Backspace();
		AssertString("abc", pTextArea->GetText());
		AssertInt(iLine, pTextArea->GetTextSize().y);

		//Word wrap needs sprites; turning it off again goes back to tiles.
		pTextArea->SetWordWrap(true);
		AssertTrue(pTextArea->IsTiledText());
		AssertFalse(pTextArea->IsDrawnAsTiles());
		pTextArea->SetWordWrap(false);
		AssertTrue(pTextArea->IsDrawnAsTiles());

		//A font that isn't fixed width is drawn as sprites.
		pTextArea->SetFont(ReadLabelFont());
		AssertFalse(pTextArea->GetFont()->IsFixedWidth());
		AssertFalse(pTextArea->IsDrawnAsTiles());

		pFont = NULL;
		pTextArea = NULL;
		pFamily = NULL;
		pFill = NULL;
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
// The text area's line starts are those of its text.
//
//////////////////////////////////////////////////////////////////////////
void AssertTextAreaLineStarts(Ptr<CTextArea> pTextArea)
{
	char*	szText;
	size	uiLength;
	size	uiPosition;
	size	uiLine;

	szText = pTextArea->GetText();
	uiLength = StrLen(szText);
	uiLine = 0;
	AssertSize(0, pTextArea->GetLineStart(0));
	for (uiPosition = 0; uiPosition < uiLength; uiPosition++)
	{
		AssertSize(uiLine, pTextArea->FindLine(uiPosition));
		if (szText[uiPosition] == '\n')
		{
			uiLine++;
			AssertSize(uiPosition + 1, pTextArea->GetLineStart(uiLine));
		}
	}
	AssertSize(uiLine + 1, pTextArea->NumLines());
	AssertSize(uiLength, pTextArea->GetLineStart(uiLine + 1));
}


//////////////////////////////////////////////////////////////////////////
//
// A layout leaves the gaps below their lines (counted by line feeds, not
// wrapped rows), the last line's too.
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaEditLayoutGaps(void)
{
	{
		Ptr<CFont>					pFont;
		Ptr<CTextLayout>			pLayout;
		Ptr<CGlyphListFontDraw>		pDraw;
		CArrayTextLayoutGap			asGaps;
		STextLayoutGap*				psGap;

		pFont = ReadLabelFont();
		pLayout = OMalloc<CTextLayout>();
		pDraw = OMalloc<CGlyphListFontDraw>();

		asGaps.Init();
		psGap = asGaps.Add();
		psGap->uiLine = 0;
		psGap->iHeight = 10;
		psGap = asGaps.Add();
		psGap->uiLine = 2;
		psGap->iHeight = 7;
		pLayout->SetLineGaps(&asGaps);
		asGaps.Kill();

		LayoutTextAreaText(pLayout, pDraw, pFont, "ab\ncd\nef");
		AssertSize(3, pLayout->NumLines());
		AssertInt(0, pLayout->GetLine(0)->iTop);
		AssertInt(24 + 10, pLayout->GetLine(1)->iTop);
		AssertInt(24 + 10 + 24, pLayout->GetLine(2)->iTop);
		AssertInt(24 + 10 + 24 + 24 + 7, pLayout->GetSize().y);

		//Line 0 wraps onto two rows: its gap is under the second.
		pLayout->SetWidth(TextAreaWidth(pFont, "aaa bbb"));
		LayoutTextAreaText(pLayout, pDraw, pFont, "aaa bbb ccc\nd");
		AssertSize(3, pLayout->NumLines());
		AssertInt(24, pLayout->GetLine(1)->iTop);
		AssertInt(48 + 10, pLayout->GetLine(2)->iTop);

		//No gaps.
		pLayout->SetLineGaps(NULL);
		pLayout->SetWidth(-1);
		LayoutTextAreaText(pLayout, pDraw, pFont, "ab\ncd");
		AssertInt(24, pLayout->GetLine(1)->iTop);
		AssertInt(48, pLayout->GetSize().y);

		pDraw = NULL;
		pLayout = NULL;
		pFont = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Gaps below lines: the rows after are moved down, the caret and the
// pointer skip them, and they stay with their lines as text is edited.
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaEditLineGaps(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CFillContainer>			pFill;
		Ptr<CFont>					pFont;
		Ptr<CTextArea>				pTextArea;
		CInt32Rect					cRect;
		CChars						szCut;
		uint32						uiId;
		CArrayTextLayoutGap			asGaps;
		STextLayoutGap*				psGap;

		cNativeFactory.Init(&gcMemoryAllocator, 300, 200, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Text Area Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pFont = ReadLabelFont();
		pTextArea = OMalloc<CTextArea>(pTestWindow, CreateTestTheme(Ptr<CFont>()), pFont, "ab\ncd\nef");
		pTextArea->SetAlignment(CSH_Stretch, CSV_Stretch);
		pFill->AddComponent(pTextArea);
		pTestWindow->Layout();

		//The lines, and which a position is on: a line feed is on the line it ends.
		AssertTextAreaLineStarts(pTextArea);
		AssertSize(3, pTextArea->NumLines());
		AssertSize(3, pTextArea->GetLineStart(1));
		AssertSize(0, pTextArea->FindLine(2));
		AssertSize(1, pTextArea->FindLine(3));
		AssertSize(2, pTextArea->FindLine(8));

		//A gap below line 0 moves the rows after it down.
		AssertFalse(pTextArea->SetLineGap(3, 10));
		AssertTrue(pTextArea->SetLineGap(0, 50));
		AssertInt(50, pTextArea->GetLineGap(0));
		AssertInt(0, pTextArea->GetLineGap(1));
		AssertTrue(pTextArea->GetLineBounds(0, &cRect));
		AssertInt(0, cRect.GetTop());
		AssertInt(24, cRect.GetBottom());
		AssertTrue(pTextArea->GetLineGapBounds(0, &cRect));
		AssertInt(24, cRect.GetTop());
		AssertInt(24 + 50, cRect.GetBottom());
		AssertFalse(pTextArea->GetLineGapBounds(1, &cRect));
		AssertTrue(pTextArea->GetLineBounds(1, &cRect));
		AssertInt(24 + 50, cRect.GetTop());
		AssertInt(3 * 24 + 50, pTextArea->GetTextSize().y);

		//The caret is drawn below the gap, and down goes over it.
		pTextArea->SetCaretPosition(0);
		pTextArea->Down();
		AssertSize(3, pTextArea->GetCaretPosition());
		pTextArea->GetCaretBounds(&cRect);
		AssertInt(24 + 50, cRect.GetTop());
		pTextArea->Up();
		AssertSize(0, pTextArea->GetCaretPosition());

		//The pointer in a gap is on the line above it.
		AssertSize(2, pTextArea->FindPositionAt(1000, 24 + 10));
		AssertSize(2, pTextArea->FindPositionAt(1000, 24 + 49));
		AssertSize(5, pTextArea->FindPositionAt(1000, 24 + 50));

		//The last line's gap is below the text.
		AssertTrue(pTextArea->SetLineGap(2, 7));
		AssertSize(2, pTextArea->NumLineGaps());
		AssertTrue(pTextArea->GetLineGapBounds(2, &cRect));
		AssertInt(3 * 24 + 50, cRect.GetTop());
		AssertInt(3 * 24 + 57, cRect.GetBottom());
		AssertInt(3 * 24 + 57, pTextArea->GetTextSize().y);

		//0 removes a gap; new text removes them all.
		AssertTrue(pTextArea->SetLineGap(0, 0));
		AssertSize(1, pTextArea->NumLineGaps());
		AssertTrue(pTextArea->GetLineGapBounds(2, &cRect));
		AssertInt(3 * 24, cRect.GetTop());
		pTextArea->SetText("ab\ncd\nef");
		AssertSize(0, pTextArea->NumLineGaps());
		AssertInt(3 * 24, pTextArea->GetTextSize().y);

		//A line feed at the end of a line leaves the gap under it; at its start the line and its gap go down.
		pTextArea->SetLineGap(1, 30);
		pTextArea->SetCaretPosition(5);
		pTextArea->Enter();
		AssertString("ab\ncd\n\nef", pTextArea->GetText());
		AssertTextAreaLineStarts(pTextArea);
		AssertInt(30, pTextArea->GetLineGap(1));
		AssertSize(1, pTextArea->NumLineGaps());
		pTextArea->Backspace();
		AssertString("ab\ncd\nef", pTextArea->GetText());
		AssertInt(30, pTextArea->GetLineGap(1));
		pTextArea->SetCaretPosition(3);
		pTextArea->Enter();
		AssertString("ab\n\ncd\nef", pTextArea->GetText());
		AssertTextAreaLineStarts(pTextArea);
		AssertInt(0, pTextArea->GetLineGap(1));
		AssertInt(30, pTextArea->GetLineGap(2));
		AssertTrue(pTextArea->GetLineGapBounds(2, &cRect));
		AssertInt(3 * 24, cRect.GetTop());

		//Removing the empty line before it brings it back up.
		pTextArea->Backspace();
		AssertString("ab\ncd\nef", pTextArea->GetText());
		AssertTextAreaLineStarts(pTextArea);
		AssertInt(30, pTextArea->GetLineGap(1));

		//Typing on the line keeps its gap.
		pTextArea->SetCaretPosition(4);
		pTextArea->Printable('x', true);
		AssertString("ab\ncxd\nef", pTextArea->GetText());
		AssertTextAreaLineStarts(pTextArea);
		AssertInt(30, pTextArea->GetLineGap(1));

		//Deleting the line before it moves it up a line.
		pTextArea->SetCaretPosition(0);
		pTextArea->DeleteLine();
		AssertString("cxd\nef", pTextArea->GetText());
		AssertTextAreaLineStarts(pTextArea);
		AssertInt(30, pTextArea->GetLineGap(0));
		AssertSize(1, pTextArea->NumLineGaps());

		//Joining a line onto the one above loses its gap; the line above keeps its own.
		pTextArea->SetLineGap(1, 20);
		pTextArea->SetCaretPosition(3);
		pTextArea->Delete();
		AssertString("cxdef", pTextArea->GetText());
		AssertTextAreaLineStarts(pTextArea);
		AssertInt(30, pTextArea->GetLineGap(0));
		AssertSize(1, pTextArea->NumLineGaps());

		//Cutting the line with a gap takes it with it.
		szCut.Init();
		pTextArea->SetText("x\ny\nz");
		pTextArea->SetLineGap(1, 10);
		pTextArea->SetCaretPosition(2);
		pTextArea->CutLine(&szCut);
		AssertString("y\n", szCut.Text());
		AssertString("x\nz", pTextArea->GetText());
		AssertSize(0, pTextArea->NumLineGaps());
		szCut.Kill();

		//Pasting lines at the start of a line pushes it and its gap down.
		pTextArea->SetText("x\ny");
		pTextArea->SetLineGap(1, 10);
		pTextArea->SetCaretPosition(0);
		pTextArea->Paste((char*)"1\n2\n");
		AssertString("1\n2\nx\ny", pTextArea->GetText());
		AssertTextAreaLineStarts(pTextArea);
		AssertInt(10, pTextArea->GetLineGap(3));
		AssertSize(1, pTextArea->NumLineGaps());

		//Deleting from inside one line to inside another loses the gaps of the lines after the first, and moves
		//the gaps below up.
		pTextArea->SetText("a\nb\nc\nd");
		pTextArea->SetLineGap(0, 5);
		pTextArea->SetLineGap(1, 6);
		pTextArea->SetLineGap(2, 7);
		pTextArea->SetLineGap(3, 8);
		pTextArea->Select(1, 5);
		pTextArea->Delete();
		AssertString("a\nd", pTextArea->GetText());
		AssertTextAreaLineStarts(pTextArea);
		AssertSize(2, pTextArea->NumLineGaps());
		AssertInt(5, pTextArea->GetLineGap(0));
		AssertInt(8, pTextArea->GetLineGap(1));

		//Indenting lines (no line feeds) moves no gaps.
		pTextArea->SelectAll();
		pTextArea->Tab(true);
		AssertString("    a\n    d", pTextArea->GetText());
		AssertTextAreaLineStarts(pTextArea);
		AssertInt(5, pTextArea->GetLineGap(0));
		AssertInt(8, pTextArea->GetLineGap(1));

		pTextArea->ClearLineGaps();
		AssertSize(0, pTextArea->NumLineGaps());
		AssertInt(2 * 24, pTextArea->GetTextSize().y);

		//A gap keeps its id through edits and changes of height, so its line can be found; a new gap gets a new id.
		//Lines ending in a carriage return and line feed.
		pTextArea->SetText("a\r\nb\r\nc");
		AssertTextAreaLineStarts(pTextArea);
		AssertSize(3, pTextArea->NumLines());
		AssertTrue(pTextArea->SetLineGap(1, 10));
		uiId = pTextArea->GetLineGapId(1);
		AssertTrue(uiId != 0);
		AssertInt(0, pTextArea->GetLineGapId(0));
		AssertTrue(pTextArea->SetLineGap(1, 20));
		AssertInt(uiId, pTextArea->GetLineGapId(1));
		pTextArea->SetCaretPosition(0);
		pTextArea->Enter();
		AssertSize(2, pTextArea->FindLineGap(uiId));
		AssertTrue(pTextArea->GetLineGapBounds(2, &cRect));
		AssertInt(3 * 24, cRect.GetTop());
		AssertInt(3 * 24 + 20, cRect.GetBottom());
		AssertTrue(pTextArea->SetLineGap(0, 5));
		AssertTrue(pTextArea->GetLineGapId(0) > uiId);
		pTextArea->SetCaretPosition(pTextArea->GetLineStart(2));
		pTextArea->DeleteLine();
		AssertString("\na\r\nc", pTextArea->GetText());
		AssertSize(MAX_SIZE, pTextArea->FindLineGap(uiId));
		AssertSize(1, pTextArea->NumLineGaps());

		//Copied gaps keep their ids; gaps of no height or past the last line are left out.  New ids are past the kept
		//ones'.
		asGaps.Init();
		psGap = asGaps.Add();
		psGap->uiLine = 0;
		psGap->iHeight = 7;
		psGap->uiId = 100;
		psGap = asGaps.Add();
		psGap->uiLine = 1;
		psGap->iHeight = 0;
		psGap->uiId = 101;
		psGap = asGaps.Add();
		psGap->uiLine = 9;
		psGap->iHeight = 3;
		psGap->uiId = 102;
		pTextArea->SetLineGaps(&asGaps);
		AssertSize(1, pTextArea->NumLineGaps());
		AssertInt(7, pTextArea->GetLineGap(0));
		AssertInt(100, pTextArea->GetLineGapId(0));
		AssertInt(3 * 24 + 7, pTextArea->GetTextSize().y);
		pTextArea->SetLineGaps(&asGaps);
		AssertSize(1, pTextArea->NumLineGaps());
		AssertTrue(pTextArea->SetLineGap(1, 4));
		AssertTrue(pTextArea->GetLineGapId(1) > 100);
		asGaps.Kill();

		//Empty text is a line, which can have a gap.
		pTextArea->SetText("");
		AssertSize(1, pTextArea->NumLines());
		AssertTrue(pTextArea->SetLineGap(0, 10));
		AssertInt(24 + 10, pTextArea->GetTextSize().y);
		AssertTrue(pTextArea->GetLineGapBounds(0, &cRect));
		AssertInt(24, cRect.GetTop());

		pTextArea = NULL;
		pFont = NULL;
		pFill = NULL;
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
// A wrapped line's gap is under its last row.
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaEditLineGapsWrapped(void)
{
	CWinRefWindowFactory	cNativeFactory;
	Ptr<CFont>				pFont;
	int32					iWidth;

	pFont = ReadLabelFont();
	iWidth = TextAreaWidth(pFont, "aaa bbb");

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CFillContainer>			pFill;
		Ptr<CTextArea>				pTextArea;
		CInt32Rect					cRect;

		cNativeFactory.Init(&gcMemoryAllocator, iWidth, 200, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Text Area Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pTextArea = OMalloc<CTextArea>(pTestWindow, CreateTestTheme(Ptr<CFont>()), pFont, "aaa bbb ccc\nd");
		pTextArea->SetWordWrap(true);
		pTextArea->SetAlignment(CSH_Stretch, CSV_Stretch);
		pFill->AddComponent(pTextArea);
		pTestWindow->Layout();

		pTextArea->SetLineGap(0, 10);
		pTestWindow->Layout();
		AssertTrue(pTextArea->GetLineBounds(0, &cRect));
		AssertInt(0, cRect.GetTop());
		AssertInt(48, cRect.GetBottom());
		AssertTrue(pTextArea->GetLineGapBounds(0, &cRect));
		AssertInt(48, cRect.GetTop());
		AssertInt(58, cRect.GetBottom());
		AssertTrue(pTextArea->GetLineBounds(1, &cRect));
		AssertInt(58, cRect.GetTop());
		AssertInt(3 * 24 + 10, pTextArea->GetTextSize().y);

		//Down from the first row goes to the second, not over the gap.
		pTextArea->SetCaretPosition(0);
		pTextArea->Down();
		AssertSize(8, pTextArea->GetCaretPosition());
		pTextArea->Down();
		AssertSize(12, pTextArea->GetCaretPosition());

		pTextArea = NULL;
		pFill = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
	}

	pFont = NULL;
	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// In tiles a gap is whole rows, at least as high as asked for.
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaEditLineGapsTiles(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CFillContainer>			pFill;
		Ptr<CFontFamily>			pFamily;
		Ptr<CTextArea>				pTextArea;
		CInt32Rect					cRect;
		int32						iLine;

		cNativeFactory.Init(&gcMemoryAllocator, 300, 200, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Text Area Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pFamily = CreateMonoTextAreaFamily(15.0f);
		pTextArea = OMalloc<CTextArea>(pTestWindow, CreateTestTheme(Ptr<CFont>()), pFamily, "ab\ncd");
		pTextArea->SetTiledText(true);
		pTextArea->SetAlignment(CSH_Stretch, CSV_Stretch);
		pFill->AddComponent(pTextArea);
		pTestWindow->Layout();
		AssertTrue(pTextArea->IsDrawnAsTiles());
		iLine = pTextArea->GetFont()->GetLineHeight();

		//A bit more than a row is two rows.
		pTextArea->SetLineGap(0, iLine + 1);
		AssertTrue(pTextArea->IsDrawnAsTiles());
		AssertInt(iLine + 1, pTextArea->GetLineGap(0));
		AssertTrue(pTextArea->GetLineGapBounds(0, &cRect));
		AssertInt(iLine, cRect.GetTop());
		AssertInt(3 * iLine, cRect.GetBottom());
		AssertInt(4 * iLine, pTextArea->GetTextSize().y);
		pTextArea->SetCaretPosition(4);
		pTextArea->GetCaretBounds(&cRect);
		AssertInt(3 * iLine, cRect.GetTop());
		AssertSize(4, pTextArea->FindPositionAt((int)pTextArea->GetFont()->GetAverageWidth() + 1, 3 * iLine + 1));
		AssertSize(1, pTextArea->FindPositionAt((int)pTextArea->GetFont()->GetAverageWidth() + 1, 2 * iLine));

		//Exactly a row is one; the last line's gap is rows below the text.
		pTextArea->SetLineGap(0, iLine);
		pTextArea->SetLineGap(1, 1);
		AssertTrue(pTextArea->GetLineBounds(1, &cRect));
		AssertInt(2 * iLine, cRect.GetTop());
		AssertTrue(pTextArea->GetLineGapBounds(1, &cRect));
		AssertInt(3 * iLine, cRect.GetTop());
		AssertInt(4 * iLine, cRect.GetBottom());
		AssertInt(4 * iLine, pTextArea->GetTextSize().y);

		pTextArea = NULL;
		pFamily = NULL;
		pFill = NULL;
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
// Undo and redo: a word typed is one edit (with the space before it),
// moving the caret starts another, every other edit is one, and the caret
// goes after what changed.  A new edit drops what could be redone.  The
// undo id comes back to the text's when it is undone back to it.  Control
// Z and Y undo and redo.  A text area that isn't editable has no undo.
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaEditUndo(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CFillContainer>			pFill;
		Ptr<CTextArea>				pTextArea;
		size						uiLoaded;
		size						uiTyped;

		cNativeFactory.Init(&gcMemoryAllocator, 300, 200, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Text Area Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pTextArea = OMalloc<CTextArea>(pTestWindow, CreateTestTheme(Ptr<CFont>()), ReadLabelFont(), "abc");
		pTextArea->SetAlignment(CSH_Stretch, CSV_Stretch);
		pFill->AddComponent(pTextArea);
		pTestWindow->Layout();

		uiLoaded = pTextArea->GetUndoId();
		AssertFalse(pTextArea->CanUndo());
		AssertFalse(pTextArea->CanRedo());

		//A word typed, with its space, is one edit; the next word another.
		pTextArea->DocumentEnd();
		pTextArea->Printable(' ', true);
		pTextArea->Printable('d', true);
		pTextArea->Printable('e', true);
		pTextArea->Printable('f', true);
		uiTyped = pTextArea->GetUndoId();
		AssertFalse(uiTyped == uiLoaded);
		pTextArea->Printable(' ', true);
		pTextArea->Printable('g', true);
		AssertString("abc def g", pTextArea->GetText());
		AssertTrue(pTextArea->Undo());
		AssertString("abc def", pTextArea->GetText());
		AssertSize(7, pTextArea->GetCaretPosition());
		AssertSize(uiTyped, pTextArea->GetUndoId());
		AssertTrue(pTextArea->Undo());
		AssertString("abc", pTextArea->GetText());
		AssertSize(3, pTextArea->GetCaretPosition());
		AssertSize(uiLoaded, pTextArea->GetUndoId());
		AssertFalse(pTextArea->CanUndo());
		AssertFalse(pTextArea->Undo());

		//Redone, the caret after what came back.
		AssertTrue(pTextArea->Redo());
		AssertString("abc def", pTextArea->GetText());
		AssertSize(7, pTextArea->GetCaretPosition());
		AssertTrue(pTextArea->Redo());
		AssertString("abc def g", pTextArea->GetText());
		AssertFalse(pTextArea->CanRedo());

		//Moving the caret starts another edit; the change is in the middle.
		pTextArea->Left();
		pTextArea->Left();
		pTextArea->Printable('X', true);
		pTextArea->Printable('Y', true);
		AssertString("abc defXY g", pTextArea->GetText());
		AssertTrue(pTextArea->Undo());
		AssertString("abc def g", pTextArea->GetText());
		AssertSize(7, pTextArea->GetCaretPosition());

		//An edit after an undo drops the redo.  Pasting and deleting are each an edit.
		AssertTrue(pTextArea->CanRedo());
		pTextArea->Paste((char*)"Z");
		AssertFalse(pTextArea->CanRedo());
		pTextArea->Backspace();
		pTextArea->Backspace();
		AssertString("abc de g", pTextArea->GetText());
		AssertTrue(pTextArea->Undo());
		AssertString("abc def g", pTextArea->GetText());
		AssertTrue(pTextArea->Undo());
		AssertString("abc defZ g", pTextArea->GetText());
		AssertTrue(pTextArea->Undo());
		AssertString("abc def g", pTextArea->GetText());

		//Control Z and Y.
		pTestWindow->GetFocus()->SetFocus(pTextArea);
		pTestWindow->InputCommand(CIC_Undo, true);
		AssertString("abc def", pTextArea->GetText());
		pTestWindow->InputCommand(CIC_Redo, true);
		AssertString("abc def g", pTextArea->GetText());

		//A pause between characters starts another edit.
		pTextArea->SetTypingPause(0);
		pTextArea->DocumentEnd();
		pTextArea->Printable('h', true);
		pTextArea->Printable('i', true);
		AssertString("abc def ghi", pTextArea->GetText());
		AssertTrue(pTextArea->Undo());
		AssertString("abc def gh", pTextArea->GetText());
		AssertTrue(pTextArea->Undo());
		AssertString("abc def g", pTextArea->GetText());
		pTextArea->SetTypingPause(TEXT_UNDO_TYPING_PAUSE);

		//The selection a paste replaced comes back selected; redone, the caret is after the paste.
		pTextArea->Select(4, 7);
		pTextArea->Paste((char*)"XYZW");
		AssertString("abc XYZW g", pTextArea->GetText());
		AssertTrue(pTextArea->Undo());
		AssertString("abc def g", pTextArea->GetText());
		AssertTextAreaSelection(4, 7, pTextArea);
		AssertTrue(pTextArea->Redo());
		AssertString("abc XYZW g", pTextArea->GetText());
		AssertTextAreaNoSelection(pTextArea);
		AssertSize(8, pTextArea->GetCaretPosition());

		//Back to a state by its id, past several edits, and forward again.
		uiTyped = pTextArea->GetUndoId();
		AssertTrue(pTextArea->UndoTo(uiLoaded));
		AssertString("abc", pTextArea->GetText());
		AssertTrue(pTextArea->UndoTo(uiTyped));
		AssertString("abc XYZW g", pTextArea->GetText());
		AssertFalse(pTextArea->UndoTo(uiTyped));

		//Setting the text starts the history again.
		pTextArea->SetText("new");
		AssertFalse(pTextArea->CanUndo());
		AssertFalse(pTextArea->CanRedo());

		//Not editable: no undo.
		pTextArea->SetEditable(false);
		AssertFalse(pTextArea->CanUndo());
		AssertFalse(pTextArea->Undo());
		AssertSize(0, pTextArea->GetUndoId());

		pTextArea = NULL;
		pFill = NULL;
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
// Records a text area's undo event.
//
//////////////////////////////////////////////////////////////////////////
class CTestUndoListener
{
public:
	CArrayInt	maiChanges;
	CArrayInt	maiBefore;
	CArrayInt	maiAfter;

	void Init(void) { maiChanges.Init(); maiBefore.Init(); maiAfter.Init(); }
	void Kill(void) { maiAfter.Kill(); maiBefore.Kill(); maiChanges.Kill(); }
	void UndoChanged(CTextArea* pcTextArea, STextAreaUndo* psUndo)
	{
		maiChanges.Add((int)psUndo->eChange);
		maiBefore.Add((int)psUndo->uiIdBefore);
		maiAfter.Add((int)psUndo->uiIdAfter);
	}
};


//////////////////////////////////////////////////////////////////////////
//
// The undo event says each edit kept, merged into the one before, undone
// and redone, with the states it went between.
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaEditUndoEvent(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CFillContainer>			pFill;
		Ptr<CTextArea>				pTextArea;
		CTestUndoListener			cListener;
		size						uiLoaded;

		cNativeFactory.Init(&gcMemoryAllocator, 300, 200, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Text Area Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pTextArea = OMalloc<CTextArea>(pTestWindow, CreateTestTheme(Ptr<CFont>()), ReadLabelFont(), "abc");
		pFill->AddComponent(pTextArea);
		pTestWindow->Layout();
		cListener.Init();
		pTextArea->GetUndoEvent()->Add(&cListener, &CTestUndoListener::UndoChanged);
		uiLoaded = pTextArea->GetUndoId();

		pTextArea->DocumentEnd();
		pTextArea->Printable('d', true);
		pTextArea->Printable('e', true);
		pTextArea->Undo();
		pTextArea->Redo();
		AssertInt(4, (int)cListener.maiChanges.NumElements());
		AssertInt(TAU_Kept, cListener.maiChanges.GetValue(0));
		AssertInt((int)uiLoaded, cListener.maiBefore.GetValue(0));
		AssertInt(TAU_Merged, cListener.maiChanges.GetValue(1));
		AssertInt(cListener.maiAfter.GetValue(0), cListener.maiBefore.GetValue(1));
		AssertInt(TAU_Undone, cListener.maiChanges.GetValue(2));
		AssertInt(cListener.maiAfter.GetValue(1), cListener.maiBefore.GetValue(2));
		AssertInt((int)uiLoaded, cListener.maiAfter.GetValue(2));
		AssertInt(TAU_Redone, cListener.maiChanges.GetValue(3));
		AssertInt(cListener.maiAfter.GetValue(1), cListener.maiAfter.GetValue(3));

		pTextArea->GetUndoEvent()->Remove(&cListener);
		cListener.Kill();
		pTextArea = NULL;
		pFill = NULL;
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
// A rename's places in a file replaced as one edit: one undo puts them
// all back.  Ranges out of order, or in a text area that isn't editable,
// change nothing.
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaEditReplaceRanges(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CFillContainer>			pFill;
		Ptr<CTextArea>				pTextArea;
		size						auiStarts[3] = { 4, 17, 23 };
		size						auiEnds[3] = { 7, 20, 26 };
		size						auiBad[2] = { 17, 4 };

		cNativeFactory.Init(&gcMemoryAllocator, 300, 200, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Text Area Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pTextArea = OMalloc<CTextArea>(pTestWindow, CreateTestTheme(Ptr<CFont>()), ReadLabelFont(), "int foo;\nint x = foo + foo;");
		pTextArea->SetAlignment(CSH_Stretch, CSV_Stretch);
		pFill->AddComponent(pTextArea);
		pTestWindow->Layout();

		pTextArea->SetEditable(false);
		AssertFalse(pTextArea->ReplaceRanges(auiStarts, auiEnds, 3, "count"));
		pTextArea->SetEditable(true);
		AssertFalse(pTextArea->ReplaceRanges(auiBad, auiEnds, 2, "count"));
		AssertString("int foo;\nint x = foo + foo;", pTextArea->GetText());

		pTextArea->SetCaretPosition(27);
		AssertTrue(pTextArea->ReplaceRanges(auiStarts, auiEnds, 3, "count"));
		AssertString("int count;\nint x = count + count;", pTextArea->GetText());
		AssertSize(33, pTextArea->GetCaretPosition());
		AssertTrue(pTextArea->Undo());
		AssertString("int foo;\nint x = foo + foo;", pTextArea->GetText());
		AssertFalse(pTextArea->CanUndo());
		AssertTrue(pTextArea->Redo());
		AssertString("int count;\nint x = count + count;", pTextArea->GetText());

		pTextArea = NULL;
		pFill = NULL;
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
void TestTextAreaEdit(void)
{
	BeginTests();
	ObjectsInit();

	TestTextAreaEditLayoutStops();
	TestTextAreaEditNavigation();
	TestTextAreaEditWrapped();
	TestTextAreaEditModification();
	TestTextAreaEditUndo();
	TestTextAreaEditUndoEvent();
	TestTextAreaEditReplaceRanges();
	TestTextAreaEditSpans();
	TestTextAreaEditScrollPane();
	TestTextAreaEditTiles();
	TestTextAreaEditLayoutGaps();
	TestTextAreaEditLineGaps();
	TestTextAreaEditLineGapsWrapped();
	TestTextAreaEditLineGapsTiles();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}
