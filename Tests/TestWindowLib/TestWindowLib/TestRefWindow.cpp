#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/StdRandom.h"
#include "WindowLib/Window.h"
#include "WindowLib/FlowContainer.h"
#include "SupportLib/ColourARGB32.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/AssertGeometric.h"
#include "TestLib/AssertFile.h"
#include "TestRefWindowCanvasDraw.h"
#include "BorderCanvasDraw.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRefWindowCreation(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "Creation";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	ObjectsInit();
	{
		Ptr<CWindow>					pTestWindow;
		Ptr<CTestRefWindowCanvasDraw>	pDraw;
		Ptr<CTickTestRefWindow>			pTick;
		SDataTestRefWindow				cData;
		Ptr<CRoot>						pRoot;
		CChars							szExpectedDirectory;

		cNativeFactory.Init(&gcMemoryAllocator, 96, 64, szDirectory);

		pRoot = ORoot();

		pTick = OMalloc<CTickTestRefWindow>(&cData, 10);
		pDraw = OMalloc<CTestRefWindowCanvasDraw>(&cData);
		pTestWindow = OMalloc<CWindow>("Reference Test Window", &cNativeFactory, pTick, pDraw, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);

		pRoot->Add(pTestWindow);

		pTestWindow->Show();

		pRoot->Remove(pTestWindow);
		pTestWindow = NULL;
		pDraw = NULL;
		pTick = NULL;
		cNativeFactory.Kill();

		szExpectedDirectory.Init(szDirectory);
		szExpectedDirectory.Replace("Output", "Input");

		AssertDirectory(szExpectedDirectory.Text(), szDirectory);
		szExpectedDirectory.Kill();
	}
	ObjectsFlush();
	ObjectsKill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRefWindowCanvasBorder(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "CanvasBorder";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	ObjectsInit();
	{
		Ptr<CWindow>					pTestWindow;
		CChars							szExpectedDirectory;
		Ptr<CBorderCanvasDraw>			pDraw;
		Ptr<CTickTestRefWindow>			pTick;
		SDataTestRefWindow				cData;
		Ptr<CRoot>						pRoot;

		cNativeFactory.Init(&gcMemoryAllocator, 96, 64, szDirectory);

		pRoot = ORoot();

		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pDraw = OMalloc<CBorderCanvasDraw>(Set32BitColour(1.0f, 0, 0));
		pTestWindow = OMalloc<CWindow>("Reference Test Window", &cNativeFactory, pTick, pDraw, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pTestWindow->Show();

		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pDraw = NULL;
		pTick = NULL;
		cNativeFactory.Kill();

		szExpectedDirectory.Init(szDirectory);
		szExpectedDirectory.Replace("Output", "Input");

		AssertDirectory(szExpectedDirectory.Text(), szDirectory);
		szExpectedDirectory.Kill();
	}
	ObjectsFlush();
	ObjectsKill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
// Without an image path (none or empty) a reference window keeps its
// frames to itself: nothing is written into the working directory.
//
//////////////////////////////////////////////////////////////////////////
void TestRefWindowNoImagePath(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szEmpty[] = "";
	char*					aszPaths[] = { NULL, szEmpty };
	size					i;

	cFileUtil.Delete("Frame0.png");
	ObjectsInit();
	for (i = 0; i < 2; i++)
	{
		Ptr<CWindow>					pTestWindow;
		Ptr<CTestRefWindowCanvasDraw>	pDraw;
		Ptr<CTickTestRefWindow>			pTick;
		SDataTestRefWindow				cData;

		cNativeFactory.Init(&gcMemoryAllocator, 32, 16, aszPaths[i]);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 2);
		pDraw = OMalloc<CTestRefWindowCanvasDraw>(&cData);
		pTestWindow = OMalloc<CWindow>("No Image Path Window", &cNativeFactory, pTick, pDraw, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		AssertTrue(pTestWindow->Show());
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pDraw = NULL;
		pTick = NULL;
		cNativeFactory.Kill();

		AssertFalse(cFileUtil.Exists("Frame0.png"));
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRefWindow(void)
{
	BeginTests();

	TestRefWindowCreation();
	TestRefWindowCanvasBorder();
	TestRefWindowNoImagePath();

	TestStatistics();
}

