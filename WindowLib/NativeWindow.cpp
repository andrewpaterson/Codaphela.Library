#include "WindowLib/Window.h"
#include "NativeWindowFactory.h"
#include "NativeCanvas.h"
#include "NativeWindow.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNativeWindow::Init(CWindow* pcWindow, CNativeWindowFactory* pcWindowFactory)
{
	CBasicNativeComponent::Init(pcWindowFactory);
	mpcWindowFactory = pcWindowFactory;
	mpcWindow = pcWindow;
	mcLastRectangle.Init(-1, -1, -1, -1);
	mbPainting = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNativeWindow::Kill(void)
{
	CBasicNativeComponent::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNativeWindow::BeginPresent(void)
{
    if (!mbPainting)
    {
        mbPainting = true;
        return true;
    }
    return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNativeWindow::EndPresent(void)
{
    mbPainting = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNativeWindow::Present(void)
{
    bool            bResult;
    CNativeCanvas*  pcNativeCanvas;
    Ptr<CCanvas>    pCanvas;
    SInt2           sSize;

    bResult = BeginPresent();
    if (bResult)
    {
        pCanvas = mpcWindow->GetCanvas();

        pcNativeCanvas = pCanvas->GetNativeCanvas();
        if (pcNativeCanvas)
        {
            sSize = pCanvas->GetActualSize();
            Present(pcNativeCanvas, sSize.x, sSize.y);
        }

        EndPresent();
        return true;
    }
    return false;
}


