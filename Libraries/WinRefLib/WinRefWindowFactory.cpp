#include "WinRefWindow.h"
#include "WinRefCanvas.h"
#include "WinRefClipboard.h"
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
    mszClipboardText.Init();
    mcScreenArea.Init(0, 0, 0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefWindowFactory::Kill(void)
{
    mszClipboardText.Kill();
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
CNativeClipboard* CWinRefWindowFactory::CreateNativeClipboard(CClipboard* pcClipboard)
{
    CWinRefClipboard*   pcNativeClipboard;

    pcNativeClipboard = NativeMalloc<CWinRefClipboard>();
    pcNativeClipboard->Init(pcClipboard, this);
    return pcNativeClipboard;
}


//////////////////////////////////////////////////////////////////////////
//
// The text every one of the factory's clipboards has, empty until one
// sets it.
//
//////////////////////////////////////////////////////////////////////////
CChars* CWinRefWindowFactory::GetClipboardText(void)
{
    return &mszClipboardText;
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


//////////////////////////////////////////////////////////////////////////
//
// The screen the reference windows are on, for floating windows to be
// kept on.  Without one it isn't known.
//
//////////////////////////////////////////////////////////////////////////
void CWinRefWindowFactory::SetScreenArea(CInt32Rect* pcArea)
{
    mcScreenArea.Init(pcArea);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWinRefWindowFactory::GetScreenArea(CInt32Rect* pcDest)
{
    pcDest->Init(&mcScreenArea);
    return (mcScreenArea.GetWidth() > 0) && (mcScreenArea.GetHeight() > 0);
}

