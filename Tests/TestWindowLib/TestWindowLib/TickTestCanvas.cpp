#include "StandardLib/ClassDefines.h"
#include "SupportLib/ColourARGB32.h"
#include "WindowLib/Window.h"
#include "TickTestCanvas.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTickTestCanvas::Init(SDataTestRefWindow* pcData, Ptr<CMaps> pMaps, int32 iStop)
{
    PreInit();

    CWindowTick::Init();
    mpcData = pcData;
    mpcData->iTick = 0;
    mpcData->iStop = iStop;
    mpMaps = pMaps;

    PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTickTestCanvas::Class(void)
{
    CWindowTick::Class();
    U_Pointer(mpcData);
    M_Pointer(mpMaps);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTickTestCanvas::Tick(Ptr<CWindow> pWindow, int64 iUpdateTimeInMillieconds, int64 iTotalTimeInMillieconds)
{
    SInt32Vec2  sPos;

    pWindow->Paint();

    sPos = mpMaps->GetViewportPosition();
    sPos.x += 13;
    sPos.y += 9;
    mpMaps->SetViewportPosition(sPos);

    mpcData->iTick++;
    if (mpcData->iTick == mpcData->iStop)
    {
        pWindow->Stop();
    }
}

