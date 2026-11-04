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

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

