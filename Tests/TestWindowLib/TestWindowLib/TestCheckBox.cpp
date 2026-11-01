#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WindowLib/FlowContainer.h"
#include "WindowLib/JewelTheme.h"
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


#define AssertLog(e)	AssertString(e, szLog.Text()); szLog.Clear()


//////////////////////////////////////////////////////////////////////////
//
// Logs "Ticked:caption;" or "Unticked:caption;".
//
//////////////////////////////////////////////////////////////////////////
class CTestCheckBoxListener : public CCheckBoxListener
{
CONSTRUCTABLE(CTestCheckBoxListener);
public:
	CChars*		mpszLog;

	void Init(CChars* pszLog)
	{
		mpszLog = pszLog;
	}

	void CheckBoxChanged(CUnknown* pcSource, void* pvContext) override
	{
		CCheckBox*	pcCheckBox;

		pcCheckBox = (CCheckBox*)pcSource;
		mpszLog->Append(pcCheckBox->IsTicked() ? "Ticked:" : "Unticked:");
		mpszLog->Append(pcCheckBox->GetCaption());
		mpszLog->Append(";");
	}
};


//////////////////////////////////////////////////////////////////////////
//
// Two check boxes, one above the other, at the top left of the window.
//
//////////////////////////////////////////////////////////////////////////
struct STestCheckBoxes
{
	Ptr<CCheckBox>	pInverse;
	Ptr<CCheckBox>	pPower;

	void Kill(void)
	{
		pInverse = NULL;
		pPower = NULL;
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CreateTestCheckBoxes(Ptr<CWindow> pWindow, STestCheckBoxes* psBoxes)
{
	Ptr<CFlowContainer>		pFlow;
	Ptr<CWindowTheme>		pTheme;
	Ptr<CFont>				pFont;
	SInsets					sInsets;
	int						aiInsets[4] = { 10, 10, 10, 10 };

	pFlow = OMalloc<CFlowContainer>(pWindow);
	sInsets.Init(aiInsets);
	pFlow->SetInsets(&sInsets);
	pFlow->SetFlowStyle(CSD_Down, CSW_Continue, CSH_Left);
	pWindow->SetContainer(pFlow);
	pFont = GenerateMenuFont(pWindow);
	pTheme = CJewelTheme::CreateDarkTheme(pWindow, pFont);
	psBoxes->pInverse = OMalloc<CCheckBox>(pWindow, pTheme, "Inverse Out");
	psBoxes->pPower = OMalloc<CCheckBox>(pWindow, pTheme, "Explicit Power Ports", true);
	psBoxes->pInverse->SetAlignment(CSH_Left, CSV_Top);
	psBoxes->pPower->SetAlignment(CSH_Left, CSV_Top);
	pFlow->AddComponent(psBoxes->pInverse);
	pFlow->AddComponent(psBoxes->pPower);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCheckBoxClicks(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		STestCheckBoxes			sBoxes;
		Ptr<CFocus>				pFocus;
		CTestCheckBoxListener	cListener;
		CPointer				pNull;
		CChars					szLog;
		SInt32Vec2				sVec;

		szLog.Init();
		cListener.Init(&szLog);
		cNativeFactory.Init(&gcMemoryAllocator, 300, 100, NULL);
		pTestWindow = OMalloc<CWindow>("Check Box Window", &cNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFocus = pTestWindow->GetFocus();
		CreateTestCheckBoxes(pTestWindow, &sBoxes);
		AssertTrue(sBoxes.pInverse->AddListener(&cListener));
		AssertTrue(sBoxes.pPower->AddListener(&cListener));
		pTestWindow->Layout();

		//The box is 20 wide, then the gap, then the caption.
		AssertFalse(sBoxes.pInverse->IsTicked());
		AssertTrue(sBoxes.pPower->IsTicked());
		sVec = sBoxes.pInverse->GetBoxComponent()->GetActualSize();
		AssertInt(26, sVec.x);
		AssertInt(10 + 26, sBoxes.pInverse->GetCaptionLabel()->GetPosition().x);

		//The pointer over it lightens it, pressing gives it focus and its ring, and releasing ticks it.
		MoveTo(pTestWindow, ComponentCentre(sBoxes.pInverse));
		AssertInt(CBS_MouseOver, sBoxes.pInverse->GetState());
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		AssertTrue(pFocus->GetFocussedComponent() == sBoxes.pInverse);
		AssertInt(CBS_FocussedMouseOver, sBoxes.pInverse->GetState());
		AssertLog("");
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertTrue(sBoxes.pInverse->IsTicked());
		AssertLog("Ticked:Inverse Out;");

		//Releasing away from it doesn't untick it.
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		MoveTo(pTestWindow, SInt32Vec2(280, 90));
		AssertInt(CBS_Focussed, sBoxes.pInverse->GetState());
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertTrue(sBoxes.pInverse->IsTicked());
		AssertLog("");

		//Space unticks it.
		pTestWindow->InputCommand(CIC_Space, true);
		AssertFalse(sBoxes.pInverse->IsTicked());
		AssertLog("Unticked:Inverse Out;");

		//Clicking the caption unticks the other.
		MoveTo(pTestWindow, ComponentCentre(sBoxes.pPower->GetCaptionLabel()));
		Click(pTestWindow);
		AssertFalse(sBoxes.pPower->IsTicked());
		AssertTrue(pFocus->GetFocussedComponent() == sBoxes.pPower);
		AssertInt(CBS_Normal, sBoxes.pInverse->GetState());
		AssertLog("Unticked:Explicit Power Ports;");

		//SetTicked doesn't tell the listeners.
		sBoxes.pPower->SetTicked(true);
		AssertTrue(sBoxes.pPower->IsTicked());
		AssertLog("");

		ValidateLabelObjects();

		sBoxes.pInverse->RemoveListener(&cListener);
		sBoxes.pPower->RemoveListener(&cListener);
		sBoxes.Kill();
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
// Frame 0: unticked and ticked.  1: the pointer over the first.  2: the
// first pressed, focussed and ticked.  3: the pointer moved away, the
// first still focussed.
//
//////////////////////////////////////////////////////////////////////////
bool CheckBoxFrames(Ptr<CWindow> pWindow, int32 iFrame, void* pvContext)
{
	STestCheckBoxes*	psBoxes;

	psBoxes = (STestCheckBoxes*)pvContext;
	if (iFrame == 0)
	{
		MoveTo(pWindow, ComponentCentre(psBoxes->pInverse));
		return true;
	}
	else if (iFrame == 1)
	{
		Click(pWindow);
		return true;
	}
	else if (iFrame == 2)
	{
		MoveTo(pWindow, SInt32Vec2(220, 70));
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCheckBoxFrames(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "CheckBoxFrames";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CScriptFramesTick>		pTick;
		STestCheckBoxes				sBoxes;
		CChars						szExpectedDirectory;

		cNativeFactory.Init(&gcMemoryAllocator, 230, 76, szDirectory);
		pTick = OMalloc<CScriptFramesTick>(CheckBoxFrames, &sBoxes);
		pTestWindow = OMalloc<CWindow>("Check Box Frames", &cNativeFactory, pTick, OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)0x2B, 0x2D, 0x30)), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		CreateTestCheckBoxes(pTestWindow, &sBoxes);

		pTestWindow->Show();

		ValidateLabelObjects();

		sBoxes.Kill();
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
void TestCheckBox(void)
{
	BeginTests();
	ObjectsInit();

	TestCheckBoxClicks();
	TestCheckBoxFrames();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

