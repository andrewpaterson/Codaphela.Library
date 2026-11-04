#include "StandardLib/Objects.h"
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
    msLastColour = 0xffffffff;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefCanvas::Kill(void)
{
    //The image is held by the canvas, like the X11 and GDI canvases' images.
    if (mpcImage)
    {
        mcDraw.Kill();
        mpcImage = NULL;
    }
    SetImage(NULL);
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

    sSize = mpcCanvas->GetActualSize();

    eColourFormat = mpcCanvas->GetColourFormat();
    eColourOrder = mpcCanvas->GetColourOrder();
    eColourBits = mpcCanvas->GetColourBits();
    eAlphaBits = mpcCanvas->GetAlphaBits();

    pImage = OMalloc<CImage>(sSize.x, sSize.y, eColourFormat, eColourOrder, eColourBits, eAlphaBits);
    if (pImage.IsNull())
    {
        SetSize(-1, -1);
        return false;
    }

    pImage->Black();
    SetImage(pImage);
    mpcImage = &pImage;

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
void CWinRefCanvas::CopyCanvas(CNativeCanvas* pcSourceCanvas)
{
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CWinRefCanvas::GetImageOrNull(void)
{
    return GetImage();
}


//////////////////////////////////////////////////////////////////////////
//
// The canvas's pixels, even when GetImageOrNull hides them (a test canvas
// pretending to have no native image still has its frames saved).
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CWinRefCanvas::GetNativeImage(void)
{
    return GetImage();
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
    CWinRefCanvas*  pcRefSource;
    CInt32Rect      cDest;
    CInt32Rect      cCanvas;
    CInt32Rect      cSource;

    pcRefSource = (CWinRefCanvas*)pcSource;
    if ((pcRefSource == NULL) || (pcRefSource->mpcImage == NULL) || (mpcImage == NULL))
    {
        return;
    }

    if (pcRefSource->mpcCanvas->IsTransparent())
    {
        //What the transparent canvas didn't cover must show through so it is blended rather than copied.
        BlendPremultipliedImage(pcRefSource->mpcImage, mpcImage, iX, iY, pcClip);
        return;
    }

    //A canvas in another format can't be drawn onto this (as a cel blitter wouldn't).
    if (!mpcImage->IsSameFormat(pcRefSource->mpcImage))
    {
        gcLogger.Error2(__METHOD__, " Cannot draw canvas, the source and destination image formats do not match.", NULL);
        return;
    }

    //Only the part of the source inside the clip and this canvas is copied.
    cDest.Init(iX, iY, iX + pcRefSource->mpcImage->GetWidth(), iY + pcRefSource->mpcImage->GetHeight());
    if (pcClip)
    {
        cDest.ClipTo(pcClip);
    }
    cCanvas.Init(0, 0, mpcImage->GetWidth(), mpcImage->GetHeight());
    cDest.ClipTo(&cCanvas);
    if ((cDest.GetWidth() <= 0) || (cDest.GetHeight() <= 0))
    {
        return;
    }

    cSource.Init(cDest.GetLeft() - iX, cDest.GetTop() - iY, cDest.GetRight() - iX, cDest.GetBottom() - iY);
    CImageCopier::Copy(pcRefSource->mpcImage, mpcImage, cDest.GetLeft(), cDest.GetTop(), &cSource);
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

