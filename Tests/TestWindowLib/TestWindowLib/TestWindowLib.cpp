#include "BaseLib/LeakDetector.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
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
void TestTextAreaEdit(void);
void TestScrollPane(void);
void TestComponentInput(void);
void TestComponentInputDevices(void);
void TestInputAction(void);
void TestTabbedContainer(void);
void TestComponentInputWinRaw(void);
void TestInputSourceChannel(void);
void TestButton(void);
void TestButtonBar(void);
void TestList(void);
void TestMenu(void);
void TestTextField(void);
void TestCheckBox(void);
void TestComboBox(void);
void TestDialog(void);
void TestFileChooser(void);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int __cdecl main(void)
{
	CFileUtil	cFileUtil;
	int			iResult;

	BreakOnAllocation(0);
	InitTotalStatistics();

	TypesInit();
	TypeConverterInit();
	MemoryInit();
	DataIOInit();

	cFileUtil.RemoveDir("Output");
	cFileUtil.MakeDir("Output");

	TestRefWindow();
	TestFlowContainer();
	TestFillContainer();
	TestGridContainer();
	TestNestedContainers();
	TestBorderLayoutHelper();
	TestCanvas();
	TestLabel();
	TestTextArea();
	TestTextAreaEdit();
	TestScrollPane();
	TestComponentInput();
	TestComponentInputDevices();
	TestInputAction();
	TestTabbedContainer();
	TestComponentInputWinRaw();
	TestInputSourceChannel();
	TestButton();
	TestButtonBar();
	TestList();
	TestMenu();
	TestTextField();
	TestCheckBox();
	TestComboBox();
	TestDialog();
	TestFileChooser();

	cFileUtil.RemoveDir("Output");

	DataIOKill();
	MemoryKill();
	TypeConverterKill();
	TypesKill();

	iResult = TestTotalStatistics();
	LeakDetection();
	return iResult;
}

