#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/StringHelper.h"
#include "BaseLib/TextFile.h"
#ifdef _WIN32
#include "BaseLib/WindowsHeaders.h"
#undef CreateFile  //Windows makes it CreateFileA, which CJewelImageGenerator::CreateFile isn't.
#endif
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WindowLib/JewelTheme.h"
#include "WindowLib/JewelColours.h"
#include "WindowLib/JewelImageGenerator.h"
#include "WindowLib/FileChooser.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"


Ptr<CFont>		GenerateMenuFont(Ptr<CWindow> pWindow);


static SFileChooserType gasTestFileChooserTypes[] =
{
	{ "Text Files (*.txt)", "txt" },
	{ "All Files", NULL },
};


//////////////////////////////////////////////////////////////////////////
//
// The chooser's parts, to drive it without showing it.
//
//////////////////////////////////////////////////////////////////////////
class CTestFileChooser : public CFileChooser
{
CONSTRUCTABLE(CTestFileChooser);
DESTRUCTABLE(CTestFileChooser);
public:
	char*				GetDirectory(void) { return mszDirectory.Text(); }
	Ptr<CListContainer>	GetList(void) { return mpList; }
	Ptr<CTextField>		GetFileName(void) { return mpFileName; }
	Ptr<CComboBox>		GetFileType(void) { return mpFileType; }
	Ptr<CComboBox>		GetLookIn(void) { return mpLookIn; }
	Ptr<CButton>		GetUp(void) { return mpUp; }
	Ptr<CWindow>		GetDialogWindow(void) { return mpDialog->GetWindow(); }
	Ptr<CButton>		GetNewFolder(void) { return mpNewFolder; }
	bool				IsNewFolderPending(void) { return mbNewFolderPending; }
	bool				IsReplacePending(void) { return mbReplacePending; }
	bool				IsDeletePending(void) { return mbDeletePending; }
	void				ClearDeletePending(void) { mbDeletePending = false; }
	Ptr<CButton>		GetShowHidden(void) { return mpShowHidden; }
	bool				OpenFolder(const char* szDirectory) { return Open(szDirectory, NULL); }
	size				NumLookIn(void) { return maszLookIn.NumElements(); }
	char*				GetLookInPath(size uiIndex) { return maszLookIn.GetText(uiIndex); }
	char*				GetRowPath(int32 iRow) { return maszRows.GetText(iRow); }
	char*				GetClicked(void) { return mpDialog->GetClicked(); }
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void WriteTestFile(const char* szDirectory, const char* szName)
{
	CFileUtil	cFileUtil;
	CTextFile	cFile;
	CChars		szPath;

	szPath.Init(szDirectory);
	cFileUtil.AppendToPath(&szPath, szName);
	cFile.Init(szName);
	cFile.Write(szPath.Text());
	cFile.Kill();
	szPath.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether szPath is the whole path of szName in szDirectory.
//
//////////////////////////////////////////////////////////////////////////
static bool IsPathIn(const char* szPath, const char* szDirectory, const char* szName);


//////////////////////////////////////////////////////////////////////////
//
// The row of szName in szDirectory, -1 if it isn't listed.
//
//////////////////////////////////////////////////////////////////////////
static int32 FindTestRow(CTestFileChooser* pcChooser, const char* szDirectory, const char* szName)
{
	int32	i;

	for (i = 0; i < pcChooser->GetList()->NumRows(); i++)
	{
		if (IsPathIn(pcChooser->GetRowPath(i), szDirectory, szName))
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// A file that's hidden: its name starts with a dot, and on Windows it has
// the hidden attribute.
//
//////////////////////////////////////////////////////////////////////////
static void WriteHiddenTestFile(const char* szDirectory, const char* szName)
{
	CFileUtil	cFileUtil;
	CChars		szPath;

	WriteTestFile(szDirectory, szName);
#ifdef _WIN32
	szPath.Init(szDirectory);
	cFileUtil.AppendToPath(&szPath, szName);
	SetFileAttributesA(szPath.Text(), FILE_ATTRIBUTE_HIDDEN);
	szPath.Kill();
#endif
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool IsPathIn(const char* szPath, const char* szDirectory, const char* szName)
{
	CFileUtil	cFileUtil;
	CChars		szExpected;
	bool		bResult;

	szExpected.Init(szDirectory);
	cFileUtil.AppendToPath(&szExpected, szName);
	cFileUtil.FullPath(&szExpected);
	bResult = szExpected.Equals(szPath);
	szExpected.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A folder holding a folder, two text files and an XML file.  The chooser
// starts on one of the text files.
//
//////////////////////////////////////////////////////////////////////////
void TestFileChooserBrowse(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "FileChooser";
	char					szSub[] = "Output" _FS_ "FileChooser" _FS_ "Sub";
	char					szStart[] = "Output" _FS_ "FileChooser" _FS_ "b.txt";
	CChars					szFull;
	CChars					szFullSub;

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);
	cFileUtil.MakeDir(szSub);
	WriteTestFile(szDirectory, "a.txt");
	WriteTestFile(szDirectory, "b.txt");
	WriteTestFile(szDirectory, "c.xml");
	szFull.Init(szDirectory);
	cFileUtil.FullPath(&szFull);
	szFullSub.Init(szSub);
	cFileUtil.FullPath(&szFullSub);

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		SDialogLook					sLook;
		Ptr<CWindowTheme>			pTheme;  //The look only points to it.
		Ptr<CTestFileChooser>		pChooser;
		CChars						szDesktop;
		int32						iRow;
		CArrayChars					aszRoots;
		CChars						szRoot;
		size						uiRoot;
		size						uiLookIn;
		bool						bFound;
		Ptr<CFocus>					pFocus;
		CColourFormatHelper			cFormat;
		Ptr<CImageCel>				pFolderIcon;
		Ptr<CImageCel>				pFileIcon;

		cNativeFactory.Init(&gcMemoryAllocator, 300, 200, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("File Chooser Owner", &cNativeFactory, pTick, OMalloc<CClearCanvasDraw>(JEWEL_EDITOR), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pTheme = CJewelTheme::CreateDarkTheme(pTestWindow, GenerateMenuFont(pTestWindow));
		sLook.pcTheme = &pTheme;
		sLook.pcDimFont = &pTheme->GetFont();
		pTestWindow->GetColourFormat(&cFormat);
		pFolderIcon = CJewelImageGenerator::CreateFolder(JEWEL_ICON, &cFormat);
		pFileIcon = CJewelImageGenerator::CreateFile(JEWEL_ICON, &cFormat);

		//The folder first then the text files, the XML file filtered out.  The file started on is selected and named.
		pChooser = OMalloc<CTestFileChooser>(pTestWindow, &sLook, "Open File", false, szStart, gasTestFileChooserTypes, 2, pFolderIcon, pFileIcon);
		ORoot()->Add(pChooser);
		AssertString(szFull.Text(), pChooser->GetDirectory());
		AssertInt(3, pChooser->GetList()->NumRows());
		AssertTrue(IsPathIn(pChooser->GetRowPath(0), szDirectory, "Sub"));
		AssertTrue(IsPathIn(pChooser->GetRowPath(1), szDirectory, "a.txt"));
		AssertTrue(IsPathIn(pChooser->GetRowPath(2), szDirectory, "b.txt"));
		AssertInt(2, pChooser->GetList()->GetSelected());
		AssertString("b.txt", pChooser->GetFileName()->GetText());
		AssertString(szFull.Text(), pChooser->GetLookIn()->GetSelectedText());

		//Selecting a file names it; Open chooses it.
		pChooser->GetList()->Select(1);
		pChooser->SelectionChanged();
		AssertString("a.txt", pChooser->GetFileName()->GetText());
		AssertTrue(pChooser->Approve());
		AssertTrue(IsPathIn(pChooser->GetChosen(), szDirectory, "a.txt"));

		//A file that doesn't exist can't be opened.
		pChooser->GetFileName()->SetText("missing.txt");
		AssertFalse(pChooser->Approve());

		//Every file shows them all.
		pChooser->GetFileType()->Select(1);
		pChooser->FileTypeChosen();
		AssertInt(4, pChooser->GetList()->NumRows());
		AssertTrue(IsPathIn(pChooser->GetRowPath(3), szDirectory, "c.xml"));

		//A folder named in File Name is opened, not chosen.
		pChooser->GetFileName()->SetText("Sub");
		AssertFalse(pChooser->Approve());
		AssertString(szFullSub.Text(), pChooser->GetDirectory());
		AssertInt(0, pChooser->GetList()->NumRows());
		AssertString("", pChooser->GetFileName()->GetText());

		//Up goes back with the folder come from selected.
		pChooser->GetUp()->Click();
		AssertString(szFull.Text(), pChooser->GetDirectory());
		AssertInt(0, pChooser->GetList()->GetSelected());

		//Activating a file's row chooses it, as OK does.
		AssertString("", pChooser->GetClicked());
		pChooser->GetList()->Select(1);
		pChooser->SelectionChanged();
		pChooser->ItemActivated();
		AssertString("OK", pChooser->GetClicked());
		AssertTrue(IsPathIn(pChooser->GetChosen(), szDirectory, "a.txt"));

		//Activating a folder's row opens it.  The list is activated by a double click, not a click.
		AssertTrue(pChooser->GetList()->IsDoubleClickActivates());
		pChooser->GetList()->Select(0);
		pChooser->ItemActivated();
		AssertString(szFullSub.Text(), pChooser->GetDirectory());

		//Backspace goes up, as Up does, from whatever has focus: the list, or Up once it has been clicked.  File Name takes it.
		pFocus = pChooser->GetDialogWindow()->GetFocus();
		pFocus->SetFocus(pChooser->GetList());
		pFocus->Command(CIC_Backspace, false);
		AssertString(szFull.Text(), pChooser->GetDirectory());
		pChooser->GetList()->Select(0);
		pChooser->ItemActivated();
		pFocus->SetFocus(pChooser->GetUp());
		pFocus->Command(CIC_Backspace, false);
		AssertString(szFull.Text(), pChooser->GetDirectory());
		pChooser->GetList()->Select(0);
		pChooser->ItemActivated();
		pChooser->GetFileName()->SetText("ab");
		pFocus->SetFocus(pChooser->GetFileName());
		pFocus->Command(CIC_Backspace, false);
		AssertString(szFullSub.Text(), pChooser->GetDirectory());
		AssertString("a", pChooser->GetFileName()->GetText());
		pChooser->GetFileName()->SetText("");
		pFocus->SetFocus(pChooser->GetList());
		pFocus->Command(CIC_Backspace, false);
		AssertString(szFull.Text(), pChooser->GetDirectory());
		AssertInt(0, pChooser->GetList()->GetSelected());

		//Refresh reads the folder again: a file written since it was shown is listed, and what was selected stays so.
		AssertInt(4, pChooser->GetList()->NumRows());
		WriteTestFile(szDirectory, "d.txt");
		AssertInt(-1, FindTestRow(&pChooser, szDirectory, "d.txt"));
		pChooser->Refresh();
		AssertInt(5, pChooser->GetList()->NumRows());
		AssertInt(0, pChooser->GetList()->GetSelected());

		//Delete, from the list, asks first, on the next tick; deleting selects the row after or before.
		iRow = FindTestRow(&pChooser, szDirectory, "d.txt");
		AssertTrue(iRow > 0);
		pChooser->GetList()->Select(iRow);
		pFocus->Command(CIC_Delete, false);
		AssertTrue(pChooser->IsDeletePending());
		pChooser->ClearDeletePending();
		AssertTrue(pChooser->DeleteSelected());
		AssertInt(4, pChooser->GetList()->NumRows());
		AssertInt(-1, FindTestRow(&pChooser, szDirectory, "d.txt"));
		AssertTrue(pChooser->GetList()->GetSelected() >= 0);
		pChooser->GetList()->Select(-1);
		pFocus->Command(CIC_Delete, false);
		AssertFalse(pChooser->IsDeletePending());

		//Hidden files are shown while the toggle is on, which keeps it highlighted.
		WriteHiddenTestFile(szDirectory, ".hidden.txt");
		pChooser->Refresh();
		AssertInt(-1, FindTestRow(&pChooser, szDirectory, ".hidden.txt"));
		pChooser->ShowHiddenClicked();
		AssertTrue(pChooser->GetShowHidden()->IsHighlighted());
		AssertTrue(FindTestRow(&pChooser, szDirectory, ".hidden.txt") >= 0);
		pChooser->ShowHiddenClicked();
		AssertFalse(pChooser->GetShowHidden()->IsHighlighted());
		AssertInt(-1, FindTestRow(&pChooser, szDirectory, ".hidden.txt"));

		//Desktop opens the user's desktop folder, where there is one.
		szDesktop.Init();
		if (cFileUtil.DesktopDirectory(&szDesktop))
		{
			pChooser->GoDesktop();
			NormaliseDirectory(&szDesktop);
			AssertString(szDesktop.Text(), pChooser->GetDirectory());
			pChooser->OpenFolder(szFull.Text());
		}
		szDesktop.Kill();
		AssertString(szFull.Text(), pChooser->GetDirectory());

		//Look In has every drive's root, so another can be chosen, and the folder shown is selected.
		aszRoots.Init();
		cFileUtil.FindRootDirectories(&aszRoots);
		AssertTrue(pChooser->NumLookIn() > aszRoots.NumElements() || aszRoots.NumElements() == 0);
		for (uiRoot = 0; uiRoot < aszRoots.NumElements(); uiRoot++)
		{
			szRoot.Init(aszRoots.GetText(uiRoot));
			NormaliseDirectory(&szRoot);
			bFound = false;
			for (uiLookIn = 0; uiLookIn < pChooser->NumLookIn(); uiLookIn++)
			{
				bFound |= StringInsensitiveCompare(pChooser->GetLookInPath(uiLookIn), szRoot.Text()) == 0;
			}
			AssertTrue(bFound);
			szRoot.Kill();
		}
		aszRoots.Kill();
		AssertString(szFull.Text(), pChooser->GetLookInPath(pChooser->GetLookIn()->GetSelected()));
		pChooser->GetList()->Select(0);
		pChooser->ItemActivated();
		AssertString(szFullSub.Text(), pChooser->GetDirectory());

		//New Folder's dialog isn't shown from the click, which is input, but on the chooser's next tick.
		AssertFalse(pChooser->IsNewFolderPending());
		pChooser->GetNewFolder()->Click();
		AssertTrue(pChooser->IsNewFolderPending());
		ORoot()->Remove(pChooser);
		pChooser = NULL;

		//Saving adds the type's extension.
		pChooser = OMalloc<CTestFileChooser>(pTestWindow, &sLook, "Save File", true, szStart, gasTestFileChooserTypes, 2, pFolderIcon, pFileIcon);
		ORoot()->Add(pChooser);
		pChooser->GetFileName()->SetText("new");
		AssertTrue(pChooser->Approve());
		AssertTrue(IsPathIn(pChooser->GetChosen(), szDirectory, "new.txt"));
		pChooser->GetFileName()->SetText("other.txt");
		AssertTrue(pChooser->Approve());
		AssertTrue(IsPathIn(pChooser->GetChosen(), szDirectory, "other.txt"));
		AssertFalse(pChooser->IsReplacePending());

		//Saving over a file that exists asks first, on the chooser's next tick.
		pChooser->GetFileName()->SetText("a");
		AssertFalse(pChooser->Approve());
		AssertTrue(pChooser->IsReplacePending());
		AssertTrue(IsPathIn(pChooser->GetChosen(), szDirectory, "a.txt"));
		ORoot()->Remove(pChooser);
		pChooser = NULL;

		pFolderIcon = NULL;
		pFileIcon = NULL;
		sLook.Kill();
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
	szFullSub.Kill();
	szFull.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFileChooser(void)
{
	BeginTests();
	ObjectsInit();

	TestFileChooserBrowse();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

