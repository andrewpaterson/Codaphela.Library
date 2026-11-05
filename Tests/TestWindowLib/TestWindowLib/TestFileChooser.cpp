#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TextFile.h"
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
	Ptr<CButton>		GetNewFolder(void) { return mpNewFolder; }
	bool				IsNewFolderPending(void) { return mbNewFolderPending; }
	char*				GetRowPath(int32 iRow) { return maszRows.GetText(iRow); }
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
		sLook.sBackground = JEWEL_PANEL;
		sLook.sDivider = JEWEL_SEPARATOR;
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
		pChooser->ComboChanged(&pChooser->GetFileType());
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

		//Activating a folder's row opens it.  The list is activated by a double click, not a click.
		AssertTrue(pChooser->GetList()->IsDoubleClickActivates());
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

