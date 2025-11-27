#include "WinGDIWindow.h"
#include "WinGDICanvas.h"
#include "WinGDIWindowFactory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindowFactory::Init(CMallocator* pcMallocator, HINSTANCE hInstance, HINSTANCE hPrevInstance, int nCmdShow, const char* szWindowClass)
{
    CNativeWindowFactory::Init(pcMallocator);

    mszWindowClass.Init(szWindowClass);
    mhInstance = hInstance;
    mhPrevInstance = NULL;
    miCmdShow = nCmdShow;
    memset(&mhWnd, 0, sizeof(HWND));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindowFactory::Kill(void)
{
    mszWindowClass.Kill();
    CNativeWindowFactory::Kill();
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
    CWinGDICanvas*  pcNativeCanvas;
    bool            bResult;

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
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindowFactory::SetHWnd(HWND hWnd) { mhWnd = hWnd; }
HWND CWinGDIWindowFactory::GetHWnd(void) { return mhWnd; }

