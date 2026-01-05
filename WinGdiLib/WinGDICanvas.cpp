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
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDICanvas::Kill(void)
{
    if (mhMemDC)
    {
        DeleteObject(mhMemBitmap);
        DeleteDC(mhMemDC);
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWinGDICanvas::CreateNativeCanvas(void)
{
    //This is split from .Init() so that it can fail on its own.

    CWinGDIWindowFactory*   pcFactory; 
    SInt2                   sSize;
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
            return true;
        }
    }
    mhMemDC = NULL;
    mhMemBitmap = NULL;
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
    SInt2           sSize;
    HDC             hSourceDC;
    HDC             hDestDC;
    HBITMAP         hOldSourceBitmap;
    HBITMAP         hOldDestBitmap;

    if (!pcSourceCanvas)
    {
        return;
    }

    pcSourceGDICanvas = (CWinGDICanvas*)pcSourceCanvas;
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
}

