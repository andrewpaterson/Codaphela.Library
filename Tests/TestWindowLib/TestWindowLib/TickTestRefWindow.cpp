#include "StandardLib/ClassDefines.h"
#include "SupportLib/ColourARGB32.h"
#include "WindowLib/Window.h"
#include "TickTestRefWindow.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTickTestRefWindow::Init(SDataTestRefWindow* pcData, int32 iStop)
{
    PreInit();

    CWindowTick::Init();
    mpcData = pcData;
    mpcData->iTick = 0;
    mpcData->iStop = iStop;

    PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTickTestRefWindow::Tick(Ptr<CWindow> pWindow, int64 iUpdateTimeInMillieconds, int64 iTotalTimeInMillieconds)
{
    pWindow->Paint();

    mpcData->iTick++;
    if (mpcData->iTick == mpcData->iStop)
    {
        pWindow->Stop();
    }
}

