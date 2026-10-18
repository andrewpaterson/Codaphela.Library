#include "WinRefWindow.h"
#include "WinRefCanvas.h"
#include "WinRefWindowFactory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefWindowFactory::Init(CMallocator* pcMallocator, int32 iWidth, int32 iHeight, char* szImagePath)
{
    CNativeWindowFactory::Init(pcMallocator);
    mcWindowBounds.Init(iWidth, iHeight);
    mszImagePath.Init(szImagePath);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefWindowFactory::Kill(void)
{
    mszImagePath.Kill();
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
    pcNativeWindow->Init(pcWindow, this, &mcWindowBounds, mszImagePath.Text());

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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CWinRefWindowFactory::GetWidth(void)
{
    return mcWindowBounds.GetWidth();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CWinRefWindowFactory::GetHeight(void)
{
    return mcWindowBounds.GetHeight();
}

