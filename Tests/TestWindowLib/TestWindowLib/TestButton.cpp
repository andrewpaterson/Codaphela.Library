#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "SupportLib/ImageColourConverter.h"
#include "WindowLib/Window.h"
#include "WindowLib/GridContainer.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WindowLib/Button.h"
#include "WindowLib/WindowTheme.h"
#include "WindowLib/JewelImageGenerator.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/AssertGeometric.h"
#include "TestLib/AssertFile.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"
#include "SpaceCrusade.h"
#include "DarkForest.h"


Ptr<CFont> ReadLabelFont(void);
void ValidateLabelObjects(void);


#define AssertLog(e)	AssertString(e, szLog.Text()); szLog.Clear()


//////////////////////////////////////////////////////////////////////////
//
// Logs each click as "Caption:Clicked;".
//
//////////////////////////////////////////////////////////////////////////
class CTestButtonListener : public CButtonListener
{
CONSTRUCTABLE(CTestButtonListener);
public:
	CChars*		mpszLog;

	void Init(CChars* pszLog)
	{
		mpszLog = pszLog;
	}

	void ButtonClicked(CUnknown* pcSource, void* pvContext) override
	{
		CButton*	pcButton;

		pcButton = (CButton*)pcSource;
		mpszLog->Append(pcButton->GetCaption());
		mpszLog->Append(":Clicked;");
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CButtonImages> ReadBlueButtonImages(CColourFormatHelper* pcFormat)
{
	Ptr<CImage>		apStrips[NUM_BUTTON_STATES];
	char*			aszStates[NUM_BUTTON_STATES] = { (char*)"BlueBorderNormal.png", (char*)"BlueBorderMouseOver.png", (char*)"BlueBorderSelected.png", (char*)"BlueBorderPushed.png" };
	size			i;

	for (i = 0; i < NUM_BUTTON_STATES; i++)
	{
		apStrips[i] = ReadSpaceImage("BlueButton", aszStates[i]);
		apStrips[i] = CImageColourConverter::ConvertColour(apStrips[i], pcFormat->meOrder);
	}
	return OMalloc<CButtonImages>(apStrips[BS_Normal], apStrips[BS_MouseOver], apStrips[BS_Focussed], apStrips[BS_Pushed], Set32BitColour((uint8)115, 151, 255));
}


//////////////////////////////////////////////////////////////////////////
//
// A theme with only a font and button images, all a button needs.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CWindowTheme> CreateButtonTheme(Ptr<CFont> pFont, Ptr<CButtonImages> pImages, Ptr<CButtonImages> pDefaultImages)
{
	Ptr<CWindowTheme>	pTheme;

	pTheme = OMalloc<CWindowTheme>(pFont);
	pTheme->SetButtonImages(pImages, pDefaultImages);
	return pTheme;
}


//////////////////////////////////////////////////////////////////////////
//
// The window is 200x60 with an OK button centred in its left half and a
// Cancel button centred in its right half.  The grid between them is the
// background.
//
//////////////////////////////////////////////////////////////////////////
void TestButtonInput(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CGridContainer>		pGrid;
		Ptr<CButtonImages>		pImages;
		Ptr<CWindowTheme>		pTheme;
		Ptr<CFont>				pFont;
		Ptr<CButton>			pOK;
		Ptr<CButton>			pCancel;
		Ptr<CFocus>				pFocus;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow		cData;
		CColourFormatHelper		cHelper;
		CTestButtonListener		cListener;
		CPointer				pNull;
		CChars					szLog;
		SInt32Vec2				sSize;
		SInt32Vec2				sPosition;
		int32					iCaptionWidth;
		int32					iSpare;
		int						x;
		int						y;

		szLog.Init();
		cListener.Init(&szLog);
		cNativeFactory.Init(&gcMemoryAllocator, 200, 60, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Button Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pTestWindow->GetColourFormat(&cHelper);
		pFocus = pTestWindow->GetFocus();

		pFont = ReadLabelFont();
		pImages = ReadBlueButtonImages(&cHelper);
		AssertInt(8, pImages->GetTileSize());

		pTheme = CreateButtonTheme(pFont, pImages, NULL);
		pOK = OMalloc<CButton>(pTestWindow, pTheme, "OK");
		pCancel = OMalloc<CButton>(pTestWindow, pTheme, "Cancel");
		AssertTrue(pOK->AddListener(&cListener));
		AssertTrue(pCancel->AddListener(&cListener));
		pGrid = OMalloc<CGridContainer>(pTestWindow);
		pTestWindow->SetContainer(pGrid);
		AssertTrue(pGrid->AddComponent(pOK, 0, 0, 1, 1, 1, 1));
		AssertTrue(pGrid->AddComponent(pCancel, 1, 0, 1, 1, 1, 1));

		//A button is its caption, 8 pixels of padding each side and 2 above and below, inside an 8 pixel border.
		iCaptionWidth = (int32)pFont->CalculateWidth("OK");
		sSize = pOK->GetDesiredSize();
		AssertInt32Vec2(iCaptionWidth + 32, 44, &sSize);

		//Each column is its button's width plus half the spare width.  The button is centred in it.
		pTestWindow->Layout();
		sSize = pOK->GetActualSize();
		AssertInt32Vec2(iCaptionWidth + 32, 44, &sSize);
		iSpare = 200 - sSize.x - pCancel->GetActualSize().x;
		sPosition = pOK->GetPosition();
		AssertInt32Vec2((iSpare / 2) / 2, 8, &sPosition);
		AssertInt(BS_Normal, pOK->GetState());

		//The button is hit as a whole, its border and face included.
		x = sPosition.x + sSize.x / 2;
		y = sPosition.y + sSize.y / 2;
		AssertTrue(pTestWindow->FindComponentAt(x, y) == pOK);
		AssertTrue(pTestWindow->FindComponentAt(sPosition.x, sPosition.y) == pOK);
		AssertTrue(pTestWindow->FindComponentAt(sPosition.x - 1, y) == pGrid);

		pTestWindow->PointerMoved(x, y);
		AssertInt(BS_MouseOver, pOK->GetState());
		AssertTrue(pFocus->GetMouseOverComponent() == pOK);

		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		AssertInt(BS_Pushed, pOK->GetState());
		AssertTrue(pOK->IsPressed());
		AssertTrue(pOK->IsFocussed());
		AssertLog("");
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertLog("OK:Clicked;");
		AssertInt(BS_MouseOver, pOK->GetState());
		AssertFalse(pOK->IsPressed());

		//Moving off a pressed button and letting go doesn't click it.
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->PointerMoved(2, y);
		AssertInt(BS_Focussed, pOK->GetState());
		pTestWindow->PointerMoved(x, y);
		AssertInt(BS_Pushed, pOK->GetState());
		pTestWindow->PointerMoved(2, y);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertLog("");
		AssertInt(BS_Focussed, pOK->GetState());

		//Space and enter click the button with focus.
		pTestWindow->InputCommand(CIC_Space, true);
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertLog("OK:Clicked;OK:Clicked;");

		//Hovering over Cancel doesn't take focus from OK.
		x = pCancel->GetPosition().x + pCancel->GetActualSize().x / 2;
		pTestWindow->PointerMoved(x, y);
		AssertInt(BS_Focussed, pOK->GetState());
		AssertInt(BS_MouseOver, pCancel->GetState());
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		AssertInt(BS_Normal, pOK->GetState());
		AssertInt(BS_Pushed, pCancel->GetState());
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertLog("Cancel:Clicked;");

		//Clicking the background clears focus.
		pTestWindow->PointerMoved(100, 2);
		AssertInt(BS_Focussed, pCancel->GetState());
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertInt(BS_Normal, pCancel->GetState());
		AssertTrue(pFocus->GetFocussedComponent().IsNull());
		AssertLog("");

		//Deactivating the window ends a press.
		pTestWindow->PointerMoved(x, y);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		AssertInt(BS_Pushed, pCancel->GetState());
		pTestWindow->Deactivate();
		AssertInt(BS_Normal, pCancel->GetState());
		AssertFalse(pCancel->IsPressed());
		//Focus comes back but the pointer isn't known until it moves.
		pTestWindow->Activate();
		AssertInt(BS_Focussed, pCancel->GetState());
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertLog("");
		pTestWindow->PointerMoved(x, y);
		AssertInt(BS_MouseOver, pCancel->GetState());

		AssertTrue(pCancel->RemoveListener(&cListener));
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertLog("");

		//A longer caption makes the button wider.
		pOK->SetCaption("Okay");
		AssertString("Okay", pOK->GetCaption());
		sSize = pOK->GetDesiredSize();
		AssertInt32Vec2((int32)pFont->CalculateWidth("Okay") + 32, 44, &sSize);
		pOK->SetPadding(0, 0);
		sSize = pOK->GetDesiredSize();
		AssertInt32Vec2((int32)pFont->CalculateWidth("Okay") + 16, 40, &sSize);

		ValidateLabelObjects();

		pOK = NULL;
		pCancel = NULL;
		pGrid = NULL;
		pImages = NULL;
		pTheme = NULL;
		pFont = NULL;
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
// The window is 300x60 with OK, Cancel and a focusable Name label in three
// columns.  OK is the default button.
//
//////////////////////////////////////////////////////////////////////////
void TestButtonDefault(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CGridContainer>		pGrid;
		Ptr<CButtonImages>		pImages;
		Ptr<CButtonImages>		pDefaultImages;
		Ptr<CWindowTheme>		pTheme;
		Ptr<CFont>				pFont;
		Ptr<CButton>			pOK;
		Ptr<CButton>			pCancel;
		Ptr<CLabel>				pName;
		Ptr<CFocus>				pFocus;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow		cData;
		CColourFormatHelper		cHelper;
		CTestButtonListener		cListener;
		CPointer				pNull;
		CChars					szLog;
		SInt32Vec2				sSize;
		SInt32Vec2				sDefaultSize;
		int						y;

		szLog.Init();
		cListener.Init(&szLog);
		cNativeFactory.Init(&gcMemoryAllocator, 300, 60, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Default Button Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pTestWindow->GetColourFormat(&cHelper);
		pFocus = pTestWindow->GetFocus();

		pFont = ReadLabelFont();
		pImages = CJewelImageGenerator::CreateButtonImages(&cHelper);
		pDefaultImages = CJewelImageGenerator::CreateDefaultButtonImages(&cHelper);
		AssertInt(Set32BitColour((uint8)0x2B, 0x2D, 0x30), pImages->GetFaceColour(BS_Pushed));
		AssertInt(Set32BitColour((uint8)0x36, 0x6A, 0xCE), pDefaultImages->GetFaceColour(BS_Pushed));

		pTheme = CreateButtonTheme(pFont, pImages, pDefaultImages);
		pOK = OMalloc<CButton>(pTestWindow, pTheme, "OK");
		pCancel = OMalloc<CButton>(pTestWindow, pTheme, "Cancel");
		pName = OMalloc<CLabel>(pTestWindow, pFont, "Name");
		pName->SetCanGetFocus(true);
		AssertTrue(pOK->AddListener(&cListener));
		AssertTrue(pCancel->AddListener(&cListener));
		pGrid = OMalloc<CGridContainer>(pTestWindow);
		pTestWindow->SetContainer(pGrid);
		AssertTrue(pGrid->AddComponent(pOK, 0, 0, 1, 1, 1, 1));
		AssertTrue(pGrid->AddComponent(pCancel, 1, 0, 1, 1, 1, 1));
		AssertTrue(pGrid->AddComponent(pName, 2, 0, 1, 1, 1, 1));
		pTestWindow->Layout();

		//Without a default button enter goes nowhere when nothing has focus.
		AssertTrue(pTestWindow->GetDefaultButton().IsNull());
		AssertFalse(pOK->IsDefault());
		AssertTrue(pOK->GetImages() == pImages);
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertLog("");

		//The default button takes on its default images.  Default images don't change its size.
		sSize = pOK->GetDesiredSize();
		pTestWindow->SetDefaultButton(pOK);
		AssertTrue(pTestWindow->GetDefaultButton() == pOK);
		AssertTrue(pOK->IsDefault());
		AssertTrue(pOK->GetImages() == pDefaultImages);
		sDefaultSize = pOK->GetDesiredSize();
		AssertInt32Vec2(sSize.x, sSize.y, &sDefaultSize);

		//With nothing focussed enter clicks the default button.
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertLog("OK:Clicked;");

		//A focussed button wants enter for itself.
		y = pCancel->GetPosition().y + pCancel->GetActualSize().y / 2;
		pTestWindow->PointerMoved(pCancel->GetPosition().x + pCancel->GetActualSize().x / 2, y);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertLog("Cancel:Clicked;");
		AssertTrue(pCancel->IsFocussed());
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertLog("Cancel:Clicked;");

		//A focussed component that doesn't want enter leaves it to the default button.
		pTestWindow->PointerMoved(pName->GetPosition().x + 1, pName->GetPosition().y + 1);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertTrue(pName->IsFocussed());
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertLog("OK:Clicked;");

		//The default button clicks itself when focussed, once and not twice.
		pTestWindow->PointerMoved(pOK->GetPosition().x + pOK->GetActualSize().x / 2, y);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertLog("OK:Clicked;");
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertLog("OK:Clicked;");

		//Making another button the default takes it, and the default images, from the first.
		pTestWindow->SetDefaultButton(pCancel);
		AssertFalse(pOK->IsDefault());
		AssertTrue(pOK->GetImages() == pImages);
		AssertTrue(pCancel->IsDefault());
		AssertTrue(pCancel->GetImages() == pDefaultImages);

		pTestWindow->SetDefaultButton(pNull);
		AssertFalse(pCancel->IsDefault());
		AssertTrue(pTestWindow->GetDefaultButton().IsNull());

		ValidateLabelObjects();

		pOK = NULL;
		pCancel = NULL;
		pName = NULL;
		pGrid = NULL;
		pImages = NULL;
		pTheme = NULL;
		pDefaultImages = NULL;
		pFont = NULL;
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
// Draws a frame then moves the button on to its next state: normal, mouse
// over, pushed and then selected once the pointer moves away.  If asked it
// moves back over the button, which pushes it again as the press is still
// held.
//
//////////////////////////////////////////////////////////////////////////
class CButtonStatesTick : public CWindowTick
{
CONSTRUCTABLE(CButtonStatesTick);
DESTRUCTABLE(CButtonStatesTick);
protected:
	int32		miFrame;
	SInt32Vec2	msOver;
	SInt32Vec2	msAway;
	bool		mbReturnOver;

public:
	//The positions are set with SetPositions once the button has been laid out.
	void Init(bool bReturnOver)
	{
		PreInit();
		CWindowTick::Init();
		miFrame = 0;
		msOver.Init(0, 0);
		msAway.Init(0, 0);
		mbReturnOver = bReturnOver;
		PostInit();
	}

	void SetPositions(SInt32Vec2 sOver, SInt32Vec2 sAway)
	{
		msOver = sOver;
		msAway = sAway;
	}

	void Class(void) override
	{
		CWindowTick::Class();
		U_Int32(miFrame);
		U_2Int32(msOver);
		U_2Int32(msAway);
		U_Bool(mbReturnOver);
	}

	void Tick(Ptr<CWindow> pWindow, int64 iUpdateTimeInMillieconds, int64 iTotalTimeInMillieconds) override
	{
		pWindow->Paint();

		miFrame++;
		if (miFrame == 1)
		{
			pWindow->PointerMoved(msOver.x, msOver.y);
		}
		else if (miFrame == 2)
		{
			pWindow->InputCommand(CIC_ActivateComponent, true);
		}
		else if (miFrame == 3)
		{
			pWindow->PointerMoved(msAway.x, msAway.y);
		}
		else if ((miFrame == 4) && mbReturnOver)
		{
			pWindow->PointerMoved(msOver.x, msOver.y);
		}
		else
		{
			pWindow->Stop();
		}
	}
};


//////////////////////////////////////////////////////////////////////////
//
// The Jewel button's last frame is pushed after selected, so the focus
// ring drawn in the frame before must be gone from its empty band.  With
// default images the button is made the window's default button.
//
//////////////////////////////////////////////////////////////////////////
void TestButtonStates(char* szName, Ptr<CButtonImages>(*ReadImages)(CColourFormatHelper*), Ptr<CButtonImages>(*ReadDefaultImages)(CColourFormatHelper*), ARGB32 sBackground, bool bReturnOver)
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
		Ptr<CFillContainer>		pFill;
		Ptr<CButtonImages>		pImages;
		Ptr<CFont>				pFont;
		Ptr<CButton>			pButton;
		Ptr<CButtonStatesTick>		pTick;
		CColourFormatHelper		cHelper;
		CPointer				pNull;
		CChars					szExpectedDirectory;
		SInt32Vec2				sPosition;
		SInt32Vec2				sSize;

		cNativeFactory.Init(&gcMemoryAllocator, 120, 60, szDirectory.Text());
		//The tick is allocated, not on the stack, because the window is rooted and stack objects can't be under the root.
		pTick = OMalloc<CButtonStatesTick>(bReturnOver);
		pTestWindow = OMalloc<CWindow>("Button States", &cNativeFactory, pTick, OMalloc<CClearCanvasDraw>(sBackground), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pTestWindow->GetColourFormat(&cHelper);

		pFont = ReadLabelFont();
		pImages = ReadImages(&cHelper);
		if (ReadDefaultImages)
		{
			pButton = OMalloc<CButton>(pTestWindow, CreateButtonTheme(pFont, pImages, ReadDefaultImages(&cHelper)), "Press Me");
			pTestWindow->SetDefaultButton(pButton);
		}
		else
		{
			pButton = OMalloc<CButton>(pTestWindow, CreateButtonTheme(pFont, pImages, NULL), "Press Me");
		}
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pFill->AddComponent(pButton);

		pTestWindow->Layout();
		sPosition = pButton->GetPosition();
		sSize = pButton->GetActualSize();
		pTick->SetPositions(SInt32Vec2(sPosition.x + sSize.x / 2, sPosition.y + sSize.y / 2), SInt32Vec2(1, 1));

		pTestWindow->Show();
		AssertInt(bReturnOver ? BS_Pushed : BS_Focussed, pButton->GetState());

		ValidateLabelObjects();

		pButton = NULL;
		pFill = NULL;
		pImages = NULL;
		pFont = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;

		szExpectedDirectory.Init(szDirectory);
		szExpectedDirectory.Replace("Output", "Input");
		AssertDirectory(szExpectedDirectory.Text(), szDirectory.Text());
		szExpectedDirectory.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	AssertSize(2, gcObjects.NumMemoryIndexes());
	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory.Text());
	szDirectory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Three buttons wider than they need, side by side in a grid: an icon
// with a caption, an icon alone and a caption alone.  An icon is on the
// left with its caption aligned left beside it, the padding in from the
// face.  An icon or a caption alone is centred.
//
//////////////////////////////////////////////////////////////////////////
void TestButtonIcons(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "ButtonIcons";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CTickTestRefWindow>	pTick;
		SDataTestRefWindow		cData;
		Ptr<CGridContainer>		pGrid;
		Ptr<CButtonImages>		pImages;
		Ptr<CWindowTheme>		pTheme;
		Ptr<CFont>				pFont;
		Ptr<CImageCel>			pWolf;
		Ptr<CImageCel>			pMoon;
		Ptr<CButton>			pBoth;
		Ptr<CButton>			pIconOnly;
		Ptr<CButton>			pCaptionOnly;
		CColourFormatHelper		cHelper;
		CChars					szExpectedDirectory;
		SInt32Vec2				sFace;
		SInt32Vec2				sFaceSize;
		SInt32Vec2				sVec;
		int32					iWolfWidth;
		int32					iMoonWidth;
		int32					iCaptionWidth;

		cNativeFactory.Init(&gcMemoryAllocator, 300, 60, szDirectory);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Button Icons", &cNativeFactory, pTick, OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)0x1E, 0x1F, 0x22)), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pTestWindow->GetColourFormat(&cHelper);

		pFont = ReadLabelFont();
		pImages = CJewelImageGenerator::CreateButtonImages(&cHelper);
		pWolf = ReadDarkForestCel("Wolf.png", &cHelper);
		pMoon = ReadDarkForestCel("Moon.png", &cHelper);
		iWolfWidth = pWolf->GetSubImage()->GetFullWidth();
		iMoonWidth = pMoon->GetSubImage()->GetFullWidth();

		pTheme = CreateButtonTheme(pFont, pImages, NULL);
		pBoth = OMalloc<CButton>(pTestWindow, pTheme, pWolf, "Open");
		pIconOnly = OMalloc<CButton>(pTestWindow, pTheme, pMoon, (char*)NULL);
		pCaptionOnly = OMalloc<CButton>(pTestWindow, pTheme, "Open");
		pBoth->SetDesiredSize(100, -1);
		pIconOnly->SetDesiredSize(100, -1);
		pCaptionOnly->SetDesiredSize(100, -1);
		pGrid = OMalloc<CGridContainer>(pTestWindow);
		pTestWindow->SetContainer(pGrid);
		pGrid->AddComponent(pBoth, 0, 0);
		pGrid->AddComponent(pIconOnly, 1, 0);
		pGrid->AddComponent(pCaptionOnly, 2, 0);

		//The caption decides the height of a button with one, the icon of one without.
		iCaptionWidth = pBoth->GetCaptionLabel()->GetDesiredSize().x;
		sVec = pBoth->GetDesiredSize();
		AssertInt32Vec2(100, 24 + 2 * 2 + 8 * 2, &sVec);
		sVec = pIconOnly->GetDesiredSize();
		AssertInt32Vec2(100, pMoon->GetSubImage()->GetFullHeight() + 2 * 2 + 8 * 2, &sVec);

		pTestWindow->Show();

		//The face is inside the eight pixel border, the content eight in from the face's sides.
		sFace = pBoth->GetPosition();
		sFace.x += 8;
		sFace.y += 8;
		sVec = pBoth->GetIconComponent()->GetPosition();
		AssertInt(sFace.x + 8, sVec.x);
		sVec = pBoth->GetIconComponent()->GetActualSize();
		AssertInt(iWolfWidth + 4, sVec.x);
		sVec = pBoth->GetCaptionLabel()->GetPosition();
		AssertInt(sFace.x + 8 + iWolfWidth + 4, sVec.x);
		AssertInt(sFace.y + 2, sVec.y);

		//An icon alone is centred.
		sFace = pIconOnly->GetPosition();
		sFace.x += 8;
		sFace.y += 8;
		sFaceSize = pIconOnly->GetActualSize();
		sFaceSize.x -= 16;
		sVec = pIconOnly->GetIconComponent()->GetPosition();
		AssertInt(sFace.x + 8 + (sFaceSize.x - 16 - iMoonWidth) / 2, sVec.x);
		AssertFalse(pIconOnly->GetCaptionLabel()->GetParent().IsNotNull());

		//As is a caption alone.
		sFace = pCaptionOnly->GetPosition();
		sFace.x += 8;
		sVec = pCaptionOnly->GetCaptionLabel()->GetPosition();
		AssertInt(sFace.x + 8 + (100 - 16 - 16 - iCaptionWidth) / 2, sVec.x);

		//Taking the caption away centres the icon, giving it back puts it on the left again.
		pBoth->SetCaption("");
		pTestWindow->Layout();
		sFace = pBoth->GetPosition();
		sFace.x += 8;
		sVec = pBoth->GetIconComponent()->GetPosition();
		AssertInt(sFace.x + 8 + (100 - 16 - 16 - iWolfWidth) / 2, sVec.x);
		pBoth->SetCaption("Open");
		pTestWindow->Layout();
		sVec = pBoth->GetIconComponent()->GetPosition();
		AssertInt(sFace.x + 8, sVec.x);

		ValidateLabelObjects();

		pBoth = NULL;
		pIconOnly = NULL;
		pCaptionOnly = NULL;
		pGrid = NULL;
		pImages = NULL;
		pTheme = NULL;
		pFont = NULL;
		pWolf = NULL;
		pMoon = NULL;
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
void TestButton(void)
{
	BeginTests();
	ObjectsInit();

	TestButtonInput();
	TestButtonDefault();
	TestButtonIcons();
	TestButtonStates((char*)"ButtonStates", ReadBlueButtonImages, NULL, Set32BitColour((uint8)40, 40, 48), false);
	TestButtonStates((char*)"JewelButtonStates", CJewelImageGenerator::CreateButtonImages, NULL, Set32BitColour((uint8)0x1E, 0x1F, 0x22), true);
	TestButtonStates((char*)"JewelDefaultButtonStates", CJewelImageGenerator::CreateButtonImages, CJewelImageGenerator::CreateDefaultButtonImages, Set32BitColour((uint8)0x1E, 0x1F, 0x22), true);

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

