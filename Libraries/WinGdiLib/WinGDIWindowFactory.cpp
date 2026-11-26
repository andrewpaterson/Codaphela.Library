#include "WinGDIWindow.h"
#include "WinGDICanvas.h"
#include "WinGDIClipboard.h"
#include "WinGDIWindowFactory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindowFactory::Init(CMallocator* pcMallocator, HINSTANCE hInstance, HINSTANCE hPrevInstance, int nCmdShow, const char* szWindowClass)
{
	PreInit();

	CNativeWindowFactory::Init(pcMallocator);

	mszWindowClass.Init(szWindowClass);
	mhInstance = hInstance;
	mhPrevInstance = NULL;
	miCmdShow = nCmdShow;
	memset(&mhWnd, 0, sizeof(HWND));
	mfRawInput = NULL;
	mpvRawInputContext = NULL;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindowFactory::Class(void)
{
	CNativeWindowFactory::Class();

	U_String(mszWindowClass);
	U_Pointer(mhInstance);
	U_Pointer(mhPrevInstance);
	U_Pointer(mhWnd);
	U_SInt(miCmdShow);
	U_Pointer(mfRawInput);
	U_Pointer(mpvRawInputContext);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindowFactory::Free(void)
{
	mszWindowClass.Kill();
	CNativeWindowFactory::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNativeWindow* CWinGDIWindowFactory::CreateNativeWindow(CWindow* pcWindow)
{
	CWinGDIWindow*  pcNativeWindow;

	pcNativeWindow = NativeMalloc<CWinGDIWindow>();
	pcNativeWindow->Init(pcWindow,
	                     this,
	                     mhInstance,
	                     mhPrevInstance,
	                     miCmdShow,
	                     mszWindowClass.Text());

	return pcNativeWindow;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNativeCanvas* CWinGDIWindowFactory::CreateNativeCanvas(CCanvas* pcCanvas)
{
	CWinGDICanvas*	pcNativeCanvas;
	bool			bResult;

	pcNativeCanvas = NativeMalloc<CWinGDICanvas>();
	pcNativeCanvas->Init(pcCanvas, this);
	bResult = pcNativeCanvas->CreateNativeCanvas();
	if (!bResult)
	{
		DestroyNativeCanvas(pcNativeCanvas);
		return NULL;
	}
	else
	{
		return pcNativeCanvas;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The Windows clipboard.  It needs nothing made until it is used.
//
//////////////////////////////////////////////////////////////////////////
CNativeClipboard* CWinGDIWindowFactory::CreateNativeClipboard(CClipboard* pcClipboard)
{
	CWinGDIClipboard*   pcNativeClipboard;

	pcNativeClipboard = NativeMalloc<CWinGDIClipboard>();
	pcNativeClipboard->Init(pcClipboard, this);
	return pcNativeClipboard;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindowFactory::SetHWnd(HWND hWnd) { mhWnd = hWnd; }
HWND CWinGDIWindowFactory::GetHWnd(void) const { return mhWnd; }


//////////////////////////////////////////////////////////////////////////
//
// Without it WM_INPUT is ignored.
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindowFactory::SetRawInput(WinGDIRawInputFunc fRawInput, void* pvContext)
{
	mfRawInput = fRawInput;
	mpvRawInputContext = pvContext;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindowFactory::RawInput(void* pvRawInput)
{
	if (mfRawInput)
	{
		mfRawInput(mpvRawInputContext, pvRawInput);
	}
}

