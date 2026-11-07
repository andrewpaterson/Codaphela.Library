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
#include "WindowLib/JewelColours.h"
#include "WindowLib/MessageDialog.h"
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
class CTestDialog
{
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
		pOK->GetClickEvent()->Add(this, &CTestDialog::Clicked);
		pCancel->GetClickEvent()->Add(this, &CTestDialog::Clicked);
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
		pOK->GetClickEvent()->Remove(this);
		pCancel->GetClickEvent()->Remove(this);
		pName = NULL;
		pInverse = NULL;
		pOK = NULL;
		pCancel = NULL;
		ORoot()->Remove(pDialog);
		pDialog = NULL;
		pOwner = NULL;
		szResult.Kill();
	}

	void Clicked(CButton* pcButton)
	{
		szResult.Kill();
		szResult.Init(pcButton->GetCaption());
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
// The close button only closes a window its close approver lets close,
// as an editor that asks to save its changes first.
//
//////////////////////////////////////////////////////////////////////////
class CTestCloseApprover
{
public:
	bool	mbAllow;
	int		miAsked;

	bool Approve(void)
	{
		miAsked++;
		return mbAllow;
	}
};


void TestWindowCloseApprover(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>		pTestWindow;
		CPointer			pNull;
		CTestCloseApprover	cApprover;

		cNativeFactory.Init(&gcMemoryAllocator, 100, 50, NULL);
		pTestWindow = OMalloc<CWindow>("Close Window", &cNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		AssertTrue(pTestWindow->ApproveClose());
		cApprover.mbAllow = false;
		cApprover.miAsked = 0;
		pTestWindow->SetCloseApprover(&cApprover, &CTestCloseApprover::Approve);
		AssertFalse(pTestWindow->ApproveClose());
		cApprover.mbAllow = true;
		AssertTrue(pTestWindow->ApproveClose());
		AssertInt(2, cApprover.miAsked);
		pTestWindow->ClearCloseApprover();
		cApprover.mbAllow = false;
		AssertTrue(pTestWindow->ApproveClose());
		AssertInt(2, cApprover.miAsked);

		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}

//////////////////////////////////////////////////////////////////////////
//
// A call posted to a window is made at its next tick, once however many
// times it was posted; one cancelled isn't made; one made can post
// another, made at the tick after.
//
//////////////////////////////////////////////////////////////////////////
class CTestPosted
{
public:
	int				miFirst;
	int				miSecond;
	CWindow*		mpcWindow;

	void First(void)
	{
		miFirst++;
		mpcWindow->Post(this, &CTestPosted::Second);
	}

	void Second(void)
	{
		miSecond++;
	}
};


void TestWindowPost(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>	pTestWindow;
		CPointer		pNull;
		CTestPosted		cPosted;

		cNativeFactory.Init(&gcMemoryAllocator, 100, 50, NULL);
		pTestWindow = OMalloc<CWindow>("Post Window", &cNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		cPosted.miFirst = 0;
		cPosted.miSecond = 0;
		cPosted.mpcWindow = &pTestWindow;

		pTestWindow->Post(&cPosted, &CTestPosted::First);
		pTestWindow->Post(&cPosted, &CTestPosted::First);
		AssertInt(0, cPosted.miFirst);
		pTestWindow->RunPosted();
		AssertInt(1, cPosted.miFirst);
		AssertInt(0, cPosted.miSecond);
		pTestWindow->RunPosted();
		AssertInt(1, cPosted.miSecond);
		pTestWindow->RunPosted();
		AssertInt(1, cPosted.miFirst);
		AssertInt(1, cPosted.miSecond);

		pTestWindow->Post(&cPosted, &CTestPosted::First);
		pTestWindow->CancelPosted(&cPosted);
		pTestWindow->RunPosted();
		AssertInt(1, cPosted.miFirst);

		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A dialog's parts, to look at them without showing it.
//
//////////////////////////////////////////////////////////////////////////
class CTestDialogParts : public CDialog
{
CONSTRUCTABLE(CTestDialogParts);
DESTRUCTABLE(CTestDialogParts);
public:
	Ptr<CButton>	GetOK(void) { return mpOK; }
	Ptr<CButton>	GetCancel(void) { return mpCancel; }
	Ptr<CButton>	GetExtra(void) { return mpExtra; }
	bool			IsShown(Ptr<CButton> pButton) { return pButton->GetParent() == mpButtons; }
};


//////////////////////////////////////////////////////////////////////////
//
// A dialog's look, in the dark Jewel theme.
//
//////////////////////////////////////////////////////////////////////////
static void InitTestDialogLook(SDialogLook* psLook, Ptr<CWindowTheme> pTheme)
{
	psLook->pcTheme = &pTheme;
	psLook->pcDimFont = &pTheme->GetFont();
	psLook->sBackground = JEWEL_PANEL;
	psLook->sDivider = JEWEL_SEPARATOR;
}


//////////////////////////////////////////////////////////////////////////
//
// Every button is at least DIALOG_BUTTON_WIDTH wide, or as wide as its
// caption.  The extra button goes before OK and closes the dialog as its
// caption; without Cancel, escape clicks OK.
//
//////////////////////////////////////////////////////////////////////////
void TestDialogButtons(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CWindowTheme>		pTheme;
		SDialogLook				sLook;
		Ptr<CTestDialogParts>	pDialog;
		CPointer				pNull;

		cNativeFactory.Init(&gcMemoryAllocator, 300, 200, NULL);
		pTestWindow = OMalloc<CWindow>("Dialog Owner", &cNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pTheme = CJewelTheme::CreateDarkTheme(pTestWindow, GenerateMenuFont(pTestWindow));
		InitTestDialogLook(&sLook, pTheme);

		pDialog = OMalloc<CTestDialogParts>(pTestWindow, &sLook, "Buttons");
		ORoot()->Add(pDialog);
		AssertTrue(pDialog->IsShown(pDialog->GetOK()));
		AssertTrue(pDialog->IsShown(pDialog->GetCancel()));
		AssertTrue(pDialog->GetExtra().IsNull());
		AssertInt(DIALOG_BUTTON_WIDTH, pDialog->GetOK()->GetDesiredSize().x);
		AssertTrue(pDialog->GetCancel()->GetDesiredSize().x >= DIALOG_BUTTON_WIDTH);

		pDialog->SetOKCaption("Refactor Everything Now");
		AssertTrue(pDialog->GetOK()->GetDesiredSize().x > DIALOG_BUTTON_WIDTH);
		pDialog->SetOKCaption("Go");
		AssertInt(DIALOG_BUTTON_WIDTH, pDialog->GetOK()->GetDesiredSize().x);

		pDialog->AddExtraButton("Don't");
		AssertTrue(pDialog->IsShown(pDialog->GetExtra()));
		AssertInt(DIALOG_BUTTON_WIDTH, pDialog->GetExtra()->GetDesiredSize().x);
		pDialog->GetWindow()->SetWindowSize(400, 100);
		pDialog->GetWindow()->Layout();
		AssertTrue(pDialog->GetExtra()->GetPosition().x < pDialog->GetOK()->GetPosition().x);
		AssertTrue(pDialog->GetOK()->GetPosition().x < pDialog->GetCancel()->GetPosition().x);
		pDialog->GetExtra()->Click();
		AssertString("Don't", pDialog->GetClicked());

		pDialog->SetCancelCaption("Stop");
		AssertString("Stop", pDialog->GetCancel()->GetCaption());
		pDialog->GetCancel()->Click();
		AssertString("Stop", pDialog->GetClicked());

		pDialog->SetCancelCaption(NULL);
		AssertFalse(pDialog->IsShown(pDialog->GetCancel()));
		AssertTrue(pDialog->IsShown(pDialog->GetOK()));
		pDialog->GetWindow()->InputCommand(CIC_ClearSelection, true);
		AssertString("OK", pDialog->GetClicked());

		ORoot()->Remove(pDialog);
		pDialog = NULL;
		sLook.Kill();
		pTheme = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A message dialog's parts, to look at them.
//
//////////////////////////////////////////////////////////////////////////
class CTestMessageDialogParts : public CMessageDialog
{
CONSTRUCTABLE(CTestMessageDialogParts);
DESTRUCTABLE(CTestMessageDialogParts);
public:
	Ptr<CScrollPane>	GetItemsPane(void) { return mpItemsPane; }
};


//////////////////////////////////////////////////////////////////////////
//
// A message dialog shown from its owner's tick and answered from its own.
//
//////////////////////////////////////////////////////////////////////////
class CTestMessageDialog
{
public:
	SDialogLook						sLook;
	Ptr<CWindowTheme>				pTheme;
	Ptr<CTestMessageDialogParts>	pMessage;
	const char*						szAnswer;  //What the dialog's tick closes it as.
	SInt32Vec2						sShownSize;

	void Answer(void)
	{
		sShownSize = pMessage->GetDialog()->GetWindow()->GetActualSize();
		pMessage->GetDialog()->Close(szAnswer);
	}
};


//////////////////////////////////////////////////////////////////////////
//
// The owner shows a three button question about two files, then a long
// list, and a dialog that only tells something.
//
//////////////////////////////////////////////////////////////////////////
bool MessageDialogOwnerFrames(Ptr<CWindow> pWindow, int32 iFrame, void* pvContext)
{
	CTestMessageDialog*	pcTest;
	char				szItem[16];
	int32				iRow;
	int					i;

	if (iFrame != 0)
	{
		return false;
	}
	pcTest = (CTestMessageDialog*)pvContext;

	//Save, Don't Save and Cancel; the extra button's answer.
	pcTest->pMessage = OMalloc<CTestMessageDialogParts>(pWindow, &pcTest->sLook, "Save Changes", "Save changes to the following files?");
	pcTest->pMessage->AddItem("First.cpp");
	pcTest->pMessage->AddItem("Second.cpp");
	pcTest->pMessage->GetDialog()->SetOKCaption("Save");
	pcTest->pMessage->GetDialog()->AddExtraButton("Don't Save");
	pcTest->pMessage->GetDialog()->SetTicker(pcTest, &CTestMessageDialog::Answer);
	pcTest->szAnswer = "Don't Save";
	AssertFalse(pcTest->pMessage->Show());
	AssertString("Don't Save", pcTest->pMessage->GetClicked());
	AssertTrue(pcTest->sShownSize.x >= DIALOG_MIN_WIDTH);
	iRow = pcTest->pMessage->GetItemsPane()->GetDesiredSize().y;

	//A long list shows the most it can and scrolls.
	pcTest->pMessage = OMalloc<CTestMessageDialogParts>(pWindow, &pcTest->sLook, "Files", "These files:\nare many.");
	for (i = 0; i < MESSAGE_DIALOG_MAX_ITEMS + 5; i++)
	{
		sprintf(szItem, "File%d.cpp", i);
		pcTest->pMessage->AddItem(szItem);
	}
	AssertTrue(pcTest->pMessage->GetItemsPane()->GetDesiredSize().y < iRow * (MESSAGE_DIALOG_MAX_ITEMS + 5) / 2);
	pcTest->pMessage->GetDialog()->SetTicker(pcTest, &CTestMessageDialog::Answer);
	pcTest->szAnswer = "OK";
	AssertTrue(pcTest->pMessage->Show());

	//Only OK, no list.
	pcTest->pMessage = OMalloc<CTestMessageDialogParts>(pWindow, &pcTest->sLook, "Note", "Nothing to do.");
	pcTest->pMessage->GetDialog()->SetCancelCaption(NULL);
	AssertTrue(pcTest->pMessage->GetItemsPane().IsNull());
	pcTest->pMessage->GetDialog()->SetTicker(pcTest, &CTestMessageDialog::Answer);
	AssertTrue(pcTest->pMessage->Show());
	AssertTrue(pcTest->sShownSize.x == DIALOG_MIN_WIDTH);
	pcTest->pMessage = NULL;
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMessageDialog(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CScriptFramesTick>		pTick;
		CTestMessageDialog			cTest;

		cNativeFactory.Init(&gcMemoryAllocator, 300, 200, NULL);
		pTick = OMalloc<CScriptFramesTick>(MessageDialogOwnerFrames, &cTest);
		pTestWindow = OMalloc<CWindow>("Message Owner", &cNativeFactory, pTick, OMalloc<CClearCanvasDraw>(JEWEL_EDITOR), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		cTest.pTheme = CJewelTheme::CreateDarkTheme(pTestWindow, GenerateMenuFont(pTestWindow));
		InitTestDialogLook(&cTest.sLook, cTest.pTheme);

		pTestWindow->Show();

		cTest.sLook.Kill();
		cTest.pTheme = NULL;
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
void TestDialog(void)
{
	BeginTests();
	ObjectsInit();

	TestDialogModal();
	TestDialogButtons();
	TestMessageDialog();
	TestWindowCloseApprover();
	TestWindowPost();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

