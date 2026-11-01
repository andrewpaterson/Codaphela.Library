#include "StandardLib/ClassDefines.h"
#include "WindowLib/Canvas.h"
#include "BorderCanvasDraw.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBorderCanvasDraw::Init(ARGB32 sColour)
{
    PreInit();
    CCanvasDraw::Init();
    msColour = sColour;
    PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBorderCanvasDraw::Draw(Ptr<CCanvas> pCanvas)
{
    CInt32Rect      cRect;
    ARGB32	        sBlack;
    ARGB32	        sGrey;
    SInt32Vec2      sSize;

    if (pCanvas->IsValid())
    {
        sGrey = Set32BitColour((uint8)64, 64, 64);
        sBlack = Set32BitColour((uint8)0, 0, 0);

        sSize = pCanvas->GetActualSize();
        sSize.x--;
        sSize.y--;
        cRect.Init(sSize);

        pCanvas->DrawBox(&cRect, true, sBlack);
        pCanvas->DrawBox(&cRect, false, msColour);
        cRect.Shrink(1, 1, 1, 1);
        if (cRect.IsValid())
        {
            pCanvas->DrawBox(&cRect, false, sGrey);
        }
    }

    return true;
}

