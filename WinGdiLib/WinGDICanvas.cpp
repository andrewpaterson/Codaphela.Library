#include "SupportLib/ImageCopier.h"
#include "SupportLib/ImagePremultipliedBlend.h"
#include "WindowLib/Canvas.h"
#include "WinGDIWindowFactory.h"
#include "WinGDIHelper.h"
#include "WinGDICanvas.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDICanvas::Init(CCanvas* pcCanvas, CNativeWindowFactory* pcWindowFactory)
{
	CNativeCanvas::Init(pcCanvas, pcWindowFactory);
    mhMemDC = NULL;
    mhMemBitmap = NULL;
    mpuiPixelData = NULL;
    mpImage = NULL;
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDICanvas::Kill(void)
{
    //The image references the DIB section memory so it must be released before the bitmap is deleted.
    mpImage = NULL;
    if (mhMemDC)
    {
        DeleteObject(mhMemBitmap);
        DeleteDC(mhMemDC);
        mhMemDC = NULL;
        mhMemBitmap = NULL;
        mpuiPixelData = NULL;
    }
    CNativeCanvas::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWinGDICanvas::CreateNativeCanvas(void)
{
    //This is split from .Init() so that it can fail on its own.

    CWinGDIWindowFactory*   pcFactory; 
    SInt32Vec2              sSize;
    BITMAPINFO              sBitmapInfo;
    HWND                    hWnd;
    HDC                     hDC;

    pcFactory = (CWinGDIWindowFactory*)mpcWindowFactory;
    sSize = mpcCanvas->GetActualSize();

    hWnd = pcFactory->GetHWnd();
    hDC = GetDC(hWnd);

    memset(&sBitmapInfo, 0, sizeof(BITMAPINFO));
    sBitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    sBitmapInfo.bmiHeader.biWidth = sSize.x;
    sBitmapInfo.bmiHeader.biHeight = -sSize.y;
    sBitmapInfo.bmiHeader.biPlanes = 1;
    sBitmapInfo.bmiHeader.biBitCount = 32; // 32-bit ARGB
    sBitmapInfo.bmiHeader.biCompression = BI_RGB;
    
    mhMemDC = CreateCompatibleDC(hDC);
    if (mhMemDC)
    {
        mhMemBitmap = CreateDIBSection(hDC, &sBitmapInfo, DIB_RGB_COLORS, (void**)&mpuiPixelData, NULL, 0);
        if (mhMemBitmap)
        {
            SelectObject(mhMemDC, mhMemBitmap);
            ReleaseDC(hWnd, hDC);

            //A 32 bit BI_RGB DIB section is laid out in memory as B, G, R, X.  A transparent canvas uses X for its opacity
            //and, like the reference canvas, holds its colours multiplied by it.  The DIB section starts all zero; black,
            //and transparent if the canvas is.
            if (mpcCanvas->IsTransparent())
            {
                mpImage = OMalloc<CImage>(sSize.x, sSize.y, (void*)mpuiPixelData, PT_uint8, IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, IMAGE_OPACITY, CHANNEL_STOP);
            }
            else
            {
                mpImage = OMalloc<CImage>(sSize.x, sSize.y, (void*)mpuiPixelData, PT_uint8, IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, IMAGE_IGNORED, CHANNEL_STOP);
            }
            SetSize(sSize.x, sSize.y);
            return true;
        }
        DeleteDC(mhMemDC);
    }
    ReleaseDC(hWnd, hDC);
    mhMemDC = NULL;
    mhMemBitmap = NULL;
    mpuiPixelData = NULL;
    SetSize(-1, -1);
    return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
HDC CWinGDICanvas::GetMemDC(void)
{
    return mhMemDC;
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
//
//////////////////////////////////////////////////////////////////////////
void CWinGDICanvas::CopyCanvas(CNativeCanvas* pcSourceCanvas)
{
    CWinGDICanvas*  pcSourceGDICanvas;
    SInt32Vec2      sSize;
    HDC             hSourceDC;
    HDC             hDestDC;
    HBITMAP         hOldSourceBitmap;
    HBITMAP         hOldDestBitmap;

    if (!pcSourceCanvas)
    {
        return;
    }

    pcSourceGDICanvas = (CWinGDICanvas*)pcSourceCanvas;
    sSize.x = (msSize.x < pcSourceGDICanvas->msSize.x) ? msSize.x : pcSourceGDICanvas->msSize.x;
    sSize.y = (msSize.y < pcSourceGDICanvas->msSize.y) ? msSize.y : pcSourceGDICanvas->msSize.y;
    hSourceDC = pcSourceGDICanvas->mhMemDC;
    hDestDC = mhMemDC;

    hOldSourceBitmap = (HBITMAP)SelectObject(hSourceDC, pcSourceGDICanvas->mhMemBitmap);
    hOldDestBitmap = (HBITMAP)SelectObject(hDestDC, mhMemBitmap);

    BitBlt(hDestDC, 0, 0, sSize.x, sSize.y, hSourceDC, 0, 0, SRCCOPY);

    SelectObject(hSourceDC, hOldSourceBitmap);
    SelectObject(hDestDC, hOldDestBitmap);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CWinGDICanvas::GetImageOrNull(void)
{
    //Any pending GDI drawing must be written to the DIB section before the pixels are accessed directly.
    GdiFlush();
    return mpImage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDICanvas::DrawBox(CInt32Rect* pcRectangle, bool bFilled, ARGB32 sColour)
{
    RECT        sRect;
    HBRUSH      hBrush;
    COLORREF    sRef;

    //The rectangle includes its right and bottom edges but a GDI RECT does not.
    CopyRectangleToGDIRect(&sRect, pcRectangle);
    sRect.right++;
    sRect.bottom++;

    if (mpcCanvas->IsTransparent())
    {
        //GDI would write zero into the opacity so the pixels are written directly.
        if (bFilled)
        {
            FillPremultiplied(sRect.left, sRect.top, sRect.right, sRect.bottom, sColour);
        }
        else
        {
            FillPremultiplied(sRect.left, sRect.top, sRect.right, sRect.top + 1, sColour);
            FillPremultiplied(sRect.left, sRect.bottom - 1, sRect.right, sRect.bottom, sColour);
            FillPremultiplied(sRect.left, sRect.top + 1, sRect.left + 1, sRect.bottom - 1, sColour);
            FillPremultiplied(sRect.right - 1, sRect.top + 1, sRect.right, sRect.bottom - 1, sColour);
        }
        return;
    }

    sRef = RGB(Get8BitRedColour(sColour), Get8BitGreenColour(sColour), Get8BitBlueColour(sColour));
    hBrush = CreateSolidBrush(sRef);
    if (bFilled)
    {
        ::FillRect(mhMemDC, &sRect, hBrush);
    }
    else
    {
        ::FrameRect(mhMemDC, &sRect, hBrush);
    }
    DeleteObject(hBrush);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDICanvas::DrawPixel(int32 iX, int32 iY, ARGB32 sColour)
{
    COLORREF    sRef;

    if (mpcCanvas->IsTransparent())
    {
        FillPremultiplied(iX, iY, iX + 1, iY + 1, sColour);
        return;
    }

    sRef = RGB(Get8BitRedColour(sColour), Get8BitGreenColour(sColour), Get8BitBlueColour(sColour));
    ::SetPixel(mhMemDC, iX, iY, sRef);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDICanvas::DrawCanvas(int iX, int iY, CNativeCanvas* pcSource, CInt32Rect* pcClip)
{
    CWinGDICanvas*  pcGDISource;
    CInt32Rect      cDest;
    int             iSourceX;
    int             iSourceY;

    if (!pcSource)
    {
        return;
    }

    pcGDISource = (CWinGDICanvas*)pcSource;
    if (!pcGDISource->mhMemDC)
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
        //What the transparent canvas didn't cover must show through so it is blended rather than copied.
        GdiFlush();
        BlendPremultipliedImage(pcGDISource->mpImage, mpImage, iX, iY, &cDest);
        return;
    }

    iSourceX = cDest.GetLeft() - iX;
    iSourceY = cDest.GetTop() - iY;

    BitBlt(mhMemDC, cDest.GetLeft(), cDest.GetTop(), cDest.GetWidth(), cDest.GetHeight(), pcGDISource->mhMemDC, iSourceX, iSourceY, SRCCOPY);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDICanvas::DrawCel(int iX, int iY, Ptr<CImageCel> pSource)
{
    if (pSource.IsNull() || mpImage.IsNull())
    {
        return;
    }

    //Writing directly to the DIB section so any batched GDI drawing must complete first.
    GdiFlush();
    CImageCopier::Copy(pSource, mpImage, iX, iY);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDICanvas::DrawImage(int iX, int iY, Ptr<CImage> pSource)
{
    if (pSource.IsNull() || mpImage.IsNull())
    {
        return;
    }

    //Writing directly to the DIB section so any batched GDI drawing must complete first.
    GdiFlush();
    CImageCopier::Copy(pSource, mpImage, iX, iY);
}


//////////////////////////////////////////////////////////////////////////
//
// Fills left to right - 1 and top to bottom - 1, clipped to the canvas,
// with the colour multiplied by its opacity.  Only for transparent
// canvases.
//
//////////////////////////////////////////////////////////////////////////
void CWinGDICanvas::FillPremultiplied(int iLeft, int iTop, int iRight, int iBottom, ARGB32 sColour)
{
    uint32      uiRed;
    uint32      uiGreen;
    uint32      uiBlue;
    uint32      uiAlpha;
    uint32      uiPixel;
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

    Get32BitColour(&uiRed, &uiGreen, &uiBlue, &uiAlpha, sColour);
    uiRed = (uiRed * uiAlpha + 127) / 255;
    uiGreen = (uiGreen * uiAlpha + 127) / 255;
    uiBlue = (uiBlue * uiAlpha + 127) / 255;
    uiPixel = (uiAlpha << 24) | (uiRed << 16) | (uiGreen << 8) | uiBlue;

    //Writing directly to the DIB section so any batched GDI drawing must complete first.
    GdiFlush();
    for (y = iTop; y < iBottom; y++)
    {
        puiRow = &((uint32*)mpuiPixelData)[y * msSize.x];
        for (x = iLeft; x < iRight; x++)
        {
            puiRow[x] = uiPixel;
        }
    }
}
