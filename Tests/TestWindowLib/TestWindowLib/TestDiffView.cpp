#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/Canvas.h"
#include "WindowLib/DiffDividerDraw.h"
#include "WindowLib/DiffViewColours.h"
#include "WindowLib/DiffView.h"
#include "WindowLib/ScrollBarImages.h"
#include "WindowLib/JewelTheme.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/AssertFile.h"
#include "TestLib/AssertGeometric.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"


void AssertLabelObjectsRemoved(void);
Ptr<CFont> ReadLabelFont(void);
Ptr<CFontFamily> CreateMonoTextAreaFamily(float32 fDefaultPixelSize);
Ptr<CScrollBarImages> CreateScrollBarImages(void);
Ptr<CWindowTheme> CreateScrollPaneTheme(Ptr<CScrollBarImages> pImages);
void AssertTextAreaBackground(ETextAreaBackground eType, size uiStart, size uiEnd, STextAreaBackground* psBackground);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AddTestDiffShape(CArrayDiffDividerShape* pasShapes, float64 fLeftTop, float64 fLeftBottom, float64 fRightTop, float64 fRightBottom, SDiffViewColours* psColours, ETextDiffBlockType eType)
{
	SDiffDividerShape*	psShape;

	psShape = pasShapes->Add();
	psShape->fLeftTop = fLeftTop;
	psShape->fLeftBottom = fLeftBottom;
	psShape->fRightTop = fRightTop;
	psShape->fRightBottom = fRightBottom;
	psShape->sFill = psColours->GetShape(eType);
	psShape->sOutline = psColours->GetOutline(eType);
}


//////////////////////////////////////////////////////////////////////////
//
// A modified block's shape, curving down from its lines on the left to
// its lines further down on the right; an inserted block's, pinched to a
// point on the left; a deleted block's, pinched on the right; and one cut
// off by the bottom.  The same shapes again change nothing.
//
//////////////////////////////////////////////////////////////////////////
void TestDiffDividerShapes(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char*					szDirectory;
	SDiffViewColours		sColours;

	szDirectory = (char*)"Output" _FS_ "DiffDivider";
	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);
	sColours.Init();
	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CFillContainer>			pFill;
		Ptr<CCanvas>				pCanvas;
		Ptr<CDiffDividerDraw>		pDraw;
		CArrayDiffDividerShape		asShapes;

		cNativeFactory.Init(&gcMemoryAllocator, 48, 240, szDirectory);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Diff Divider Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pDraw = OMalloc<CDiffDividerDraw>(sColours.sDivider);
		pCanvas = OMalloc<CCanvas>(pTestWindow, pDraw);
		pCanvas->SetAlignment(CSH_Stretch, CSV_Stretch);
		pFill->AddComponent(pCanvas);

		asShapes.Init();
		AddTestDiffShape(&asShapes, 20, 60, 40, 100, &sColours, TDBT_Modified);
		AddTestDiffShape(&asShapes, 120, 120, 110, 150, &sColours, TDBT_Inserted);
		AddTestDiffShape(&asShapes, 160, 200, 180, 180, &sColours, TDBT_Deleted);
		AddTestDiffShape(&asShapes, 220, 260, 225, 300, &sColours, TDBT_Modified);
		AssertTrue(pDraw->SetShapes(&asShapes));
		AssertFalse(pDraw->SetShapes(&asShapes));
		AssertSize(4, pDraw->NumShapes());
		AssertDouble(110, pDraw->GetShape(1)->fRightTop, 9);
		AssertNull(pDraw->GetShape(4));
		asShapes.Kill();

		pTestWindow->Show();

		pDraw = NULL;
		pCanvas = NULL;
		pFill = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
	}
	AssertDirectory("Input" _FS_ "DiffDivider", szDirectory);
	AssertLabelObjectsRemoved();
	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
// A small change: a line inserted, a modified line with a word changed in
// it, and a deleted line, each in its colour with the divider's shapes
// between them.
//
//////////////////////////////////////////////////////////////////////////
void TestDiffViewShown(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char*					szDirectory;

	szDirectory = (char*)"Output" _FS_ "DiffView";
	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);
	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CFillContainer>			pFill;
		Ptr<CDiffView>				pView;
		Ptr<CTextArea>				pLeft;
		Ptr<CTextArea>				pRight;

		cNativeFactory.Init(&gcMemoryAllocator, 760, 220, szDirectory);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Diff View Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pView = OMalloc<CDiffView>(pTestWindow, CJewelTheme::CreateDarkTheme(pTestWindow, ReadLabelFont()), ReadLabelFont(), CreateMonoTextAreaFamily(15.0f));
		pView->SetAlignment(CSH_Stretch, CSV_Stretch);
		pFill->AddComponent(pView);
		pLeft = pView->GetEditor(TDS_Left)->GetTextArea();
		pRight = pView->GetEditor(TDS_Right)->GetTextArea();
		pLeft->SetColour(Set32BitColour((uint8)0xBC, (uint8)0xBE, (uint8)0xC4));
		pRight->SetColour(Set32BitColour((uint8)0xBC, (uint8)0xBE, (uint8)0xC4));

		pView->ShowTexts("Old.cpp", "#include \"Engine.h\"\r\n\r\nvoid Run(int iCount)\r\n{\r\n\tStart();\r\n\tStop();\r\n}\r\n",
						 "New.cpp", "#include \"Engine.h\"\n#include \"Timer.h\"\n\nvoid Run(int iTimes)\n{\n\tStart();\n}\n");

		//Each side's lines joined with line feeds, whatever ended them.
		AssertString("Old.cpp", pView->GetTitle(TDS_Left)->GetText());
		AssertString("New.cpp", pView->GetTitle(TDS_Right)->GetText());
		AssertString("#include \"Engine.h\"\n\nvoid Run(int iCount)\n{\n\tStart();\n\tStop();\n}", pLeft->GetText());
		AssertSize(3, pView->GetDiff()->NumBlocks());
		AssertFalse(pLeft->IsEditable());
		AssertString("3 differences", pView->GetCountLabel()->GetText());
		AssertInt(TDW_Default, pView->GetWhitespaceBox()->GetSelected());
		AssertInt(TDH_Words, pView->GetHighlightBox()->GetSelected());

		//Left: a line where the inserted line would be, the modified line and its changed word, the deleted line.
		AssertSize(4, pLeft->NumBackgrounds());
		AssertTextAreaBackground(TAB_Boundary, 1, 1, pLeft->GetBackground(0));
		AssertTextAreaBackground(TAB_Lines, 2, 3, pLeft->GetBackground(1));
		AssertTextAreaBackground(TAB_Range, 34, 40, pLeft->GetBackground(2));
		AssertTextAreaBackground(TAB_Lines, 5, 6, pLeft->GetBackground(3));
		AssertSize(4, pRight->NumBackgrounds());
		AssertTextAreaBackground(TAB_Lines, 1, 2, pRight->GetBackground(0));
		AssertTextAreaBackground(TAB_Boundary, 6, 6, pRight->GetBackground(3));
		AssertSize(3, pView->GetEditor(TDS_Left)->NumChangeBars());

		pTestWindow->Show();
		AssertSize(3, pView->GetDividerDraw()->NumShapes());

		//Spans (syntax highlighting) stay when the diff is made again from the same texts, and go with a new text.
		AssertTrue(pLeft->AddSpan(0, 8, Set32BitColour((uint8)0xB3, (uint8)0xAE, (uint8)0x60)));
		pView->SetWhitespace(TDW_Ignore);
		AssertSize(1, pLeft->NumSpans());
		AssertSize(4, pLeft->NumBackgrounds());
		AssertInt(TDW_Ignore, pView->GetWhitespaceBox()->GetSelected());

		//Chosen in the toolbar.
		pView->GetHighlightBox()->Select(TDH_Lines);
		pView->HighlightChosen();
		AssertInt(TDH_Lines, pView->GetDiff()->GetHighlight());
		AssertSize(0, pView->GetDiff()->NumFragments());
		pView->GetWhitespaceBox()->Select(TDW_Default);
		pView->WhitespaceChosen();
		AssertInt(TDW_Default, pView->GetDiff()->GetWhitespace());
		pView->SetHighlight(TDH_Words);
		AssertInt(TDH_Words, pView->GetHighlightBox()->GetSelected());

		//New texts keep the titles.
		pView->SetTexts("int a;\nint c;\n", "int a;\n");
		AssertString("Old.cpp", pView->GetTitle(TDS_Left)->GetText());
		AssertString("1 difference", pView->GetCountLabel()->GetText());
		pView->SetTexts("x\n", "x\n");
		AssertString("No differences", pView->GetCountLabel()->GetText());
		pView->ShowTexts("Old.cpp", "int a;\n", "New.cpp", "int b;\n");
		AssertSize(0, pLeft->NumSpans());
		AssertSize(2, pLeft->NumBackgrounds());

		pLeft = NULL;
		pRight = NULL;
		pView = NULL;
		pFill = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
	}
	AssertDirectory("Input" _FS_ "DiffView", szDirectory);
	AssertLabelObjectsRemoved();
	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
// Appends szPrefix and n for lines n from uiStart up to uiEnd.
//
//////////////////////////////////////////////////////////////////////////
void AppendTestDiffLines(CChars* psz, const char* szPrefix, size uiStart, size uiEnd)
{
	size	i;

	for (i = uiStart; i < uiEnd; i++)
	{
		psz->Append(szPrefix);
		psz->Append((uint32)i);
		psz->Append('\n');
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Shown, the sides are at the first change, and can be scrolled to any.
// Scrolling one side scrolls the other to match: unchanged lines line up,
// shifted by the 30 lines inserted on the right before them; a middle
// inside the inserted lines maps to where they would be on the left.
// Sideways the other side goes as far.  The side scrolled to match doesn't
// scroll the first back.
//
//////////////////////////////////////////////////////////////////////////
void TestDiffViewScrolling(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CFillContainer>			pFill;
		Ptr<CDiffView>				pView;
		Ptr<CScrollPane>			pLeft;
		Ptr<CScrollPane>			pRight;
		CChars						szLeft;
		CChars						szRight;
		int32						iRow;
		int32						iHalf;
		int32						iQuarter;
		SDiffDividerShape*			psShape;
		SInt32Vec2					sVec;

		cNativeFactory.Init(&gcMemoryAllocator, 400, 200, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Diff View Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pView = OMalloc<CDiffView>(pTestWindow, CJewelTheme::CreateDarkTheme(pTestWindow, ReadLabelFont()), ReadLabelFont(), CreateMonoTextAreaFamily(15.0f));
		pView->SetAlignment(CSH_Stretch, CSV_Stretch);
		pFill->AddComponent(pView);

		//100 lines; the right has 30 lines inserted before line 40 and lines 80 to 89 deleted.
		szLeft.Init();
		AppendTestDiffLines(&szLeft, "a line long enough to scroll sideways ", 0, 100);
		szRight.Init();
		AppendTestDiffLines(&szRight, "a line long enough to scroll sideways ", 0, 40);
		AppendTestDiffLines(&szRight, "a new line long enough to scroll sideways ", 0, 30);
		AppendTestDiffLines(&szRight, "a line long enough to scroll sideways ", 40, 80);
		AppendTestDiffLines(&szRight, "a line long enough to scroll sideways ", 90, 100);
		pView->ShowTexts("Left", szLeft.Text(), "Right", szRight.Text());
		szRight.Kill();
		szLeft.Kill();
		AssertSize(2, pView->GetDiff()->NumBlocks());
		pTestWindow->Layout();

		pLeft = pView->GetPane(TDS_Left);
		pRight = pView->GetPane(TDS_Right);
		iRow = pView->GetEditor(TDS_Left)->GetTextArea()->GetFont()->GetLineHeight();
		iHalf = pLeft->GetActualSize().y / 2;
		iQuarter = pLeft->GetActualSize().y / 4;
		AssertTrue(pLeft->GetActualSize().y < 20 * iRow);

		//Shown, each side is at the first change: its lines a quarter of the way down.
		sVec = pLeft->GetScrollPosition();
		AssertInt32Vec2(0, 40 * iRow - iQuarter, &sVec);
		sVec = pRight->GetScrollPosition();
		AssertInt32Vec2(0, 40 * iRow - iQuarter, &sVec);

		//The second change: lines removed on the left, a point on the right.  Neither side follows the other.
		AssertSize(0, pView->GetCurrentBlock());
		AssertTrue(pView->ScrollToBlock(1));
		AssertFalse(pView->ScrollToBlock(2));
		pTestWindow->Layout();
		sVec = pLeft->GetScrollPosition();
		AssertInt32Vec2(0, 80 * iRow - iQuarter, &sVec);
		sVec = pRight->GetScrollPosition();
		AssertInt32Vec2(0, 110 * iRow - iQuarter, &sVec);
		AssertSize(1, pView->GetCurrentBlock());

		//Next and previous go from the difference scrolled to; F7 and Shift+F7 from in the view.
		AssertFalse(pView->NextDifference());
		AssertTrue(pView->PreviousDifference());
		pTestWindow->Layout();
		AssertSize(0, pView->GetCurrentBlock());
		sVec = pLeft->GetScrollPosition();
		AssertInt32Vec2(0, 40 * iRow - iQuarter, &sVec);
		AssertFalse(pView->PreviousDifference());
		pTestWindow->GetFocus()->SetFocus(pView->GetEditor(TDS_Left)->GetTextArea());
		pTestWindow->InputCommand(CIC_NextDifference, true);
		pTestWindow->Layout();
		AssertSize(1, pView->GetCurrentBlock());
		pTestWindow->InputCommand(CIC_PreviousDifference, true);
		pTestWindow->Layout();
		AssertSize(0, pView->GetCurrentBlock());

		//Scrolled by the user, they go from a quarter of the way down the left side.
		pLeft->SetScrollPosition(0, 60 * iRow);
		pTestWindow->Layout();
		AssertSize(DIFF_VIEW_NO_BLOCK, pView->GetCurrentBlock());
		AssertTrue(pView->NextDifference());
		AssertSize(1, pView->GetCurrentBlock());
		pTestWindow->Layout();
		pLeft->SetScrollPosition(0, 60 * iRow);
		pTestWindow->Layout();
		AssertSize(DIFF_VIEW_NO_BLOCK, pView->GetCurrentBlock());
		AssertTrue(pView->PreviousDifference());
		AssertSize(0, pView->GetCurrentBlock());
		pTestWindow->Layout();
		AssertString("2 differences", pView->GetCountLabel()->GetText());

		//Before the blocks the sides line up.
		pLeft->SetScrollPosition(7, 10 * iRow);
		pTestWindow->Layout();
		sVec = pRight->GetScrollPosition();
		AssertInt32Vec2(7, 10 * iRow, &sVec);
		sVec = pLeft->GetScrollPosition();
		AssertInt32Vec2(7, 10 * iRow, &sVec);

		//After the inserted lines the right is 30 lines further down.
		pLeft->SetScrollPosition(0, 50 * iRow);
		pTestWindow->Layout();
		sVec = pRight->GetScrollPosition();
		AssertInt32Vec2(0, 80 * iRow, &sVec);
		sVec = pLeft->GetScrollPosition();
		AssertInt32Vec2(0, 50 * iRow, &sVec);

		//The right's middle in the inserted lines: the left's middle goes to where they would be.  The left is laid
		//out before the right, so the window lays it out again.
		pRight->SetScrollPosition(0, 50 * iRow - iHalf);
		pTestWindow->Layout();
		sVec = pLeft->GetScrollPosition();
		AssertInt32Vec2(0, 40 * iRow - iHalf, &sVec);
		sVec = pRight->GetScrollPosition();
		AssertInt32Vec2(0, 50 * iRow - iHalf, &sVec);
		AssertDouble(40, pView->GetMiddleLine(TDS_Left), 6);

		//The inserted block's shape is a point on the left at the middle, and its lines on the right.
		AssertSize(1, pView->GetDividerDraw()->NumShapes());
		psShape = pView->GetDividerDraw()->GetShape(0);
		AssertDouble(psShape->fLeftTop, psShape->fLeftBottom, 6);
		AssertDouble(psShape->fRightTop + 30 * iRow, psShape->fRightBottom, 6);
		AssertTrue(psShape->fRightTop < psShape->fLeftTop);

		pLeft = NULL;
		pRight = NULL;
		pView = NULL;
		pFill = NULL;
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
void TestDiffView(void)
{
	BeginTests();
	ObjectsInit();

	TestDiffDividerShapes();
	TestDiffViewShown();
	TestDiffViewScrolling();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}
