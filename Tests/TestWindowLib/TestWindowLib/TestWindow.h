#ifndef __TEST_WINDOW_H__
#define __TEST_WINDOW_H__
#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"


//A window for a test, drawn into memory (a 24 bit RGB reference window): its native window factory, its tick and the
//window itself, which is rooted.  A test sets it up, builds its components in the window and, once every pointer of
//its own into the window is NULL again, kills it: that checks nothing of the window is left behind.
struct STestWindow
{
	CWinRefWindowFactory	cFactory;
	SDataTestRefWindow		cData;
	Ptr<CTickTestRefWindow>	pTick;
	Ptr<CWindow>			pWindow;

	//pDraw draws the window's canvas before its components, a null pointer for nothing.
	void	Init(const char* szTitle, int32 iWidth, int32 iHeight, Ptr<CCanvasDraw> pDraw)
	{
		cFactory.Init(&gcMemoryAllocator, iWidth, iHeight, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pWindow = OMalloc<CWindow>(szTitle, &cFactory, pTick, pDraw, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);
	}

	void	Init(const char* szTitle, int32 iWidth, int32 iHeight)
	{
		Init(szTitle, iWidth, iHeight, Ptr<CCanvasDraw>());
	}

	void	Kill(void)
	{
		ORoot()->Remove(pWindow);
		pWindow = NULL;
		pTick = NULL;
		AssertSize(0, gcObjects.NumFreeObjects());
		AssertTrue(ORoot()->RemoveAll());
		cFactory.Kill();
	}
};


#endif // __TEST_WINDOW_H__

