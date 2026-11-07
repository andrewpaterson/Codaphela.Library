#include "SupportLib/ImageCopier.h"
#include "SupportLib/ImagePremultipliedBlend.h"
#include "WindowLib/Canvas.h"
#include "WinGDIWindowFactory.h"
#include "WinGDICanvas.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDICanvas::Init(CCanvas* pcCanvas, CNativeWindowFactory* pcWindowFactory)
{
	CNativeCanvas::Init(pcCanvas, pcWindowFactory);
    mpuiPixelData = NULL;
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDICanvas::Kill(void)
{
    //The image owns the pixels; whoever else holds it keeps them.
    SetImage(NULL);
    mpuiPixelData = NULL;
    CNativeCanvas::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWinGDICanvas::CreateNativeCanvas(void)
{
    //This is split from .Init() so that it can fail on its own.

    SInt32Vec2      sSize;
    Ptr<CImage>     pImage;

    sSize = mpcCanvas->GetNativeSize();
    if ((sSize.x <= 0) || (sSize.y <= 0))
    {
        SetSize(-1, -1);
        return false;
    }

    //Laid out as a 32 bit BI_RGB DIB (B, G, R, X) so Paint can show it as it is.  A transparent canvas uses X for its
    //opacity and, like the reference canvas, holds its colours multiplied by it.  It starts all zero; black, and
    //transparent if the canvas is.
    if (mpcCanvas->IsTransparent())
    {
        pImage = OMalloc<CImage>(sSize.x, sSize.y, PT_uint8, IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, IMAGE_OPACITY, CHANNEL_STOP);
    }
    else
    {
        pImage = OMalloc<CImage>(sSize.x, sSize.y, PT_uint8, IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, IMAGE_IGNORED, CHANNEL_STOP);
    }
    if (pImage.IsNull() || (pImage->GetData() == NULL))
    {
        SetSize(-1, -1);
        return false;
    }
    pImage->Black();
    mpuiPixelData = (uint8*)pImage->GetData();
    SetImage(pImage);
    SetSize(sSize.x, sSize.y);
    return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Shows the canvas's pixels in hDC (the window's, while it paints), from
// its top left, at most iWidth by iHeight of them.
//
//////////////////////////////////////////////////////////////////////////
void CWinGDICanvas::Paint(HDC hDC, int32 iWidth, int32 iHeight)
{
    BITMAPINFO  sBitmapInfo;

    if (mpuiPixelData == NULL)
    {
        return;
    }
    iWidth = (iWidth < msSize.x) ? iWidth : msSize.x;
    iHeight = (iHeight < msSize.y) ? iHeight : msSize.y;

    //Top down (a negative height): the first row in memory is the top of the window.
    memset(&sBitmapInfo, 0, sizeof(BITMAPINFO));
    sBitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    sBitmapInfo.bmiHeader.biWidth = msSize.x;
    sBitmapInfo.bmiHeader.biHeight = -msSize.y;
    sBitmapInfo.bmiHeader.biPlanes = 1;
    sBitmapInfo.bmiHeader.biBitCount = 32;
    sBitmapInfo.bmiHeader.biCompression = BI_RGB;

    //With a top down DIB the first scan line is the top row, so all msSize.y rows are given and the destination clips.
    SetDIBitsToDevice(hDC, 0, 0, iWidth, iHeight, 0, 0, 0, msSize.y, mpuiPixelData, &sBitmapInfo, DIB_RGB_COLORS);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8* CWinGDICanvas::GetPixelData(void)
{
    return mpuiPixelData;
}


//////////////////////////////////////////////////////////////////////////
//
// As much of the source as fits, from the top left, row by row.
//
//////////////////////////////////////////////////////////////////////////
void CWinGDICanvas::CopyCanvas(CNativeCanvas* pcSourceCanvas)
{
    CWinGDICanvas*  pcSource;
    SInt32Vec2      sSize;
    int32           y;

    if (!pcSourceCanvas)
    {
        return;
    }

    pcSource = (CWinGDICanvas*)pcSourceCanvas;
    if ((mpuiPixelData == NULL) || (pcSource->mpuiPixelData == NULL))
    {
        return;
    }
    sSize.x = (msSize.x < pcSource->msSize.x) ? msSize.x : pcSource->msSize.x;
    sSize.y = (msSize.y < pcSource->msSize.y) ? msSize.y : pcSource->msSize.y;
    for (y = 0; y < sSize.y; y++)
    {
        memcpy(&((uint32*)mpuiPixelData)[y * msSize.x], &((uint32*)pcSource->mpuiPixelData)[y * pcSource->msSize.x], (size_t)sSize.x * sizeof(uint32));
    }
}


//////////////////////////////////////////////////////////////////////////
//
// The DIB section's pixels.  Nothing draws on them with GDI (see the
// class), so they are always up to date.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CWinGDICanvas::GetImageOrNull(void)
{
    return GetImage();
}


//////////////////////////////////////////////////////////////////////////
//
// The rectangle includes its right and bottom edges.  An opaque canvas
// takes the colour without its alpha, as GDI's FillRect did, with its
// fourth byte 0; a transparent one takes it multiplied by its opacity.
//
//////////////////////////////////////////////////////////////////////////
void CWinGDICanvas::DrawBox(CInt32Rect* pcRectangle, bool bFilled, ARGB32 sColour)
{
    int32   iLeft;
    int32   iTop;
    int32   iRight;
    int32   iBottom;
    uint32  uiPixel;

    iLeft = pcRectangle->GetLeft();
    iTop = pcRectangle->GetTop();
    iRight = pcRectangle->GetRight() + 1;
    iBottom = pcRectangle->GetBottom() + 1;
    uiPixel = GetPixel(sColour);
    if (bFilled)
    {
        FillPixels(iLeft, iTop, iRight, iBottom, uiPixel);
    }
    else
    {
        FillPixels(iLeft, iTop, iRight, iTop + 1, uiPixel);
        FillPixels(iLeft, iBottom - 1, iRight, iBottom, uiPixel);
        FillPixels(iLeft, iTop + 1, iLeft + 1, iBottom - 1, uiPixel);
        FillPixels(iRight - 1, iTop + 1, iRight, iBottom - 1, uiPixel);
    }
}


//////////////////////////////////////////////////////////////////////////
//
// As DrawBox, one pixel.
//
//////////////////////////////////////////////////////////////////////////
void CWinGDICanvas::DrawPixel(int32 iX, int32 iY, ARGB32 sColour)
{
    FillPixels(iX, iY, iX + 1, iY + 1, GetPixel(sColour));
}


//////////////////////////////////////////////////////////////////////////
//
// Draws the source with its top left at iX, iY, clipped to this canvas
// and to pcClip if there is one.  An opaque source is copied a row at a
// time, all four bytes; a transparent one is blended so what it didn't
// cover shows through.
//
//////////////////////////////////////////////////////////////////////////
void CWinGDICanvas::DrawCanvas(int iX, int iY, CNativeCanvas* pcSource, CInt32Rect* pcClip)
{
    CWinGDICanvas*  pcGDISource;
    CInt32Rect      cDest;
    CInt32Rect      cCanvas;
    int             iSourceX;
    int             iSourceY;
    int             iWidth;
    int             y;

    if (!pcSource)
    {
        return;
    }

    pcGDISource = (CWinGDICanvas*)pcSource;
    if ((pcGDISource->mpuiPixelData == NULL) || (mpuiPixelData == NULL))
    {
        return;
    }

    //Only the part of the source inside the clip is copied.
    cDest.Init(iX, iY, iX + pcGDISource->msSize.x, iY + pcGDISource->msSize.y);
    if (pcClip)
    {
        cDest.ClipTo(pcClip);
    }
    if ((cDest.GetWidth() <= 0) || (cDest.GetHeight() <= 0))
    {
        return;
    }
    if (pcGDISource->mpcCanvas->IsTransparent())
    {
        BlendPremultipliedImage(pcGDISource->GetImage(), GetImage(), iX, iY, &cDest);
        return;
    }

    cCanvas.Init(0, 0, msSize.x, msSize.y);
    cDest.ClipTo(&cCanvas);
    if ((cDest.GetWidth() <= 0) || (cDest.GetHeight() <= 0))
    {
        return;
    }
    iSourceX = cDest.GetLeft() - iX;
    iSourceY = cDest.GetTop() - iY;
    iWidth = cDest.GetWidth();
    for (y = 0; y < cDest.GetHeight(); y++)
    {
        memcpy(&((uint32*)mpuiPixelData)[(cDest.GetTop() + y) * msSize.x + cDest.GetLeft()],
               &((uint32*)pcGDISource->mpuiPixelData)[(iSourceY + y) * pcGDISource->msSize.x + iSourceX],
               (size_t)iWidth * sizeof(uint32));
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDICanvas::DrawCel(int iX, int iY, Ptr<CImageCel> pSource)
{
    if (pSource.IsNull() || GetImage().IsNull())
    {
        return;
    }

    CImageCopier::Copy(pSource, GetImage(), iX, iY);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDICanvas::DrawImage(int iX, int iY, Ptr<CImage> pSource)
{
    if (pSource.IsNull() || GetImage().IsNull())
    {
        return;
    }

    CImageCopier::Copy(pSource, GetImage(), iX, iY);
}


//////////////////////////////////////////////////////////////////////////
//
// The pixel a colour is on this canvas: on an opaque one the colour
// without its alpha and 0 in the fourth byte, as GDI wrote it; on a
// transparent one the colour multiplied by its opacity, which is in the
// fourth byte.  Blue, green, red then the fourth byte in memory.
//
//////////////////////////////////////////////////////////////////////////
uint32 CWinGDICanvas::GetPixel(ARGB32 sColour)
{
    uint32      uiRed;
    uint32      uiGreen;
    uint32      uiBlue;
    uint32      uiAlpha;

    Get32BitColour(&uiRed, &uiGreen, &uiBlue, &uiAlpha, sColour);
    if (!mpcCanvas->IsTransparent())
    {
        return (uiRed << 16) | (uiGreen << 8) | uiBlue;
    }
    uiRed = (uiRed * uiAlpha + 127) / 255;
    uiGreen = (uiGreen * uiAlpha + 127) / 255;
    uiBlue = (uiBlue * uiAlpha + 127) / 255;
    return (uiAlpha << 24) | (uiRed << 16) | (uiGreen << 8) | uiBlue;
}


//////////////////////////////////////////////////////////////////////////
//
// Fills left to right - 1 and top to bottom - 1, clipped to the canvas.
//
//////////////////////////////////////////////////////////////////////////
void CWinGDICanvas::FillPixels(int iLeft, int iTop, int iRight, int iBottom, uint32 uiPixel)
{
    uint32*     puiRow;
    int         x;
    int         y;

    if (mpuiPixelData == NULL)
    {
        return;
    }

    if (iLeft < 0)
    {
        iLeft = 0;
    }
    if (iTop < 0)
    {
        iTop = 0;
    }
    if (iRight > msSize.x)
    {
        iRight = msSize.x;
    }
    if (iBottom > msSize.y)
    {
        iBottom = msSize.y;
    }

    for (y = iTop; y < iBottom; y++)
    {
        puiRow = &((uint32*)mpuiPixelData)[y * msSize.x];
        for (x = iLeft; x < iRight; x++)
        {
            puiRow[x] = uiPixel;
        }
    }
}
