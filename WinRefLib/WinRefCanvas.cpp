#include "StandardLib/Objects.h"
#include "StandardLib/FreeObjects.h"
#include "SupportLib/ImageAccessorCreator.h"
#include "SupportLib/ImageCopier.h"
#include "WindowLib/Canvas.h"
#include "WinRefWindowFactory.h"
#include "WinRefCanvas.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefCanvas::Init(CCanvas* pcCanvas, CNativeWindowFactory* pcWindowFactory)
{
	CNativeCanvas::Init(pcCanvas, pcWindowFactory);
    mpcImage = NULL;
    msLastColour = 0xffffffff;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefCanvas::Kill(void)
{
    if (mpcImage)
    {
        mcDraw.Kill();
        gcObjects.RemoveFreeObject(mpcImage);
        mpcImage = NULL;
    }
    CNativeCanvas::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWinRefCanvas::CreateNativeCanvas(void)
{
    //This is split from .Init() so that it can fail on its own.

    CWinRefWindowFactory*   pcFactory; 
    SInt32Vec2              sSize;
    EColourFormat           eColourFormat;
    EColourOrder            eColourOrder;
    ERGBColourBits          eColourBits;
    ERGBAlphaBits           eAlphaBits;
    Ptr<CImage>             pImage;

    pcFactory = (CWinRefWindowFactory*)mpcWindowFactory;
    sSize = mpcCanvas->GetActualSize();

    eColourFormat = mpcCanvas->GetColourFormat();
    eColourOrder = mpcCanvas->GetColourOrder();
    eColourBits = mpcCanvas->GetColourBits();
    eAlphaBits = mpcCanvas->GetAlphaBits();

    pImage = OMalloc<CImage>(sSize.x, sSize.y, eColourFormat, eColourOrder, eColourBits, eAlphaBits);
    mpcImage = gcObjects.AddFreeObject<CImage>(pImage);

    if (mpcImage == NULL)
    {
        SetSize(-1, -1);
        return false;
    }

    mpcImage->Black();

    SetSize(sSize.x, sSize.y);
    mcDraw.Init(mpcImage);

    return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8* CWinRefCanvas::GetPixelData(void)
{
    return (uint8*)mpcImage->GetData();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CWinRefCanvas::GetImage(void)
{
    return mpcImage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefCanvas::CopyCanvas(CNativeCanvas* pcSourceCanvas)
{
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CWinRefCanvas::GetImageOrNull(void)
{
    return mpcImage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefCanvas::DrawBox(CRectangle* pcRectangle, bool bFilled, ARGB32 sColour)
{
    SetColour(sColour);
    mcDraw.DrawBox(pcRectangle, bFilled);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefCanvas::DrawPixel(int32 iX, int32 iY, ARGB32 sColour)
{
    SetColour(sColour);
    mcDraw.DrawPixel(iX, iY);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefCanvas::DrawCanvas(int iX, int iY, CNativeCanvas* pcSource)
{
    CWinRefCanvas*   pcRefSource;

    pcRefSource = (CWinRefCanvas*)pcSource;
    mcDraw.DrawImage(iX, iY, &pcRefSource->GetImage());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefCanvas::SetColour(ARGB32 sColour)
{
    CImageColourRGB	    cColour;
    float               r;
    float               g;
    float               b;

    if (sColour != msLastColour)
    {
        Get32BitColour(&r, &g, &b, sColour);
        cColour.Init(r, g, b);
        mcDraw.SetColour(&cColour);
        msLastColour = sColour;
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefCanvas::DrawCel(int iX, int iY, Ptr<CImageCel> pSourceCel)
{
    CImageCopier::Copy(pSourceCel, mpcImage, iX, iY);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefCanvas::DrawImage(int iX, int iY, Ptr<CImage> pSourceImage)
{
    CImageCopier::Copy(pSourceImage, mpcImage, iX, iY);
}

