#include "WinRefWindow.h"
#include "WinRefCanvas.h"
#include "WinRefWindowFactory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefWindowFactory::Init(CMallocator* pcMallocator)
{
    CNativeWindowFactory::Init(pcMallocator);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefWindowFactory::Kill(void)
{
    CNativeWindowFactory::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNativeWindow* CWinRefWindowFactory::CreateNativeWindow(CWindow* pcWindow)
{
    CWinRefWindow*  pcNativeWindow;

    pcNativeWindow = NativeMalloc<CWinRefWindow>();
    pcNativeWindow->Init(pcWindow, 
                         this);

    return pcNativeWindow;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNativeCanvas* CWinRefWindowFactory::CreateNativeCanvas(CCanvas* pcCanvas)
{
    CWinRefCanvas*  pcNativeCanvas;
    bool            bResult;

    pcNativeCanvas = NativeMalloc<CWinRefCanvas>();
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

