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
    mpImage = NULL;
    msLastColour = 0xffffffff;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefCanvas::Kill(void)
{
    if (mpImage.IsNotNull())
    {
        mcDraw.Kill();
        mpImage = NULL;
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

    pcFactory = (CWinRefWindowFactory*)mpcWindowFactory;
    sSize = mpcCanvas->GetActualSize();

    eColourFormat = mpcCanvas->GetColourFormat();
    eColourOrder = mpcCanvas->GetColourOrder();
    eColourBits = mpcCanvas->GetColourBits();
    eAlphaBits = mpcCanvas->GetAlphaBits();

    mpImage = OMalloc<CImage>(sSize.x, sSize.y, eColourFormat, eColourOrder, eColourBits, eAlphaBits);

    if (mpImage.IsNull())
    {
        SetSize(-1, -1);
        return false;
    }

    mpImage->Black();

    SetSize(sSize.x, sSize.y);
    mcDraw.Init(&mpImage);

    return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8* CWinRefCanvas::GetPixelData(void)
{
    return (uint8*)mpImage->GetData();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CWinRefCanvas::GetImage(void)
{
    return mpImage;
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
    return mpImage;
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
    CImageCopier::Copy(pSourceCel, mpImage, iX, iY);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefCanvas::DrawImage(int iX, int iY, Ptr<CImage> pSourceImage)
{
    CImageCopier::Copy(pSourceImage, mpImage, iX, iY);
}

