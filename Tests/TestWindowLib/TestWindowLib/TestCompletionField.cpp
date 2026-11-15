#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "StandardLib/Objects.h"
#include "StandardLib/ClassDefines.h"
#include "WindowLib/Window.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/JewelTheme.h"
#include "WindowLib/CompletionField.h"
#include "WindowLib/MenuContainer.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/AssertGeometric.h"
#include "TestLib/AssertFile.h"
#include "ScriptFramesTick.h"


Ptr<CFont>		GenerateMenuFont(Ptr<CWindow> pWindow);
SInt32Vec2		ComponentCentre(Ptr<CComponent> pComponent);
SInt32Vec2		MenuRowCentre(Ptr<CMenuContainer> pMenu, int32 iRow);
void			MoveTo(Ptr<CWindow> pWindow, SInt32Vec2 sPosition);
void			Click(Ptr<CWindow> pWindow);


#define AssertLog(e)	AssertString(e, szLog.Text()); szLog.Clear()


//////////////////////////////////////////////////////////////////////////
//
// Logs "Changed:text;" and completes the field's text to the files that
// have it in their names.
//
//////////////////////////////////////////////////////////////////////////
class CTestCompletionListener
{
public:
	CChars*		mpszLog;

	void Init(CChars* pszLog)
	{
		mpszLog = pszLog;
	}

	void TextChanged(CTextField* pcField)
	{
		static const char*	aszFiles[] = { "Pump.h", "Pipe.h", "Tank.h" };
		static const char*	aszPaths[] = { "Source", "Source/Plumbing", "" };
		CArrayChars			aszTexts;
		CArrayChars			aszNotes;
		CChars				szFile;
		size				i;

		mpszLog->Append("Changed:");
		mpszLog->Append(pcField->GetText());
		mpszLog->Append(";");

		aszTexts.Init();
		aszNotes.Init();
		for (i = 0; (pcField->GetText()[0] != '\0') && (i < 3); i++)
		{
			szFile.Init(aszFiles[i]);
			if (szFile.ContainsIgnoreCase(pcField->GetText()))
			{
				aszTexts.Add(aszFiles[i]);
				aszNotes.Add(aszPaths[i]);
			}
			szFile.Kill();
		}
		((CCompletionField*)pcField)->SetCompletions(&aszTexts, &aszNotes);
		aszNotes.Kill();
		aszTexts.Kill();
	}
};


//////////////////////////////////////////////////////////////////////////
//
// A completion field at the top left of a dark window.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CCompletionField> CreateTestCompletionField(Ptr<CWindow> pWindow)
{
	Ptr<CFillContainer>		pFill;
	Ptr<CCompletionField>	pField;
	SInsets					sInsets;
	int						aiInsets[4] = { 10, 10, 10, 10 };

	pFill = OMalloc<CFillContainer>(pWindow);
	sInsets.Init(aiInsets);
	pFill->SetInsets(&sInsets);
	pWindow->SetContainer(pFill);
	pField = OMalloc<CCompletionField>(pWindow, CJewelTheme::CreateDarkTheme(pWindow, GenerateMenuFont(pWindow)), "");
	pField->SetAlignment(CSH_Left, CSV_Top);
	pFill->AddComponent(pField);
	return pField;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCompletionFieldCompleting(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CCompletionField>	pField;
		Ptr<CMenuContainer>		pMenu;
		Ptr<CFocus>				pFocus;
		Ptr<CPopup>				pPopup;
		CTestCompletionListener	cListener;
		CPointer				pNull;
		CChars					szLog;
		SInt32Vec2				sVec;

		szLog.Init();
		cListener.Init(&szLog);
		cNativeFactory.Init(&gcMemoryAllocator, 300, 200, NULL);
		pTestWindow = OMalloc<CWindow>("Completion Field Window", &cNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFocus = pTestWindow->GetFocus();
		pField = CreateTestCompletionField(pTestWindow);
		pMenu = pField->GetMenu();
		pField->GetChangeEvent()->Add(&cListener, &CTestCompletionListener::TextChanged);
		pTestWindow->Layout();

		//Nothing is offered until something is typed.
		MoveTo(pTestWindow, ComponentCentre(pField));
		Click(pTestWindow);
		AssertTrue(pFocus->GetFocussedComponent() == pField);
		AssertFalse(pField->IsCompleting());
		AssertFalse(pField->WantsEnter());

		//Typing opens the list under the field, as wide as it; the keys stay with the field and no row is chosen.
		pTestWindow->InputPrintable('P', true);
		AssertLog("Changed:P;");
		AssertTrue(pField->IsCompleting());
		AssertSize(2, pField->NumCompletions());
		AssertString("Pump.h", pField->GetCompletion(0));
		AssertString("Source", pField->GetCompletionNote(0));
		AssertString("Pipe.h", pField->GetCompletion(1));
		AssertString("Source/Plumbing", pField->GetCompletionNote(1));
		AssertTrue(pFocus->GetFocussedComponent() == pField);
		pPopup = pTestWindow->GetPopup(pMenu);
		sVec = pPopup->GetPosition();
		AssertInt32Vec2(10, 10 + pField->GetActualSize().y, &sVec);
		AssertInt(pField->GetActualSize().x, pPopup->GetActualSize().x);
		pPopup = NULL;
		AssertInt(-1, pMenu->GetSelected());
		AssertFalse(pField->WantsEnter());

		//Typing on narrows it.
		pTestWindow->InputPrintable('i', true);
		AssertLog("Changed:Pi;");
		AssertTrue(pField->IsCompleting());
		AssertSize(1, pField->NumCompletions());
		AssertString("Pipe.h", pField->GetCompletion(0));
		pTestWindow->InputCommand(CIC_Backspace, true);
		AssertLog("Changed:P;");
		AssertSize(2, pField->NumCompletions());

		//Down and up choose a row, and then enter is the field's: it takes the row, which isn't completed again.
		pTestWindow->InputCommand(CIC_Down, true);
		AssertInt(0, pMenu->GetSelected());
		AssertTrue(pField->WantsEnter());
		pTestWindow->InputCommand(CIC_Down, true);
		AssertInt(1, pMenu->GetSelected());
		pTestWindow->InputCommand(CIC_Up, true);
		AssertInt(0, pMenu->GetSelected());
		AssertString("P", pField->GetText());
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertString("Pump.h", pField->GetText());
		AssertSize(6, pField->GetCaretPosition());
		AssertFalse(pField->IsCompleting());
		AssertFalse(pField->WantsEnter());
		AssertTrue(pFocus->GetFocussedComponent() == pField);
		AssertLog("Changed:Pump.h;");

		//Escape closes the list and leaves what was typed.
		pField->SelectAll();
		pTestWindow->InputPrintable('a', true);
		AssertLog("Changed:a;");
		AssertTrue(pField->IsCompleting());
		AssertSize(1, pField->NumCompletions());
		AssertString("Tank.h", pField->GetCompletion(0));
		AssertString("", pField->GetCompletionNote(0));
		pTestWindow->InputCommand(CIC_ClearSelection, true);
		AssertFalse(pField->IsCompleting());
		AssertString("a", pField->GetText());
		AssertTrue(pFocus->GetFocussedComponent() == pField);

		//A row can be pressed.
		pField->SelectAll();
		pTestWindow->InputPrintable('p', true);
		AssertLog("Changed:p;");
		AssertTrue(pField->IsCompleting());
		MoveTo(pTestWindow, MenuRowCentre(pMenu, 1));
		Click(pTestWindow);
		AssertString("Pipe.h", pField->GetText());
		AssertFalse(pField->IsCompleting());
		AssertTrue(pFocus->GetFocussedComponent() == pField);
		AssertLog("Changed:Pipe.h;");

		//Nothing to offer closes it; and one can be taken by its index while there are some.
		pTestWindow->InputPrintable('z', true);
		AssertLog("Changed:Pipe.hz;");
		AssertFalse(pField->IsCompleting());
		AssertSize(0, pField->NumCompletions());
		AssertFalse(pField->TakeCompletion(0));
		pField->SelectAll();
		pTestWindow->InputPrintable('T', true);
		AssertTrue(pField->IsCompleting());
		AssertFalse(pField->TakeCompletion(1));
		AssertTrue(pField->TakeCompletion(0));
		AssertString("Tank.h", pField->GetText());
		AssertFalse(pField->IsCompleting());
		AssertLog("Changed:T;Changed:Tank.h;");

		//The window is freed with the list open: the field, its list and the popup go together.
		pField->SelectAll();
		pTestWindow->InputPrintable('P', true);
		AssertTrue(pField->IsCompleting());

		AssertTrue(pField->GetChangeEvent()->Remove(&cListener));
		pMenu = NULL;
		pField = NULL;
		pFocus = NULL;
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
void TestCompletionField(void)
{
	BeginTests();
	ObjectsInit();

	TestCompletionFieldCompleting();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

