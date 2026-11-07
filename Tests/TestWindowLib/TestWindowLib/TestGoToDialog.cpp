#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WindowLib/JewelTheme.h"
#include "WindowLib/JewelColours.h"
#include "WindowLib/GoToDialog.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"


Ptr<CFont>		GenerateMenuFont(Ptr<CWindow> pWindow);


static const char*	gaszTestGoToClasses[] = { "CTextArea", "CTextField", "CTabbedContainer", "CListContainer", "CGoToDialog" };
static const char*	gaszTestGoToFiles[] = { "/Code/WindowLib/TextArea.cpp", "/Code/WindowLib/TextField.h", "/Code/BaseLib/TextFile.cpp" };


//////////////////////////////////////////////////////////////////////////
//
// Classes in tab 0, files in tab 1.
//
//////////////////////////////////////////////////////////////////////////
class CTestGoToSearcher : public CGoToSearcher
{
public:
	size	muiSearches;

	void GoToSearch(size uiTab, CNameMatcher* pcMatcher, CGoToItems* pcDest) override
	{
		SNameMatch	sMatch;
		size		i;

		muiSearches++;
		if (uiTab == 0)
		{
			for (i = 0; i < sizeof(gaszTestGoToClasses) / sizeof(char*); i++)
			{
				if (pcMatcher->Match(gaszTestGoToClasses[i], &sMatch))
				{
					pcDest->Add(gaszTestGoToClasses[i], "WindowLib", 0, i, &sMatch);
				}
			}
		}
		else
		{
			for (i = 0; i < sizeof(gaszTestGoToFiles) / sizeof(char*); i++)
			{
				if (pcMatcher->MatchPath(gaszTestGoToFiles[i], &sMatch))
				{
					pcDest->Add(strrchr(gaszTestGoToFiles[i], '/') + 1, gaszTestGoToFiles[i], 1, i, &sMatch);
				}
			}
		}
	}
};


//////////////////////////////////////////////////////////////////////////
//
// The results' names as "a,b,c".
//
//////////////////////////////////////////////////////////////////////////
static char* PrintGoToResults(Ptr<CGoToDialog> pDialog, CChars* pszDest)
{
	size	i;

	pszDest->Clear();
	for (i = 0; i < pDialog->NumResults(); i++)
	{
		if (i > 0)
		{
			pszDest->Append(",");
		}
		pszDest->Append(pDialog->GetResult(i)->szName.Text());
	}
	return pszDest->Text();
}


//////////////////////////////////////////////////////////////////////////
//
// Typing finds and orders the results, the keys move through them and
// the tabs, and enter chooses, without the dialog being shown.
//
//////////////////////////////////////////////////////////////////////////
void TestGoToDialogSearch(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		SDialogLook					sLook;
		Ptr<CWindowTheme>			pTheme;
		Ptr<CGoToDialog>			pDialog;
		Ptr<CWindow>				pDialogWindow;
		CTestGoToSearcher			cSearcher;
		CChars						szResults;
		size						i;

		szResults.Init();
		cSearcher.muiSearches = 0;
		cNativeFactory.Init(&gcMemoryAllocator, 300, 200, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Go To Owner", &cNativeFactory, pTick, OMalloc<CClearCanvasDraw>(JEWEL_EDITOR), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pTheme = CJewelTheme::CreateDarkTheme(pTestWindow, GenerateMenuFont(pTestWindow));
		sLook.pcTheme = &pTheme;
		sLook.pcDimFont = &pTheme->GetFont();
		sLook.sBackground = JEWEL_PANEL;
		sLook.sDivider = JEWEL_SEPARATOR;

		pDialog = OMalloc<CGoToDialog>(pTestWindow, &sLook, "Go to", &cSearcher, pTheme->GetFont());
		ORoot()->Add(pDialog);
		AssertSize(0, pDialog->AddTab("Classes"));
		AssertSize(1, pDialog->AddTab("Files"));
		pDialogWindow = pDialog->GetDialog()->GetWindow();

		//Started on Classes with what was typed last, selected; everything it matches is found, the best first.
		pDialog->Start(0, "ta");
		AssertSize(0, pDialog->GetTab());
		AssertString("CTabbedContainer,CTextArea", PrintGoToResults(pDialog, &szResults));
		AssertInt(0, pDialog->GetSelected());
		AssertInt(2, pDialog->GetList()->NumRows());

		//Typing replaces the selected text, and searches as it goes.
		pDialogWindow->GetFocus()->SetFocus(pDialog->GetField());
		pDialogWindow->InputPrintable('C', true);
		pDialogWindow->InputPrintable('T', true);
		AssertString("CT", pDialog->GetPattern());
		AssertString("CTextArea,CTextField,CTabbedContainer,CGoToDialog", PrintGoToResults(pDialog, &szResults));
		pDialogWindow->InputPrintable('F', true);
		AssertString("CTextField", PrintGoToResults(pDialog, &szResults));
		pDialogWindow->InputCommand(CIC_Backspace, true);
		pDialogWindow->InputCommand(CIC_Backspace, true);
		pDialogWindow->InputCommand(CIC_Backspace, true);
		AssertSize(5, pDialog->NumResults());

		//Down and up move through them, going round at the ends.
		pDialogWindow->InputCommand(CIC_Down, true);
		AssertInt(1, pDialog->GetSelected());
		pDialogWindow->InputCommand(CIC_Up, true);
		pDialogWindow->InputCommand(CIC_Up, true);
		AssertInt(4, pDialog->GetSelected());
		pDialogWindow->InputCommand(CIC_Down, true);
		AssertInt(0, pDialog->GetSelected());

		//Tab goes to Files, the text kept; shift tab back.
		pDialog->SetText("tf");
		pDialogWindow->InputCommand(CIC_Tab, true);
		AssertSize(1, pDialog->GetTab());
		AssertString("tf", pDialog->GetPattern());
		AssertString("TextField.h,TextFile.cpp", PrintGoToResults(pDialog, &szResults));
		pDialog->SetText("wl/t");
		AssertString("TextField.h,TextArea.cpp", PrintGoToResults(pDialog, &szResults));
		pDialogWindow->InputCommand(CIC_BackTab, true);
		AssertSize(0, pDialog->GetTab());

		//A line after the name.
		pDialog->SetText("CListContainer:42");
		AssertString("CListContainer", PrintGoToResults(pDialog, &szResults));
		AssertInt(42, pDialog->GetLine());
		pDialog->SetText("CListContainer:");
		AssertString("CListContainer", PrintGoToResults(pDialog, &szResults));
		AssertInt(-1, pDialog->GetLine());

		//Enter chooses the selected result.
		AssertTrue(pDialog->GetChosen() == NULL);
		pDialogWindow->InputCommand(CIC_Enter, true);
		AssertTrue(pDialog->GetChosen() != NULL);
		AssertString("CListContainer", pDialog->GetChosen()->szName.Text());
		AssertSize(3, pDialog->GetChosen()->uiData);

		//Nothing matched, nothing chosen.
		pDialog->Start(0, "zzz");
		AssertSize(0, pDialog->NumResults());
		AssertFalse(pDialog->Approve());
		AssertTrue(pDialog->GetChosen() == NULL);

		//A double click on a result chooses it, as OK does.
		pDialog->Start(0, "CListContainer");
		pDialog->ResultActivated();
		AssertTrue(pDialog->GetChosen() != NULL);
		AssertString("CListContainer", pDialog->GetChosen()->szName.Text());

		//Only the best are listed.
		pDialog->Start(0, "");
		for (i = 0; i < pDialog->NumResults(); i++)
		{
			AssertTrue(pDialog->GetResult(i)->sMatch.uiNumRanges == 0);
		}
		AssertInt((int32)pDialog->NumResults(), pDialog->GetList()->NumRows());

		ORoot()->Remove(pDialog);
		pDialog = NULL;
		pDialogWindow = NULL;
		sLook.Kill();
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		szResults.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGoToDialog(void)
{
	BeginTests();
	ObjectsInit();

	TestGoToDialogSearch();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}
