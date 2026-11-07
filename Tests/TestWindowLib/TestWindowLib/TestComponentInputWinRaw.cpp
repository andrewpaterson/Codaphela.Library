#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Objects.h"
#include "InputLib/Input.h"
#include "InputLib/NativeInput.h"
#include "WindowLib/Window.h"
#include "WindowLib/GridContainer.h"
#include "WindowLib/ComponentInput.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"
#include "InputTestComponent.h"


#define AssertLog(e)	AssertString(e, szLog.Text()); szLog.Clear()


//////////////////////////////////////////////////////////////////////////
//
// Laid out as SRawMouseEvent and SRawKeyboardEvent in WinInputLib, which
// follow the 'WinRAW Mouse' and 'WinRAW Keyboard' formats in
// WindowsFormats.xml.
//
//////////////////////////////////////////////////////////////////////////
struct STestRawMouse
{
	uint16		usFlags;
	uint16		usPadding;
	uint16		usButtonFlags;
	int16		sButtonData;
	uint32		ulRawButtons;
	int32		lLastX;
	int32		lLastY;
	uint32		ulExtraInformation;
};


struct STestRawKeyboard
{
	uint16	usMakeCode;
	uint16	usFlags;
	uint16	usReserved;
	uint16	usVKey;
	uint32	uiMessage;
	uint32	uiExtraInformation;
};


#define TEST_RI_KEY_BREAK			1
#define TEST_RI_KEY_E0				2
#define TEST_RI_MOUSE_BUTTON_1_DOWN	0x0001
#define TEST_RI_MOUSE_BUTTON_1_UP	0x0002
#define TEST_RI_MOUSE_BUTTON_2_DOWN	0x0004
#define TEST_RI_MOUSE_BUTTON_2_UP	0x0008
#define TEST_RI_MOUSE_BUTTON_3_DOWN	0x0010
#define TEST_RI_MOUSE_BUTTON_3_UP	0x0020
#define TEST_RI_MOUSE_WHEEL			0x0400


//////////////////////////////////////////////////////////////////////////
//
// Stands in for CWinRawInput: one standard keyboard and one standard
// mouse described by WinInputLib's XML and given raw events.
//
//////////////////////////////////////////////////////////////////////////
class CTestRawInput : public CNativeInput
{
public:
	CInput*				mpcInput;
	CComponentInput*	mpcComponentInput;  //Updates the input through its source once it is set.
	int					miKeyboard;
	int					miMouse;
	STestRawKeyboard	msKeyboard;
	STestRawMouse		msMouse;
	bool				mbKeyboard;
	bool				mbMouse;

	void Init(CInput* pcInput)
	{
		mpcInput = pcInput;
		mpcComponentInput = NULL;
		miKeyboard = -1;
		miMouse = -1;
		mbKeyboard = false;
		mbMouse = false;
	}

	void Kill(void) override
	{
	}

	void AddDevicesTo(CInputDevices* pcDevices) override
	{
		CInputDevice*	pcDevice;

		pcDevice = pcDevices->CreateDevice(pcDevices->GetDescription("WinRAW Keyboard: Standard 104 Button Keyboard"));
		pcDevice->GenerateRestState();
		miKeyboard = pcDevice->GetDeviceID();

		pcDevice = pcDevices->CreateDevice(pcDevices->GetDescription("WinRAW Mouse: Standard 5 Button Mouse"));
		pcDevice->GenerateRestState();
		miMouse = pcDevice->GetDeviceID();
	}

	void Update(CInputDevices* pcDevices, uint32 uiSequence) override
	{
		if (mbKeyboard)
		{
			pcDevices->GetDevice(miKeyboard)->Process(&msKeyboard, uiSequence);
			mbKeyboard = false;
		}
		if (mbMouse)
		{
			pcDevices->GetDevice(miMouse)->Process(&msMouse, uiSequence);
			mbMouse = false;
		}
	}

	void Key(uint16 usVKey, uint16 usFlags, uint16 usMakeCode)
	{
		memset(&msKeyboard, 0, sizeof(STestRawKeyboard));
		msKeyboard.usVKey = usVKey;
		msKeyboard.usFlags = usFlags;
		msKeyboard.usMakeCode = usMakeCode;
		mbKeyboard = true;
		Update();
	}

	void Update(void)
	{
		if (mpcComponentInput)
		{
			mpcComponentInput->Update();
		}
		else
		{
			mpcInput->Update();
		}
	}

	void Press(uint16 usVKey, bool bExtended = false, uint16 usMakeCode = 0) { Key(usVKey, bExtended ? TEST_RI_KEY_E0 : 0, usMakeCode); }
	void Release(uint16 usVKey, bool bExtended = false, uint16 usMakeCode = 0) { Key(usVKey, (bExtended ? TEST_RI_KEY_E0 : 0) | TEST_RI_KEY_BREAK, usMakeCode); }
	void Tap(uint16 usVKey, bool bExtended = false) { Press(usVKey, bExtended); Release(usVKey, bExtended); }

	void Mouse(uint16 usButtonFlags, int16 sButtonData = 0)
	{
		memset(&msMouse, 0, sizeof(STestRawMouse));
		msMouse.usButtonFlags = usButtonFlags;
		msMouse.sButtonData = sButtonData;
		mbMouse = true;
		Update();
	}
};


#define VK_TEST_SHIFT		16
#define MAKE_LEFT_SHIFT		42
#define MAKE_RIGHT_SHIFT	54
#define VK_TEST_CONTROL		17
#define VK_TEST_CAPITAL		20
#define VK_TEST_END			35
#define VK_TEST_UP			38
#define VK_TEST_INSERT		45
#define VK_TEST_NUMPAD8		104


//////////////////////////////////////////////////////////////////////////
//
// The window is 64x32 with A on the left and B on the right, both can
// take focus.
//
//////////////////////////////////////////////////////////////////////////
void TestComponentInputWinRawDevices(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CGridContainer>			pGrid;
		Ptr<CInputTestComponent>	pA;
		Ptr<CInputTestComponent>	pB;
		Ptr<CTickTestRefWindow>			pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		CChars						szLog;
		Ptr<CInput>					pInput;
		CTestRawInput				cRaw;
		Ptr<CWindowInputSource>		pInputSource;
		CComponentInput				cComponentInput;

		szLog.Init();
		cNativeFactory.Init(&gcMemoryAllocator, 64, 32, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Input Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pGrid = OMalloc<CGridContainer>(pTestWindow);
		pTestWindow->SetContainer(pGrid);
		pA = OMalloc<CInputTestComponent>(pTestWindow, "A", &szLog, true);
		pB = OMalloc<CInputTestComponent>(pTestWindow, "B", &szLog, true);
		AssertTrue(pGrid->AddComponent(pA, 0, 0, 1, 1, 1, 1));
		AssertTrue(pGrid->AddComponent(pB, 1, 0, 1, 1, 1, 1));
		pTestWindow->Layout();

		//The devices are described by the XML so they are added after it is read.
		pInput = OMalloc<CInput>((CNativeInput*)NULL);
		ORoot()->Add(pInput);
		AssertTrue(pInput->Read("../../../Libraries/WinInputLib/xml"));
		cRaw.Init(&pInput);
		AssertTrue(pInput->AddNativeInput(&cRaw));

		pInputSource = OMalloc<CWindowInputSource>(pInput);
		cComponentInput.Init(&pInputSource, &pTestWindow);
		cRaw.mpcComponentInput = &cComponentInput;
		AssertSize(1, cComponentInput.AddKeyboards());
		AssertSize(1, cComponentInput.AddMice());
		pTestWindow->SetComponentInput(&cComponentInput);
		AssertTrue(pTestWindow->GetComponentInput() == &cComponentInput);

		pTestWindow->PointerMoved(40, 10);
		cRaw.Mouse(TEST_RI_MOUSE_BUTTON_1_DOWN);
		cRaw.Mouse(TEST_RI_MOUSE_BUTTON_1_UP);
		AssertLog("B:Into;B:Gained;B:Activate(8,10);B:Execute(8,10);");

		cRaw.Tap('A');
		cRaw.Tap(13);
		cRaw.Tap(32);
		cRaw.Tap(9);
		cRaw.Tap(VK_TEST_UP, true);
		cRaw.Tap(VK_TEST_END, true);
		AssertLog("B:Printable(a,Insert);B:Enter;B:Space(Insert);B:Tab(Insert);B:Up;B:End;");

		//Both shift keys have the same virtual key, the make code tells them apart.
		cRaw.Press(VK_TEST_SHIFT, false, MAKE_LEFT_SHIFT);
		cRaw.Tap('A');
		cRaw.Release(VK_TEST_SHIFT, false, MAKE_LEFT_SHIFT);
		cRaw.Press(VK_TEST_SHIFT, false, MAKE_RIGHT_SHIFT);
		cRaw.Tap('1');
		cRaw.Release(VK_TEST_SHIFT, false, MAKE_RIGHT_SHIFT);
		cRaw.Tap('A');
		AssertLog("B:Printable(A,Insert);B:Printable(!,Insert);B:Printable(a,Insert);");

		cRaw.Press(VK_TEST_CONTROL);
		cRaw.Tap('C');
		cRaw.Release(VK_TEST_CONTROL);
		AssertLog("B:Copy;");

		cRaw.Tap(VK_TEST_CAPITAL);
		cRaw.Tap('A');
		cRaw.Tap(VK_TEST_CAPITAL);
		cRaw.Tap(VK_TEST_INSERT, true);
		cRaw.Tap('A');
		cRaw.Tap(VK_TEST_INSERT, true);
		AssertLog("B:Printable(A,Insert);B:Printable(a,Overwrite);");

		//Windows gives the numpad keys different virtual keys with num lock on.  The window sets the locks when it is activated.
		cRaw.Tap(VK_TEST_UP);
		cComponentInput.SetLocks(false, true);
		cRaw.Tap(VK_TEST_NUMPAD8);
		cComponentInput.SetLocks(false, false);
		AssertLog("B:Up;B:Printable(8,Insert);");

		cRaw.Mouse(TEST_RI_MOUSE_BUTTON_2_DOWN);
		cRaw.Mouse(TEST_RI_MOUSE_BUTTON_2_UP);
		cRaw.Mouse(TEST_RI_MOUSE_BUTTON_3_DOWN);
		cRaw.Mouse(TEST_RI_MOUSE_BUTTON_3_UP);
		cRaw.Mouse(TEST_RI_MOUSE_WHEEL, 120);
		cRaw.Mouse(TEST_RI_MOUSE_WHEEL, -120);
		AssertLog("B:ContextMenu(8,10);B:StartDragScroll(8,10);B:StopDragScroll(8,10);B:Wheel(0,-1);B:Wheel(0,1);");

		cRaw.Press(VK_TEST_SHIFT, false, MAKE_LEFT_SHIFT);
		cRaw.Mouse(TEST_RI_MOUSE_BUTTON_1_DOWN);
		pTestWindow->PointerMoved(10, 10);
		cRaw.Release(VK_TEST_SHIFT, false, MAKE_LEFT_SHIFT);
		cRaw.Mouse(TEST_RI_MOUSE_BUTTON_1_UP);
		AssertLog("B:StartDragSelect(8,10);B:Outof;A:Into;B:ContDragSelect(-22,10);B:StopDragSelect(-22,10);");

		//Keys released while another window has focus are never seen.
		cRaw.Press(VK_TEST_SHIFT, false, MAKE_LEFT_SHIFT);
		pTestWindow->Deactivate();
		pTestWindow->Activate();
		cRaw.Tap('A');
		cRaw.Release(VK_TEST_SHIFT, false, MAKE_LEFT_SHIFT);
		cRaw.Tap('A');
		AssertLog("A:Outof;B:Lost;B:Gained;B:Printable(a,Insert);B:Printable(a,Insert);");

		//A modifier's event is held back while it could start a chord but the device has it down.
		cRaw.Press(VK_TEST_CONTROL);
		cRaw.Press(VK_TEST_SHIFT, false, MAKE_LEFT_SHIFT);
		AssertTrue(cComponentInput.IsSourceDown("Left Control"));
		AssertTrue(cComponentInput.IsSourceDown("Left Shift"));
		AssertFalse(cComponentInput.IsSourceDown("Right Control"));
		AssertFalse(cComponentInput.IsSourceDown("Right Shift"));
		AssertFalse(cComponentInput.IsSourceDown("A"));
		cRaw.Release(VK_TEST_SHIFT, false, MAKE_LEFT_SHIFT);
		cRaw.Release(VK_TEST_CONTROL);
		AssertFalse(cComponentInput.IsSourceDown("Left Control"));
		AssertFalse(cComponentInput.IsSourceDown("Left Shift"));
		cRaw.Mouse(TEST_RI_MOUSE_BUTTON_3_DOWN);
		AssertTrue(cComponentInput.IsSourceDown("Button 3"));
		AssertFalse(cComponentInput.IsSourceDown("Button 1"));
		AssertFalse(cComponentInput.IsSourceDown("No Such Key"));
		cRaw.Mouse(TEST_RI_MOUSE_BUTTON_3_UP);
		AssertFalse(cComponentInput.IsSourceDown("Button 3"));

		pTestWindow->SetComponentInput(NULL);
		cComponentInput.Kill();
		cRaw.mpcComponentInput = NULL;
		pInputSource = NULL;
		ORoot()->Remove(pInput);
		pInput = NULL;
		cRaw.Kill();

		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		pGrid = NULL;
		pA = NULL;
		pB = NULL;
		szLog.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	AssertSize(2, gcObjects.NumMemoryIndexes());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A key typing a character, pressed and released, only paints the window
// again; a command or a chord lays it out.
//
//////////////////////////////////////////////////////////////////////////
void TestComponentInputWinRawTypingLayout(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CGridContainer>			pGrid;
		Ptr<CInputTestComponent>	pA;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		CChars						szLog;
		Ptr<CInput>					pInput;
		CTestRawInput				cRaw;
		Ptr<CWindowInputSource>		pInputSource;
		CComponentInput				cComponentInput;

		szLog.Init();
		cNativeFactory.Init(&gcMemoryAllocator, 64, 32, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Typing Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		pTestWindow->SetCheckLayoutCache(false);
		ORoot()->Add(pTestWindow);
		pGrid = OMalloc<CGridContainer>(pTestWindow);
		pTestWindow->SetContainer(pGrid);
		pA = OMalloc<CInputTestComponent>(pTestWindow, "A", &szLog, true);
		AssertTrue(pGrid->AddComponent(pA, 0, 0, 1, 1, 1, 1));

		pInput = OMalloc<CInput>((CNativeInput*)NULL);
		ORoot()->Add(pInput);
		AssertTrue(pInput->Read("../../../Libraries/WinInputLib/xml"));
		cRaw.Init(&pInput);
		AssertTrue(pInput->AddNativeInput(&cRaw));
		pInputSource = OMalloc<CWindowInputSource>(pInput);
		cComponentInput.Init(&pInputSource, &pTestWindow);
		cRaw.mpcComponentInput = &cComponentInput;
		AssertSize(1, cComponentInput.AddKeyboards());
		AssertSize(1, cComponentInput.AddMice());
		pTestWindow->SetComponentInput(&cComponentInput);

		pTestWindow->Paint();
		pTestWindow->PointerMoved(10, 10);
		cRaw.Mouse(TEST_RI_MOUSE_BUTTON_1_DOWN);
		cRaw.Mouse(TEST_RI_MOUSE_BUTTON_1_UP);
		pTestWindow->Paint();
		AssertFalse(pTestWindow->IsLayoutDirty());
		szLog.Clear();

		cRaw.Press('A');
		AssertFalse(pTestWindow->IsLayoutDirty());
		cRaw.Release('A');
		AssertFalse(pTestWindow->IsLayoutDirty());
		AssertLog("A:Printable(a,Insert);");

		cRaw.Tap(VK_TEST_END, true);
		AssertTrue(pTestWindow->IsLayoutDirty());
		AssertLog("A:End;");
		pTestWindow->Paint();

		//Shift's press is held back while it could start a chord, and given with the next key: it lays out (it isn't a
		//character), the capitals typed while it is held don't.
		cRaw.Press(VK_TEST_SHIFT, false, MAKE_LEFT_SHIFT);
		cRaw.Tap('A');
		AssertTrue(pTestWindow->IsLayoutDirty());
		pTestWindow->Paint();
		cRaw.Tap('B');
		AssertFalse(pTestWindow->IsLayoutDirty());
		cRaw.Release(VK_TEST_SHIFT, false, MAKE_LEFT_SHIFT);
		pTestWindow->Paint();
		AssertLog("A:Printable(A,Insert);A:Printable(B,Insert);");

		cRaw.Press(VK_TEST_CONTROL);
		pTestWindow->Paint();
		cRaw.Press('C');
		AssertTrue(pTestWindow->IsLayoutDirty());
		cRaw.Release('C');
		cRaw.Release(VK_TEST_CONTROL);
		AssertLog("A:Copy;");

		pTestWindow->SetComponentInput(NULL);
		cComponentInput.Kill();
		cRaw.mpcComponentInput = NULL;
		pInputSource = NULL;
		ORoot()->Remove(pInput);
		pInput = NULL;
		cRaw.Kill();

		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		pGrid = NULL;
		pA = NULL;
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
void TestComponentInputWinRaw(void)
{
	BeginTests();
	ObjectsInit();

	TestComponentInputWinRawDevices();
	TestComponentInputWinRawTypingLayout();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

