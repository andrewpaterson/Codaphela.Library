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
    mhMemDC = NULL;
    mhMemBitmap = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWinGDICanvas::CreateNativeCanvas()
{
    CWinGDIWindowFactory*   pcFactory; 
    BITMAPINFO              sBitmapInfo;
    HWND                    hWnd;
    HDC                     hDC;

    pcFactory = (CWinGDIWindowFactory*)mpcWindowFactory;

    hWnd = pcFactory->GetHWnd();
    hDC = GetDC(hWnd);

    memset(&sBitmapInfo, 0, sizeof(BITMAPINFO));
    sBitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    sBitmapInfo.bmiHeader.biWidth = mpcCanvas->GetWidth();
    sBitmapInfo.bmiHeader.biHeight = -mpcCanvas->GetHeight();
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
bool CWinGDICanvas::DestroyNativeCanvas(void)
{
    if (mhMemDC)
    {
        DeleteObject(mhMemBitmap);
        DeleteDC(mhMemDC);
        return true;
    }
    else
    {
        return false;
    }
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
void CWinGDICanvas::CopyRect(CNativeCanvas* pcSourceCanvas)
{
    CWinGDICanvas*  pcSourceGDICanvas;

    pcSourceGDICanvas = (CWinGDICanvas*)pcSourceCanvas;
    
    // Create device contexts for source and destination
    HDC hSourceDC = pcSourceGDICanvas->mhMemDC;
    HDC hDestDC = mhMemDC;

    // Select DIB sections into their respective DCs
    HBITMAP hOldSourceBitmap = (HBITMAP)SelectObject(hSourceDC, pcSourceGDICanvas->mhMemBitmap);
    HBITMAP hOldDestBitmap = (HBITMAP)SelectObject(hDestDC, mhMemBitmap);

    // Copy the source DIB to the destination DIB using BitBlt
    BitBlt(hDestDC, 0, 0, mpcCanvas->GetWidth(), mpcCanvas->GetHeight(), hSourceDC, 0, 0, SRCCOPY);

    // Clean up: Restore original bitmaps and delete DCs
    SelectObject(hSourceDC, hOldSourceBitmap);
    SelectObject(hDestDC, hOldDestBitmap);
}


