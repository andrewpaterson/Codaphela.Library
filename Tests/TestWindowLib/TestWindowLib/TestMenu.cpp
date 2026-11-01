#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "SupportLib/FontReader.h"
#include "SupportLib/ImageColour.h"
#include "SupportLib/ImageColourConverter.h"
#include "SupportLib/TrueTypeFont.h"
#include "SupportLib/TrueTypeFontFactory.h"
#include "WindowLib/Window.h"
#include "WindowLib/WindowTick.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WindowLib/GridContainer.h"
#include "WindowLib/MenuBar.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/AssertGeometric.h"
#include "TestLib/AssertFile.h"
#include "DarkForest.h"


void ValidateLabelObjects(void);


#define AssertLog(e)	AssertString(e, szLog.Text()); szLog.Clear()


//////////////////////////////////////////////////////////////////////////
//
// Logs "Clicked:text;".
//
//////////////////////////////////////////////////////////////////////////
class CTestMenuListener : public CMenuListener
{
CONSTRUCTABLE(CTestMenuListener);
public:
	CChars*		mpszLog;

	void Init(CChars* pszLog)
	{
		mpszLog = pszLog;
	}

	void MenuItemClicked(CUnknown* pcSource, void* pvContext) override
	{
		mpszLog->Append("Clicked:");
		mpszLog->Append(((CMenuItem*)pcSource)->GetText());
		mpszLog->Append(";");
	}
};


//////////////////////////////////////////////////////////////////////////
//
// Inter at 13 pixels in IntelliJ's dark theme's light grey, the printable
// ASCII characters rasterised from the TrueType font into a font in the
// window's colour order.  Like ReadLabelFont it turns object validation
// off while the glyphs are made; ValidateLabelObjects turns it back on.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> GenerateMenuFont(Ptr<CWindow> pWindow)
{
	CTrueTypeFont			cTrueTypeFont;
	CTrueTypeFontFactory	cFactory;
	CImageColourRGB			cColour;
	Ptr<CFontDescription>	pDescription;
	Ptr<CImage>				pImage;
	Ptr<CFont>				pFont;
	char					szCharacters[96];
	int						i;

	gcObjects.DisableValidation();

	for (i = 0; i < 95; i++)
	{
		szCharacters[i] = (char)(' ' + i);
	}
	szCharacters[95] = '\0';

	cTrueTypeFont.Init();
	AssertTrue(cTrueTypeFont.Load((char*)"Input" _FS_ "TrueTypeFont" _FS_ "InterVariable.ttf"));
	cColour.Init(0xDF / 255.0f, 0xE1 / 255.0f, 0xE5 / 255.0f);
	cFactory.Init(&cTrueTypeFont, 13.0f);
	pDescription = cFactory.Describe((char*)"Inter 13", szCharacters);
	pImage = cFactory.Rasterise(pDescription, &cColour);
	AssertTrue(pImage.IsNotNull());
	pImage = CImageColourConverter::ConvertColour(pImage, pWindow->GetColourOrder());
	pFont = OMalloc<CFont>(pDescription, pImage);
	cColour.Kill();
	cFactory.Kill();
	cTrueTypeFont.Kill();
	return pFont;
}


//////////////////////////////////////////////////////////////////////////
//
// The menus used by every test.  File has icons on some of its items, a
// sub menu and a separator.  Edit has neither icons nor a sub menu.
//
//////////////////////////////////////////////////////////////////////////
struct STestMenus
{
	Ptr<CWindowTheme>	pTheme;
	Ptr<CMenuStyle>		pStyle;
	Ptr<CMenuBar>		pBar;
	Ptr<CMenuContainer>	pFile;
	Ptr<CMenuContainer>	pRecent;
	Ptr<CMenuContainer>	pEdit;
	Ptr<CMenuBarItem>	pFileItem;
	Ptr<CMenuBarItem>	pEditItem;
	Ptr<CCanvas>		pBelow;  //The rest of the window, under the bar.

	void Kill(void)
	{
		pTheme = NULL;
		pStyle = NULL;
		pBar = NULL;
		pFile = NULL;
		pRecent = NULL;
		pEdit = NULL;
		pFileItem = NULL;
		pEditItem = NULL;
		pBelow = NULL;
	}
};


//////////////////////////////////////////////////////////////////////////
//
// The bar is along the top of the window and the rest of the window is a
// plain canvas.
//
//////////////////////////////////////////////////////////////////////////
void CreateTestMenus(Ptr<CWindow> pWindow, STestMenus* psMenus)
{
	CColourFormatHelper		cHelper;
	Ptr<CGridContainer>		pGrid;

	pWindow->GetColourFormat(&cHelper);
	psMenus->pStyle = OMalloc<CMenuStyle>(pWindow, GenerateMenuFont(pWindow));
	psMenus->pTheme = OMalloc<CWindowTheme>(psMenus->pStyle->GetFont());
	psMenus->pTheme->SetMenuStyle(psMenus->pStyle);

	psMenus->pRecent = OMalloc<CMenuContainer>(pWindow, psMenus->pTheme);
	psMenus->pRecent->AddItem("One");
	psMenus->pRecent->AddItem("Two");

	psMenus->pFile = OMalloc<CMenuContainer>(pWindow, psMenus->pTheme);
	psMenus->pFile->AddItem("New", ReadDarkForestCel("Pine.png", &cHelper));
	psMenus->pFile->AddItem("Open", ReadDarkForestCel("Wolf.png", &cHelper));
	psMenus->pFile->AddItem("Recent", NULL, psMenus->pRecent);
	psMenus->pFile->AddSeparator();
	psMenus->pFile->AddItem("Exit", ReadDarkForestCel("Moon.png", &cHelper));

	psMenus->pEdit = OMalloc<CMenuContainer>(pWindow, psMenus->pTheme);
	psMenus->pEdit->AddItem("Undo");
	psMenus->pEdit->AddItem("Redo");
	psMenus->pEdit->AddSeparator();
	psMenus->pEdit->AddItem("Copy");
	psMenus->pEdit->AddItem("Paste");

	psMenus->pBar = OMalloc<CMenuBar>(pWindow, psMenus->pTheme);
	psMenus->pFileItem = psMenus->pBar->AddMenu("File", psMenus->pFile);
	psMenus->pEditItem = psMenus->pBar->AddMenu("Edit", psMenus->pEdit);

	pGrid = OMalloc<CGridContainer>(pWindow);
	pWindow->SetContainer(pGrid);
	psMenus->pBar->SetAlignment(CSH_Stretch, CSV_Stretch);
	pGrid->AddComponent(psMenus->pBar, 0, 0, 1, 1, 1, 0);
	psMenus->pBelow = OMalloc<CCanvas>(pWindow, OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)0x1E, 0x1F, 0x22)));
	pGrid->AddComponent(psMenus->pBelow, 0, 1, 1, 1, 1, 1);
}


//////////////////////////////////////////////////////////////////////////
//
// The middle of row iRow of pMenu, in window client space.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 MenuRowCentre(Ptr<CMenuContainer> pMenu, int32 iRow)
{
	int32	iTop;
	int32	iHeight;

	AssertTrue(pMenu->GetTrackBounds(false, iRow, &iTop, &iHeight));
	return SInt32Vec2(pMenu->GetPosition().x + pMenu->GetActualSize().x / 2, iTop + iHeight / 2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 ComponentCentre(Ptr<CComponent> pComponent)
{
	return SInt32Vec2(pComponent->GetPosition().x + pComponent->GetActualSize().x / 2, pComponent->GetPosition().y + pComponent->GetActualSize().y / 2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void MoveTo(Ptr<CWindow> pWindow, SInt32Vec2 sPosition)
{
	pWindow->PointerMoved(sPosition.x, sPosition.y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Click(Ptr<CWindow> pWindow)
{
	pWindow->InputCommand(CIC_ActivateComponent, true);
	pWindow->InputCommand(CIC_ExecuteComponent, true);
}


//////////////////////////////////////////////////////////////////////////
//
// Every item in a menu is three columns, the icon and chevron columns a
// fixed width, so the labels and chevrons line up whatever the icons are.
//
//////////////////////////////////////////////////////////////////////////
void TestMenuItemColumns(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		STestMenus				sMenus;
		Ptr<CMenuItem>			pItem;
		Ptr<CPopup>				pPopup;
		CPointer				pNull;
		SInt32Vec2				sVec;
		SInt32Vec2				sMenuSize;
		size					i;
		int32					iLabelX;
		int32					iChevronX;
		int32					iTop;
		int32					iHeight;
		int32					iItemHeight;

		cNativeFactory.Init(&gcMemoryAllocator, 300, 200, NULL);
		pTestWindow = OMalloc<CWindow>("Menu Window", &cNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		CreateTestMenus(pTestWindow, &sMenus);
		pTestWindow->Layout();

		pPopup = sMenus.pFile->Open(SInt32Vec2(10, 20), pNull, false);
		AssertTrue(sMenus.pFile->IsOpen());
		AssertSize(1, pTestWindow->NumPopups());

		//The popup is the menu plus the style's insets, with the menu inside them.
		sMenuSize = sMenus.pFile->GetDesiredSize();
		sVec = pPopup->GetPosition();
		AssertInt32Vec2(10, 20, &sVec);
		sVec = pPopup->GetActualSize();
		AssertInt32Vec2(sMenuSize.x + 2, sMenuSize.y + 12, &sVec);
		sVec = sMenus.pFile->GetPosition();
		AssertInt32Vec2(11, 26, &sVec);

		AssertSize(4, sMenus.pFile->NumItems());
		AssertInt(5, sMenus.pFile->NumRows());
		AssertFalse(sMenus.pFile->IsRowEnabled(3));
		AssertTrue(sMenus.pFile->GetItemInRow(3).IsNull());
		AssertString("Exit", sMenus.pFile->GetItemInRow(4)->GetText());

		iItemHeight = sMenus.pFile->GetItem(0)->GetLabel()->GetDesiredSize().y + 10;
		iLabelX = 11 + 32;
		iChevronX = 11 + sMenuSize.x - 24;
		for (i = 0; i < sMenus.pFile->NumItems(); i++)
		{
			pItem = sMenus.pFile->GetItem(i);

			//Each item is as wide as the menu.
			sVec = pItem->GetActualSize();
			AssertInt32Vec2(sMenuSize.x, iItemHeight, &sVec);
			sVec = pItem->GetIconComponent()->GetActualSize();
			AssertInt32Vec2(32, iItemHeight, &sVec);
			AssertInt(iLabelX, pItem->GetLabel()->GetPosition().x);
			AssertInt(24, pItem->GetChevronComponent()->GetActualSize().x);
			AssertInt(iChevronX, pItem->GetChevronComponent()->GetPosition().x);
			AssertBool(i == 2, pItem->GetChevronComponent()->GetCel().IsNotNull());
			AssertBool(i != 2, pItem->GetIcon().IsNotNull());
		}

		//The separator's row is the style's separator height.
		AssertTrue(sMenus.pFile->GetTrackBounds(false, 3, &iTop, &iHeight));
		AssertInt(9, iHeight);

		//The widest label decides the menu's width.
		AssertInt(32 + sMenus.pFile->GetItem(2)->GetLabel()->GetDesiredSize().x + 24, sMenuSize.x);

		sMenus.pFile->Close();
		AssertFalse(sMenus.pFile->IsOpen());
		AssertSize(0, pTestWindow->NumPopups());

		ValidateLabelObjects();

		pItem = NULL;
		pPopup = NULL;
		sMenus.Kill();
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	AssertSize(2, gcObjects.NumMemoryIndexes());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMenuBarPointer(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		STestMenus				sMenus;
		Ptr<CPopup>				pPopup;
		Ptr<CPopup>				pSubPopup;
		Ptr<CFocus>				pFocus;
		CTestMenuListener		cListener;
		CPointer				pNull;
		CChars					szLog;
		SInt32Vec2				sVec;
		int32					iBarHeight;
		int32					iTop;
		int32					iHeight;

		szLog.Init();
		cListener.Init(&szLog);
		cNativeFactory.Init(&gcMemoryAllocator, 300, 200, NULL);
		pTestWindow = OMalloc<CWindow>("Menu Window", &cNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFocus = pTestWindow->GetFocus();
		CreateTestMenus(pTestWindow, &sMenus);
		AssertTrue(sMenus.pBar->AddListener(&cListener));
		pTestWindow->Layout();

		//The bar is across the window, as tall as its buttons, with the buttons on its left.
		iBarHeight = sMenus.pFileItem->GetActualSize().y;
		sVec = sMenus.pBar->GetActualSize();
		AssertInt32Vec2(300, iBarHeight, &sVec);
		sVec = sMenus.pFileItem->GetPosition();
		AssertInt32Vec2(0, 0, &sVec);
		sVec = sMenus.pEditItem->GetPosition();
		AssertInt32Vec2(sMenus.pFileItem->GetActualSize().x, 0, &sVec);

		//Over a button it is highlighted.
		MoveTo(pTestWindow, ComponentCentre(sMenus.pFileItem));
		AssertInt(BS_MouseOver, sMenus.pFileItem->GetState());
		AssertSize(0, pTestWindow->NumPopups());

		//Pressing it opens its menu below it, which takes the keys.  It isn't drawn pushed.
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		AssertTrue(sMenus.pFile->IsOpen());
		AssertSize(1, pTestWindow->NumPopups());
		AssertTrue(sMenus.pBar->GetOpenItem() == sMenus.pFileItem);
		AssertInt(BS_MouseOver, sMenus.pFileItem->GetState());
		AssertTrue(pFocus->GetFocussedComponent() == sMenus.pFile);
		AssertInt(-1, sMenus.pFile->GetSelected());
		pPopup = pTestWindow->GetPopup(sMenus.pFile);
		sVec = pPopup->GetPosition();
		AssertInt32Vec2(0, iBarHeight, &sVec);

		//Releasing doesn't close it.  Moving off the button leaves it highlighted while its menu is open.
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertTrue(sMenus.pFile->IsOpen());
		MoveTo(pTestWindow, SInt32Vec2(250, iBarHeight / 2));
		AssertInt(BS_MouseOver, sMenus.pFileItem->GetState());

		//Moving over the next button opens its menu instead.
		MoveTo(pTestWindow, ComponentCentre(sMenus.pEditItem));
		AssertFalse(sMenus.pFile->IsOpen());
		AssertTrue(sMenus.pEdit->IsOpen());
		AssertSize(1, pTestWindow->NumPopups());
		AssertInt(BS_Normal, sMenus.pFileItem->GetState());
		AssertInt(BS_MouseOver, sMenus.pEditItem->GetState());
		AssertTrue(pFocus->GetFocussedComponent() == sMenus.pEdit);

		//Moving down the menu selects the row under the pointer, but not the separator.
		MoveTo(pTestWindow, MenuRowCentre(sMenus.pEdit, 1));
		AssertInt(1, sMenus.pEdit->GetSelected());
		AssertInt(BS_MouseOver, sMenus.pEditItem->GetState());
		MoveTo(pTestWindow, MenuRowCentre(sMenus.pEdit, 2));
		AssertInt(1, sMenus.pEdit->GetSelected());
		MoveTo(pTestWindow, MenuRowCentre(sMenus.pEdit, 3));
		AssertInt(3, sMenus.pEdit->GetSelected());

		//Clicking an item closes the menu and tells the bar's listeners.
		Click(pTestWindow);
		AssertLog("Clicked:Copy;");
		AssertSize(0, pTestWindow->NumPopups());
		AssertTrue(sMenus.pBar->GetOpenItem().IsNull());
		AssertInt(BS_Normal, sMenus.pEditItem->GetState());

		//Pressing the button of an open menu closes it.
		MoveTo(pTestWindow, ComponentCentre(sMenus.pFileItem));
		Click(pTestWindow);
		AssertTrue(sMenus.pFile->IsOpen());
		Click(pTestWindow);
		AssertFalse(sMenus.pFile->IsOpen());
		AssertLog("");

		//Moving over an item with a sub menu opens the sub menu beside the menu, its first item level with the item.
		Click(pTestWindow);
		AssertTrue(sMenus.pFile->IsOpen());
		pPopup = pTestWindow->GetPopup(sMenus.pFile);
		MoveTo(pTestWindow, MenuRowCentre(sMenus.pFile, 2));
		AssertInt(2, sMenus.pFile->GetSelected());
		AssertTrue(sMenus.pRecent->IsOpen());
		AssertSize(2, pTestWindow->NumPopups());
		AssertTrue(sMenus.pFile->GetOpenSubMenu() == sMenus.pRecent);
		pSubPopup = pTestWindow->GetPopup(sMenus.pRecent);
		AssertInt(pPopup->GetPosition().x + pPopup->GetActualSize().x, pSubPopup->GetPosition().x);
		AssertTrue(sMenus.pFile->GetTrackBounds(false, 2, &iTop, &iHeight));
		AssertInt(iTop, sMenus.pRecent->GetPosition().y);
		AssertInt(-1, sMenus.pRecent->GetSelected());

		//Moving to another row closes it.  Moving back opens it again.
		MoveTo(pTestWindow, MenuRowCentre(sMenus.pFile, 1));
		AssertFalse(sMenus.pRecent->IsOpen());
		AssertSize(1, pTestWindow->NumPopups());
		MoveTo(pTestWindow, MenuRowCentre(sMenus.pFile, 2));
		AssertTrue(sMenus.pRecent->IsOpen());

		//Moving into the sub menu leaves the menu's item selected.  Clicking a sub menu's item closes both menus and tells
		//the bar's listeners.
		MoveTo(pTestWindow, MenuRowCentre(sMenus.pRecent, 1));
		AssertInt(1, sMenus.pRecent->GetSelected());
		AssertInt(2, sMenus.pFile->GetSelected());
		Click(pTestWindow);
		AssertLog("Clicked:Two;");
		AssertSize(0, pTestWindow->NumPopups());
		AssertTrue(sMenus.pBar->GetOpenItem().IsNull());
		AssertInt(BS_Normal, sMenus.pFileItem->GetState());

		//Pressing anywhere else closes the menus without clicking anything.
		MoveTo(pTestWindow, ComponentCentre(sMenus.pFileItem));
		Click(pTestWindow);
		MoveTo(pTestWindow, MenuRowCentre(sMenus.pFile, 2));
		AssertSize(2, pTestWindow->NumPopups());
		MoveTo(pTestWindow, SInt32Vec2(290, 190));
		Click(pTestWindow);
		AssertSize(0, pTestWindow->NumPopups());
		AssertTrue(sMenus.pBar->GetOpenItem().IsNull());
		AssertInt(BS_Normal, sMenus.pFileItem->GetState());
		AssertTrue(pFocus->GetFocussedComponent().IsNull());
		AssertLog("");

		//A menu opens with nothing selected however it was left.
		MoveTo(pTestWindow, ComponentCentre(sMenus.pFileItem));
		Click(pTestWindow);
		AssertInt(-1, sMenus.pFile->GetSelected());
		AssertFalse(sMenus.pRecent->IsOpen());
		sMenus.pBar->CloseMenu();
		AssertSize(0, pTestWindow->NumPopups());

		ValidateLabelObjects();

		AssertTrue(sMenus.pBar->RemoveListener(&cListener));
		pPopup = NULL;
		pSubPopup = NULL;
		pFocus = NULL;
		sMenus.Kill();
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		szLog.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	AssertSize(2, gcObjects.NumMemoryIndexes());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMenuBarKeys(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		STestMenus				sMenus;
		Ptr<CFocus>				pFocus;
		CTestMenuListener		cListener;
		CPointer				pNull;
		CChars					szLog;

		szLog.Init();
		cListener.Init(&szLog);
		cNativeFactory.Init(&gcMemoryAllocator, 300, 200, NULL);
		pTestWindow = OMalloc<CWindow>("Menu Window", &cNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFocus = pTestWindow->GetFocus();
		CreateTestMenus(pTestWindow, &sMenus);
		AssertTrue(sMenus.pBar->AddListener(&cListener));
		pTestWindow->Layout();

		//A focussed bar button moves along the bar with left and right, round from one end to the other.
		pFocus->SetFocus(sMenus.pFileItem);
		pTestWindow->InputCommand(CIC_Right, true);
		AssertTrue(pFocus->GetFocussedComponent() == sMenus.pEditItem);
		pTestWindow->InputCommand(CIC_Right, true);
		AssertTrue(pFocus->GetFocussedComponent() == sMenus.pFileItem);
		pTestWindow->InputCommand(CIC_Left, true);
		AssertTrue(pFocus->GetFocussedComponent() == sMenus.pEditItem);
		pTestWindow->InputCommand(CIC_Left, true);
		AssertTrue(pFocus->GetFocussedComponent() == sMenus.pFileItem);
		AssertSize(0, pTestWindow->NumPopups());

		//Down opens its menu with the first item selected.
		pTestWindow->InputCommand(CIC_Down, true);
		AssertTrue(sMenus.pFile->IsOpen());
		AssertTrue(pFocus->GetFocussedComponent() == sMenus.pFile);
		AssertInt(0, sMenus.pFile->GetSelected());

		//Up and down skip the separator.
		pTestWindow->InputCommand(CIC_Down, true);
		AssertInt(1, sMenus.pFile->GetSelected());
		pTestWindow->InputCommand(CIC_End, true);
		AssertInt(4, sMenus.pFile->GetSelected());
		pTestWindow->InputCommand(CIC_Up, true);
		AssertInt(2, sMenus.pFile->GetSelected());

		//Right goes into a sub menu, left comes back out of it.
		pTestWindow->InputCommand(CIC_Right, true);
		AssertTrue(sMenus.pRecent->IsOpen());
		AssertTrue(pFocus->GetFocussedComponent() == sMenus.pRecent);
		AssertInt(0, sMenus.pRecent->GetSelected());
		pTestWindow->InputCommand(CIC_Down, true);
		AssertInt(1, sMenus.pRecent->GetSelected());
		pTestWindow->InputCommand(CIC_Left, true);
		AssertFalse(sMenus.pRecent->IsOpen());
		AssertTrue(pFocus->GetFocussedComponent() == sMenus.pFile);
		AssertInt(2, sMenus.pFile->GetSelected());

		//Enter on an item with a sub menu opens it too.
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertTrue(sMenus.pRecent->IsOpen());
		AssertTrue(pFocus->GetFocussedComponent() == sMenus.pRecent);
		pTestWindow->InputCommand(CIC_Left, true);

		//Right on an item without a sub menu opens the bar's next menu, left the one before.
		pTestWindow->InputCommand(CIC_Up, true);
		AssertInt(1, sMenus.pFile->GetSelected());
		pTestWindow->InputCommand(CIC_Right, true);
		AssertFalse(sMenus.pFile->IsOpen());
		AssertTrue(sMenus.pEdit->IsOpen());
		AssertTrue(sMenus.pBar->GetOpenItem() == sMenus.pEditItem);
		AssertInt(BS_MouseOver, sMenus.pEditItem->GetState());
		AssertTrue(pFocus->GetFocussedComponent() == sMenus.pEdit);
		AssertInt(0, sMenus.pEdit->GetSelected());
		pTestWindow->InputCommand(CIC_Left, true);
		AssertTrue(sMenus.pFile->IsOpen());
		AssertInt(0, sMenus.pFile->GetSelected());

		//Enter clicks the selected item.
		pTestWindow->InputCommand(CIC_Down, true);
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertLog("Clicked:Open;");
		AssertSize(0, pTestWindow->NumPopups());

		//Space and enter on a bar button open its menu.
		pFocus->SetFocus(sMenus.pEditItem);
		pTestWindow->InputCommand(CIC_Space, true);
		AssertTrue(sMenus.pEdit->IsOpen());
		AssertInt(0, sMenus.pEdit->GetSelected());
		pTestWindow->InputCommand(CIC_Space, true);
		AssertLog("Clicked:Undo;");

		ValidateLabelObjects();

		AssertTrue(sMenus.pBar->RemoveListener(&cListener));
		pFocus = NULL;
		sMenus.Kill();
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		szLog.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	AssertSize(2, gcObjects.NumMemoryIndexes());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Draws a frame then moves on: over File, pressing File, then down onto
// Recent so its sub menu opens.
//
//////////////////////////////////////////////////////////////////////////
class CMenuFramesTick : public CWindowTick
{
CONSTRUCTABLE(CMenuFramesTick);
DESTRUCTABLE(CMenuFramesTick);
protected:
	int32			miFrame;
	STestMenus*		mpsMenus;

public:
	void Init(STestMenus* psMenus)
	{
		PreInit();
		CWindowTick::Init();
		miFrame = 0;
		mpsMenus = psMenus;
		PostInit();
	}

	void Class(void) override
	{
		CWindowTick::Class();
		U_Int32(miFrame);
		U_Pointer(mpsMenus);
	}

	void Tick(Ptr<CWindow> pWindow, int64 iUpdateTimeInMillieconds, int64 iTotalTimeInMillieconds) override
	{
		pWindow->Paint();

		miFrame++;
		if (miFrame == 1)
		{
			MoveTo(pWindow, ComponentCentre(mpsMenus->pFileItem));
		}
		else if (miFrame == 2)
		{
			pWindow->InputCommand(CIC_ActivateComponent, true);
			pWindow->InputCommand(CIC_ExecuteComponent, true);
		}
		else if (miFrame == 3)
		{
			MoveTo(pWindow, MenuRowCentre(mpsMenus->pFile, 2));
		}
		else
		{
			pWindow->Stop();
		}
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMenuBarFrames(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "MenuBarFrames";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CMenuFramesTick>	pTick;
		STestMenus				sMenus;
		CChars					szExpectedDirectory;

		cNativeFactory.Init(&gcMemoryAllocator, 260, 200, szDirectory);
		pTick = OMalloc<CMenuFramesTick>(&sMenus);
		pTestWindow = OMalloc<CWindow>("Menu Frames", &cNativeFactory, pTick, OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)0x1E, 0x1F, 0x22)), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		CreateTestMenus(pTestWindow, &sMenus);

		pTestWindow->Show();
		AssertTrue(sMenus.pRecent->IsOpen());

		ValidateLabelObjects();

		sMenus.Kill();
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;

		szExpectedDirectory.Init(szDirectory);
		szExpectedDirectory.Replace("Output", "Input");
		AssertDirectory(szExpectedDirectory.Text(), szDirectory);
		szExpectedDirectory.Kill();
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
void TestMenu(void)
{
	BeginTests();
	ObjectsInit();

	TestMenuItemColumns();
	TestMenuBarPointer();
	TestMenuBarKeys();
	TestMenuBarFrames();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

