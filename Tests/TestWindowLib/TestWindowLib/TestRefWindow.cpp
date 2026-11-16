#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/StdRandom.h"
#include "BaseLib/CountingAllocator.h"
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
	Ptr<CWinRefWindowFactory>	pNativeFactory;
	CFileUtil					cFileUtil;
	char						szDirectory[] = "Output" _FS_ "Creation";

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

		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 96, 64, szDirectory);

		pRoot = ORoot();

		pTick = OMalloc<CTickTestRefWindow>(&cData, 10);
		pDraw = OMalloc<CTestRefWindowCanvasDraw>(&cData);
		pTestWindow = OMalloc<CWindow>("Reference Test Window", &pNativeFactory, pTick, pDraw, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);

		pRoot->Add(pTestWindow);

		pTestWindow->Show();

		pRoot->Remove(pTestWindow);
		pTestWindow = NULL;
		pDraw = NULL;
		pTick = NULL;
		pNativeFactory = NULL;

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
	Ptr<CWinRefWindowFactory>	pNativeFactory;
	CFileUtil					cFileUtil;
	char						szDirectory[] = "Output" _FS_ "CanvasBorder";

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

		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 96, 64, szDirectory);

		pRoot = ORoot();

		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pDraw = OMalloc<CBorderCanvasDraw>(Set32BitColour(1.0f, 0, 0));
		pTestWindow = OMalloc<CWindow>("Reference Test Window", &pNativeFactory, pTick, pDraw, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pTestWindow->Show();

		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pDraw = NULL;
		pTick = NULL;
		pNativeFactory = NULL;

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
	Ptr<CWinRefWindowFactory>	pNativeFactory;
	CFileUtil					cFileUtil;
	char						szEmpty[] = "";
	char*						aszPaths[] = { NULL, szEmpty };
	size						i;

	cFileUtil.Delete("Frame0.png");
	ObjectsInit();
	for (i = 0; i < 2; i++)
	{
		Ptr<CWindow>					pTestWindow;
		Ptr<CTestRefWindowCanvasDraw>	pDraw;
		Ptr<CTickTestRefWindow>			pTick;
		SDataTestRefWindow				cData;

		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 32, 16, aszPaths[i]);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 2);
		pDraw = OMalloc<CTestRefWindowCanvasDraw>(&cData);
		pTestWindow = OMalloc<CWindow>("No Image Path Window", &pNativeFactory, pTick, pDraw, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		AssertTrue(pTestWindow->Show());
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pDraw = NULL;
		pTick = NULL;
		pNativeFactory = NULL;

		AssertFalse(cFileUtil.Exists("Frame0.png"));
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Records, when it's freed, how much of what it made (native windows,
// canvases and clipboards, allocated from its counting mallocator) is
// still allocated.
//
//////////////////////////////////////////////////////////////////////////
struct SFactoryFreed
{
	bool	bFreed;
	lsize	uiLeft;
};


class CFreeCheckedRefWindowFactory : public CWinRefWindowFactory
{
CONSTRUCTABLE(CFreeCheckedRefWindowFactory);
protected:
	CCountingAllocator*		mpcCounting;
	SFactoryFreed*			mpsFreed;

public:
	void Init(CCountingAllocator* pcCounting, SFactoryFreed* psFreed)
	{
		PreInit();
		CWinRefWindowFactory::Init(pcCounting, 32, 16, (char*)NULL);
		mpcCounting = pcCounting;
		mpsFreed = psFreed;
		PostInit();
	}

	void Class(void) override
	{
		CWinRefWindowFactory::Class();
		U_Pointer(mpcCounting);
		U_Pointer(mpsFreed);
	}

	void Free(void) override
	{
		mpsFreed->bFreed = true;
		mpsFreed->uiLeft = mpcCounting->AllocatedUserSize();
		CWinRefWindowFactory::Free();
	}
};


//////////////////////////////////////////////////////////////////////////
//
// The app lets go of its factory while a window it made is still rooted.
// The window holds the factory, so ObjectsKill frees the window (and its
// canvas and clipboard) first, and the factory after them, when nothing
// it made is left.
//
//////////////////////////////////////////////////////////////////////////
void TestRefWindowFactoryOutlivesWindows(void)
{
	CCountingAllocator	cCounting;
	SFactoryFreed		sFreed;

	cCounting.Init(&gcMemoryAllocator);
	sFreed.bFreed = false;
	sFreed.uiLeft = 0;

	ObjectsInit();
	{
		Ptr<CFreeCheckedRefWindowFactory>	pFactory;
		Ptr<CWindow>						pWindow;
		Ptr<CTickTestRefWindow>				pTick;
		SDataTestRefWindow					cData;
		CPointer							pNull;

		pFactory = OMalloc<CFreeCheckedRefWindowFactory>(&cCounting, &sFreed);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pWindow = OMalloc<CWindow>("Outlived Factory", &pFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);
		AssertTrue(cCounting.AllocatedUserSize() > 0);

		pFactory = NULL;
		AssertFalse(sFreed.bFreed);
		pWindow = NULL;
		pTick = NULL;
		AssertFalse(sFreed.bFreed);
	}
	ObjectsFlush();
	ObjectsKill();

	AssertTrue(sFreed.bFreed);
	AssertLSize(0, sFreed.uiLeft);
	AssertLSize(0, cCounting.AllocatedUserSize());
	cCounting.Kill();
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
	TestRefWindowFactoryOutlivesWindows();

	TestStatistics();
}

