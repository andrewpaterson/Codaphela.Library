#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WindowLib/GridContainer.h"
#include "WindowLib/ButtonBar.h"
#include "WindowLib/Button.h"
#include "WindowLib/JewelTheme.h"
#include "WindowLib/TextField.h"
#include "WindowLib/CheckBox.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/AssertGeometric.h"
#include "TestLib/AssertFile.h"
#include "ScriptFramesTick.h"


void			ValidateLabelObjects(void);
Ptr<CFont>		GenerateMenuFont(Ptr<CWindow> pWindow);
SInt32Vec2		ComponentCentre(Ptr<CComponent> pComponent);
void			MoveTo(Ptr<CWindow> pWindow, SInt32Vec2 sPosition);
void			Click(Ptr<CWindow> pWindow);


//////////////////////////////////////////////////////////////////////////
//
// A small dialog: a name, a check box and OK and Cancel.  OK and Cancel
// stop it and remember which was clicked.
//
//////////////////////////////////////////////////////////////////////////
class CTestDialog : public CButtonListener
{
CONSTRUCTABLE(CTestDialog);
public:
	Ptr<CWindow>		pDialog;
	Ptr<CWindow>		pOwner;
	Ptr<CTextField>		pName;
	Ptr<CCheckBox>		pInverse;
	Ptr<CButton>		pOK;
	Ptr<CButton>		pCancel;
	CChars				szResult;
	int32				iFrames;  //Painted while shown.
	bool				bOwnerDisabledWhileShown;
	bool				bPressEscape;

	void Init(Ptr<CWindow> pOwnerWindow, CNativeWindowFactory* pcFactory, Ptr<CWindowTick> pTick)
	{
		Ptr<CGridContainer>		pGrid;
		Ptr<CButtonBar>			pBar;
		Ptr<CFont>				pFont;
		Ptr<CWindowTheme>		pTheme;

		szResult.Init();
		iFrames = 0;
		bOwnerDisabledWhileShown = false;
		bPressEscape = false;
		pOwner = pOwnerWindow;

		pDialog = OMalloc<CWindow>("Test Dialog", pcFactory, pTick, OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)0x2B, 0x2D, 0x30)), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pDialog);
		pDialog->SetWindowSize(240, 140);
		pFont = GenerateMenuFont(pDialog);
		pTheme = CJewelTheme::CreateDarkTheme(pDialog, pFont);

		pGrid = OMalloc<CGridContainer>(pDialog);
		pDialog->SetContainer(pGrid);
		pName = OMalloc<CTextField>(pDialog, pTheme, "Clock");
		pName->SetColumns(12);
		pName->SetAlignment(CSH_Stretch, CSV_Center);
		pGrid->AddComponent(pName, 0, 0, 1, 1, 1, 0);
		pInverse = OMalloc<CCheckBox>(pDialog, pTheme, "Inverse Out");
		pInverse->SetAlignment(CSH_Left, CSV_Center);
		pGrid->AddComponent(pInverse, 0, 1, 1, 1, 1, 0);

		pOK = OMalloc<CButton>(pDialog, pTheme, "OK");
		pCancel = OMalloc<CButton>(pDialog, pTheme, "Cancel");
		pOK->AddListener(this);
		pCancel->AddListener(this);
		pDialog->SetDefaultButton(pOK);
		pDialog->SetCancelButton(pCancel);
		pBar = OMalloc<CButtonBar>(pDialog);
		pBar->SetAlignment(CSH_Stretch, CSV_Bottom);
		pBar->AddComponent(pOK);
		pBar->AddComponent(pCancel);
		pGrid->AddComponent(pBar, 0, 2, 1, 1, 1, 1);
	}

	void Kill(void)
	{
		pOK->RemoveListener(this);
		pCancel->RemoveListener(this);
		pName = NULL;
		pInverse = NULL;
		pOK = NULL;
		pCancel = NULL;
		ORoot()->Remove(pDialog);
		pDialog = NULL;
		pOwner = NULL;
		szResult.Kill();
	}

	void ButtonClicked(CUnknown* pcSource, void* pvContext) override
	{
		szResult.Kill();
		szResult.Init(((CButton*)pcSource)->GetCaption());
		pDialog->Stop();
	}
};


//////////////////////////////////////////////////////////////////////////
//
// The dialog's frames: it is drawn, then the name is changed and the box
// ticked, then enter (or escape) closes it.
//
//////////////////////////////////////////////////////////////////////////
bool DialogFrames(Ptr<CWindow> pWindow, int32 iFrame, void* pvContext)
{
	CTestDialog*	pcDialog;
	int32			iShownFrame;

	//The tick counts every frame, the dialog counts the frames of each showing.
	pcDialog = (CTestDialog*)pvContext;
	iShownFrame = pcDialog->iFrames;
	pcDialog->iFrames++;
	pcDialog->bOwnerDisabledWhileShown = !pcDialog->pOwner->GetNativeWindow()->IsEnabled() && (pWindow->GetNativeWindow()->GetOwner() == pcDialog->pOwner->GetNativeWindow());
	if (iShownFrame == 0)
	{
		MoveTo(pWindow, ComponentCentre(pcDialog->pName));
		Click(pWindow);
		pWindow->InputCommand(CIC_End, true);
		pWindow->InputPrintable('2', true);
		MoveTo(pWindow, ComponentCentre(pcDialog->pInverse));
		Click(pWindow);
		return true;
	}
	else if (iShownFrame == 1)
	{
		//Enter clicks OK even though the check box has focus, escape clicks cancel.
		pWindow->InputCommand(pcDialog->bPressEscape ? CIC_ClearSelection : CIC_Enter, true);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The owner shows the dialog from its tick, twice.
//
//////////////////////////////////////////////////////////////////////////
bool DialogOwnerFrames(Ptr<CWindow> pWindow, int32 iFrame, void* pvContext)
{
	CTestDialog*	pcDialog;

	pcDialog = (CTestDialog*)pvContext;
	if (iFrame == 0)
	{
		AssertTrue(pcDialog->pDialog->ShowModal(pWindow));
		AssertString("OK", pcDialog->szResult.Text());
		AssertInt(2, pcDialog->iFrames);
		AssertTrue(pcDialog->bOwnerDisabledWhileShown);
		AssertTrue(pWindow->GetNativeWindow()->IsEnabled());
		AssertTrue(pcDialog->pDialog->GetNativeWindow()->GetOwner() == NULL);
		AssertString("Clock2", pcDialog->pName->GetText());
		AssertTrue(pcDialog->pInverse->IsTicked());

		pcDialog->bPressEscape = true;
		pcDialog->iFrames = 0;
		AssertTrue(pcDialog->pDialog->ShowModal(pWindow));
		AssertString("Cancel", pcDialog->szResult.Text());
		AssertString("Clock22", pcDialog->pName->GetText());
		AssertFalse(pcDialog->pInverse->IsTicked());
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDialogModal(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "DialogFrames";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CScriptFramesTick>		pTick;
		Ptr<CScriptFramesTick>		pDialogTick;
		CTestDialog					cDialog;
		CChars						szExpectedDirectory;

		cNativeFactory.Init(&gcMemoryAllocator, 300, 200, szDirectory);
		pTick = OMalloc<CScriptFramesTick>(DialogOwnerFrames, &cDialog);
		pTestWindow = OMalloc<CWindow>("Dialog Owner", &cNativeFactory, pTick, OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)0x1E, 0x1F, 0x22)), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pDialogTick = OMalloc<CScriptFramesTick>(DialogFrames, &cDialog);
		cDialog.Init(pTestWindow, &cNativeFactory, pDialogTick);

		pTestWindow->Show();

		ValidateLabelObjects();

		cDialog.Kill();
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		pDialogTick = NULL;

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
void TestDialog(void)
{
	BeginTests();
	ObjectsInit();

	TestDialogModal();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

