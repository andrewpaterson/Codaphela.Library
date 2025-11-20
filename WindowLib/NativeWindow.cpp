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
bool CNativeWindow::BeginPresent(CRectangle* pcRectangle)
{
    int32           iWidth;
    int32           iHeight;

    if (!mbPainting)
    {
        mbPainting = true;
  
        GetRectangle(pcRectangle);

        iWidth = pcRectangle->GetWidth();
        iHeight = pcRectangle->GetHeight();

        if (!mcLastRectangle.Equals(pcRectangle))
        {
            mpcWindow->CreateCanvas(CF_R8G8B8, iWidth, iHeight);
        }
        return true;
    }
    return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNativeWindow::EndPresent(CRectangle* pcRectangle)
{
    mcLastRectangle.Init(pcRectangle);

    mbPainting = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNativeWindow::Present(void)
{
    CRectangle      cRectangle;
    bool            bResult;
    CNativeCanvas*  pcNativeCanvas;
    CCanvas*        pcCanvas;

    bResult = BeginPresent(&cRectangle);
    if (bResult)
    {
        pcCanvas = mpcWindow->GetCanvas();

        pcNativeCanvas = pcCanvas->GetNativeCanvas();
        if (pcNativeCanvas)
        {
            Present(pcNativeCanvas, cRectangle.GetWidth(), cRectangle.GetHeight());
        }

        EndPresent(&cRectangle);
        return true;
    }
    return false;
}


