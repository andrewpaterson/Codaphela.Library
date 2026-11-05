#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/ScrollPane.h"
#include "WindowLib/JewelTheme.h"
#include "WindowLib/CodeEditor.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"


Ptr<CFont> ReadLabelFont(void);
Ptr<CFontFamily> CreateMonoTextAreaFamily(float32 fDefaultPixelSize);


//////////////////////////////////////////////////////////////////////////
//
// A component that only wants to be iHeight high, to go under a line.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFillContainer> CreateCodeEditorLineComponent(Ptr<CWindow> pWindow, int32 iHeight)
{
	Ptr<CFillContainer>	pComponent;

	pComponent = OMalloc<CFillContainer>(pWindow);
	pComponent->SetDesiredSize(-1, iHeight);
	return pComponent;
}


//Keeps the lines the gutter was clicked next to.
class CTestCodeEditorListener
{
public:
	size	muiClicks;
	size	muiLine;

	void Init(void)
	{
		muiClicks = 0;
		muiLine = MAX_SIZE;
	}

	void GutterClicked(CCodeEditor* pcEditor, size uiLine)
	{
		muiClicks++;
		muiLine = uiLine;
	}
};


//////////////////////////////////////////////////////////////////////////
//
// The top of line uiLine of pTextArea, relative to the window.
//
//////////////////////////////////////////////////////////////////////////
int32 CodeEditorLineTop(Ptr<CTextArea> pTextArea, size uiLine)
{
	CInt32Rect	cLine;

	if (!pTextArea->GetLineBounds(uiLine, &cLine))
	{
		return -1;
	}
	return pTextArea->GetPosition().y + cLine.GetTop();
}


//////////////////////////////////////////////////////////////////////////
//
// The gutter numbers the lines, right aligned after a column for marks,
// and is as wide as its widest number needs.  The code is to its right.
//
//////////////////////////////////////////////////////////////////////////
void TestCodeEditorGutter(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CFillContainer>			pFill;
		Ptr<CCodeEditor>			pEditor;
		Ptr<CTextArea>				pTextArea;
		int32						iCell;
		int32						iLine;

		cNativeFactory.Init(&gcMemoryAllocator, 300, 200, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Code Editor Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pEditor = OMalloc<CCodeEditor>(pTestWindow, CreateMonoTextAreaFamily(15.0f), "a\nb\nc");
		pEditor->SetAlignment(CSH_Stretch, CSV_Stretch);
		pFill->AddComponent(pEditor);
		pTestWindow->Layout();

		pTextArea = pEditor->GetTextArea();
		iCell = (int32)pTextArea->GetFont()->GetAverageWidth();
		iLine = pTextArea->GetFont()->GetLineHeight();

		AssertString(" 1 \n 2 \n 3 ", pEditor->GetGutter()->GetText());
		AssertInt(3 * iCell, pEditor->GetGutterWidth());
		AssertInt(pEditor->GetPosition().x + 3 * iCell, pTextArea->GetPosition().x);
		AssertInt(3 * iLine, pEditor->GetDesiredSize().y);
		AssertFalse(pEditor->GetGutter()->CanGetFocus());

		//Ten lines need two digits; the gutter widens and the code moves right.
		pTextArea->SetText("1\n2\n3\n4\n5\n6\n7\n8\n9\n10");
		pTestWindow->Layout();
		AssertString("  1 \n  2 \n  3 \n  4 \n  5 \n  6 \n  7 \n  8 \n  9 \n 10 ", pEditor->GetGutter()->GetText());
		AssertInt(4 * iCell, pEditor->GetGutterWidth());
		AssertInt(pEditor->GetPosition().x + 4 * iCell, pTextArea->GetPosition().x);

		//Back to one digit when lines are deleted.
		pTextArea->SetCaretPosition(0);
		pTextArea->DeleteLine();
		pTestWindow->Layout();
		AssertSize(9, pTextArea->NumLines());
		AssertInt(3 * iCell, pEditor->GetGutterWidth());

		//The gutter is tiles when the code is.
		AssertFalse(pEditor->GetGutter()->IsDrawnAsTiles());
		pTextArea->SetTiledText(true);
		pTestWindow->Layout();
		AssertTrue(pTextArea->IsDrawnAsTiles());
		AssertTrue(pEditor->GetGutter()->IsDrawnAsTiles());

		pTextArea = NULL;
		pEditor = NULL;
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
// A line component is laid out in a gap under its line, as high as it
// wants and as wide as the code; the code's and the gutter's rows below
// move down.  It stays with its line through edits, and goes with it.
//
//////////////////////////////////////////////////////////////////////////
void TestCodeEditorLineComponents(bool bTiles)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CFillContainer>			pFill;
		Ptr<CCodeEditor>			pEditor;
		Ptr<CTextArea>				pTextArea;
		Ptr<CTextArea>				pGutter;
		Ptr<CFillContainer>			pZone;
		Ptr<CFillContainer>			pOther;
		int32						iLine;
		int32						iGap;
		int32						iTop;

		cNativeFactory.Init(&gcMemoryAllocator, 300, 200, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Code Editor Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pEditor = OMalloc<CCodeEditor>(pTestWindow, CreateMonoTextAreaFamily(15.0f), "ab\ncd\nef");
		pEditor->SetAlignment(CSH_Stretch, CSV_Stretch);
		pFill->AddComponent(pEditor);
		pTextArea = pEditor->GetTextArea();
		pGutter = pEditor->GetGutter();
		pTextArea->SetTiledText(bTiles);
		pTestWindow->Layout();
		iLine = pTextArea->GetFont()->GetLineHeight();
		iTop = pEditor->GetPosition().y;

		//In tiles a gap is whole rows.
		iGap = bTiles ? ((30 + iLine - 1) / iLine) * iLine : 30;

		//One component a line, under a line there is.
		pZone = CreateCodeEditorLineComponent(pTestWindow, 30);
		pOther = CreateCodeEditorLineComponent(pTestWindow, 10);
		AssertTrue(pEditor->AddLineComponent(0, pZone));
		AssertFalse(pEditor->AddLineComponent(0, pOther));
		AssertFalse(pEditor->AddLineComponent(1, pZone));
		AssertFalse(pEditor->AddLineComponent(3, pOther));
		AssertSize(1, pEditor->NumLineComponents());
		AssertSize(0, pEditor->GetLineOf(pZone));
		AssertSize(MAX_SIZE, pEditor->GetLineOf(pOther));
		AssertTrue(&pEditor->GetComponentOn(0) == &pZone);
		AssertTrue(pEditor->GetComponentOn(1).IsNull());
		pTestWindow->Layout();

		//Under line 0, right of the gutter, as wide as the code and as high as the gap.
		AssertInt(iTop + iLine, pZone->GetPosition().y);
		AssertInt(iGap, pZone->GetActualSize().y);
		AssertInt(pTextArea->GetPosition().x, pZone->GetPosition().x);
		AssertInt(pTextArea->GetActualSize().x, pZone->GetActualSize().x);
		AssertInt(iTop + iLine + iGap, CodeEditorLineTop(pTextArea, 1));
		AssertInt(iTop + iLine + iGap, CodeEditorLineTop(pGutter, 1));
		AssertInt(3 * iLine + iGap, pEditor->GetDesiredSize().y);

		//A line feed at the start of line 0 pushes it, and the component, down.
		pTextArea->SetCaretPosition(0);
		pTextArea->Enter();
		pTestWindow->Layout();
		AssertString("\nab\ncd\nef", pTextArea->GetText());
		AssertSize(1, pEditor->GetLineOf(pZone));
		AssertInt(iTop + 2 * iLine, pZone->GetPosition().y);
		AssertInt(iTop + 2 * iLine + iGap, CodeEditorLineTop(pTextArea, 2));
		AssertInt(iTop + 2 * iLine + iGap, CodeEditorLineTop(pGutter, 2));
		AssertString(" 1 \n 2 \n 3 \n 4 ", pGutter->GetText());

		//A component that wants to be higher gets a higher gap.
		pZone->SetDesiredSize(-1, 3 * iLine);
		pTestWindow->Layout();
		AssertInt(3 * iLine, pTextArea->GetLineGap(1));
		AssertInt(3 * iLine, pZone->GetActualSize().y);
		AssertInt(iTop + 2 * iLine + 3 * iLine, CodeEditorLineTop(pGutter, 2));

		//Deleting its line removes it.
		pTextArea->SetCaretPosition(1);
		pTextArea->DeleteLine();
		pTestWindow->Layout();
		AssertString("\ncd\nef", pTextArea->GetText());
		AssertSize(0, pEditor->NumLineComponents());
		AssertSize(MAX_SIZE, pEditor->GetLineOf(pZone));
		AssertSize(0, pTextArea->NumLineGaps());
		AssertSize(0, pGutter->NumLineGaps());
		AssertInt(3 * iLine, pEditor->GetDesiredSize().y);

		//Removing a component closes its gap.
		AssertTrue(pEditor->AddLineComponent(2, pOther));
		pTestWindow->Layout();
		AssertSize(1, pGutter->NumLineGaps());
		AssertTrue(pEditor->RemoveLineComponent(pOther));
		AssertFalse(pEditor->RemoveLineComponent(pOther));
		pTestWindow->Layout();
		AssertSize(0, pTextArea->NumLineGaps());
		AssertSize(0, pGutter->NumLineGaps());

		//And all of them.
		AssertTrue(pEditor->AddLineComponent(0, pZone));
		AssertTrue(pEditor->AddLineComponent(2, pOther));
		pEditor->RemoveAllLineComponents();
		AssertSize(0, pEditor->NumLineComponents());
		AssertSize(0, pTextArea->NumLineGaps());

		pOther = NULL;
		pZone = NULL;
		pGutter = NULL;
		pTextArea = NULL;
		pEditor = NULL;
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
// In a scroll pane the code editor is as high as its code and gaps, and
// the caret is kept in view.
//
//////////////////////////////////////////////////////////////////////////
void TestCodeEditorScrollPane(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CWindowTheme>			pTheme;
		Ptr<CScrollPane>			pScroll;
		Ptr<CCodeEditor>			pEditor;
		Ptr<CTextArea>				pTextArea;
		CChars						szText;
		int32						i;
		int32						iLine;
		CInt32Rect					cCaret;
		int32						iCaretTop;

		cNativeFactory.Init(&gcMemoryAllocator, 200, 100, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Code Editor Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pTheme = CJewelTheme::CreateDarkTheme(pTestWindow, ReadLabelFont());
		pScroll = OMalloc<CScrollPane>(pTestWindow, pTheme);
		pTestWindow->SetContainer(pScroll);

		szText.Init();
		for (i = 0; i < 20; i++)
		{
			if (i > 0)
			{
				szText.Append('\n');
			}
			szText.Append("x");
		}
		pEditor = OMalloc<CCodeEditor>(pTestWindow, CreateMonoTextAreaFamily(15.0f), szText.Text());
		szText.Kill();
		pTextArea = pEditor->GetTextArea();
		pTextArea->SetTiledText(true);
		pScroll->SetContent(pEditor);
		AssertTrue(pEditor->AddLineComponent(0, CreateCodeEditorLineComponent(pTestWindow, 30)));
		pTestWindow->Layout();
		iLine = pTextArea->GetFont()->GetLineHeight();

		//The gap is whole rows of tiles.
		AssertInt(20 * iLine + ((30 + iLine - 1) / iLine) * iLine - 100, pScroll->GetMaximumScrollPosition().y);

		//The end of the code is scrolled into view.
		pTextArea->DocumentEnd();
		pTestWindow->Layout();
		AssertInt(pScroll->GetMaximumScrollPosition().y, pScroll->GetScrollPosition().y);
		pTextArea->GetCaretBounds(&cCaret);
		iCaretTop = pTextArea->GetPosition().y + cCaret.GetTop();
		AssertTrue(iCaretTop >= 0);
		AssertTrue(iCaretTop + cCaret.GetHeight() <= 100);

		pTextArea->DocumentHome();
		pTestWindow->Layout();
		AssertInt(0, pScroll->GetScrollPosition().y);

		pTextArea = NULL;
		pEditor = NULL;
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
// A mark is shown in the gutter's first column, next to its line's number.
// A press in the gutter tells the listeners the line it was next to (one
// in a line component's gap is the line above it) and doesn't select in
// the gutter; a press in the code is the code's.
//
//////////////////////////////////////////////////////////////////////////
void TestCodeEditorGutterMarks(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CFillContainer>			pFill;
		Ptr<CCodeEditor>			pEditor;
		Ptr<CTextArea>				pTextArea;
		Ptr<CTextArea>				pGutter;
		CTestCodeEditorListener		cListener;
		int32						iLine;
		int32						iTop;
		int32						iLeft;

		cNativeFactory.Init(&gcMemoryAllocator, 300, 200, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Code Editor Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pEditor = OMalloc<CCodeEditor>(pTestWindow, CreateMonoTextAreaFamily(15.0f), "a\nb\nc");
		pEditor->SetAlignment(CSH_Stretch, CSV_Stretch);
		pFill->AddComponent(pEditor);
		pTextArea = pEditor->GetTextArea();
		pGutter = pEditor->GetGutter();
		pTestWindow->Layout();
		iLine = pTextArea->GetFont()->GetLineHeight();
		iTop = pEditor->GetPosition().y;
		iLeft = pEditor->GetPosition().x;

		pEditor->SetLineMark(1, '+');
		pTestWindow->Layout();
		AssertString(" 1 \n+2 \n 3 ", pGutter->GetText());
		AssertInt('+', pEditor->GetLineMark(1));
		AssertInt('\0', pEditor->GetLineMark(0));
		AssertInt('\0', pEditor->GetLineMark(9));
		pEditor->SetLineMark(1, '-');
		pEditor->SetLineMark(2, '+');
		pTestWindow->Layout();
		AssertString(" 1 \n-2 \n+3 ", pGutter->GetText());

		//Next to line 2.
		cListener.Init();
		pEditor->GetGutterClickEvent()->Add(&cListener, &CTestCodeEditorListener::GutterClicked);
		pTestWindow->PointerMoved(iLeft + 2, iTop + iLine + iLine / 2);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertSize(1, cListener.muiClicks);
		AssertSize(1, cListener.muiLine);
		AssertSize(0, pGutter->GetCaretPosition());
		AssertFalse(pGutter->IsSelectable());
		AssertTrue(pTestWindow->GetFocus()->GetFocussedComponent() == pTextArea);

		//In line 1's gap, next to it.
		AssertTrue(pEditor->AddLineComponent(0, CreateCodeEditorLineComponent(pTestWindow, 2 * iLine)));
		pTestWindow->Layout();
		pTestWindow->PointerMoved(iLeft + 2, iTop + iLine + iLine / 2);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertSize(2, cListener.muiClicks);
		AssertSize(0, cListener.muiLine);

		//The code takes its own presses: its caret moves to line 3 (below the gap).
		pTestWindow->PointerMoved(pTextArea->GetPosition().x + 2, iTop + 4 * iLine + iLine / 2);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertSize(2, cListener.muiClicks);
		AssertSize(4, pTextArea->GetCaretPosition());

		pEditor->ClearLineMarks();
		pTestWindow->Layout();
		AssertString(" 1 \n 2 \n 3 ", pGutter->GetText());
		AssertTrue(pEditor->GetGutterClickEvent()->Remove(&cListener));

		pGutter = NULL;
		pTextArea = NULL;
		pEditor = NULL;
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
// Scrolled sideways the gutter stays at the view's left, on its
// background, over the code (and line components) scrolled under it,
// and still says which line it was clicked next to.  A caret moved to a
// line's start is shown right of the gutter.
//
//////////////////////////////////////////////////////////////////////////
void TestCodeEditorScrolledSideways(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CScrollPane>			pScroll;
		Ptr<CCodeEditor>			pEditor;
		Ptr<CTextArea>				pTextArea;
		Ptr<CTextArea>				pGutter;
		Ptr<CFillContainer>			pZone;
		CTestCodeEditorListener		cListener;
		CChars						szText;
		int32						iGutter;
		int32						iLine;
		int32						iLeft;
		int32						iTop;
		CInt32Rect					cGap;

		cNativeFactory.Init(&gcMemoryAllocator, 200, 100, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Code Editor Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pScroll = OMalloc<CScrollPane>(pTestWindow, CJewelTheme::CreateDarkTheme(pTestWindow, ReadLabelFont()));
		pTestWindow->SetContainer(pScroll);

		//A line far wider than the view.
		szText.Init("a\n");
		szText.Append('x', 60);
		szText.Append("\nc");
		pEditor = OMalloc<CCodeEditor>(pTestWindow, CreateMonoTextAreaFamily(15.0f), szText.Text());
		szText.Kill();
		pScroll->SetContent(pEditor);
		pTextArea = pEditor->GetTextArea();
		pGutter = pEditor->GetGutter();
		pTestWindow->Layout();
		iGutter = pEditor->GetGutterWidth();
		iLine = pTextArea->GetFont()->GetLineHeight();
		iLeft = pScroll->GetPosition().x;
		iTop = pEditor->GetPosition().y;
		AssertInt(iLeft, pGutter->GetPosition().x);
		AssertInt(iLeft + iGutter, pTextArea->GetPosition().x);

		pScroll->SetScrollPosition(50, 0);
		pTestWindow->Layout();
		AssertInt(50, pScroll->GetScrollPosition().x);
		AssertInt(iLeft, pGutter->GetPosition().x);
		AssertInt(iLeft + iGutter - 50, pTextArea->GetPosition().x);
		AssertTrue(pTestWindow->FindComponentAt(iLeft + 2, iTop + iLine + iLine / 2) == pGutter);

		//Next to line 2.
		cListener.Init();
		pEditor->GetGutterClickEvent()->Add(&cListener, &CTestCodeEditorListener::GutterClicked);
		pTestWindow->PointerMoved(iLeft + 2, iTop + iLine + iLine / 2);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertSize(1, cListener.muiClicks);
		AssertSize(1, cListener.muiLine);

		//A line component added since is under the gutter too.
		pZone = CreateCodeEditorLineComponent(pTestWindow, iLine);
		AssertTrue(pEditor->AddLineComponent(0, pZone));
		pTestWindow->Layout();
		AssertTrue(pTextArea->GetLineGapBounds(0, &cGap));
		AssertTrue(pZone->GetPosition().x < iLeft + iGutter);
		AssertTrue(pTestWindow->FindComponentAt(iLeft + 2, pTextArea->GetPosition().y + cGap.GetTop() + 1) == pGutter);

		//The caret at a line's start scrolls the code back out from under the gutter.
		pTextArea->SetCaretPosition(pTextArea->GetLineStart(2));
		pTestWindow->Layout();
		AssertInt(0, pScroll->GetScrollPosition().x);
		AssertInt(iLeft + iGutter, pTextArea->GetPosition().x);
		AssertTrue(pEditor->GetGutterClickEvent()->Remove(&cListener));

		pZone = NULL;
		pGutter = NULL;
		pTextArea = NULL;
		pEditor = NULL;
		pScroll = NULL;
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
void TestCodeEditor(void)
{
	BeginTests();
	ObjectsInit();

	TestCodeEditorGutter();
	TestCodeEditorLineComponents(false);
	TestCodeEditorLineComponents(true);
	TestCodeEditorScrollPane();
	TestCodeEditorGutterMarks();
	TestCodeEditorScrolledSideways();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

