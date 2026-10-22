#include "StandardLib/Objects.h"
#include "StandardLib/FreeObjects.h"
#include "SupportLib/ImageAccessorCreator.h"
#include "SupportLib/ImageCopier.h"
#include "SupportLib/ImagePremultipliedBlend.h"
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
    mpcCel = NULL;
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
        if (mpcCel)
        {
            gcObjects.RemoveFreeObject(mpcCel);
            mpcCel = NULL;
        }
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

    SInt32Vec2              sSize;
    EColourFormat           eColourFormat;
    EColourOrder            eColourOrder;
    ERGBColourBits          eColourBits;
    ERGBAlphaBits           eAlphaBits;
    Ptr<CImage>             pImage;
    Ptr<CImageCel>          pCel;

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

    //The cel covers the whole image so that this canvas can be blitted onto another.
    pCel = OMalloc<CImageCel>(pImage);
    mpcCel = gcObjects.AddFreeObject<CImageCel>(pCel);

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
Ptr<CImageCel> CWinRefCanvas::GetCel(void)
{
    return mpcCel;
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
void CWinRefCanvas::DrawBox(CInt32Rect* pcRectangle, bool bFilled, ARGB32 sColour)
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
void CWinRefCanvas::DrawCanvas(int iX, int iY, CNativeCanvas* pcSource, CInt32Rect* pcClip)
{
    CWinRefCanvas*   pcRefSource;

    pcRefSource = (CWinRefCanvas*)pcSource;
    if (pcRefSource->mpcCanvas->IsTransparent())
    {
        //What the transparent canvas didn't cover must show through so it is blended rather than copied.
        BlendPremultipliedImage(pcRefSource->mpcImage, mpcImage, iX, iY, pcClip);
    }
    else
    {
        mcDraw.DrawCel(iX, iY, pcRefSource->GetCel(), pcClip);
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefCanvas::SetColour(ARGB32 sColour)
{
    CImageColourRGB	    cColour;
    CImageColourARGB    cAlphaColour;
    float               r;
    float               g;
    float               b;
    uint32              uiRed;
    uint32              uiGreen;
    uint32              uiBlue;
    uint32              uiAlpha;

    if (sColour != msLastColour)
    {
        if (mpcImage->HasChannel(IMAGE_OPACITY))
        {
            //A canvas with opacity (a transparent canvas) holds its colours multiplied by their opacity.
            Get32BitColour(&uiRed, &uiGreen, &uiBlue, &uiAlpha, sColour);
            uiRed = (uiRed * uiAlpha + 127) / 255;
            uiGreen = (uiGreen * uiAlpha + 127) / 255;
            uiBlue = (uiBlue * uiAlpha + 127) / 255;
            cAlphaColour.Init(uiAlpha / 255.0f, uiRed / 255.0f, uiGreen / 255.0f, uiBlue / 255.0f);
            mcDraw.SetColour(&cAlphaColour);
        }
        else
        {
            Get32BitColour(&r, &g, &b, sColour);
            cColour.Init(r, g, b);
            mcDraw.SetColour(&cColour);
        }
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

