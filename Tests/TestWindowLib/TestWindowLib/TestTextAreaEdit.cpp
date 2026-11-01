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


Ptr<CFont> ReadLabelFont(void);
int32 TextAreaWidth(Ptr<CFont> pFont, const char* szText);
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
		pTextArea = OMalloc<CTextArea>(pTestWindow, pFont, "  abc def\nxy");
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
		pTextArea = OMalloc<CTextArea>(pTestWindow, pFont, "aaa bbb ccc");
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
		pTextArea = OMalloc<CTextArea>(pTestWindow, ReadLabelFont(), "abc");
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
		pTextArea = OMalloc<CTextArea>(pTestWindow, ReadLabelFont(), "abc");
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
		pTextArea = OMalloc<CTextArea>(pTestWindow, ReadLabelFont(), szText.Text());
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
void TestTextAreaEdit(void)
{
	BeginTests();
	ObjectsInit();

	TestTextAreaEditLayoutStops();
	TestTextAreaEditNavigation();
	TestTextAreaEditWrapped();
	TestTextAreaEditModification();
	TestTextAreaEditSpans();
	TestTextAreaEditScrollPane();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}
