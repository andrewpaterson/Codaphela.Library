#include "BaseLib/LeakDetector.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/WindowTheme.h"
#include "TestLib/Assert.h"


void TestRefWindow(void);
void TestFlowContainer(void);
void TestFillContainer(void);
void TestGridContainer(void);
void TestNestedContainers(void);
void TestBorderLayoutHelper(void);
void TestCanvas(void);
void TestLabel(void);
void TestTextArea(void);
void TestTextAreaBackgrounds(void);
void TestTextAreaEdit(void);
void TestTextLineGaps(void);
void TestTextBackgrounds(void);
void TestTextSpans(void);
void TestTextAreaRows(void);
void TestTextAreaBands(void);
void TestTextAreaShapes(void);
void TestTextEditingInput(void);
void TestCodeEditor(void);
void TestDiffView(void);
void TestLineZones(void);
void TestScrollPane(void);
void TestProgressBar(void);
void TestSplitContainer(void);
void TestToolbar(void);
void TestStatusBar(void);
void TestToolTips(void);
void TestComponentInput(void);
void TestComponentInputDevices(void);
void TestInputAction(void);
void TestKeymap(void);
void TestTabbedContainer(void);
void TestDocking(void);
void TestComponentInputWinRaw(void);
void TestInputSourceChannel(void);
void TestButton(void);
void TestButtonBar(void);
void TestList(void);
void TestTree(void);
void TestMenu(void);
void TestTextField(void);
void TestCheckBox(void);
void TestComboBox(void);
void TestDialog(void);
void TestFileChooser(void);
void TestGoToDialog(void);
void TestFindInFilesDialog(void);
void TestCaretBlink(void);
void TestSoundMixer(void);
void TestWindowRedraw(void);
void TestPartialRedraw(void);
void TestWindowTheme(void);
void TestNoUnsetThemeReads(size uiUnsetReadsBefore);
#ifdef _WIN32
void TestWinGDICanvas(void);
#endif // _WIN32


//////////////////////////////////////////////////////////////////////////
//
// Every test that uses windows.
//
//////////////////////////////////////////////////////////////////////////
void RunWindowTests(void)
{
	TestRefWindow();
	TestFlowContainer();
	TestFillContainer();
	TestGridContainer();
	TestNestedContainers();
	TestBorderLayoutHelper();
	TestCanvas();
	TestLabel();
	TestTextArea();
	TestTextAreaBackgrounds();
	TestTextAreaEdit();
	TestTextLineGaps();
	TestTextBackgrounds();
	TestTextSpans();
	TestTextAreaRows();
	TestTextAreaBands();
	TestTextAreaShapes();
	TestTextEditingInput();
	TestCodeEditor();
	TestDiffView();
	TestLineZones();
	TestScrollPane();
	TestProgressBar();
	TestSplitContainer();
	TestToolbar();
	TestStatusBar();
	TestToolTips();
	TestComponentInput();
	TestComponentInputDevices();
	TestInputAction();
	TestKeymap();
	TestTabbedContainer();
	TestDocking();
	TestComponentInputWinRaw();
	TestInputSourceChannel();
	TestButton();
	TestButtonBar();
	TestList();
	TestTree();
	TestMenu();
	TestTextField();
	TestCheckBox();
	TestComboBox();
	TestDialog();
	TestFileChooser();
	TestGoToDialog();
	TestFindInFilesDialog();
	TestCaretBlink();
	TestSoundMixer();
	TestWindowRedraw();
}


//////////////////////////////////////////////////////////////////////////
//
// No window's partial paint differed from drawing all of it while every
// window drew only what changed.
//
//////////////////////////////////////////////////////////////////////////
void TestPartialRedrawEverywhere(size uiMismatchesBefore)
{
	BeginTests();
	AssertSize(uiMismatchesBefore, CWindow::GetTotalRedrawMismatches());
	TestStatistics();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int __cdecl main(void)
{
	CFileUtil	cFileUtil;
	int			iResult;
	size		uiMismatches;
	size		uiUnsetThemeReads;

	BreakOnAllocation(0);
	InitTotalStatistics();

	TypesInit();
	TypeConverterInit();
	MemoryInit();
	DataIOInit();

	cFileUtil.RemoveDir("Output");
	cFileUtil.MakeDir("Output");

	TestWindowTheme();
	uiUnsetThemeReads = CWindowTheme::GetTotalUnsetReads();
	RunWindowTests();
	TestPartialRedraw();

	//Again with every window drawing only what changed, each partial paint checked against drawing all of it.
	uiMismatches = CWindow::GetTotalRedrawMismatches();
	CWindow::SetDefaultRedraw(true, true);
	RunWindowTests();
	CWindow::SetDefaultRedraw(false, false);
	TestPartialRedrawEverywhere(uiMismatches);
	TestNoUnsetThemeReads(uiUnsetThemeReads);

#ifdef _WIN32
	TestWinGDICanvas();
#endif // _WIN32

	cFileUtil.RemoveDir("Output");

	DataIOKill();
	MemoryKill();
	TypeConverterKill();
	TypesKill();

	iResult = TestTotalStatistics();
	LeakDetection();
	return iResult;
}

