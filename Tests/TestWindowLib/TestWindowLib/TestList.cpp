#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WindowLib/ListContainer.h"
#include "WindowLib/ScrollPane.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/AssertGeometric.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"
#include "TestWindow.h"


Ptr<CWindowTheme> CreateTestTheme(Ptr<CFont> pFont);


#define AssertLog(e)	AssertString(e, szLog.Text()); szLog.Clear()


Ptr<CScrollBarImages> CreateScrollBarImages(void);
Ptr<CWindowTheme> CreateScrollPaneTheme(Ptr<CScrollBarImages> pImages);


//////////////////////////////////////////////////////////////////////////
//
// Logs "Selected:row;" and "Activated:row;".
//
//////////////////////////////////////////////////////////////////////////
class CTestListListener
{
public:
	CChars*		mpszLog;

	void Init(CChars* pszLog)
	{
		mpszLog = pszLog;
	}

	void SelectionChanged(CListContainer* pcList)
	{
		mpszLog->Append("Selected:");
		mpszLog->Append(pcList->GetSelected());
		mpszLog->Append(";");
	}

	void ItemActivated(CListContainer* pcList)
	{
		mpszLog->Append("Activated:");
		mpszLog->Append(pcList->GetSelected());
		mpszLog->Append(";");
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CCanvas> CreateListItem(Ptr<CWindow> pWindow, int32 iWidth, int32 iHeight, ARGB32 sColour)
{
	Ptr<CCanvas>	pItem;

	pItem = OMalloc<CCanvas>(pWindow, OMalloc<CClearCanvasDraw>(sColour));
	pItem->SetDesiredSize(iWidth, iHeight);
	return pItem;
}


//////////////////////////////////////////////////////////////////////////
//
// The window is 100x100 and is the list.  The list has five rows, each a
// 40x10 canvas, and the middle one is disabled like a separator.  The rows
// don't fill the list's height so they are centred, the first starting 25
// down.  They take the list's whole width.
//
//////////////////////////////////////////////////////////////////////////
void TestListInput(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CListContainer>		pList;
		Ptr<CFocus>				pFocus;
		Ptr<CTickTestRefWindow>	pTick;
		SDataTestRefWindow		cData;
		CTestListListener		cListener;
		CPointer				pNull;
		CChars					szLog;
		SInt32Vec2				sVec;
		int32					i;
		int32					iTop;
		int32					iHeight;
		int						x;
		SLogConfig				sLogConfig;

		szLog.Init();
		cListener.Init(&szLog);
		cNativeFactory.Init(&gcMemoryAllocator, 100, 100, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("List Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFocus = pTestWindow->GetFocus();

		pList = OMalloc<CListContainer>(pTestWindow, CreateTestTheme(Ptr<CFont>()));
		pList->GetSelectionEvent()->Add(&cListener, &CTestListListener::SelectionChanged);
		pList->GetActivationEvent()->Add(&cListener, &CTestListListener::ItemActivated);
		pTestWindow->SetContainer(pList);
		for (i = 0; i < 5; i++)
		{
			AssertInt(i, pList->AddItem(CreateListItem(pTestWindow, 40, 10, Set32BitColour((uint8)0x10, (uint8)0x20, (uint8)0x30)), i != 2));
		}
		AssertInt(5, pList->NumRows());
		AssertTrue(pList->IsRowEnabled(1));
		AssertFalse(pList->IsRowEnabled(2));
		AssertFalse(pList->IsRowEnabled(5));
		gcLogger.GetConfig(&sLogConfig);
		gcLogger.SetBreakOnError(false);
		AssertFalse(pList->AddToRow(5, CreateListItem(pTestWindow, 10, 10, 0)));
		gcLogger.SetConfig(&sLogConfig);

		//The list needs its rows' size.
		sVec = pList->GetDesiredSize();
		AssertInt32Vec2(40, 50, &sVec);

		pTestWindow->Layout();
		for (i = 0; i < 5; i++)
		{
			AssertTrue(pList->GetTrackBounds(false, i, &iTop, &iHeight));
			AssertInt(25 + i * 10, iTop);
			AssertInt(10, iHeight);
		}
		AssertFalse(pList->GetTrackBounds(false, 5, &iTop, &iHeight));
		AssertInt(-1, pList->FindRowAt(50, 24));
		AssertInt(0, pList->FindRowAt(50, 25));
		AssertInt(0, pList->FindRowAt(0, 34));
		AssertInt(1, pList->FindRowAt(99, 35));
		AssertInt(4, pList->FindRowAt(50, 74));
		AssertInt(-1, pList->FindRowAt(50, 75));
		AssertInt(-1, pList->FindRowAt(100, 40));

		//The list is hit as a whole, not its rows.
		AssertTrue(pTestWindow->FindComponentAt(50, 40) == pList);
		AssertTrue(pTestWindow->FindComponentAt(50, 5) == pList);

		//Moving over a row marks it.  The hover highlight covers the row across the list.
		x = 50;
		pTestWindow->PointerMoved(x, 37);
		AssertInt(1, pList->GetHover());
		AssertInt(-1, pList->GetSelected());
		sVec = pList->GetHoverHighlight()->GetPosition();
		AssertInt32Vec2(0, 35, &sVec);
		sVec = pList->GetHoverHighlight()->GetActualSize();
		AssertInt32Vec2(100, 10, &sVec);
		pTestWindow->PointerMoved(x, 66);
		AssertInt(4, pList->GetHover());
		pTestWindow->PointerMoved(x, 10);
		AssertInt(-1, pList->GetHover());

		//Pressing a row selects it and releasing over it activates it.
		pTestWindow->PointerMoved(x, 37);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		AssertTrue(pList->IsFocussed());
		AssertLog("Selected:1;");
		sVec = pList->GetSelectedHighlight()->GetPosition();
		AssertInt32Vec2(0, 35, &sVec);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertLog("Activated:1;");

		//A disabled row can't be selected, nor can below the rows.
		pTestWindow->PointerMoved(x, 47);
		AssertInt(2, pList->GetHover());
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		pTestWindow->PointerMoved(x, 90);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertLog("");
		AssertInt(1, pList->GetSelected());

		//Releasing over another row, or off the list, doesn't activate.
		pTestWindow->PointerMoved(x, 57);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		AssertLog("Selected:3;");
		pTestWindow->PointerMoved(x, 67);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		pTestWindow->PointerMoved(x, 57);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->PointerLeft();
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertLog("");
		AssertInt(-1, pList->GetHover());

		//The keys move the selection over the enabled rows.
		pTestWindow->InputCommand(CIC_Up, true);
		AssertLog("Selected:1;");
		pTestWindow->InputCommand(CIC_Down, true);
		AssertLog("Selected:3;");
		pTestWindow->InputCommand(CIC_End, true);
		AssertLog("Selected:4;");
		pTestWindow->InputCommand(CIC_Down, true);
		AssertLog("");
		pTestWindow->InputCommand(CIC_Home, true);
		AssertLog("Selected:0;");
		pTestWindow->InputCommand(CIC_Up, true);
		AssertLog("");
		pTestWindow->InputCommand(CIC_DocumentEnd, true);
		pTestWindow->InputCommand(CIC_DocumentHome, true);
		AssertLog("Selected:4;Selected:0;");

		//Page down goes back to the nearest enabled row before a disabled one, unless that is where it started.
		pList->SetPageRows(2);
		pTestWindow->InputCommand(CIC_PageDown, true);
		AssertLog("Selected:1;");
		pTestWindow->InputCommand(CIC_PageDown, true);
		AssertLog("Selected:3;");
		pTestWindow->InputCommand(CIC_PageDown, true);
		AssertLog("Selected:4;");
		pTestWindow->InputCommand(CIC_PageDown, true);
		AssertLog("");
		pTestWindow->InputCommand(CIC_PageUp, true);
		AssertLog("Selected:3;");
		pTestWindow->InputCommand(CIC_PageUp, true);
		AssertLog("Selected:1;");
		pTestWindow->InputCommand(CIC_PageUp, true);
		AssertLog("Selected:0;");
		pTestWindow->InputCommand(CIC_PageUp, true);
		AssertLog("");

		//Enter, space and jump to source (F4) activate the selected row.
		AssertTrue(pList->WantsEnter());
		pTestWindow->InputCommand(CIC_Enter, true);
		pTestWindow->InputCommand(CIC_Space, true);
		pTestWindow->InputCommand(CIC_JumpToSource, true);
		AssertLog("Activated:0;Activated:0;Activated:0;");

		//Disabling the selected row clears the selection.
		AssertFalse(pList->Select(2));
		AssertFalse(pList->Select(5));
		AssertInt(0, pList->GetSelected());
		pList->SetRowEnabled(0, false);
		AssertLog("Selected:-1;");
		AssertFalse(pList->WantsEnter());
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertLog("");
		pTestWindow->InputCommand(CIC_Down, true);
		AssertLog("Selected:1;");
		pList->SetRowEnabled(0, true);
		AssertTrue(pList->Select(-1));
		AssertLog("Selected:-1;");
		pTestWindow->InputCommand(CIC_Up, true);
		AssertLog("Selected:4;");

		//When the selection follows the pointer, pressing, dragging to a row and releasing activates that row.
		pList->SetSelectOnHover(true);
		pTestWindow->PointerMoved(x, 27);
		AssertLog("Selected:0;");
		pTestWindow->PointerMoved(x, 47);
		AssertLog("");
		AssertInt(2, pList->GetHover());
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->PointerMoved(x, 37);
		pTestWindow->PointerMoved(x, 67);
		AssertLog("Selected:1;Selected:4;");
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertLog("Activated:4;");
		pTestWindow->PointerMoved(x, 10);
		AssertInt(4, pList->GetSelected());

		//Deactivating the window ends a press.
		pTestWindow->PointerMoved(x, 57);
		AssertLog("Selected:3;");
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->Deactivate();
		pTestWindow->Activate();
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertLog("");

		//When double clicks activate, a row is only activated by pressing and releasing it twice quickly.  Releasing on
		//another row in between starts again.  Enter still activates.
		pList->SetDoubleClickActivates(true);
		pTestWindow->PointerMoved(x, 57);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertLog("");
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertLog("Activated:3;");
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertLog("");
		pTestWindow->PointerMoved(x, 67);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertLog("Selected:4;");
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertLog("Activated:4;");
		pList->SetDoubleClickActivates(false);

		//A row that comes under the pointer without it moving, as when a popup opens under it, is activated by pressing
		//and releasing it.  A sixth row moves the rows up so the first is under the pointer.
		pList->SetSelectOnHover(false);
		pTestWindow->PointerMoved(x, 20);
		AssertInt(-1, pList->GetHover());
		AssertInt(5, pList->AddItem(CreateListItem(pTestWindow, 40, 10, 0)));
		pTestWindow->Layout();
		AssertInt(0, pList->FindRowAt(x, 20));
		AssertInt(-1, pList->GetHover());
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertLog("Selected:0;Activated:0;");

		//Removing everything leaves only the highlights.
		AssertTrue(pList->RemoveAllComponents());
		AssertLog("Selected:-1;");
		AssertInt(0, pList->NumRows());
		AssertSize(2, pList->NumComponents());
		pTestWindow->InputCommand(CIC_Down, true);
		AssertLog("");
		AssertInt(0, pList->AddItem(CreateListItem(pTestWindow, 40, 10, 0)));
		pTestWindow->Layout();
		pTestWindow->InputCommand(CIC_Down, true);
		AssertLog("Selected:0;");

		AssertTrue(pList->GetSelectionEvent()->Remove(&cListener));
		AssertTrue(pList->GetActivationEvent()->Remove(&cListener));

		pList = NULL;
		pFocus = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		szLog.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	AssertSize(2, gcObjects.NumMemoryIndexes());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A 100x40 window with a 40x10 white item in each of four rows filling it.
// Row 1 is selected and the pointer is over row 2.  The selected row is
// red under its item and the hovered row green, both across the window;
// the other rows show the black background.  The painted frame is written
// to Output/ListDraw.
//
//////////////////////////////////////////////////////////////////////////
void TestListDraw(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	const char*				szDirectory;

	szDirectory = "Output" _FS_ "ListDraw";
	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CListContainer>		pList;
		Ptr<CCanvas>			pItem;
		Ptr<CTickTestRefWindow>	pTick;
		SDataTestRefWindow		cData;
		CPointer				pNull;
		int32					i;
		uint8*					puiPixels;
		uint8*					puiPixel;

		cNativeFactory.Init(&gcMemoryAllocator, 100, 40, (char*)szDirectory);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("List Draw Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pList = OMalloc<CListContainer>(pTestWindow, CreateTestTheme(Ptr<CFont>()));
		pList->SetHighlightColours(Set32BitColour((uint8)0xFF, (uint8)0, (uint8)0), Set32BitColour((uint8)0, (uint8)0xFF, (uint8)0));
		pTestWindow->SetContainer(pList);
		for (i = 0; i < 4; i++)
		{
			pItem = CreateListItem(pTestWindow, 40, 10, Set32BitColour((uint8)0xFF, (uint8)0xFF, (uint8)0xFF));
			pItem->SetAlignment(CSH_Left, CSV_Top);
			pList->AddItem(pItem);
		}

		pTestWindow->Layout();
		AssertTrue(pList->Select(1));
		pTestWindow->PointerMoved(50, 25);
		AssertInt(2, pList->GetHover());
		pTestWindow->Paint();

		puiPixels = pTestWindow->GetCanvas()->GetPixelData();
		puiPixel = &puiPixels[(5 * 100 + 70) * 3];
		AssertInt(0, puiPixel[0]);
		AssertInt(0, puiPixel[1]);
		AssertInt(0, puiPixel[2]);
		puiPixel = &puiPixels[(15 * 100 + 70) * 3];
		AssertInt(0xFF, puiPixel[0]);
		AssertInt(0, puiPixel[1]);
		AssertInt(0, puiPixel[2]);
		puiPixel = &puiPixels[(15 * 100 + 10) * 3];
		AssertInt(0xFF, puiPixel[0]);
		AssertInt(0xFF, puiPixel[1]);
		AssertInt(0xFF, puiPixel[2]);
		puiPixel = &puiPixels[(25 * 100 + 99) * 3];
		AssertInt(0, puiPixel[0]);
		AssertInt(0xFF, puiPixel[1]);
		AssertInt(0, puiPixel[2]);
		puiPixel = &puiPixels[(35 * 100 + 70) * 3];
		AssertInt(0, puiPixel[0]);
		AssertInt(0, puiPixel[1]);
		AssertInt(0, puiPixel[2]);

		pItem = NULL;
		pList = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	AssertSize(2, gcObjects.NumMemoryIndexes());
	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
// Twenty rows ten high in a scroll pane that shows five.  The keys keep
// the selected row in view, a page is the rows shown less one, and with
// wrap around up from the first row goes to the last.
//
//////////////////////////////////////////////////////////////////////////
void TestListKeysScroll(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CScrollPane>		pScrollPane;
		Ptr<CListContainer>		pList;
		Ptr<CTickTestRefWindow>	pTick;
		SDataTestRefWindow		cData;
		CPointer				pNull;
		SInt32Vec2				sVec;
		int32					i;

		cNativeFactory.Init(&gcMemoryAllocator, 100, 50, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("List Keys Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pScrollPane = OMalloc<CScrollPane>(pTestWindow, CreateScrollPaneTheme(CreateScrollBarImages()));
		pTestWindow->SetContainer(pScrollPane);
		pList = OMalloc<CListContainer>(pTestWindow, CreateTestTheme(Ptr<CFont>()));
		pList->SetAlignment(CSH_Stretch, CSV_Top);
		pScrollPane->SetContent(pList);
		for (i = 0; i < 20; i++)
		{
			pList->AddItem(CreateListItem(pTestWindow, 40, 10, Set32BitColour((uint8)0x10, (uint8)0x20, (uint8)0x30)));
		}
		pTestWindow->Layout();
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(0, 0, &sVec);

		//Down past the view's bottom scrolls just far enough to show the row.
		for (i = 0; i < 6; i++)
		{
			pList->Down();
		}
		AssertInt(5, pList->GetSelected());
		pTestWindow->Layout();
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(0, 10, &sVec);

		//The view shows five rows, so a page is four.
		pList->PageDown();
		AssertInt(9, pList->GetSelected());
		pTestWindow->Layout();
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(0, 50, &sVec);
		pList->PageUp();
		AssertInt(5, pList->GetSelected());
		pTestWindow->Layout();
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(0, 50, &sVec);

		//Without wrap around the ends stop the selection.
		pList->Home();
		pList->Up();
		AssertInt(0, pList->GetSelected());
		pTestWindow->Layout();
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(0, 0, &sVec);

		pList->SetWrapAround(true);
		pList->Up();
		AssertInt(19, pList->GetSelected());
		pTestWindow->Layout();
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(0, 150, &sVec);
		pList->Down();
		AssertInt(0, pList->GetSelected());
		pTestWindow->Layout();
		sVec = pScrollPane->GetScrollPosition();
		AssertInt32Vec2(0, 0, &sVec);

		pList = NULL;
		pScrollPane = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
	}
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The rows selected, as "1,2,4 lead 4".
//
//////////////////////////////////////////////////////////////////////////
static char* PrintListSelection(Ptr<CListContainer> pList, CChars* psz)
{
	int32	i;
	bool	bFirst;

	psz->Clear();
	bFirst = true;
	for (i = 0; i < pList->NumRows(); i++)
	{
		if (pList->IsRowSelected(i))
		{
			psz->Append(bFirst ? "" : ",");
			psz->Append(i);
			bFirst = false;
		}
	}
	psz->Append(" lead ");
	psz->Append(pList->GetSelected());
	return psz->Text();
}


//////////////////////////////////////////////////////////////////////////
//
// The window's painted pixel as 0xRRGGBB.  The window is 24 bit RGB.
//
//////////////////////////////////////////////////////////////////////////
static uint32 ListWindowPixel(Ptr<CWindow> pWindow, int32 x, int32 y)
{
	uint8*	puiPixel;

	puiPixel = &pWindow->GetCanvas()->GetPixelData()[(y * pWindow->GetCanvas()->GetActualSize().x + x) * 3];
	return ((uint32)puiPixel[0] << 16) | ((uint32)puiPixel[1] << 8) | (uint32)puiPixel[2];
}


//////////////////////////////////////////////////////////////////////////
//
// A list that lets several rows be selected: Shift with the keys or a
// press selects from where the selection started, Control with a press
// selects or unselects a row, Control+A selects them all, and a plain move
// selects one again.  Every selected row is drawn selected.  A list that
// doesn't selects one row whatever it is asked.
//
// Six rows of 40x10 at the list's left, centred down it (the first is 20
// down); row 3 is disabled.
//
//////////////////////////////////////////////////////////////////////////
void TestListMultiSelect(void)
{
	STestWindow		cWindow;

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CListContainer>		pList;
		Ptr<CCanvas>			pItem;
		CTestListListener		cListener;
		SComponentInputEvent	sEvent;
		CChars					szLog;
		CChars					sz;
		int32					i;

		szLog.Init();
		sz.Init();
		cListener.Init(&szLog);
		//The window clears behind the list, whose selected rows are drawn on a transparent canvas.
		cWindow.Init("List Multi Window", 100, 100, OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)0, (uint8)0, (uint8)0)));
		pTestWindow = cWindow.pWindow;
		pList = OMalloc<CListContainer>(pTestWindow, CreateTestTheme(Ptr<CFont>()));
		pList->SetAlignment(CSH_Stretch, CSV_Top);
		pList->SetHighlightColours(Set32BitColour((uint8)0x80, (uint8)0x20, (uint8)0x20), Set32BitColour((uint8)0x20, (uint8)0x80, (uint8)0x20));
		pTestWindow->SetContainer(pList);
		for (i = 0; i < 6; i++)
		{
			pItem = CreateListItem(pTestWindow, 40, 10, Set32BitColour((uint8)0x10, (uint8)0x20, (uint8)0x30));
			pItem->SetAlignment(CSH_Left, CSV_Top);
			pList->AddItem(pItem, i != 3);
		}
		pList->GetSelectionEvent()->Add(&cListener, &CTestListListener::SelectionChanged);
		pTestWindow->Layout();
		pTestWindow->GetFocus()->SetFocus(pList);

		//One row at a time: a range, a toggle and all select one row or nothing, and Shift's keys aren't the list's.
		AssertFalse(pList->IsMultiSelect());
		pList->SelectRange(1, 2);
		AssertString("2 lead 2", PrintListSelection(pList, &sz));
		pList->ToggleRow(1);
		AssertString("1 lead 1", PrintListSelection(pList, &sz));
		pList->SelectAll();
		AssertString("1 lead 1", PrintListSelection(pList, &sz));
		AssertInt(1, pList->NumSelected());
		pTestWindow->InputCommand(CIC_SelectDown, true);
		AssertString("1 lead 1", PrintListSelection(pList, &sz));
		AssertLog("Selected:2;Selected:1;");

		//Several: Shift and down selects on from where the selection started, over the disabled row but not it.
		pList->SetMultiSelect(true);
		AssertTrue(pList->IsMultiSelect());
		AssertString("1 lead 1", PrintListSelection(pList, &sz));
		pTestWindow->InputCommand(CIC_SelectDown, true);
		AssertString("1,2 lead 2", PrintListSelection(pList, &sz));
		pTestWindow->InputCommand(CIC_SelectDown, true);
		AssertString("1,2,4 lead 4", PrintListSelection(pList, &sz));
		AssertInt(3, pList->NumSelected());
		AssertLog("Selected:2;Selected:4;");

		//Each selected row is drawn selected, right of its item; the disabled row between them isn't.
		pTestWindow->Paint();
		AssertInt(0x802020, ListWindowPixel(pTestWindow, 60, 35));
		AssertInt(0x802020, ListWindowPixel(pTestWindow, 60, 45));
		AssertInt(0x802020, ListWindowPixel(pTestWindow, 60, 65));
		AssertTrue(ListWindowPixel(pTestWindow, 60, 55) != 0x802020);
		AssertTrue(ListWindowPixel(pTestWindow, 60, 25) != 0x802020);

		//Back up shortens it; past where it started it goes the other way.
		pTestWindow->InputCommand(CIC_SelectUp, true);
		AssertString("1,2 lead 2", PrintListSelection(pList, &sz));
		pTestWindow->InputCommand(CIC_SelectDocumentHome, true);
		AssertString("0,1 lead 0", PrintListSelection(pList, &sz));
		pTestWindow->InputCommand(CIC_SelectDocumentEnd, true);
		AssertString("1,2,4,5 lead 5", PrintListSelection(pList, &sz));
		szLog.Clear();

		//A plain move selects one row again.
		pTestWindow->InputCommand(CIC_Up, true);
		AssertString("4 lead 4", PrintListSelection(pList, &sz));
		pTestWindow->Paint();
		AssertTrue(ListWindowPixel(pTestWindow, 60, 35) != 0x802020);
		AssertInt(0x802020, ListWindowPixel(pTestWindow, 60, 65));

		//Control with a press adds a row, or takes it away: the row pressed is the selected row either way.
		memset(&sEvent, 0, sizeof(SComponentInputEvent));
		sEvent.eType = CIET_Command;
		sEvent.eCommand = CIC_StartDragSelect;
		sEvent.bControl = true;
		sEvent.x = 20;
		sEvent.y = 25;
		pList->SelectPress(&sEvent);
		AssertString("0,4 lead 0", PrintListSelection(pList, &sz));
		sEvent.y = 65;
		pList->SelectPress(&sEvent);
		AssertString("0 lead 4", PrintListSelection(pList, &sz));
		AssertFalse(pList->IsRowSelected(4));

		//Shift with a press, through the window, selects from there to the row pressed.
		pTestWindow->PointerMoved(20, 45);
		pTestWindow->InputCommand(CIC_StartDragSelect, true);
		pTestWindow->InputCommand(CIC_StopDragSelect, true);
		AssertString("2,4 lead 2", PrintListSelection(pList, &sz));

		//All of them that are enabled; the selected row stays.
		pTestWindow->InputCommand(CIC_SelectAll, true);
		AssertString("0,1,2,4,5 lead 2", PrintListSelection(pList, &sz));
		szLog.Clear();

		//A plain press selects one.
		pTestWindow->PointerMoved(20, 35);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertString("1 lead 1", PrintListSelection(pList, &sz));
		AssertLog("Selected:1;");

		//One at a time again: only the selected row stays selected.
		pList->SelectRange(1, 4);
		pList->SetMultiSelect(false);
		AssertString("4 lead 4", PrintListSelection(pList, &sz));
		pTestWindow->Paint();
		AssertInt(0x802020, ListWindowPixel(pTestWindow, 60, 65));
		AssertTrue(ListWindowPixel(pTestWindow, 60, 35) != 0x802020);

		//Emptied, nothing is selected.
		pList->SetMultiSelect(true);
		pList->SelectAll();
		pList->RemoveAllComponents();
		AssertInt(0, pList->NumSelected());
		AssertInt(-1, pList->GetSelected());

		sz.Kill();
		szLog.Kill();
		pItem = NULL;
		pList = NULL;
		pTestWindow = NULL;
	}

	cWindow.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestList(void)
{
	BeginTests();
	ObjectsInit();

	TestListInput();
	TestListDraw();
	TestListKeysScroll();
	TestListMultiSelect();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

