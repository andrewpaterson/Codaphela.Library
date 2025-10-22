#include "WinGDIWindow.h"
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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindowFactory::Kill(void)
{
    CNativeWindowFactory::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNativeWindow* CWinGDIWindowFactory::CreateNativeWindow(char* szWindowTitle)
{
    CWinGDIWindow*  pcWindow;

    pcWindow = NativeMalloc<CWinGDIWindow>();
    pcWindow->Init(szWindowTitle, this, mhInstance, mhPrevInstance, miCmdShow, mszWindowClass.Text());

    return pcWindow;
}

