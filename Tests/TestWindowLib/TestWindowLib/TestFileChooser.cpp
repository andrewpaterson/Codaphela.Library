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
public:
	char*				GetDirectory(void) { return mszDirectory.Text(); }
	Ptr<CListContainer>	GetList(void) { return mpList; }
	Ptr<CTextField>		GetFileName(void) { return mpFileName; }
	Ptr<CComboBox>		GetFileType(void) { return mpFileType; }
	Ptr<CComboBox>		GetLookIn(void) { return mpLookIn; }
	Ptr<CButton>		GetUp(void) { return mpUp; }
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
		CTestFileChooser			cChooser;
		CColourFormatHelper			cFormat;
		Ptr<CImageCel>				pFolderIcon;
		Ptr<CImageCel>				pFileIcon;

		cNativeFactory.Init(&gcMemoryAllocator, 300, 200, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("File Chooser Owner", &cNativeFactory, pTick, OMalloc<CClearCanvasDraw>(JEWEL_EDITOR), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		sLook.pTheme = CJewelTheme::CreateDarkTheme(pTestWindow, GenerateMenuFont(pTestWindow));
		sLook.pDimFont = sLook.pTheme->GetFont();
		sLook.sBackground = JEWEL_PANEL;
		sLook.sDivider = JEWEL_SEPARATOR;
		pTestWindow->GetColourFormat(&cFormat);
		pFolderIcon = CJewelImageGenerator::CreateFolder(JEWEL_ICON, &cFormat);
		pFileIcon = CJewelImageGenerator::CreateFile(JEWEL_ICON, &cFormat);

		//The folder first then the text files, the XML file filtered out.  The file started on is selected and named.
		cChooser.Init(pTestWindow, &sLook, "Open File", false, szStart, gasTestFileChooserTypes, 2, pFolderIcon, pFileIcon);
		AssertString(szFull.Text(), cChooser.GetDirectory());
		AssertInt(3, cChooser.GetList()->NumRows());
		AssertTrue(IsPathIn(cChooser.GetRowPath(0), szDirectory, "Sub"));
		AssertTrue(IsPathIn(cChooser.GetRowPath(1), szDirectory, "a.txt"));
		AssertTrue(IsPathIn(cChooser.GetRowPath(2), szDirectory, "b.txt"));
		AssertInt(2, cChooser.GetList()->GetSelected());
		AssertString("b.txt", cChooser.GetFileName()->GetText());
		AssertString(szFull.Text(), cChooser.GetLookIn()->GetSelectedText());

		//Selecting a file names it; Open chooses it.
		cChooser.GetList()->Select(1);
		cChooser.SelectionChanged();
		AssertString("a.txt", cChooser.GetFileName()->GetText());
		AssertTrue(cChooser.Approve());
		AssertTrue(IsPathIn(cChooser.GetChosen(), szDirectory, "a.txt"));

		//A file that doesn't exist can't be opened.
		cChooser.GetFileName()->SetText("missing.txt");
		AssertFalse(cChooser.Approve());

		//Every file shows them all.
		cChooser.GetFileType()->Select(1);
		cChooser.ComboChanged(&cChooser.GetFileType());
		AssertInt(4, cChooser.GetList()->NumRows());
		AssertTrue(IsPathIn(cChooser.GetRowPath(3), szDirectory, "c.xml"));

		//A folder named in File Name is opened, not chosen.
		cChooser.GetFileName()->SetText("Sub");
		AssertFalse(cChooser.Approve());
		AssertString(szFullSub.Text(), cChooser.GetDirectory());
		AssertInt(0, cChooser.GetList()->NumRows());
		AssertString("", cChooser.GetFileName()->GetText());

		//Up goes back with the folder come from selected.
		cChooser.ButtonClicked(&cChooser.GetUp());
		AssertString(szFull.Text(), cChooser.GetDirectory());
		AssertInt(0, cChooser.GetList()->GetSelected());

		//Activating a folder's row opens it.
		cChooser.ItemActivated();
		AssertString(szFullSub.Text(), cChooser.GetDirectory());
		cChooser.Kill();

		//Saving adds the type's extension.
		cChooser.Init(pTestWindow, &sLook, "Save File", true, szStart, gasTestFileChooserTypes, 2, pFolderIcon, pFileIcon);
		cChooser.GetFileName()->SetText("new");
		AssertTrue(cChooser.Approve());
		AssertTrue(IsPathIn(cChooser.GetChosen(), szDirectory, "new.txt"));
		cChooser.GetFileName()->SetText("other.txt");
		AssertTrue(cChooser.Approve());
		AssertTrue(IsPathIn(cChooser.GetChosen(), szDirectory, "other.txt"));
		cChooser.Kill();

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

