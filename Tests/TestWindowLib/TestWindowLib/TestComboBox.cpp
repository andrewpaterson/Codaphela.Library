#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/JewelTheme.h"
#include "WindowLib/ComboBox.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/AssertGeometric.h"
#include "TestLib/AssertFile.h"
#include "ScriptFramesTick.h"


void			ValidateLabelObjects(void);
Ptr<CFont>		GenerateMenuFont(Ptr<CWindow> pWindow);
SInt32Vec2		ComponentCentre(Ptr<CComponent> pComponent);
SInt32Vec2		MenuRowCentre(Ptr<CMenuContainer> pMenu, int32 iRow);
void			MoveTo(Ptr<CWindow> pWindow, SInt32Vec2 sPosition);
void			Click(Ptr<CWindow> pWindow);


#define AssertLog(e)	AssertString(e, szLog.Text()); szLog.Clear()


//////////////////////////////////////////////////////////////////////////
//
// Logs "Chose:text;".
//
//////////////////////////////////////////////////////////////////////////
class CTestComboBoxListener : public CComboBoxListener
{
CONSTRUCTABLE(CTestComboBoxListener);
public:
	CChars*		mpszLog;

	void Init(CChars* pszLog)
	{
		mpszLog = pszLog;
	}

	void SelectionChanged(CUnknown* pcSource, void* pvContext) override
	{
		mpszLog->Append("Chose:");
		mpszLog->Append(((CComboBox*)pcSource)->GetSelectedText());
		mpszLog->Append(";");
	}
};


//////////////////////////////////////////////////////////////////////////
//
// A combo box of directions at the top left of the window.  Its list is a
// menu with a narrow icon column.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComboBox> CreateTestComboBox(Ptr<CWindow> pWindow)
{
	Ptr<CFillContainer>		pFill;
	Ptr<CComboBox>			pCombo;
	Ptr<CFont>				pFont;
	SInsets					sInsets;
	int						aiInsets[4] = { 10, 10, 10, 10 };

	pFill = OMalloc<CFillContainer>(pWindow);
	sInsets.Init(aiInsets);
	pFill->SetInsets(&sInsets);
	pWindow->SetContainer(pFill);

	pFont = GenerateMenuFont(pWindow);
	pCombo = OMalloc<CComboBox>(pWindow, CJewelTheme::CreateDarkTheme(pWindow, pFont));
	pCombo->AddItem("North");
	pCombo->AddItem("East");
	pCombo->AddItem("South");
	pCombo->AddItem("West");
	pCombo->SetAlignment(CSH_Left, CSV_Top);
	pFill->AddComponent(pCombo);
	return pCombo;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestComboBoxChoosing(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CComboBox>			pCombo;
		Ptr<CMenuContainer>		pMenu;
		Ptr<CFocus>				pFocus;
		Ptr<CPopup>				pPopup;
		CTestComboBoxListener	cListener;
		CPointer				pNull;
		CChars					szLog;
		SInt32Vec2				sVec;

		szLog.Init();
		cListener.Init(&szLog);
		cNativeFactory.Init(&gcMemoryAllocator, 300, 200, NULL);
		pTestWindow = OMalloc<CWindow>("Combo Box Window", &cNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFocus = pTestWindow->GetFocus();
		pCombo = CreateTestComboBox(pTestWindow);
		pMenu = pCombo->GetMenu();
		AssertTrue(pCombo->AddListener(&cListener));
		pTestWindow->Layout();

		//The first item is chosen.
		AssertSize(4, pCombo->NumItems());
		AssertInt(0, pCombo->GetSelected());
		AssertString("North", pCombo->GetSelectedText());
		AssertString("North", pCombo->GetTextLabel()->GetText());

		//Pressing it opens the list below it, as wide as it, with the chosen item selected.
		MoveTo(pTestWindow, ComponentCentre(pCombo));
		AssertInt(BS_MouseOver, pCombo->GetBorderState());
		Click(pTestWindow);
		AssertTrue(pCombo->IsOpen());
		AssertInt(BS_Focussed, pCombo->GetBorderState());
		AssertTrue(pFocus->GetFocussedComponent() == pMenu);
		pPopup = pTestWindow->GetPopup(pMenu);
		sVec = pPopup->GetPosition();
		AssertInt32Vec2(10, 10 + pCombo->GetActualSize().y, &sVec);
		AssertInt(pCombo->GetActualSize().x, pPopup->GetActualSize().x);
		AssertInt(0, pMenu->GetSelected());

		//Choosing an item closes the list, shows the item and gives the keys back.
		MoveTo(pTestWindow, MenuRowCentre(pMenu, 2));
		Click(pTestWindow);
		AssertFalse(pCombo->IsOpen());
		AssertInt(2, pCombo->GetSelected());
		AssertString("South", pCombo->GetTextLabel()->GetText());
		AssertTrue(pFocus->GetFocussedComponent() == pCombo);
		AssertLog("Chose:South;");

		//Up and down choose while it is closed.
		pTestWindow->InputCommand(CIC_Down, true);
		AssertString("West", pCombo->GetSelectedText());
		pTestWindow->InputCommand(CIC_Down, true);
		AssertString("West", pCombo->GetSelectedText());
		pTestWindow->InputCommand(CIC_Up, true);
		AssertString("South", pCombo->GetSelectedText());
		AssertLog("Chose:West;Chose:South;");

		//Space opens it and escape closes it again without choosing.
		pTestWindow->InputCommand(CIC_Space, true);
		AssertTrue(pCombo->IsOpen());
		pTestWindow->InputCommand(CIC_Down, true);
		pTestWindow->InputCommand(CIC_ClearSelection, true);
		AssertFalse(pCombo->IsOpen());
		AssertString("South", pCombo->GetSelectedText());
		AssertTrue(pFocus->GetFocussedComponent() == pCombo);
		AssertLog("");

		//Enter on the open list chooses the selected item.
		pTestWindow->InputCommand(CIC_Space, true);
		pTestWindow->InputCommand(CIC_Up, true);
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertFalse(pCombo->IsOpen());
		AssertString("East", pCombo->GetSelectedText());
		AssertLog("Chose:East;");

		//Pressing it again while open closes it.
		MoveTo(pTestWindow, ComponentCentre(pCombo));
		Click(pTestWindow);
		AssertTrue(pCombo->IsOpen());
		Click(pTestWindow);
		AssertFalse(pCombo->IsOpen());

		//Choosing the chosen item doesn't tell the listeners.
		Click(pTestWindow);
		MoveTo(pTestWindow, MenuRowCentre(pMenu, 1));
		Click(pTestWindow);
		AssertString("East", pCombo->GetSelectedText());
		AssertLog("");

		//The window is freed with the list open, the combo box, its list and the popup go together.
		MoveTo(pTestWindow, ComponentCentre(pCombo));
		Click(pTestWindow);
		AssertTrue(pCombo->IsOpen());

		ValidateLabelObjects();

		pCombo->RemoveListener(&cListener);
		pPopup = NULL;
		pMenu = NULL;
		pCombo = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		szLog.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	AssertSize(2, gcObjects.NumMemoryIndexes());
	cNativeFactory.Kill();
}


struct SComboBoxFrames
{
	Ptr<CComboBox>	pCombo;
};


//////////////////////////////////////////////////////////////////////////
//
// Frame 0: closed.  1: the pointer over it.  2: open with the pointer over
// the third item.
//
//////////////////////////////////////////////////////////////////////////
bool ComboBoxFrames(Ptr<CWindow> pWindow, int32 iFrame, void* pvContext)
{
	Ptr<CComboBox>	pCombo;

	pCombo = ((SComboBoxFrames*)pvContext)->pCombo;
	if (iFrame == 0)
	{
		MoveTo(pWindow, ComponentCentre(pCombo));
		return true;
	}
	else if (iFrame == 1)
	{
		Click(pWindow);
		MoveTo(pWindow, MenuRowCentre(pCombo->GetMenu(), 2));
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestComboBoxFrames(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "ComboBoxFrames";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CScriptFramesTick>		pTick;
		SComboBoxFrames				sFrames;
		CChars						szExpectedDirectory;

		cNativeFactory.Init(&gcMemoryAllocator, 160, 190, szDirectory);
		pTick = OMalloc<CScriptFramesTick>(ComboBoxFrames, &sFrames);
		pTestWindow = OMalloc<CWindow>("Combo Box Frames", &cNativeFactory, pTick, OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)0x2B, 0x2D, 0x30)), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		sFrames.pCombo = CreateTestComboBox(pTestWindow);
		sFrames.pCombo->SetDesiredSize(120, -1);

		pTestWindow->Show();

		ValidateLabelObjects();

		sFrames.pCombo = NULL;
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
void TestComboBox(void)
{
	BeginTests();
	ObjectsInit();

	TestComboBoxChoosing();
	TestComboBoxFrames();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

