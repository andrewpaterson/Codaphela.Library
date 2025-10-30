#include "WinRefWindow.h"
#include "WinRefCanvas.h"
#include "WinRefWindowFactory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefWindowFactory::Init(CMallocator* pcMallocator, int32 iWidth, int32 iHeight)
{
    CNativeWindowFactory::Init(pcMallocator);
    mcBounds.Init(iWidth, iHeight);
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
    pcNativeWindow->Init(pcWindow, this, &mcBounds);

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

