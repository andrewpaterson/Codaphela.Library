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
#include "WindowLib/TextArea.h"
#include "WindowLib/TreeContainer.h"
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
class CTestMenuListener
{
public:
	CChars*		mpszLog;
	const char*	mszPrefix;

	void Init(CChars* pszLog)
	{
		mpszLog = pszLog;
		mszPrefix = "Clicked:";
	}

	void MenuItemClicked(CMenuItem* pcItem)
	{
		mpszLog->Append(mszPrefix);
		mpszLog->Append(pcItem->GetText());
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
		sMenus.pBar->GetItemClickEvent()->Add(&cListener, &CTestMenuListener::MenuItemClicked);
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

		AssertTrue(sMenus.pBar->GetItemClickEvent()->Remove(&cListener));
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
		sMenus.pBar->GetItemClickEvent()->Add(&cListener, &CTestMenuListener::MenuItemClicked);
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

		AssertTrue(sMenus.pBar->GetItemClickEvent()->Remove(&cListener));
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
// A popup without a look draws nothing behind its content, so the window
// shows through content that doesn't draw.  With a look its background
// is drawn.
//
//////////////////////////////////////////////////////////////////////////
void TestPopupDraw(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	const char*				szDirectory;

	szDirectory = "Output" _FS_ "PopupDraw";
	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CIconComponent>		pContent;
		Ptr<CPopup>				pPopup;
		CPointer				pNull;
		SInsets					sInsets;
		uint8*					puiPixel;

		cNativeFactory.Init(&gcMemoryAllocator, 100, 40, (char*)szDirectory);
		pTestWindow = OMalloc<CWindow>("Popup Window", &cNativeFactory, pNull, OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)0xFF, (uint8)0xFF, (uint8)0xFF)), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pContent = OMalloc<CIconComponent>(pTestWindow, Ptr<CImageCel>(), 20, 10);
		pTestWindow->Layout();

		pPopup = pTestWindow->OpenPopup(pContent, SInt32Vec2(10, 10), pNull);
		pTestWindow->Paint();
		puiPixel = &pTestWindow->GetCanvas()->GetPixelData()[(15 * 100 + 20) * 3];
		AssertInt(0xFF, puiPixel[0]);
		AssertInt(0xFF, puiPixel[1]);
		AssertInt(0xFF, puiPixel[2]);

		sInsets.Init();
		pPopup->SetLook(Set32BitColour((uint8)0xFF, (uint8)0, (uint8)0), &sInsets);
		pTestWindow->Paint();
		puiPixel = &pTestWindow->GetCanvas()->GetPixelData()[(15 * 100 + 20) * 3];
		AssertInt(0xFF, puiPixel[0]);
		AssertInt(0, puiPixel[1]);
		AssertInt(0, puiPixel[2]);

		pTestWindow->CloseAllPopups();
		pPopup = NULL;
		pContent = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	AssertSize(2, gcObjects.NumMemoryIndexes());
	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
// A context menu on a grid opens over the text area in it (which has no
// menu of its own) with the right button at the pointer, moving the text
// area's caret there unless it is in the selection, and with the
// Menu key or Shift+F10 under the text area's caret.  The keys are in the
// menu while it is open and back in the text area when it closes, however
// it closes: escape, an item clicked, or a press outside it, in its owner
// too.
//
//////////////////////////////////////////////////////////////////////////
void TestMenuContextMenu(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CWindowTheme>		pTheme;
		Ptr<CMenuStyle>			pStyle;
		Ptr<CMenuContainer>		pMenu;
		Ptr<CMenuContainer>		pMore;
		Ptr<CMenuContainer>		pEmpty;
		Ptr<CGridContainer>		pGrid;
		Ptr<CTextArea>			pText;
		Ptr<CPopup>				pPopup;
		Ptr<CFocus>				pFocus;
		CTestMenuListener		cListener;
		CPointer				pNull;
		CChars					szLog;
		SInt32Vec2				sPointer;
		SInt32Vec2				sVec;
		CInt32Rect				cCaret;
		size					uiActions;
		size					uiStart;
		size					uiEnd;
		size					i;

		szLog.Init();
		cListener.Init(&szLog);
		cNativeFactory.Init(&gcMemoryAllocator, 300, 200, NULL);
		pTestWindow = OMalloc<CWindow>("Menu Window", &cNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFocus = pTestWindow->GetFocus();
		pStyle = OMalloc<CMenuStyle>(pTestWindow, GenerateMenuFont(pTestWindow));
		pTheme = OMalloc<CWindowTheme>(pStyle->GetFont());
		pTheme->SetMenuStyle(pStyle);

		pMore = OMalloc<CMenuContainer>(pTestWindow, pTheme);
		pMore->AddItem("One");
		pMore->AddItem("Two");
		pMenu = OMalloc<CMenuContainer>(pTestWindow, pTheme);
		pMenu->AddItem("Cut");
		pMenu->AddItem("Copy");
		pMenu->AddSeparator();
		pMenu->AddItem("More", NULL, pMore);
		pMenu->GetItemClickEvent()->Add(&cListener, &CTestMenuListener::MenuItemClicked);
		pMore->GetItemClickEvent()->Add(&cListener, &CTestMenuListener::MenuItemClicked);

		pText = OMalloc<CTextArea>(pTestWindow, pStyle->GetFont(), "The quick brown fox\njumps over the lazy dog.");
		pText->SetAlignment(CSH_Stretch, CSV_Stretch);
		pGrid = OMalloc<CGridContainer>(pTestWindow);
		pGrid->AddComponent(pText, 0, 0, 1, 1, 1, 1);
		pTestWindow->SetContainer(pGrid);
		uiActions = pGrid->NumInputActions();
		pGrid->SetContextMenu(pMenu);
		AssertTrue(pGrid->GetContextMenu() == pMenu);
		AssertSize(uiActions + 2, pGrid->NumInputActions());
		pTestWindow->Layout();

		//The right button over the text area focusses it, then the grid opens its menu at the pointer, the keys in it.
		sPointer = SInt32Vec2(40, 30);
		MoveTo(pTestWindow, sPointer);
		pTestWindow->InputCommand(CIC_ContextMenu, true);
		AssertTrue(pMenu->IsOpen());
		pPopup = pTestWindow->GetPopup(pMenu);
		sVec = pPopup->GetPosition();
		AssertInt32Vec2(40, 30, &sVec);
		AssertTrue(pFocus->GetFocussedComponent() == pMenu);

		//Escape closes it and the keys go back to the text area.  The right button moved the text area's caret to the
		//pointer, as Visual Studio's does.
		pTestWindow->InputCommand(CIC_ClearSelection, true);
		AssertFalse(pMenu->IsOpen());
		AssertTrue(pFocus->GetFocussedComponent() == pText);
		AssertSize(pText->FindPositionAt(40, 30), pText->GetCaretPosition());
		AssertTrue(pText->GetCaretPosition() > 9);

		//Pressed in the selection ("The quick") the selection is kept, so the menu acts on it.  Escape closes the menu
		//and leaves the selection.
		pText->DocumentHome();
		for (i = 0; i < 9; i++)
		{
			pText->SelectRight();
		}
		MoveTo(pTestWindow, SInt32Vec2(3, 5));
		AssertTrue(pText->FindPositionAt(3, 5) < 9);
		pTestWindow->InputCommand(CIC_ContextMenu, true);
		AssertTrue(pMenu->IsOpen());
		AssertTrue(pText->GetSelection(&uiStart, &uiEnd));
		AssertSize(0, uiStart);
		AssertSize(9, uiEnd);
		pTestWindow->InputCommand(CIC_ClearSelection, true);
		AssertFalse(pMenu->IsOpen());
		AssertTrue(pText->GetSelection(&uiStart, &uiEnd));

		//Pressed outside it the caret moves there and nothing is selected.
		MoveTo(pTestWindow, sPointer);
		pTestWindow->InputCommand(CIC_ContextMenu, true);
		AssertTrue(pMenu->IsOpen());
		AssertFalse(pText->GetSelection(&uiStart, &uiEnd));
		AssertSize(pText->FindPositionAt(40, 30), pText->GetCaretPosition());
		pTestWindow->InputCommand(CIC_ClearSelection, true);

		//An item clicked with the keys closes the menu, then calls the menu's item click event.
		pTestWindow->InputCommand(CIC_ContextMenu, true);
		pTestWindow->InputCommand(CIC_Down, true);
		pTestWindow->InputCommand(CIC_Down, true);
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertLog("Clicked:Copy;");
		AssertFalse(pMenu->IsOpen());
		AssertTrue(pFocus->GetFocussedComponent() == pText);

		//An item in a sub menu clicked with the pointer closes them both.
		pTestWindow->InputCommand(CIC_ContextMenu, true);
		pTestWindow->Layout();
		MoveTo(pTestWindow, MenuRowCentre(pMenu, 3));
		Click(pTestWindow);
		AssertTrue(pMore->IsOpen());
		pTestWindow->Layout();
		MoveTo(pTestWindow, MenuRowCentre(pMore, 1));
		Click(pTestWindow);
		AssertLog("Clicked:Two;");
		AssertSize(0, pTestWindow->NumPopups());
		AssertTrue(pFocus->GetFocussedComponent() == pText);

		//A press in the text area, which is in the menu's owner, closes it too.  The right button again opens it again
		//where the pointer now is.
		MoveTo(pTestWindow, sPointer);
		pTestWindow->InputCommand(CIC_ContextMenu, true);
		AssertTrue(pMenu->IsOpen());
		MoveTo(pTestWindow, SInt32Vec2(10, 10));
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		AssertFalse(pMenu->IsOpen());
		AssertTrue(pFocus->GetFocussedComponent() == pText);
		pTestWindow->InputCommand(CIC_ContextMenu, true);
		pTestWindow->InputCommand(CIC_ContextMenu, true);
		AssertSize(1, pTestWindow->NumPopups());
		pPopup = pTestWindow->GetPopup(pMenu);
		sVec = pPopup->GetPosition();
		AssertInt32Vec2(10, 10, &sVec);
		pTestWindow->InputCommand(CIC_ClearSelection, true);
		AssertTrue(pFocus->GetFocussedComponent() == pText);

		//The Menu key (or Shift+F10) opens it under the text area's caret, not at the pointer.
		pText->DocumentEnd();
		pTestWindow->Layout();
		pTestWindow->InputCommand(CIC_ContextMenuKey, true);
		AssertTrue(pMenu->IsOpen());
		pText->GetCaretBounds(&cCaret);
		pPopup = pTestWindow->GetPopup(pMenu);
		sVec = pPopup->GetPosition();
		AssertInt32Vec2(pText->GetPosition().x + cCaret.GetLeft(), pText->GetPosition().y + cCaret.GetBottom(), &sVec);
		pTestWindow->InputCommand(CIC_ClearSelection, true);
		AssertTrue(pFocus->GetFocussedComponent() == pText);

		//A menu with no items doesn't open.
		pEmpty = OMalloc<CMenuContainer>(pTestWindow, pTheme);
		pGrid->SetContextMenu(pEmpty);
		AssertSize(uiActions + 2, pGrid->NumInputActions());
		pTestWindow->InputCommand(CIC_ContextMenu, true);
		AssertSize(0, pTestWindow->NumPopups());
		AssertTrue(pFocus->GetFocussedComponent() == pText);

		//Without a menu the right button does nothing, and the grid's actions for it are gone.
		pGrid->SetContextMenu(NULL);
		AssertTrue(pGrid->GetContextMenu().IsNull());
		AssertSize(uiActions, pGrid->NumInputActions());
		pTestWindow->InputCommand(CIC_ContextMenu, true);
		AssertSize(0, pTestWindow->NumPopups());
		AssertLog("");

		pMenu->GetItemClickEvent()->Remove(&cListener);
		pMore->GetItemClickEvent()->Remove(&cListener);
		pPopup = NULL;
		pFocus = NULL;
		pEmpty = NULL;
		pText = NULL;
		pGrid = NULL;
		pMenu = NULL;
		pMore = NULL;
		pTheme = NULL;
		pStyle = NULL;
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
// A tree with a context menu, as Solution Explorer's: the right button
// selects the row under the pointer before the menu opens, so the menu
// acts on it, as Windows does.  Below the rows the selection stays.  The
// Menu key opens it under the selected row.
//
//////////////////////////////////////////////////////////////////////////
void TestMenuTreeContextMenu(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CWindowTheme>		pTheme;
		Ptr<CMenuStyle>			pStyle;
		Ptr<CMenuContainer>		pMenu;
		Ptr<CTreeContainer>		pTree;
		Ptr<CPopup>				pPopup;
		CTestMenuListener		cListener;
		CPointer				pNull;
		CChars					szLog;
		SInt32Vec2				sVec;
		int32					iFirst;
		int32					iSecond;
		int32					iTop;
		int32					iHeight;

		szLog.Init();
		cListener.Init(&szLog);
		cNativeFactory.Init(&gcMemoryAllocator, 300, 200, NULL);
		pTestWindow = OMalloc<CWindow>("Tree Menu Window", &cNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pStyle = OMalloc<CMenuStyle>(pTestWindow, GenerateMenuFont(pTestWindow));
		pTheme = OMalloc<CWindowTheme>(pStyle->GetFont());
		pTheme->SetMenuStyle(pStyle);

		pMenu = OMalloc<CMenuContainer>(pTestWindow, pTheme);
		pMenu->AddItem("Set as Startup");
		pMenu->AddItem("Remove");
		pMenu->GetItemClickEvent()->Add(&cListener, &CTestMenuListener::MenuItemClicked);
		pTree = OMalloc<CTreeContainer>(pTestWindow, pTheme);
		iFirst = pTree->AddNode(TREE_NO_NODE, "First");
		iSecond = pTree->AddNode(TREE_NO_NODE, "Second");
		pTree->SetContextMenu(pMenu);
		pTestWindow->SetContainer(pTree);
		pTree->SelectNode(iFirst);
		pTestWindow->Layout();

		//The right button on the second row selects it, then opens the menu at the pointer.
		AssertTrue(pTree->GetTrackBounds(false, 1, &iTop, &iHeight));
		MoveTo(pTestWindow, SInt32Vec2(20, iTop + 2));
		pTestWindow->InputCommand(CIC_ContextMenu, true);
		AssertInt(iSecond, pTree->GetSelectedNode());
		AssertTrue(pMenu->IsOpen());
		pPopup = pTestWindow->GetPopup(pMenu);
		sVec = pPopup->GetPosition();
		AssertInt32Vec2(20, iTop + 2, &sVec);
		pTestWindow->InputCommand(CIC_Down, true);
		pTestWindow->InputCommand(CIC_Down, true);
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertLog("Clicked:Remove;");
		AssertFalse(pMenu->IsOpen());
		AssertInt(iSecond, pTree->GetSelectedNode());

		//Below the rows it opens with the selection kept.
		MoveTo(pTestWindow, SInt32Vec2(20, 150));
		pTestWindow->InputCommand(CIC_ContextMenu, true);
		AssertTrue(pMenu->IsOpen());
		AssertInt(iSecond, pTree->GetSelectedNode());
		pTestWindow->InputCommand(CIC_ClearSelection, true);
		AssertFalse(pMenu->IsOpen());

		//The Menu key opens it under the selected row, at the tree's left.
		pTree->SelectNode(iFirst);
		pTestWindow->Layout();
		AssertTrue(pTree->GetTrackBounds(false, 0, &iTop, &iHeight));
		pTestWindow->InputCommand(CIC_ContextMenuKey, true);
		AssertTrue(pMenu->IsOpen());
		pPopup = pTestWindow->GetPopup(pMenu);
		sVec = pPopup->GetPosition();
		AssertInt32Vec2(pTree->GetPosition().x, iTop + iHeight, &sVec);
		pTestWindow->InputCommand(CIC_ClearSelection, true);
		AssertFalse(pMenu->IsOpen());
		AssertInt(iFirst, pTree->GetSelectedNode());
		AssertLog("");

		pMenu->GetItemClickEvent()->Remove(&cListener);
		pPopup = NULL;
		pTree = NULL;
		pMenu = NULL;
		pTheme = NULL;
		pStyle = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		szLog.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Disables the menu's second item as the menu opens, if it should.
//
//////////////////////////////////////////////////////////////////////////
class CTestMenuOpeningListener
{
public:
	bool	mbCanRedo;
	size	muiOpenings;

	void Init(void)
	{
		mbCanRedo = false;
		muiOpenings = 0;
	}

	void MenuOpening(CMenuContainer* pcMenu)
	{
		muiOpenings++;
		pcMenu->GetItem(1)->SetEnabled(mbCanRedo);
	}
};


//////////////////////////////////////////////////////////////////////////
//
// As IntelliJ's Edit menu: shortcuts on the right in the dim font and
// items disabled, as the menu opens, when they can't be done.  The keys
// skip a disabled item and clicking it does nothing.
//
//////////////////////////////////////////////////////////////////////////
void TestMenuEnabledAndShortcuts(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CWindowTheme>			pTheme;
		Ptr<CMenuStyle>				pStyle;
		Ptr<CMenuContainer>			pMenu;
		Ptr<CMenuItem>				pUndo;
		Ptr<CMenuItem>				pRedo;
		Ptr<CFont>					pDim;
		CTestMenuListener			cListener;
		CTestMenuOpeningListener	cOpening;
		CPointer					pNull;
		CChars						szLog;

		szLog.Init();
		cListener.Init(&szLog);
		cOpening.Init();
		cNativeFactory.Init(&gcMemoryAllocator, 300, 200, NULL);
		pTestWindow = OMalloc<CWindow>("Menu Window", &cNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pStyle = OMalloc<CMenuStyle>(pTestWindow, GenerateMenuFont(pTestWindow));
		AssertTrue(pStyle->GetDimFont() == pStyle->GetFont());
		pDim = GenerateMenuFont(pTestWindow);
		pStyle->SetDimFont(pDim);
		pTheme = OMalloc<CWindowTheme>(pStyle->GetFont());
		pTheme->SetMenuStyle(pStyle);

		pMenu = OMalloc<CMenuContainer>(pTestWindow, pTheme);
		pUndo = pMenu->AddItem("Undo");
		pRedo = pMenu->AddItem("Redo");
		pUndo->SetShortcut("Ctrl+Z");
		pRedo->SetShortcut("Ctrl+Shift+Z");
		AssertString("Ctrl+Z", pUndo->GetShortcut());
		AssertString("Ctrl+Shift+Z", pRedo->GetShortcut());
		AssertTrue(pUndo->GetShortcutLabel()->GetFont() == pDim);
		pMenu->GetItemClickEvent()->Add(&cListener, &CTestMenuListener::MenuItemClicked);
		pMenu->GetOpeningEvent()->Add(&cOpening, &CTestMenuOpeningListener::MenuOpening);

		//Opening disables Redo, its label in the dim font and its row not selectable.  Down stops on Undo, and clicking
		//Redo does nothing.
		AssertTrue(pRedo->IsEnabled());
		pMenu->Open(SInt32Vec2(10, 10), NULL, true);
		AssertSize(1, cOpening.muiOpenings);
		AssertFalse(pRedo->IsEnabled());
		AssertFalse(pMenu->IsRowEnabled(pMenu->GetItemRow(pRedo)));
		AssertTrue(pRedo->GetLabel()->GetFont() == pDim);
		AssertTrue(pUndo->GetLabel()->GetFont() == pStyle->GetFont());
		pTestWindow->InputCommand(CIC_Down, true);
		pTestWindow->InputCommand(CIC_Down, true);
		AssertInt(pMenu->GetItemRow(pUndo), pMenu->GetSelected());
		pMenu->Click(pRedo);
		AssertLog("");
		AssertTrue(pMenu->IsOpen());
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertLog("Clicked:Undo;");
		AssertFalse(pMenu->IsOpen());

		//Opened again it can be redone.
		cOpening.mbCanRedo = true;
		pMenu->Open(SInt32Vec2(10, 10), NULL, true);
		AssertSize(2, cOpening.muiOpenings);
		AssertTrue(pRedo->IsEnabled());
		AssertTrue(pRedo->GetLabel()->GetFont() == pStyle->GetFont());
		pTestWindow->InputCommand(CIC_Down, true);
		pTestWindow->InputCommand(CIC_Down, true);
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertLog("Clicked:Redo;");

		//No shortcut.
		pUndo->SetShortcut(NULL);
		AssertString("", pUndo->GetShortcut());

		pMenu->GetItemClickEvent()->Remove(&cListener);
		pMenu->GetOpeningEvent()->Remove(&cOpening);
		pUndo = NULL;
		pRedo = NULL;
		pDim = NULL;
		pMenu = NULL;
		pTheme = NULL;
		pStyle = NULL;
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
// Disabling an item and disabling its row are the same: either way the
// item is drawn in the dim font, reports itself disabled and its row
// can't be selected.  Disabling the selected item clears the selection.
//
//////////////////////////////////////////////////////////////////////////
void TestMenuItemAndRowEnabledAgree(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>		pTestWindow;
		Ptr<CWindowTheme>	pTheme;
		Ptr<CMenuStyle>		pStyle;
		Ptr<CMenuContainer>	pMenu;
		Ptr<CMenuItem>		pCut;
		Ptr<CMenuItem>		pCopy;
		Ptr<CMenuItem>		pPaste;
		Ptr<CFont>			pDim;
		CPointer			pNull;

		cNativeFactory.Init(&gcMemoryAllocator, 300, 200, NULL);
		pTestWindow = OMalloc<CWindow>("Menu Window", &cNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pStyle = OMalloc<CMenuStyle>(pTestWindow, GenerateMenuFont(pTestWindow));
		pDim = GenerateMenuFont(pTestWindow);
		pStyle->SetDimFont(pDim);
		pTheme = OMalloc<CWindowTheme>(pStyle->GetFont());
		pTheme->SetMenuStyle(pStyle);

		pMenu = OMalloc<CMenuContainer>(pTestWindow, pTheme);
		pCut = pMenu->AddItem("Cut");
		pCopy = pMenu->AddItem("Copy");
		pPaste = pMenu->AddItem("Paste");

		//Through the item.
		pCopy->SetEnabled(false);
		AssertFalse(pCopy->IsEnabled());
		AssertFalse(pMenu->IsRowEnabled(pMenu->GetItemRow(pCopy)));
		AssertTrue(pCopy->GetLabel()->GetFont() == pDim);
		AssertFalse(pMenu->Select(pMenu->GetItemRow(pCopy)));

		//Through the row.
		pMenu->SetRowEnabled(pMenu->GetItemRow(pCopy), true);
		AssertTrue(pCopy->IsEnabled());
		AssertTrue(pCopy->GetLabel()->GetFont() == pStyle->GetFont());
		pMenu->SetRowEnabled(pMenu->GetItemRow(pPaste), false);
		AssertFalse(pPaste->IsEnabled());
		AssertTrue(pPaste->GetLabel()->GetFont() == pDim);

		//The selected item disabled.
		AssertTrue(pMenu->Select(pMenu->GetItemRow(pCut)));
		pCut->SetEnabled(false);
		AssertInt(-1, pMenu->GetSelected());

		pCut = NULL;
		pCopy = NULL;
		pPaste = NULL;
		pDim = NULL;
		pMenu = NULL;
		pTheme = NULL;
		pStyle = NULL;
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
// An item's own click event is called before its menu's (and its
// sub menu's root's), for the item clicked only.  Click clicks as Enter
// does: not a disabled item, nor one that opens a sub menu.
//
//////////////////////////////////////////////////////////////////////////
void TestMenuItemClickEvent(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CWindowTheme>			pTheme;
		Ptr<CMenuStyle>				pStyle;
		Ptr<CMenuContainer>			pMenu;
		Ptr<CMenuContainer>			pSub;
		Ptr<CMenuItem>				pOpen;
		Ptr<CMenuItem>				pSave;
		Ptr<CMenuItem>				pRecent;
		Ptr<CMenuItem>				pFirst;
		CTestMenuListener			cMenuListener;
		CTestMenuListener			cItemListener;
		CPointer					pNull;
		CChars						szLog;

		szLog.Init();
		cMenuListener.Init(&szLog);
		cItemListener.Init(&szLog);
		cItemListener.mszPrefix = "Item:";
		cNativeFactory.Init(&gcMemoryAllocator, 300, 200, NULL);
		pTestWindow = OMalloc<CWindow>("Menu Window", &cNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pStyle = OMalloc<CMenuStyle>(pTestWindow, GenerateMenuFont(pTestWindow));
		pTheme = OMalloc<CWindowTheme>(pStyle->GetFont());
		pTheme->SetMenuStyle(pStyle);

		pMenu = OMalloc<CMenuContainer>(pTestWindow, pTheme);
		pSub = OMalloc<CMenuContainer>(pTestWindow, pTheme);
		pFirst = pSub->AddItem("First");
		pOpen = pMenu->AddItem("Open");
		pSave = pMenu->AddItem("Save");
		pRecent = pMenu->AddItem("Recent", NULL, pSub);
		pMenu->GetItemClickEvent()->Add(&cMenuListener, &CTestMenuListener::MenuItemClicked);
		pSave->GetClickEvent()->Add(&cItemListener, &CTestMenuListener::MenuItemClicked);
		pFirst->GetClickEvent()->Add(&cItemListener, &CTestMenuListener::MenuItemClicked);

		//Enter on Save: its own event first, then the menu's.  Open has no listener of its own.
		pMenu->Open(SInt32Vec2(10, 10), NULL, true);
		pTestWindow->InputCommand(CIC_Down, true);
		pTestWindow->InputCommand(CIC_Down, true);
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertLog("Item:Save;Clicked:Save;");
		pMenu->Click(pOpen);
		AssertLog("Clicked:Open;");

		//An item of a sub menu: its own, then the menu it was reached from.
		pMenu->Open(SInt32Vec2(10, 10), NULL, true);
		pTestWindow->Layout();
		pMenu->OpenSubMenu(pMenu->GetItemRow(pRecent), true);
		pSub->Click(pFirst);
		AssertLog("Item:First;Clicked:First;");
		AssertFalse(pMenu->IsOpen());

		//Disabled or with a sub menu, Click does nothing.
		pSave->SetEnabled(false);
		pMenu->Click(pSave);
		pMenu->Click(pRecent);
		AssertLog("");

		pSave->GetClickEvent()->Remove(&cItemListener);
		pFirst->GetClickEvent()->Remove(&cItemListener);
		pMenu->GetItemClickEvent()->Remove(&cMenuListener);
		pOpen = NULL;
		pSave = NULL;
		pRecent = NULL;
		pFirst = NULL;
		pSub = NULL;
		pMenu = NULL;
		pTheme = NULL;
		pStyle = NULL;
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
void TestMenu(void)
{
	BeginTests();
	ObjectsInit();

	TestMenuItemColumns();
	TestMenuBarPointer();
	TestMenuBarKeys();
	TestMenuBarFrames();
	TestPopupDraw();
	TestMenuContextMenu();
	TestMenuTreeContextMenu();
	TestMenuEnabledAndShortcuts();
	TestMenuItemAndRowEnabledAgree();
	TestMenuItemClickEvent();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

