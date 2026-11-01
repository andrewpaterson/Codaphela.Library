#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "StandardLib/Objects.h"
#include "SupportLib/FontReader.h"
#include "WindowLib/Window.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WindowLib/ScrollPane.h"
#include "WindowLib/WindowTheme.h"
#include "WindowLib/TextArea.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/AssertGeometric.h"
#include "TestLib/AssertFile.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"


Ptr<CFont> ReadLabelFont(void);
void ValidateLabelObjects(void);


//////////////////////////////////////////////////////////////////////////
//
// A light translucent thumb 8 pixels thick, like IntelliJ IDEA's dark
// theme.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CScrollBarImages> CreateScrollBarImages(void)
{
	return OMalloc<CScrollBarImages>(8, Set32BitColour((uint8)200, (uint8)200, (uint8)200, (uint8)128));
}


//////////////////////////////////////////////////////////////////////////
//
// A theme with only scroll bar images, all a scroll pane needs.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CWindowTheme> CreateScrollPaneTheme(Ptr<CScrollBarImages> pImages)
{
	Ptr<CWindowTheme>	pTheme;

	pTheme = OMalloc<CWindowTheme>(Ptr<CFont>());
	pTheme->SetScrollBarImages(pImages);
	return pTheme;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertScrollThumb(int32 iLeft, int32 iTop, int32 iRight, int32 iBottom, Ptr<CScrollBar> pScrollBar)
{
	CInt32Rect	cThumb;

	pScrollBar->GetThumbBounds(&cThumb);
	AssertInt(iLeft, cThumb.GetLeft());
	AssertInt(iTop, cThumb.GetTop());
	AssertInt(iRight, cThumb.GetRight());
	AssertInt(iBottom, cThumb.GetBottom());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertScrollPaneObjectsRemoved(void)
{
	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	AssertSize(2, gcObjects.NumMemoryIndexes());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestScrollBarImages(void)
{
	{
		Ptr<CScrollBarImages>	pImages;
		Ptr<CImageCel>			pCel;
		CSubImage*				pcSubImage;
		CColourFormatHelper		cFormat;

		pImages = OMalloc<CScrollBarImages>(7, Set32BitColour((uint8)200, (uint8)200, (uint8)200, (uint8)128));
		AssertInt(7, pImages->GetThickness());
		AssertInt(4, pImages->GetCapLength());

		//The ends are half circles as wide as the thumb is thick, the middle is one pixel long.
		pCel = pImages->GetCel(SBO_Horizontal, SBP_Start);
		pcSubImage = pCel->GetSubImage();
		AssertInt(4, pcSubImage->GetFullWidth());
		AssertInt(7, pcSubImage->GetFullHeight());
		pCel = pImages->GetCel(SBO_Horizontal, SBP_Middle);
		pcSubImage = pCel->GetSubImage();
		AssertInt(1, pcSubImage->GetFullWidth());
		AssertInt(7, pcSubImage->GetFullHeight());
		pCel = pImages->GetCel(SBO_Vertical, SBP_End);
		pcSubImage = pCel->GetSubImage();
		AssertInt(7, pcSubImage->GetFullWidth());
		AssertInt(4, pcSubImage->GetFullHeight());
		pCel = pImages->GetCel(SBO_Vertical, SBP_Middle);
		pcSubImage = pCel->GetSubImage();
		AssertInt(7, pcSubImage->GetFullWidth());
		AssertInt(1, pcSubImage->GetFullHeight());

		AssertInt(CCO_RGB, pCel->GetSourceImage()->GetColourOrder());

		//The pieces are blitted onto the window's canvases so they are made in the window's colour order.
		cFormat.Init(CFT_RGBX, CCO_BGR, CRGB_24bit, ARGB_8bit);
		pImages = OMalloc<CScrollBarImages>(7, Set32BitColour((uint8)200, (uint8)100, (uint8)50, (uint8)128), &cFormat);
		pCel = pImages->GetCel(SBO_Vertical, SBP_Middle);
		AssertInt(CCO_BGR, pCel->GetSourceImage()->GetColourOrder());
		pCel = pImages->GetCel(SBO_Horizontal, SBP_Start);
		AssertInt(CCO_BGR, pCel->GetSourceImage()->GetColourOrder());

		pCel = NULL;
		pImages = NULL;
	}

	AssertScrollPaneObjectsRemoved();
}


//////////////////////////////////////////////////////////////////////////
//
// The scroll pane fills a 200 x 100 window.  Its content is 400 x 300.
// Each track is 2 in from the edge and the thumbs are 8 thick so the
// horizontal track is 186 long (200 - 2 - 2 - 8 - 2) and the vertical 86.
//
//////////////////////////////////////////////////////////////////////////
void TestScrollPaneLayout(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CFillContainer>		pFill;
		Ptr<CScrollPane>		pScrollPane;
		Ptr<CCanvas>			pContent;
		Ptr<CScrollBar>			pHorizontal;
		Ptr<CScrollBar>			pVertical;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow		cData;
		SInt32Vec2				sVec;
		CPointer				pNull;

		cNativeFactory.Init(&gcMemoryAllocator, 200, 100, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Scroll Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);

		pScrollPane = OMalloc<CScrollPane>(pTestWindow, CreateScrollPaneTheme(CreateScrollBarImages()));
		pFill->AddComponent(pScrollPane);
		pContent = OMalloc<CCanvas>(pTestWindow, OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)90, (uint8)30, (uint8)30)));
		pContent->SetDesiredSize(400, 300);
		pScrollPane->SetContent(pContent);
		AssertTrue(pScrollPane->GetContent() == pContent);
		pHorizontal = pScrollPane->GetScrollBar(SBO_Horizontal);
		pVertical = pScrollPane->GetScrollBar(SBO_Vertical);
		AssertInt(SBO_Horizontal, pHorizontal->GetOrientation());
		AssertInt(SBO_Vertical, pVertical->GetOrientation());

		pTestWindow->Layout();

		sVec = pScrollPane->GetActualSize();
		AssertInt32Vec2(200, 100, &sVec);
		sVec = pScrollPane->GetContentSize();
		AssertInt32Vec2(400, 300, &sVec);
		sVec = pScrollPane->GetMaximumScrollPosition();
		AssertInt32Vec2(200, 200, &sVec);
		sVec = pContent->GetPosition();
		AssertInt32Vec2(0, 0, &sVec);
		sVec = pContent->GetActualSize();
		AssertInt32Vec2(400, 300, &sVec);

		//Both thumbs are as much of their track as the view is of the content.  The vertical thumb is at least twice
		//as long as it is thick.
		AssertTrue(pHorizontal->IsShown());
		AssertTrue(pVertical->IsShown());
		sVec = pHorizontal->GetPosition();
		AssertInt32Vec2(2, 90, &sVec);
		sVec = pHorizontal->GetActualSize();
		AssertInt32Vec2(186, 8, &sVec);
		sVec = pVertical->GetPosition();
		AssertInt32Vec2(190, 2, &sVec);
		sVec = pVertical->GetActualSize();
		AssertInt32Vec2(8, 86, &sVec);
		AssertScrollThumb(2, 90, 2 + 93, 98, pHorizontal);
		AssertScrollThumb(190, 2, 198, 2 + 28, pVertical);

		//Scrolling moves the content and the thumbs once laid out again.
		pScrollPane->SetScrollPosition(100, 50);
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(100, 50, &sVec);
		pTestWindow->Layout();
		sVec = pContent->GetPosition();
		AssertInt32Vec2(-100, -50, &sVec);
		AssertScrollThumb(2 + 46, 90, 2 + 46 + 93, 98, pHorizontal);
		AssertScrollThumb(190, 2 + 14, 198, 2 + 14 + 28, pVertical);

		//The scroll position is kept between 0 and the maximum.
		pScrollPane->SetScrollPosition(1000, 1000);
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(200, 200, &sVec);
		pTestWindow->Layout();
		AssertScrollThumb(2 + 93, 90, 2 + 186, 98, pHorizontal);
		AssertScrollThumb(190, 2 + 58, 198, 2 + 86, pVertical);
		pScrollPane->SetScrollPosition(SBO_Vertical, -5);
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(200, 0, &sVec);

		//The whole of each track is hit, not just the thumb.  The margins and the corner between the tracks aren't.
		pTestWindow->Layout();
		AssertTrue(pScrollPane->FindComponentAt(150, 94) == pHorizontal);
		AssertTrue(pScrollPane->FindComponentAt(50, 94) == pHorizontal);
		AssertTrue(pScrollPane->FindComponentAt(194, 10) == pVertical);
		AssertTrue(pScrollPane->FindComponentAt(194, 60) == pVertical);
		AssertTrue(pScrollPane->FindComponentAt(1, 94) == pContent);
		AssertTrue(pScrollPane->FindComponentAt(194, 95) == pContent);
		AssertTrue(pScrollPane->FindComponentAt(100, 50) == pContent);

		//Never scrolled horizontally the content gets the view's width and only the vertical scroll bar is shown, the
		//whole height of the scroll pane.
		pScrollPane->SetPolicy(SBO_Horizontal, SPP_Never);
		AssertInt(SPP_Never, pScrollPane->GetPolicy(SBO_Horizontal));
		pTestWindow->Layout();
		sVec = pContent->GetActualSize();
		AssertInt32Vec2(200, 300, &sVec);
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(0, 0, &sVec);
		AssertFalse(pHorizontal->IsShown());
		AssertTrue(pVertical->IsShown());
		sVec = pVertical->GetActualSize();
		AssertInt32Vec2(8, 96, &sVec);
		AssertTrue(pScrollPane->FindComponentAt(150, 94) == pContent);
		pScrollPane->SetPolicy(SBO_Horizontal, SPP_AsNeeded);

		//Content smaller than the view isn't scrolled and is placed by its alignment.
		pContent->SetDesiredSize(50, 50);
		pScrollPane->SetScrollPosition(10, 10);
		pTestWindow->Layout();
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(0, 0, &sVec);
		sVec = pContent->GetPosition();
		AssertInt32Vec2(75, 25, &sVec);
		AssertFalse(pHorizontal->IsShown());
		AssertFalse(pVertical->IsShown());
		AssertTrue(pScrollPane->FindComponentAt(194, 10) == pScrollPane);

		//Variable sized content fills the view.
		pContent->SetDesiredSize(-1, -1);
		pTestWindow->Layout();
		sVec = pContent->GetActualSize();
		AssertInt32Vec2(200, 100, &sVec);

		pVertical = NULL;
		pHorizontal = NULL;
		pContent = NULL;
		pScrollPane = NULL;
		pFill = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
	}

	AssertScrollPaneObjectsRemoved();
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Pressing a thumb and dragging it scrolls the content.
//
//////////////////////////////////////////////////////////////////////////
void TestScrollPaneDrag(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CFillContainer>		pFill;
		Ptr<CScrollPane>		pScrollPane;
		Ptr<CCanvas>			pContent;
		Ptr<CScrollBar>			pVertical;
		Ptr<CScrollBar>			pHorizontal;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow		cData;
		SInt32Vec2				sVec;
		CPointer				pNull;

		cNativeFactory.Init(&gcMemoryAllocator, 200, 100, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Scroll Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);

		pScrollPane = OMalloc<CScrollPane>(pTestWindow, CreateScrollPaneTheme(CreateScrollBarImages()));
		pFill->AddComponent(pScrollPane);
		pContent = OMalloc<CCanvas>(pTestWindow, OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)90, (uint8)30, (uint8)30)));
		pContent->SetDesiredSize(400, 300);
		pScrollPane->SetContent(pContent);
		pVertical = pScrollPane->GetScrollBar(SBO_Vertical);
		pHorizontal = pScrollPane->GetScrollBar(SBO_Horizontal);
		pTestWindow->Layout();

		//The vertical thumb is at 190, 2 to 198, 30.  Pressing it 8 down from its top grabs it there.
		pTestWindow->PointerMoved(194, 10);
		AssertTrue(pTestWindow->GetFocus()->GetMouseOverComponent() == pVertical);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		AssertTrue(pTestWindow->GetFocus()->GetFocussedComponent() == pVertical);
		AssertTrue(pVertical->IsDragging());

		//The thumb's 58 pixels of travel are the content's 200 pixels of scroll.
		pTestWindow->PointerMoved(194, 39);
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(0, 100, &sVec);
		pTestWindow->Layout();
		AssertScrollThumb(190, 2 + 29, 198, 2 + 29 + 28, pVertical);
		sVec = pContent->GetPosition();
		AssertInt32Vec2(0, -100, &sVec);

		//The drag goes on wherever the pointer goes, and the thumb stops at the ends of its track.
		pTestWindow->PointerMoved(20, 500);
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(0, 200, &sVec);
		pTestWindow->PointerMoved(20, -500);
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(0, 0, &sVec);
		pTestWindow->PointerMoved(194, 25);
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(0, 52, &sVec);

		//Releasing the button stops the drag.
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertFalse(pVertical->IsDragging());
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(0, 52, &sVec);
		pTestWindow->PointerMoved(194, 60);
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(0, 52, &sVec);

		//The horizontal thumb is 2, 90 to 95, 98.  Dragged half its travel it scrolls half the content's.
		pTestWindow->Layout();
		pTestWindow->PointerMoved(10, 94);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		AssertTrue(pHorizontal->IsDragging());
		pTestWindow->PointerMoved(10 + 46, 94);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(99, 52, &sVec);
		AssertFalse(pHorizontal->IsDragging());

		//Pressing the track below the thumb (at 17 to 45) jumps the thumb's middle to the pointer, as far as the end of
		//the track, and drags it from there.
		pTestWindow->Layout();
		AssertScrollThumb(190, 2 + 15, 198, 2 + 15 + 28, pVertical);
		pTestWindow->PointerMoved(194, 80);
		AssertTrue(pTestWindow->GetFocus()->GetMouseOverComponent() == pVertical);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		AssertTrue(pVertical->IsDragging());
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(99, 200, &sVec);
		pTestWindow->PointerMoved(194, 60);
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(99, 131, &sVec);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertFalse(pVertical->IsDragging());

		//Pressing the track above the thumb (at 39 to 67) centres the thumb on the pointer.
		pTestWindow->Layout();
		AssertScrollThumb(190, 2 + 37, 198, 2 + 37 + 28, pVertical);
		pTestWindow->PointerMoved(194, 22);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(99, 21, &sVec);
		pTestWindow->Layout();
		AssertScrollThumb(190, 2 + 6, 198, 2 + 6 + 28, pVertical);

		pHorizontal = NULL;
		pVertical = NULL;
		pContent = NULL;
		pScrollPane = NULL;
		pFill = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
	}

	AssertScrollPaneObjectsRemoved();
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The wheel scrolls the scroll pane under the pointer.
//
//////////////////////////////////////////////////////////////////////////
void TestScrollPaneWheel(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CFillContainer>		pFill;
		Ptr<CScrollPane>		pScrollPane;
		Ptr<CScrollPane>		pInner;
		Ptr<CCanvas>			pContent;
		Ptr<CScrollBarImages>	pImages;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow		cData;
		SInt32Vec2				sVec;
		CPointer				pNull;

		cNativeFactory.Init(&gcMemoryAllocator, 200, 100, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Scroll Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);

		pImages = CreateScrollBarImages();
		pScrollPane = OMalloc<CScrollPane>(pTestWindow, CreateScrollPaneTheme(pImages));
		pFill->AddComponent(pScrollPane);
		pContent = OMalloc<CCanvas>(pTestWindow, OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)90, (uint8)30, (uint8)30)));
		pContent->SetDesiredSize(400, 300);
		pScrollPane->SetContent(pContent);
		AssertInt(48, pScrollPane->GetWheelStep());
		pTestWindow->Layout();

		//Each notch scrolls by the wheel step, as far as the end.
		pTestWindow->PointerMoved(100, 50);
		pTestWindow->InputCommand(CIC_WheelDown, true);
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(0, 48, &sVec);
		pTestWindow->InputCommand(CIC_WheelDown, true);
		pTestWindow->InputCommand(CIC_WheelDown, true);
		pTestWindow->InputCommand(CIC_WheelDown, true);
		pTestWindow->InputCommand(CIC_WheelDown, true);
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(0, 200, &sVec);
		pTestWindow->InputCommand(CIC_WheelUp, true);
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(0, 152, &sVec);

		//Sideways scrolls horizontally.
		pScrollPane->SetWheelStep(30);
		pTestWindow->InputCommand(CIC_WheelRight, true);
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(30, 152, &sVec);
		pTestWindow->InputCommand(CIC_WheelLeft, true);
		pTestWindow->InputCommand(CIC_WheelLeft, true);
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(0, 152, &sVec);

		//Over a scroll bar the wheel still scrolls its scroll pane.
		pTestWindow->Layout();
		pTestWindow->PointerMoved(194, 50);
		AssertTrue(pTestWindow->GetFocus()->GetMouseOverComponent() == pScrollPane->GetScrollBar(SBO_Vertical));
		pTestWindow->InputCommand(CIC_WheelUp, true);
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(0, 122, &sVec);

		//With the pointer outside the window the wheel does nothing.
		pTestWindow->PointerLeft();
		pTestWindow->InputCommand(CIC_WheelUp, true);
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(0, 122, &sVec);

		//An inner scroll pane that can only scroll horizontally passes vertical turns to the outer one.
		pInner = OMalloc<CScrollPane>(pTestWindow, CreateScrollPaneTheme(pImages));
		pInner->SetDesiredSize(-1, 300);
		pScrollPane->SetContent(pInner);
		pInner->SetContent(pContent);
		pScrollPane->SetPolicy(SBO_Horizontal, SPP_Never);
		pTestWindow->Layout();
		sVec = pInner->GetActualSize();
		AssertInt32Vec2(200, 300, &sVec);
		pTestWindow->PointerMoved(100, 50);
		AssertTrue(pTestWindow->GetFocus()->GetMouseOverComponent() == pContent);
		pTestWindow->InputCommand(CIC_WheelDown, true);
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(0, 30, &sVec);
		sVec = pInner->GetScrollPosition();
		AssertInt32Vec2(0, 0, &sVec);
		pTestWindow->InputCommand(CIC_WheelRight, true);
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(0, 30, &sVec);
		sVec = pInner->GetScrollPosition();
		AssertInt32Vec2(48, 0, &sVec);

		pInner = NULL;
		pImages = NULL;
		pContent = NULL;
		pScrollPane = NULL;
		pFill = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
	}

	AssertScrollPaneObjectsRemoved();
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Draws pContent in a scroll pane inside a fill container with insets,
// scrolled to sScroll, and compares the frame with the expected one.  The
// content mustn't be drawn in the insets.
//
//////////////////////////////////////////////////////////////////////////
void TestScrollPaneDraw(char* szName, int32 iWidth, int32 iHeight, const char* szText, bool bWordWrap, int32 iScrollX, int32 iScrollY)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	CChars					szDirectory;

	szDirectory.Init("Output" _FS_);
	szDirectory.Append(szName);
	cFileUtil.RemoveDir(szDirectory.Text());
	cFileUtil.MakeDir(szDirectory.Text());

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CFont>				pFont;
		Ptr<CFillContainer>		pFill;
		Ptr<CClearCanvasDraw>	pBackground;
		Ptr<CScrollPane>		pScrollPane;
		Ptr<CTextArea>			pTextArea;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow		cData;
		SInsets					sInsets;
		int						aiInsets[4];
		CChars					szExpectedDirectory;
		CPointer				pNull;
		SInt32Vec2				sVec;

		cNativeFactory.Init(&gcMemoryAllocator, iWidth, iHeight, szDirectory.Text());
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pBackground = OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)40, (uint8)60, (uint8)110));
		pTestWindow = OMalloc<CWindow>("Scroll Window", &cNativeFactory, pTick, pBackground, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pFill = OMalloc<CFillContainer>(pTestWindow);
		aiInsets[BEI_Top] = 10;
		aiInsets[BEI_Right] = 10;
		aiInsets[BEI_Bottom] = 10;
		aiInsets[BEI_Left] = 10;
		sInsets.Init(aiInsets);
		pFill->SetInsets(&sInsets);
		pTestWindow->SetContainer(pFill);

		pFont = ReadLabelFont();
		AssertTrue(pFont.IsNotNull());

		pScrollPane = OMalloc<CScrollPane>(pTestWindow, CreateScrollPaneTheme(CreateScrollBarImages()));
		pFill->AddComponent(pScrollPane);
		pTextArea = OMalloc<CTextArea>(pTestWindow, pFont, szText);
		pTextArea->SetWordWrap(bWordWrap);
		pScrollPane->SetContent(pTextArea);
		pScrollPane->SetScrollPosition(iScrollX, iScrollY);

		pTestWindow->Show();

		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(iScrollX, iScrollY, &sVec);

		ValidateLabelObjects();

		pTextArea = NULL;
		pScrollPane = NULL;
		pFont = NULL;
		pFill = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		pBackground = NULL;

		szExpectedDirectory.Init(szDirectory);
		szExpectedDirectory.Replace("Output", "Input");
		AssertDirectory(szExpectedDirectory.Text(), szDirectory.Text());
		szExpectedDirectory.Kill();
	}

	AssertScrollPaneObjectsRemoved();
	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory.Text());
	szDirectory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestScrollPaneDraws(void)
{
	//A word wrapped text area only scrolls vertically.
	TestScrollPaneDraw("ScrollPaneWrappedText", 200, 120,
		"The quick brown fox jumps over the lazy dog.\nPack my box with five dozen liquor jugs.\nHow vexingly quick daft zebras jump!",
		true, 0, 40);

	//Without word wrap the text is wider than the view too.
	TestScrollPaneDraw("ScrollPaneBothBars", 200, 120,
		"The quick brown fox jumps over the lazy dog.\nPack my box with five dozen liquor jugs.\nHow vexingly quick daft zebras jump!\nSphinx of black quartz, judge my vow.\nThe five boxing wizards jump quickly.",
		false, 60, 15);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestScrollPane(void)
{
	BeginTests();
	ObjectsInit();

	TestScrollBarImages();
	TestScrollPaneLayout();
	TestScrollPaneDrag();
	TestScrollPaneWheel();
	TestScrollPaneDraws();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

