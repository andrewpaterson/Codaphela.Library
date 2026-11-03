#ifdef _WIN32
#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Objects.h"
#include "SupportLib/ColourARGB32.h"
#include "SupportLib/Image.h"
#include "SupportLib/ImageCel.h"
#include "WindowLib/Window.h"
#include "WindowLib/Canvas.h"
#include "WinGdiLib/WinGDIWindowFactory.h"
#include "WinGdiLib/WinGDICanvas.h"
#include "TestLib/Assert.h"
#include "WaitTestRefWindow.h"


//Pixels as a 32 bit DIB section holds them: blue, green, red then the fourth byte, so 0xXXRRGGBB.
#define GDI_PIXEL(x, r, g, b)	(((uint32)(x) << 24) | ((uint32)(r) << 16) | ((uint32)(g) << 8) | (uint32)(b))


//////////////////////////////////////////////////////////////////////////
//
// A canvas with its DIB section; the window it's in is never shown.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CCanvas> NewGDICanvas(Ptr<CWindow> pWindow, int32 iWidth, int32 iHeight, bool bTransparent)
{
	Ptr<CCanvas>	pCanvas;
	CPointer		pNull;

	pCanvas = OMalloc<CCanvas>(pWindow, pNull);
	pCanvas->SetActualSize(iWidth, iHeight);
	pCanvas->SetTransparent(bTransparent);
	pCanvas->CreateNativeCanvas();
	AssertTrue(pCanvas->GetPixelData() != NULL);
	return pCanvas;
}


//////////////////////////////////////////////////////////////////////////
//
// The pixel as it is in memory once the canvas's drawing has finished.
//
//////////////////////////////////////////////////////////////////////////
static uint32 GetGDIPixel(Ptr<CCanvas> pCanvas, int32 x, int32 y)
{
	pCanvas->GetImageOrNull();
	return ((uint32*)pCanvas->GetPixelData())[y * pCanvas->GetActualSize().x + x];
}


//////////////////////////////////////////////////////////////////////////
//
// Every pixel of the canvas, row by row, as the expected string of
// "XXRRGGBB" separated by spaces.
//
//////////////////////////////////////////////////////////////////////////
static void AssertGDIPixels(const char* szExpected, Ptr<CCanvas> pCanvas)
{
	CChars		sz;
	SInt32Vec2	sSize;
	int32		x;
	int32		y;
	char		szPixel[12];

	sSize = pCanvas->GetActualSize();
	sz.Init();
	for (y = 0; y < sSize.y; y++)
	{
		for (x = 0; x < sSize.x; x++)
		{
			if ((x != 0) || (y != 0))
			{
				sz.Append(" ");
			}
			sprintf(szPixel, "%08X", GetGDIPixel(pCanvas, x, y));
			sz.Append(szPixel);
		}
	}
	AssertString(szExpected, sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CWindow> NewGDITestWindow(CWinGDIWindowFactory* pcFactory)
{
	Ptr<CWindow>	pWindow;
	CPointer		pNull;

	pcFactory->Init(&gcMemoryAllocator, GetModuleHandle(NULL), NULL, SW_HIDE, "TestWinGDICanvas");
	pWindow = OMalloc<CWindow>("GDI Canvas Window", pcFactory, OMalloc<CCountingTick>(0, 0, -1), pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
	ORoot()->Add(pWindow);
	return pWindow;
}


//////////////////////////////////////////////////////////////////////////
//
// Boxes and pixels.  An opaque canvas takes the colour without its alpha
// and its fourth byte is 0.  A transparent canvas holds the colour
// multiplied by its opacity, which is in the fourth byte.  Both are
// clipped to the canvas.
//
//////////////////////////////////////////////////////////////////////////
void TestWinGDICanvasBoxesAndPixels(void)
{
	CWinGDIWindowFactory	cFactory;

	ObjectsInit();
	{
		Ptr<CWindow>	pWindow;
		Ptr<CCanvas>	pCanvas;
		CInt32Rect		cRect;

		pWindow = NewGDITestWindow(&cFactory);
		pCanvas = NewGDICanvas(pWindow, 6, 4, false);
		AssertGDIPixels("00000000 00000000 00000000 00000000 00000000 00000000 "
						"00000000 00000000 00000000 00000000 00000000 00000000 "
						"00000000 00000000 00000000 00000000 00000000 00000000 "
						"00000000 00000000 00000000 00000000 00000000 00000000", pCanvas);

		//The rectangle includes its right and bottom.
		cRect.Init(1, 1, 3, 2);
		pCanvas->DrawBox(&cRect, true, Set32BitColour((uint8)10, (uint8)20, (uint8)30));
		AssertInt(GDI_PIXEL(0, 10, 20, 30), GetGDIPixel(pCanvas, 1, 1));
		AssertInt(GDI_PIXEL(0, 10, 20, 30), GetGDIPixel(pCanvas, 3, 2));
		AssertInt(0, GetGDIPixel(pCanvas, 4, 1));
		AssertInt(0, GetGDIPixel(pCanvas, 1, 3));

		//The alpha of a colour on an opaque canvas is ignored.
		cRect.Init(4, 3, 5, 3);
		pCanvas->DrawBox(&cRect, true, Set32BitColour((uint8)200, (uint8)100, (uint8)50, (uint8)128));
		AssertInt(GDI_PIXEL(0, 200, 100, 50), GetGDIPixel(pCanvas, 4, 3));
		AssertInt(GDI_PIXEL(0, 200, 100, 50), GetGDIPixel(pCanvas, 5, 3));

		//A frame is the edge only.
		cRect.Init(0, 0, 5, 3);
		pCanvas->DrawBox(&cRect, false, Set32BitColour((uint8)1, (uint8)2, (uint8)3));
		AssertGDIPixels("00010203 00010203 00010203 00010203 00010203 00010203 "
						"00010203 000A141E 000A141E 000A141E 00000000 00010203 "
						"00010203 000A141E 000A141E 000A141E 00000000 00010203 "
						"00010203 00010203 00010203 00010203 00010203 00010203", pCanvas);

		//Clipped: only the corners that are on the canvas.
		cRect.Init(-3, -3, 0, 0);
		pCanvas->DrawBox(&cRect, true, Set32BitColour((uint8)7, (uint8)7, (uint8)7));
		cRect.Init(5, 3, 9, 9);
		pCanvas->DrawBox(&cRect, false, Set32BitColour((uint8)8, (uint8)8, (uint8)8));
		AssertInt(GDI_PIXEL(0, 7, 7, 7), GetGDIPixel(pCanvas, 0, 0));
		AssertInt(GDI_PIXEL(0, 1, 2, 3), GetGDIPixel(pCanvas, 1, 0));
		AssertInt(GDI_PIXEL(0, 8, 8, 8), GetGDIPixel(pCanvas, 5, 3));
		AssertInt(GDI_PIXEL(0, 1, 2, 3), GetGDIPixel(pCanvas, 4, 3));

		pCanvas->DrawPixel(2, 0, Set32BitColour((uint8)9, (uint8)8, (uint8)7));
		pCanvas->DrawPixel(-1, 0, Set32BitColour((uint8)9, (uint8)8, (uint8)7));
		pCanvas->DrawPixel(6, 0, Set32BitColour((uint8)9, (uint8)8, (uint8)7));
		pCanvas->DrawPixel(0, 4, Set32BitColour((uint8)9, (uint8)8, (uint8)7));
		pCanvas->DrawPixel(3, 3, Set32BitColour((uint8)9, (uint8)8, (uint8)7, (uint8)1));
		AssertInt(GDI_PIXEL(0, 9, 8, 7), GetGDIPixel(pCanvas, 2, 0));
		AssertInt(GDI_PIXEL(0, 9, 8, 7), GetGDIPixel(pCanvas, 3, 3));
		AssertInt(GDI_PIXEL(0, 1, 2, 3), GetGDIPixel(pCanvas, 0, 3));

		//Transparent: premultiplied, with the opacity in the fourth byte.
		pCanvas = NewGDICanvas(pWindow, 4, 2, true);
		cRect.Init(0, 0, 1, 0);
		pCanvas->DrawBox(&cRect, true, Set32BitColour((uint8)255, (uint8)0, (uint8)0, (uint8)128));
		pCanvas->DrawPixel(3, 1, Set32BitColour((uint8)0, (uint8)0, (uint8)255));
		cRect.Init(1, 0, 3, 1);
		pCanvas->DrawBox(&cRect, false, Set32BitColour((uint8)0, (uint8)255, (uint8)0, (uint8)64));
		AssertGDIPixels("80800000 40004000 40004000 40004000 "
						"00000000 40004000 40004000 40004000", pCanvas);

		pCanvas = NULL;
		ORoot()->Remove(pWindow);
		pWindow = NULL;
	}
	AssertTrue(ORoot()->RemoveAll());
	cFactory.Kill();
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// An opaque canvas drawn on another is copied, all four bytes, clipped to
// the destination and to the clip.  A transparent one is blended: its
// premultiplied colours over the destination's.  CopyCanvas copies as
// much of the source as fits from the top left.
//
//////////////////////////////////////////////////////////////////////////
void TestWinGDICanvasComposites(void)
{
	CWinGDIWindowFactory	cFactory;

	ObjectsInit();
	{
		Ptr<CWindow>	pWindow;
		Ptr<CCanvas>	pSource;
		Ptr<CCanvas>	pDest;
		Ptr<CCanvas>	pTransparent;
		CInt32Rect		cRect;
		CInt32Rect		cClip;

		pWindow = NewGDITestWindow(&cFactory);

		//A three by two source, its right column another colour.
		pSource = NewGDICanvas(pWindow, 3, 2, false);
		cRect.Init(0, 0, 2, 1);
		pSource->DrawBox(&cRect, true, Set32BitColour((uint8)1, (uint8)1, (uint8)1));
		cRect.Init(2, 0, 2, 1);
		pSource->DrawBox(&cRect, true, Set32BitColour((uint8)2, (uint8)2, (uint8)2));

		pDest = NewGDICanvas(pWindow, 5, 3, false);
		cRect.Init(0, 0, 4, 2);
		pDest->DrawBox(&cRect, true, Set32BitColour((uint8)9, (uint8)9, (uint8)9));
		pDest->DrawCanvas(2, 1, pSource);
		AssertGDIPixels("00090909 00090909 00090909 00090909 00090909 "
						"00090909 00090909 00010101 00010101 00020202 "
						"00090909 00090909 00010101 00010101 00020202", pDest);

		//Off the top left, and clipped.
		pDest->DrawCanvas(-1, -1, pSource);
		cClip.Init(4, 0, 5, 3);
		pDest->DrawCanvas(3, 0, pSource, &cClip);
		AssertGDIPixels("00010101 00020202 00090909 00090909 00010101 "
						"00090909 00090909 00010101 00010101 00010101 "
						"00090909 00090909 00010101 00010101 00020202", pDest);

		//Half red (premultiplied) and nothing, blended over 10, 20, 30: 128 + 10 * 127 / 255...
		pTransparent = NewGDICanvas(pWindow, 2, 1, true);
		cRect.Init(0, 0, 0, 0);
		pTransparent->DrawBox(&cRect, true, Set32BitColour((uint8)255, (uint8)0, (uint8)0, (uint8)128));
		cRect.Init(0, 0, 4, 2);
		pDest->DrawBox(&cRect, true, Set32BitColour((uint8)10, (uint8)20, (uint8)30));
		pDest->DrawCanvas(1, 1, pTransparent);
		AssertInt(GDI_PIXEL(0, 133, 10, 15), GetGDIPixel(pDest, 1, 1));
		AssertInt(GDI_PIXEL(0, 10, 20, 30), GetGDIPixel(pDest, 2, 1));

		//As much as fits, from the top left.
		pDest->CopyCanvas(&pSource);
		AssertGDIPixels("00010101 00010101 00020202 000A141E 000A141E "
						"00010101 00010101 00020202 000A141E 000A141E "
						"000A141E 000A141E 000A141E 000A141E 000A141E", pDest);

		pTransparent = NULL;
		pDest = NULL;
		pSource = NULL;
		ORoot()->Remove(pWindow);
		pWindow = NULL;
	}
	AssertTrue(ORoot()->RemoveAll());
	cFactory.Kill();
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// Images and cels are copied onto the canvas.
//
//////////////////////////////////////////////////////////////////////////
void TestWinGDICanvasImages(void)
{
	CWinGDIWindowFactory	cFactory;

	ObjectsInit();
	{
		Ptr<CWindow>	pWindow;
		Ptr<CCanvas>	pCanvas;
		Ptr<CImage>		pImage;
		Ptr<CImageCel>	pCel;
		uint8*			puiData;
		int				i;

		pWindow = NewGDITestWindow(&cFactory);
		pCanvas = NewGDICanvas(pWindow, 4, 2, false);

		//Red, green, blue, white.
		pImage = OMalloc<CImage>(2, 2, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		puiData = (uint8*)pImage->GetData();
		for (i = 0; i < 12; i++)
		{
			puiData[i] = 0;
		}
		puiData[0] = 255;
		puiData[4] = 255;
		puiData[8] = 255;
		puiData[9] = 255;
		puiData[10] = 255;
		puiData[11] = 255;
		pCanvas->DrawImage(1, 0, pImage);
		AssertGDIPixels("00000000 00FF0000 0000FF00 00000000 "
						"00000000 000000FF 00FFFFFF 00000000", pCanvas);

		//Its right column.
		pCel = OMalloc<CImageCel>(pImage, 1, 0, 2, 2);
		((CWinGDICanvas*)pCanvas->GetNativeCanvas())->DrawCel(3, 0, pCel);
		AssertGDIPixels("00000000 00FF0000 0000FF00 0000FF00 "
						"00000000 000000FF 00FFFFFF 00FFFFFF", pCanvas);

		pCel = NULL;
		pImage = NULL;
		pCanvas = NULL;
		ORoot()->Remove(pWindow);
		pWindow = NULL;
	}
	AssertTrue(ORoot()->RemoveAll());
	cFactory.Kill();
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// Boxes, pixels and canvases drawn in turn with images over the same
// pixels: each is over what was drawn before it, whatever draws it.
// Repeated, as a race between them would only show some of the time.
//
//////////////////////////////////////////////////////////////////////////
void TestWinGDICanvasInOrder(void)
{
	CWinGDIWindowFactory	cFactory;

	ObjectsInit();
	{
		Ptr<CWindow>	pWindow;
		Ptr<CCanvas>	pCanvas;
		Ptr<CCanvas>	pYellow;
		Ptr<CImage>		pGreen;
		Ptr<CImage>		pBlack;
		CInt32Rect		cRect;
		uint8*			puiData;
		int				i;
		bool			bInOrder;

		pWindow = NewGDITestWindow(&cFactory);
		pCanvas = NewGDICanvas(pWindow, 64, 64, false);
		pYellow = NewGDICanvas(pWindow, 64, 1, false);
		cRect.Init(0, 0, 63, 0);
		pYellow->DrawBox(&cRect, true, Set32BitColour((uint8)255, (uint8)255, (uint8)0));
		pGreen = OMalloc<CImage>(64, 64, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		puiData = (uint8*)pGreen->GetData();
		for (i = 0; i < 64 * 64 * 3; i++)
		{
			puiData[i] = ((i % 3) == 1) ? 255 : 0;
		}
		pBlack = OMalloc<CImage>(1, 1, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		puiData = (uint8*)pBlack->GetData();
		puiData[0] = 0;
		puiData[1] = 0;
		puiData[2] = 0;

		bInOrder = true;
		for (i = 0; i < 200; i++)
		{
			cRect.Init(0, 0, 63, 63);
			pCanvas->DrawBox(&cRect, true, Set32BitColour((uint8)255, (uint8)0, (uint8)0));
			pCanvas->DrawImage(0, 0, pGreen);
			cRect.Init(0, 0, 31, 63);
			pCanvas->DrawBox(&cRect, true, Set32BitColour((uint8)0, (uint8)0, (uint8)255));
			pCanvas->DrawPixel(40, 10, Set32BitColour((uint8)255, (uint8)255, (uint8)255));
			pCanvas->DrawCanvas(0, 20, pYellow);
			pCanvas->DrawImage(10, 20, pBlack);

			bInOrder &= (GetGDIPixel(pCanvas, 50, 50) == GDI_PIXEL(0, 0, 255, 0));
			bInOrder &= (GetGDIPixel(pCanvas, 10, 50) == GDI_PIXEL(0, 0, 0, 255));
			bInOrder &= (GetGDIPixel(pCanvas, 40, 10) == GDI_PIXEL(0, 255, 255, 255));
			bInOrder &= (GetGDIPixel(pCanvas, 50, 20) == GDI_PIXEL(0, 255, 255, 0));
			bInOrder &= (GetGDIPixel(pCanvas, 10, 20) == GDI_PIXEL(0, 0, 0, 0));
		}
		AssertTrue(bInOrder);

		pBlack = NULL;
		pGreen = NULL;
		pYellow = NULL;
		pCanvas = NULL;
		ORoot()->Remove(pWindow);
		pWindow = NULL;
	}
	AssertTrue(ORoot()->RemoveAll());
	cFactory.Kill();
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// The Windows canvas, a DIB section.
//
//////////////////////////////////////////////////////////////////////////
void TestWinGDICanvas(void)
{
	BeginTests();

	TestWinGDICanvasBoxesAndPixels();
	TestWinGDICanvasComposites();
	TestWinGDICanvasImages();
	TestWinGDICanvasInOrder();

	TestStatistics();
}
#endif // _WIN32
