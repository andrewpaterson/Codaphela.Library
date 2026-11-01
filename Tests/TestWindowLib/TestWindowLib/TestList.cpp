#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WindowLib/ListContainer.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/AssertGeometric.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"


#define AssertLog(e)	AssertString(e, szLog.Text()); szLog.Clear()


//////////////////////////////////////////////////////////////////////////
//
// Logs "Selected:row;" and "Activated:row;".
//
//////////////////////////////////////////////////////////////////////////
class CTestListListener : public CListListener
{
CONSTRUCTABLE(CTestListListener);
public:
	CChars*		mpszLog;

	void Init(CChars* pszLog)
	{
		mpszLog = pszLog;
	}

	void SelectionChanged(CUnknown* pcSource, void* pvContext) override
	{
		mpszLog->Append("Selected:");
		mpszLog->Append(((CListContainer*)pcSource)->GetSelected());
		mpszLog->Append(";");
	}

	void ItemActivated(CUnknown* pcSource, void* pvContext) override
	{
		mpszLog->Append("Activated:");
		mpszLog->Append(((CListContainer*)pcSource)->GetSelected());
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

		pList = OMalloc<CListContainer>(pTestWindow, OMalloc<CWindowTheme>(Ptr<CFont>()));
		AssertTrue(pList->AddListener(&cListener));
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

		//Enter and space activate the selected row.
		AssertTrue(pList->WantsEnter());
		pTestWindow->InputCommand(CIC_Enter, true);
		pTestWindow->InputCommand(CIC_Space, true);
		AssertLog("Activated:0;Activated:0;");

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

		AssertTrue(pList->RemoveListener(&cListener));

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

		pList = OMalloc<CListContainer>(pTestWindow, OMalloc<CWindowTheme>(Ptr<CFont>()));
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
//
//////////////////////////////////////////////////////////////////////////
void TestList(void)
{
	BeginTests();
	ObjectsInit();

	TestListInput();
	TestListDraw();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

