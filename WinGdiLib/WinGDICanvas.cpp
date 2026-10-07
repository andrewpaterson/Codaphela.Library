#include "SupportLib/ImageCopier.h"
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

            //A 32 bit BI_RGB DIB section is laid out in memory as B, G, R, X.
            mpImage = OMalloc<CImage>(sSize.x, sSize.y, (void*)mpuiPixelData, PT_uint8, IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, IMAGE_IGNORED, CHANNEL_STOP);
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
void CWinGDICanvas::DrawBox(CRectangle* pcRectangle, bool bFilled, ARGB32 sColour)
{
    RECT        sRect;
    HBRUSH      hBrush;
    COLORREF    sRef;

    CopyRectangleToGDIRect(&sRect, pcRectangle);

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

    sRef = RGB(Get8BitRedColour(sColour), Get8BitGreenColour(sColour), Get8BitBlueColour(sColour));
    ::SetPixel(mhMemDC, iX, iY, sRef);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDICanvas::DrawCanvas(int iX, int iY, CNativeCanvas* pcSource)
{
    CWinGDICanvas*  pcGDISource;

    if (!pcSource)
    {
        return;
    }

    pcGDISource = (CWinGDICanvas*)pcSource;
    if (!pcGDISource->mhMemDC)
    {
        return;
    }

    BitBlt(mhMemDC, iX, iY, pcGDISource->msSize.x, pcGDISource->msSize.y, pcGDISource->mhMemDC, 0, 0, SRCCOPY);
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

