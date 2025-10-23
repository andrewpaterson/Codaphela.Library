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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWinGDICanvas::CreateNativeCanvas()
{
    CWinGDIWindowFactory* pcFactory; 

    pcFactory = (CWinGDIWindowFactory*)mpcWindowFactory;

    HDC hdc = GetDC(pcFactory->GetHWnd());

    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = mpcCanvas->GetWidth();
    bmi.bmiHeader.biHeight = -mpcCanvas->GetHeight();
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32; // 32-bit ARGB
    bmi.bmiHeader.biCompression = BI_RGB;
    uint32_t* g_pixelBuffer = NULL;
    HDC g_memDC = CreateCompatibleDC(hdc);
    HBITMAP g_memBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)&g_pixelBuffer, NULL, 0);

	return false;
}

