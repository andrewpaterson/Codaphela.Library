#include "BaseLib/GlobalMemory.h"
#include "BaseLib/TextSearch.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WindowLib/JewelTheme.h"
#include "WindowLib/JewelColours.h"
#include "WindowLib/FindInFilesDialog.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"


Ptr<CFont>			GenerateMenuFont(Ptr<CWindow> pWindow);
Ptr<CFontFamily>	CreateMonoTextAreaFamily(float32 fDefaultPixelSize);


static const char*	gaszTestFindFiles[] = { "/Code/Alpha.cpp", "/Code/Beta.cpp", "/Other/Gamma.cpp" };
static const char*	gaszTestFindTexts[] =
{
	"int Alpha(void)\n{\n\treturn alphabet + 1;\n}\n",
	"int Beta(void)\n{\n\treturn Alpha() + ALPHA;\n}\n",
	"//alpha\n",
};


//////////////////////////////////////////////////////////////////////////
//
// Scope 0 is every file, scope 1 those in the directory typed.  A step
// searches one file.  With muiRepeat each match is found that many times.
//
//////////////////////////////////////////////////////////////////////////
class CTestFindSearcher : public CFindInFilesSearcher
{
public:
	CTextSearch	mcSearch;
	CChars		mszDirectory;
	CChars		mszFileMask;
	CChars		mszReplace;
	bool		mbReplace;
	size		muiScope;
	size		muiFile;
	size		muiStarts;
	size		muiSteps;
	size		muiTexts;
	size		muiShown;
	size		muiRepeat;
	bool		mbStarted;

	void Init(void)
	{
		mszDirectory.Init();
		mszFileMask.Init();
		mszReplace.Init();
		mbReplace = false;
		muiScope = 0;
		muiFile = 0;
		muiStarts = 0;
		muiSteps = 0;
		muiTexts = 0;
		muiShown = 0;
		muiRepeat = 1;
		mbStarted = false;
	}

	void Kill(void)
	{
		if (mbStarted)
		{
			mcSearch.Kill();
		}
		mszReplace.Kill();
		mszFileMask.Kill();
		mszDirectory.Kill();
	}

	bool FindChooseDirectory(const char* szDirectory, CChars* pszDest) override
	{
		//From /Code it chooses /Other; from anywhere else, nothing.
		if (strcmp(szDirectory, "/Code") != 0)
		{
			return false;
		}
		pszDest->Append("/Other");
		return true;
	}

	bool FindProblem(CChars* pszDest) override
	{
		if (mbStarted && !mcSearch.IsValid())
		{
			pszDest->Append("Not a regular expression: ");
			pszDest->Append(mcSearch.GetError());
			return true;
		}
		return false;
	}

	void FindStart(SFindInFilesQuery* psQuery) override
	{
		if (mbStarted)
		{
			mcSearch.Kill();
		}
		mcSearch.Init(psQuery->szText, psQuery->bMatchCase, psQuery->bWholeWords, psQuery->bRegex);
		mbStarted = true;
		mszDirectory.Set(psQuery->szDirectory);
		mszFileMask.Set(psQuery->szFileMask);
		mbReplace = psQuery->szReplace != NULL;
		mszReplace.Set(mbReplace ? psQuery->szReplace : "");
		muiScope = psQuery->uiScope;
		muiFile = 0;
		muiStarts++;
	}

	bool FindStep(CFindInFilesItems* pcDest) override
	{
		const char*	szText;
		const char*	szLine;
		const char*	szEnd;
		CChars		szShown;
		CChars		szLocation;
		size		uiLength;
		size		uiPosition;
		size		uiLine;
		size		i;

		muiSteps++;
		if (muiFile >= 3)
		{
			return false;
		}
		szText = gaszTestFindTexts[muiFile];
		uiLength = strlen(szText);
		if ((muiScope == 0) || (strncmp(gaszTestFindFiles[muiFile], mszDirectory.Text(), mszDirectory.Length()) == 0))
		{
			uiPosition = mcSearch.Find(szText, uiLength);
			while (uiPosition != TEXT_SEARCH_NOT_FOUND)
			{
				uiLine = 0;
				szLine = szText;
				for (i = 0; i < uiPosition; i++)
				{
					if (szText[i] == '\n')
					{
						uiLine++;
						szLine = &szText[i + 1];
					}
				}
				while (*szLine == '\t')
				{
					szLine++;
				}
				szEnd = strchr(szLine, '\n');
				szShown.Init();
				szShown.AppendSubString(szLine, szEnd);
				szLocation.Init(strrchr(gaszTestFindFiles[muiFile], '/') + 1);
				szLocation.Append(" ");
				szLocation.Append((uint32)(uiLine + 1));
				for (i = 0; i < muiRepeat; i++)
				{
					pcDest->Add(szShown.Text(), uiPosition - (size)(szLine - szText), mcSearch.GetFoundLength(), szLocation.Text(), muiFile, uiPosition);
				}
				szLocation.Kill();
				szShown.Kill();
				uiPosition = mcSearch.Find(szText, uiLength, uiPosition + mcSearch.GetFoundLength());
			}
		}
		muiFile++;
		return muiFile < 3;
	}

	bool FindPreviewPlace(SFindInFilesItem* psItem, CChars* pszTitle, size* puiStart, size* puiLength) override
	{
		pszTitle->Append(gaszTestFindFiles[psItem->uiFile]);
		*puiStart = psItem->uiData;
		*puiLength = psItem->uiMatchLength;
		return true;
	}

	bool FindPreviewText(SFindInFilesItem* psItem, CChars* pszText) override
	{
		muiTexts++;
		pszText->Append(gaszTestFindTexts[psItem->uiFile]);
		return true;
	}

	void FindPreviewShown(SFindInFilesItem* psItem, Ptr<CTextArea> pPreview, bool bNewText) override
	{
		muiShown++;
	}
};


//////////////////////////////////////////////////////////////////////////
//
// The results as "line@location,...".
//
//////////////////////////////////////////////////////////////////////////
static char* PrintFindResults(Ptr<CFindInFilesDialog> pDialog, CChars* pszDest)
{
	size				i;
	SFindInFilesItem*	psItem;

	pszDest->Clear();
	for (i = 0; i < pDialog->NumResults(); i++)
	{
		psItem = pDialog->GetResult(i);
		if (i > 0)
		{
			pszDest->Append(",");
		}
		pszDest->AppendSubString(psItem->szText.Text(), psItem->uiMatchStart, psItem->uiMatchStart + psItem->uiMatchLength);
		pszDest->Append("@");
		pszDest->Append(psItem->szLocation.Text());
	}
	return pszDest->Text();
}


//////////////////////////////////////////////////////////////////////////
//
// Typing searches a step at a time, the options and scopes search again,
// the keys move through the results with the preview following, and
// enter chooses, without the dialog being shown.
//
//////////////////////////////////////////////////////////////////////////
void TestFindInFilesDialogSearch(void)
{
	Ptr<CWinRefWindowFactory>	pNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		SDialogLook					sLook;
		Ptr<CWindowTheme>			pTheme;
		Ptr<CFindInFilesDialog>		pDialog;
		Ptr<CWindow>				pDialogWindow;
		CTestFindSearcher			cSearcher;
		CChars						szResults;

		szResults.Init();
		cSearcher.Init();
		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 300, 200, (char*)NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Find Owner", &pNativeFactory, pTick, OMalloc<CClearCanvasDraw>(JEWEL_EDITOR), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pTheme = CJewelTheme::CreateDarkTheme(pTestWindow, GenerateMenuFont(pTestWindow));
		sLook.pcTheme = &pTheme;
		sLook.pcDimFont = &pTheme->GetFont();

		pDialog = OMalloc<CFindInFilesDialog>(pTestWindow, &sLook, "Find in Files", &cSearcher, pTheme->GetFont(), CreateMonoTextAreaFamily(15.0f));
		ORoot()->Add(pDialog);
		AssertSize(0, pDialog->AddScope("In Solution", "Every file"));
		AssertSize(1, pDialog->AddDirectoryScope("Directory", "/Code"));
		pDialogWindow = pDialog->GetDialog()->GetWindow();

		//Nothing typed, nothing searched.
		pDialog->Start(0, "");
		AssertFalse(pDialog->IsSearching());
		AssertSize(0, cSearcher.muiStarts);
		AssertString("", pDialog->GetStatus());
		AssertString("", pDialog->GetPreviewTitle());

		//Started with what was typed last: the first step finds the first file's, the first of them is selected and
		//previewed, and the rest are found in later steps.
		pDialog->Start(0, "alpha");
		AssertSize(1, cSearcher.muiStarts);
		AssertSize(1, cSearcher.muiSteps);
		AssertTrue(pDialog->IsSearching());
		AssertString("Alpha@Alpha.cpp 1,alpha@Alpha.cpp 3", PrintFindResults(pDialog, &szResults));
		AssertString("2 matches in 1 file, searching...", pDialog->GetStatus());
		AssertInt(0, pDialog->GetSelected());
		AssertString("/Code/Alpha.cpp", pDialog->GetPreviewTitle());
		AssertString(gaszTestFindTexts[0], pDialog->GetPreview()->GetTextArea()->GetText());
		AssertSize(1, cSearcher.muiTexts);
		AssertTrue(pDialog->Step());
		AssertFalse(pDialog->Step());
		AssertFalse(pDialog->Step());
		AssertSize(3, cSearcher.muiSteps);
		AssertString("Alpha@Alpha.cpp 1,alpha@Alpha.cpp 3,Alpha@Beta.cpp 3,ALPHA@Beta.cpp 3,alpha@Gamma.cpp 1", PrintFindResults(pDialog, &szResults));
		AssertString("5 matches in 3 files", pDialog->GetStatus());
		AssertInt(5, pDialog->GetList()->NumRows());
		AssertInt(0, pDialog->GetSelected());
		AssertFalse(pDialog->HasMore());

		//Down through the results: the preview's file is only read when it changes.
		pDialogWindow->GetFocus()->SetFocus(pDialog->GetField());
		pDialogWindow->InputCommand(CIC_Down, true);
		AssertInt(1, pDialog->GetSelected());
		AssertString("/Code/Alpha.cpp", pDialog->GetPreviewTitle());
		AssertSize(1, cSearcher.muiTexts);
		pDialogWindow->InputCommand(CIC_Down, true);
		AssertInt(2, pDialog->GetSelected());
		AssertString("/Code/Beta.cpp", pDialog->GetPreviewTitle());
		AssertString(gaszTestFindTexts[1], pDialog->GetPreview()->GetTextArea()->GetText());
		AssertSize(2, cSearcher.muiTexts);
		AssertSize(3, cSearcher.muiShown);
		pDialogWindow->InputCommand(CIC_Up, true);
		pDialogWindow->InputCommand(CIC_Up, true);
		pDialogWindow->InputCommand(CIC_Up, true);
		AssertInt(4, pDialog->GetSelected());
		AssertString("/Other/Gamma.cpp", pDialog->GetPreviewTitle());

		//Typing replaces the selected text and searches again as it goes.
		pDialogWindow->InputPrintable('B', true);
		pDialog->Finish();
		AssertString("B", pDialog->GetText());
		AssertString("b@Alpha.cpp 3,B@Beta.cpp 1", PrintFindResults(pDialog, &szResults));
		pDialogWindow->InputPrintable('e', true);
		pDialog->Finish();
		AssertString("be@Alpha.cpp 3,Be@Beta.cpp 1", PrintFindResults(pDialog, &szResults));
		AssertString("2 matches in 2 files", pDialog->GetStatus());

		//The case, and whole words.
		pDialog->GetMatchCaseBox()->Toggle();
		pDialog->Finish();
		AssertTrue(pDialog->IsMatchCase());
		AssertString("Be@Beta.cpp 1", PrintFindResults(pDialog, &szResults));
		AssertString("1 match in 1 file", pDialog->GetStatus());
		pDialog->SetText("Alpha");
		pDialog->GetWholeWordsBox()->Toggle();
		pDialog->Finish();
		AssertString("Alpha@Alpha.cpp 1,Alpha@Beta.cpp 3", PrintFindResults(pDialog, &szResults));
		pDialog->SetOptions(false, false);
		AssertFalse(pDialog->IsMatchCase());
		AssertFalse(pDialog->IsWholeWords());

		//Nothing found.
		pDialog->SetText("zzz");
		pDialog->Finish();
		AssertSize(0, pDialog->NumResults());
		AssertString("Nothing found", pDialog->GetStatus());
		AssertString("", pDialog->GetPreviewTitle());
		AssertString("", pDialog->GetPreview()->GetTextArea()->GetText());
		AssertFalse(pDialog->Approve());

		//The directory's scope searches for the text typed, in the directory typed.
		pDialog->SetText("alpha");
		pDialog->SelectScope(1);
		pDialog->Finish();
		AssertSize(1, pDialog->GetScope());
		AssertString("/Code", cSearcher.mszDirectory.Text());
		AssertString("Alpha@Alpha.cpp 1,alpha@Alpha.cpp 3,Alpha@Beta.cpp 3,ALPHA@Beta.cpp 3", PrintFindResults(pDialog, &szResults));
		AssertTrue(pDialogWindow->GetFocus()->GetFocussedComponent() == pDialog->GetField());
		//The button beside the directory has the searcher choose one, at the next tick, and it is searched.
		AssertTrue(pDialog->GetBrowseButton().IsNotNull());
		pDialog->GetBrowseButton()->Click();
		AssertString("/Code", pDialog->GetDirectory());
		pDialogWindow->RunPosted();
		AssertString("/Other", pDialog->GetDirectory());
		pDialog->Finish();
		AssertString("alpha@Gamma.cpp 1", PrintFindResults(pDialog, &szResults));
		pDialog->GetBrowseButton()->Click();
		pDialogWindow->RunPosted();
		AssertString("/Other", pDialog->GetDirectory());
		pDialog->SetDirectory("/Code");
		pDialog->SetDirectory("/Other");
		pDialog->Finish();
		AssertString("alpha@Gamma.cpp 1", PrintFindResults(pDialog, &szResults));
		pDialog->SelectScope(0);
		pDialog->Finish();
		AssertSize(5, pDialog->NumResults());

		//Enter chooses the selected result.
		AssertTrue(pDialog->GetChosen() == NULL);
		pDialogWindow->InputCommand(CIC_Down, true);
		pDialogWindow->InputCommand(CIC_Enter, true);
		AssertTrue(pDialog->GetChosen() != NULL);
		AssertString("Alpha.cpp 3", pDialog->GetChosen()->szLocation.Text());
		AssertSize(7, pDialog->GetChosen()->uiMatchStart);
		AssertString("return alphabet + 1;", pDialog->GetChosen()->szText.Text());

		//Only the first are listed, and the search stops once there are that many.
		cSearcher.muiRepeat = 60;
		pDialog->Start(0, "alpha");
		pDialog->Finish();
		AssertTrue(pDialog->HasMore());
		AssertFalse(pDialog->IsSearching());
		AssertInt(FIND_IN_FILES_DIALOG_ROWS, pDialog->GetList()->NumRows());
		AssertString("100+ matches in 1+ files", pDialog->GetStatus());
		cSearcher.muiRepeat = 1;

		//A regular expression: what it finds, and what is wrong with one that isn't.
		pDialog->Start(0, "al.ha\\w");
		pDialog->Finish();
		AssertSize(0, pDialog->NumResults());
		AssertFalse(pDialog->IsRegex());
		pDialog->GetRegexBox()->Toggle();
		pDialog->Finish();
		AssertTrue(pDialog->IsRegex());
		AssertString("alphab@Alpha.cpp 3", PrintFindResults(pDialog, &szResults));
		pDialog->SetText("al(");
		pDialog->Finish();
		AssertSize(0, pDialog->NumResults());
		AssertString("Not a regular expression: A ( has no ) after it.", pDialog->GetStatus());
		pDialog->SetOptions(false, false, true);
		AssertTrue(pDialog->IsRegex());
		pDialog->SetOptions(false, false);
		AssertFalse(pDialog->IsRegex());

		//The file mask is the searcher's to use: typing one searches again.
		pDialog->SetText("alpha");
		pDialog->Finish();
		cSearcher.muiStarts = 0;
		AssertString("", cSearcher.mszFileMask.Text());
		pDialog->GetFileMaskField()->SetText("*.h");
		pDialog->Finish();
		AssertSize(1, cSearcher.muiStarts);
		AssertString("*.h", cSearcher.mszFileMask.Text());
		AssertString("*.h", pDialog->GetFileMask());
		pDialog->SetFileMask("");
		AssertSize(1, cSearcher.muiStarts);

		//A dialog that only finds has nothing to replace with, and Replace All does nothing.
		AssertFalse(pDialog->IsReplacing());
		AssertFalse(cSearcher.mbReplace);
		pDialog->ReplaceAll();
		AssertInt(FIFR_Cancelled, pDialog->GetOutcome());

		//Open in Find Window closes it, unless nothing is typed.
		pDialog->Start(0, "");
		pDialog->OpenInFindWindow();
		AssertInt(FIFR_Cancelled, pDialog->GetOutcome());
		pDialog->Start(0, "alpha");
		pDialog->OpenInFindWindow();
		AssertInt(FIFR_FindWindow, pDialog->GetOutcome());

		ORoot()->Remove(pDialog);
		pDialog = NULL;

		//A dialog that replaces has what to replace with, which the searcher is told, and Replace All closes it.
		pDialog = OMalloc<CFindInFilesDialog>(pTestWindow, &sLook, "Replace in Files", &cSearcher, pTheme->GetFont(), CreateMonoTextAreaFamily(15.0f));
		ORoot()->Add(pDialog);
		pDialog->AddScope("In Project", "Every file");
		pDialog->SetReplacing("beta");
		AssertTrue(pDialog->IsReplacing());
		AssertString("beta", pDialog->GetReplaceText());
		pDialog->Start(0, "");
		pDialog->ReplaceAll();
		AssertInt(FIFR_Cancelled, pDialog->GetOutcome());
		pDialog->Start(0, "alpha");
		pDialog->Finish();
		AssertTrue(cSearcher.mbReplace);
		AssertString("beta", cSearcher.mszReplace.Text());
		AssertSize(5, pDialog->NumResults());
		pDialog->GetReplaceField()->SetText("gamma");
		AssertString("gamma", pDialog->GetReplaceText());
		pDialog->GetReplaceAllButton()->Click();
		AssertInt(FIFR_ReplaceAll, pDialog->GetOutcome());

		ORoot()->Remove(pDialog);
		pDialog = NULL;
		pDialogWindow = NULL;
		sLook.Kill();
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		cSearcher.Kill();
		szResults.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	pNativeFactory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFindInFilesDialog(void)
{
	BeginTests();
	ObjectsInit();

	TestFindInFilesDialogSearch();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

