#include <math.h>
#include <string.h>
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "StandardLib/ArrayObject.h"
#include "SupportLib/ImageReader.h"
#include "SupportLib/ImageWriter.h"
#include "SupportLib/ImageDraw.h"
#include "SupportLib/ImageCel.h"
#include "SupportLib/ImageCelBlitterCache.h"
#include "SupportLib/ImageBlitterContext.h"
#include "SupportLib/ImageColourConverter.h"
#include "BaseLib/Logger.h"
#include "BaseLib/Numbers.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDrawBox1(void)
{
	CImageDraw			cDraw;
	Ptr<CImage>			pImage = OMalloc<CImage>(32, 32);
	CImageColourRGB		cColour;

	pImage->Black();

	cDraw.Init(&pImage);

	cColour.Init(0, 0, 0);
	cDraw.SetColour(&cColour);
	cColour.Kill();
	cDraw.DrawBox(0, 0, 31, 31, true);

	cColour.Init(1, 1, 1);
	cDraw.SetColour(&cColour);
	cColour.Kill();
	cDraw.DrawBox(0, 0, 31, 31, false);

	cDraw.Kill();
	WriteImage(pImage, "Output" _FS_ "DrawBox1.raw");
	WriteImage(pImage, "Output" _FS_ "DrawBox1.png");
	AssertFileMemory("Input" _FS_ "DrawBox1.raw", pImage->GetData(), pImage->GetByteSize());

	pImage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDrawBox2(void)
{
	CImageDraw			cDraw;
	Ptr<CImage>			pImage = OMalloc<CImage>(32, 32);
	CImageColourRGB		cColour;
	uint32				i;

	pImage->Black();

	cDraw.Init(&pImage);

	for (i = 0; i < 6; i++)
	{
		cColour.Init(1.0f, 0.7f + (i / 20.0f) * 1.05263162f, 0.4f + (i / 10.0f) * 1.11111116f);
		cDraw.SetColour(&cColour);
		cDraw.DrawBox(8 + i, 10 + i, 23 - i, 21 - i, false);
		cColour.Kill();
	}

	cDraw.Kill();
	WriteImage(pImage, "Output" _FS_ "DrawBox2.raw");
	WriteImage(pImage, "Output" _FS_ "DrawBox2.png");
	AssertFileMemory("Input" _FS_ "DrawBox2.raw", pImage->GetData(), pImage->GetByteSize());

	pImage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDrawLine(void)
{
	CImageDraw			cDraw;
	Ptr<CImage>			pImage = OMalloc<CImage>(64, 32);
	CImageColourRGB		cColour;

	pImage->Black();

	cDraw.Init(&pImage);
	cColour.Init(1.0f, 1, 1);
	cDraw.SetColour(&cColour);

	cDraw.DrawLine(0, 0, 63, 31);
	cDraw.DrawLine(0, 0, 63, 16);
	cDraw.DrawLine(0, 0, 31, 31);
	cDraw.DrawLine(0, 0, 63, 0);
	cDraw.DrawLine(0, 0, 0, 31);

	WriteImage(pImage, "Output" _FS_ "DrawLine1.raw");
	WriteImage(pImage, "Output" _FS_ "DrawLine1.png");
	AssertFileMemory("Input" _FS_ "DrawLine1.raw", pImage->GetData(), pImage->GetByteSize());

	pImage->Black();

	cColour.Init(0.4f, 0.9f, 0.4f);
	cDraw.SetColour(&cColour);

	cDraw.DrawLine(63, 31, 0, 0);
	cDraw.DrawLine(63, 16, 0, 0);
	cDraw.DrawLine(31, 31, 0, 0);
	cDraw.DrawLine(63, 0 , 0, 0);
	cDraw.DrawLine(0, 31 , 0, 0);

	WriteImage(pImage, "Output" _FS_ "DrawLine2.raw");
	WriteImage(pImage, "Output" _FS_ "DrawLine2.png");
	AssertFileMemory("Input" _FS_ "DrawLine2.raw", pImage->GetData(), pImage->GetByteSize());

	pImage->Black();

	cColour.Init(0.4f, 0.4f, 0.99f);
	cDraw.SetColour(&cColour);

	cDraw.DrawLine(63, 0, 0, 31);
	cDraw.DrawLine(63, 0, 0, 16);
	cDraw.DrawLine(31, 0, 0, 31);
	cDraw.DrawLine(63, 0, 0, 0);
	cDraw.DrawLine(0, 0, 0, 31);

	WriteImage(pImage, "Output" _FS_ "DrawLine3.raw");
	WriteImage(pImage, "Output" _FS_ "DrawLine3.png");
	AssertFileMemory("Input" _FS_ "DrawLine3.raw", pImage->GetData(), pImage->GetByteSize());

	cDraw.Kill();
	cColour.Kill();
	pImage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The image is cleared then the semicircle is drawn in magenta.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> DrawSemicircleImage(int iWidth, int iHeight, int x, int y, int iDiameter, int iStartAngle, int iEndAngle, float32 fOpacity, bool bFilled)
{
	CImageDraw			cDraw;
	Ptr<CImage>			pImage;
	CImageColourARGB	cColour;

	pImage = OMalloc<CImage>(iWidth, iHeight, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP);
	pImage->Black();

	cDraw.Init(&pImage);
	cColour.Init(fOpacity, 1.0f, 0.0f, 1.0f);
	cDraw.SetColour(&cColour);
	cColour.Kill();
	AssertTrue(cDraw.DrawSemicircle(x, y, iDiameter, iStartAngle, iEndAngle, bFilled));
	cDraw.Kill();
	return pImage;
}


//////////////////////////////////////////////////////////////////////////
//
// The image's channels are red, green, blue then opacity.
//
//////////////////////////////////////////////////////////////////////////
uint8 GetCircleChannel(Ptr<CImage> pImage, int x, int y, int iChannel)
{
	uint8*	puiData;

	puiData = (uint8*)pImage->GetData();
	return puiData[(y * pImage->GetWidth() + x) * 4 + iChannel];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 GetCircleOpacity(Ptr<CImage> pImage, int x, int y)
{
	return GetCircleChannel(pImage, x, y, 3);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDrawSemicircle(void)
{
	Ptr<CImage>			pLeft;
	Ptr<CImage>			pRight;
	Ptr<CImage>			pTop;
	Ptr<CImage>			pBottom;
	Ptr<CImage>			pImage;
	CImageDraw			cDraw;
	CImageColourRGB		cColour;
	SLogConfig			sLogConfig;
	int					x;
	int					y;
	bool				bSymmetric;
	uint8				uiOpacity;

	//The left half of an even diameter circle: its centre is on the corner between pixels in the middle of the image's
	//right side.
	pLeft = DrawSemicircleImage(4, 8, 0, 0, 8, 90, 270, 1.0f, true);

	//Inside the circle is the colour, outside is untouched and the edge is between.  The colour isn't multiplied by
	//the edge's opacity.
	AssertInt(255, GetCircleOpacity(pLeft, 3, 3));
	AssertInt(255, GetCircleOpacity(pLeft, 3, 4));
	AssertInt(255, GetCircleChannel(pLeft, 3, 3, 0));
	AssertInt(0, GetCircleChannel(pLeft, 3, 3, 1));
	AssertInt(255, GetCircleChannel(pLeft, 3, 3, 2));
	AssertInt(0, GetCircleOpacity(pLeft, 0, 0));
	AssertInt(0, GetCircleChannel(pLeft, 0, 0, 0));
	uiOpacity = GetCircleOpacity(pLeft, 0, 3);
	AssertTrue((uiOpacity > 0) && (uiOpacity < 255));
	AssertInt(255, GetCircleChannel(pLeft, 0, 3, 0));

	//The half circle is symmetric about its middle.
	bSymmetric = true;
	for (y = 0; y < 8; y++)
	{
		for (x = 0; x < 4; x++)
		{
			bSymmetric &= GetCircleOpacity(pLeft, x, y) == GetCircleOpacity(pLeft, x, 7 - y);
		}
	}
	AssertTrue(bSymmetric);

	//The other halves are the left half mirrored and turned.  The right half's circle starts left of the image and an
	//end angle before the start goes on round through 0.
	pRight = DrawSemicircleImage(4, 8, -4, 0, 8, 270, 90, 1.0f, true);
	pTop = DrawSemicircleImage(8, 4, 0, 0, 8, 0, 180, 1.0f, true);
	pBottom = DrawSemicircleImage(8, 4, 0, -4, 8, 180, 360, 1.0f, true);
	bSymmetric = true;
	for (y = 0; y < 8; y++)
	{
		for (x = 0; x < 4; x++)
		{
			bSymmetric &= GetCircleOpacity(pRight, x, y) == GetCircleOpacity(pLeft, 3 - x, y);
			bSymmetric &= GetCircleOpacity(pTop, y, x) == GetCircleOpacity(pLeft, x, y);
			bSymmetric &= GetCircleOpacity(pBottom, y, x) == GetCircleOpacity(pLeft, 3 - x, y);
		}
	}
	AssertTrue(bSymmetric);

	//The top left quarter is the top of the left half and nothing else.
	pImage = DrawSemicircleImage(8, 8, 0, 0, 8, 90, 180, 1.0f, true);
	bSymmetric = true;
	for (y = 0; y < 8; y++)
	{
		for (x = 0; x < 8; x++)
		{
			if ((x < 4) && (y < 4))
			{
				bSymmetric &= GetCircleOpacity(pImage, x, y) == GetCircleOpacity(pLeft, x, y);
			}
			else
			{
				bSymmetric &= GetCircleOpacity(pImage, x, y) == 0;
			}
		}
	}
	AssertTrue(bSymmetric);

	//More than half: three quarters leaves out the bottom right.  Angles can be negative.
	pImage = DrawSemicircleImage(8, 8, 0, 0, 8, -360, -90, 1.0f, true);
	bSymmetric = true;
	for (y = 0; y < 8; y++)
	{
		for (x = 0; x < 4; x++)
		{
			bSymmetric &= GetCircleOpacity(pImage, x, y) == GetCircleOpacity(pLeft, x, y);
			if (y < 4)
			{
				bSymmetric &= GetCircleOpacity(pImage, 7 - x, y) == GetCircleOpacity(pLeft, x, y);
			}
			else
			{
				bSymmetric &= GetCircleOpacity(pImage, 7 - x, y) == 0;
			}
		}
	}
	AssertTrue(bSymmetric);

	//An odd diameter: the centre is in the middle of a pixel.  A whole circle clipped by the image keeps all of its
	//middle column.
	pImage = DrawSemicircleImage(7, 7, 0, 0, 7, 0, 360, 1.0f, true);
	AssertInt(255, GetCircleOpacity(pImage, 3, 3));
	uiOpacity = GetCircleOpacity(pImage, 0, 3);
	AssertTrue((uiOpacity > 0) && (uiOpacity < 255));
	bSymmetric = true;
	for (y = 0; y < 7; y++)
	{
		for (x = 0; x < 7; x++)
		{
			bSymmetric &= GetCircleOpacity(pImage, x, y) == GetCircleOpacity(pImage, 6 - x, y);
			bSymmetric &= GetCircleOpacity(pImage, x, y) == GetCircleOpacity(pImage, x, 6 - y);
		}
	}
	AssertTrue(bSymmetric);
	pTop = DrawSemicircleImage(4, 7, 0, 0, 7, 0, 360, 1.0f, true);
	for (y = 0; y < 7; y++)
	{
		for (x = 0; x < 4; x++)
		{
			bSymmetric &= GetCircleOpacity(pTop, x, y) == GetCircleOpacity(pImage, x, y);
		}
	}
	AssertTrue(bSymmetric);

	//The colour's opacity scales every pixel's.
	pImage = DrawSemicircleImage(4, 8, 0, 0, 8, 90, 270, 0.5f, true);
	uiOpacity = GetCircleOpacity(pImage, 3, 3);
	AssertTrue((uiOpacity == 127) || (uiOpacity == 128));
	AssertTrue(GetCircleOpacity(pImage, 0, 3) < GetCircleOpacity(pLeft, 0, 3));

	//Not filled: only the curved edge, one pixel thick, is drawn.  The middle and the flat side are left alone.
	pImage = DrawSemicircleImage(4, 8, 0, 0, 8, 90, 270, 1.0f, false);
	AssertInt(0, GetCircleOpacity(pImage, 3, 3));
	AssertInt(0, GetCircleOpacity(pImage, 3, 4));
	AssertInt(0, GetCircleOpacity(pImage, 0, 0));
	AssertTrue(GetCircleOpacity(pImage, 0, 3) > 0);
	AssertTrue(GetCircleOpacity(pImage, 3, 0) > 0);
	bSymmetric = true;
	for (y = 0; y < 8; y++)
	{
		for (x = 0; x < 4; x++)
		{
			bSymmetric &= GetCircleOpacity(pImage, x, y) == GetCircleOpacity(pImage, x, 7 - y);
			bSymmetric &= GetCircleOpacity(pImage, x, y) <= GetCircleOpacity(pLeft, x, y);
		}
	}
	AssertTrue(bSymmetric);

	//Drawing over what is already there blends.  The same colour over itself stays the same colour and its edges get
	//more opaque.
	pImage = DrawSemicircleImage(4, 8, 0, 0, 8, 90, 270, 1.0f, true);
	cDraw.Init(&pImage);
	cColour.Init(1.0f, 0.0f, 1.0f);
	cDraw.SetColour(&cColour);
	cColour.Kill();
	AssertTrue(cDraw.DrawSemicircle(0, 0, 8, 90, 270, true));
	cDraw.Kill();
	AssertInt(255, GetCircleOpacity(pImage, 3, 3));
	AssertInt(255, GetCircleChannel(pImage, 0, 3, 0));
	AssertInt(0, GetCircleChannel(pImage, 0, 3, 1));
	AssertInt(255, GetCircleChannel(pImage, 0, 3, 2));
	AssertTrue(GetCircleOpacity(pImage, 0, 3) > GetCircleOpacity(pLeft, 0, 3));

	//A smaller circle in a different colour mixes with the one under it along its edge.
	cDraw.Init(&pImage);
	cColour.Init(0.0f, 1.0f, 0.0f);
	cDraw.SetColour(&cColour);
	cColour.Kill();
	AssertTrue(cDraw.DrawSemicircle(2, 2, 4, 90, 270, true));
	cDraw.Kill();
	AssertInt(255, GetCircleOpacity(pImage, 3, 3));
	AssertInt(0, GetCircleChannel(pImage, 3, 3, 0));
	AssertInt(255, GetCircleChannel(pImage, 3, 3, 1));
	AssertInt(255, GetCircleOpacity(pImage, 2, 3));
	AssertTrue(GetCircleChannel(pImage, 2, 3, 0) > 0);
	AssertTrue(GetCircleChannel(pImage, 2, 3, 1) > 0);
	AssertInt(255, GetCircleChannel(pImage, 1, 3, 0));
	AssertInt(0, GetCircleChannel(pImage, 1, 3, 1));

	//An image without opacity can't show the anti-aliased edge.
	pImage = OMalloc<CImage>(4, 8, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	cDraw.Init(&pImage);
	cColour.Init(1.0f, 1.0f, 1.0f);
	cDraw.SetColour(&cColour);
	cColour.Kill();
	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cDraw.DrawSemicircle(0, 0, 8, 90, 270, true));
	gcLogger.SetConfig(&sLogConfig);
	cDraw.Kill();

	pImage = NULL;
	pLeft = NULL;
	pRight = NULL;
	pTop = NULL;
	pBottom = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDrawLineExtents(void)
{
	CImageDraw			cDraw;
	Ptr<CImage>			pImage = OMalloc<CImage>(16, 16);
	CImageColourRGB		cColour1;
	CImageColourRGB		cColour2;
	int32				x;
	int32				y;

	pImage->Black();

	cDraw.Init(&pImage);
	cColour1.Init(1.0f, 0.1f, 0.1f);
	cColour2.Init(0.1f, 0.1f, 1.0f);

	for (x = 0; x < 16; x += 2)
	{
		cDraw.SetColour(&cColour1);
		cDraw.DrawLine(x, 0, x, 1);
		cDraw.SetColour(&cColour2);
		cDraw.DrawVerticalLine(x+1, 0, 1);
	}

	WriteImage(pImage, "Output" _FS_ "DrawLine4.raw");
	WriteImage(pImage, "Output" _FS_ "DrawLine4.png");
	AssertFileMemory("Input" _FS_ "DrawLine4.raw", pImage->GetData(), pImage->GetByteSize());

	pImage->Black();

	for (y = 0; y < 16; y += 2)
	{
		cDraw.SetColour(&cColour1);
		cDraw.DrawLine(15, y, 16, y);
		cDraw.SetColour(&cColour2);
		cDraw.DrawHorizontalLine(15, 16, y + 1);
	}

	WriteImage(pImage, "Output" _FS_ "DrawLine5.raw");
	WriteImage(pImage, "Output" _FS_ "DrawLine5.png");
	AssertFileMemory("Input" _FS_ "DrawLine5.raw", pImage->GetData(), pImage->GetByteSize());

	cDraw.Kill();
	cColour1.Kill();
	cColour2.Kill();
	pImage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CreateDrawCelImage(int32 iWidth, int32 iHeight, EColourFormat eFormat, ERGBAlphaBits eAlphaBits, uint32 uiStride, bool bGaps)
{
	Ptr<CImage>		pImage;
	uint8*			puiData;
	int32			x;
	int32			y;
	uint32			uiPixel;

	//Every pixel is different so a pixel drawn in the wrong place is always noticed.
	//With gaps the fourth byte is opacity in runs of two opaque and two transparent pixels that shift on each row.
	pImage = OMalloc<CImage>(iWidth, iHeight, eFormat, CCO_RGB, CRGB_24bit, eAlphaBits);
	puiData = (uint8*)pImage->GetData();
	for (y = 0; y < iHeight; y++)
	{
		for (x = 0; x < iWidth; x++)
		{
			uiPixel = (x + y * iWidth) * uiStride;
			puiData[uiPixel + 0] = (uint8)(10 + x * 30);
			puiData[uiPixel + 1] = (uint8)(20 + y * 40);
			puiData[uiPixel + 2] = 200;
			if (uiStride == 4)
			{
				puiData[uiPixel + 3] = bGaps ? ((((x + y) / 2) % 2 == 0) ? 255 : 0) : 77;
			}
		}
	}
	return pImage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CreateDrawCelSourceImage(int32 iWidth, int32 iHeight)
{
	return CreateDrawCelImage(iWidth, iHeight, CFT_RGB, ARGB_None, 3, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertDrawCelPixels(Ptr<CImage> pDest, uint32 uiDestStride, Ptr<CImage> pSource, uint32 uiSourceStride, bool bSourceGaps, int32 iDestX, int32 iDestY, CInt32Rect* pcClip)
{
	uint8*	puiDest;
	uint8*	puiSource;
	int32	x;
	int32	y;
	int32	xs;
	int32	ys;
	bool	bDrawn;
	uint32	uiDest;
	uint32	uiSource;
	uint32	uiWrong;
	int32	iDestHeight;
	int32	iDestWidth;
	int32	iSourceHeight;
	int32	iSourceWidth;

	//A destination pixel must hold the source pixel over it if it is inside the clip and opaque, otherwise it must still be black.
	//Only the colour bytes are compared.
	puiDest = (uint8*)pDest->GetData();
	puiSource = (uint8*)pSource->GetData();
	uiWrong = 0;
	iDestHeight = (int32)pDest->GetImageHeight();
	iDestWidth = (int32)pDest->GetImageWidth();
	iSourceHeight = (int32)pSource->GetImageHeight();
	iSourceWidth = (int32)pSource->GetImageWidth();
	for (y = 0; y < iDestHeight; y++)
	{
		for (x = 0; x < iDestWidth; x++)
		{
			xs = x - iDestX;
			ys = y - iDestY;
			uiSource = 0;
			bDrawn = (xs >= 0) && (xs < iSourceWidth) && (ys >= 0) && (ys < iSourceHeight);
			bDrawn &= (pcClip == NULL) || ((x >= pcClip->msTopLeft.x) && (x < pcClip->msBottomRight.x) && (y >= pcClip->msTopLeft.y) && (y < pcClip->msBottomRight.y));
			if (bDrawn)
			{
				uiSource = (xs + ys * iSourceWidth) * uiSourceStride;
				if (bSourceGaps)
				{
					bDrawn = puiSource[uiSource + 3] == 255;
				}
			}

			uiDest = (x + y * iDestWidth) * uiDestStride;
			if (bDrawn)
			{
				if ((puiDest[uiDest] != puiSource[uiSource]) || (puiDest[uiDest + 1] != puiSource[uiSource + 1]) || (puiDest[uiDest + 2] != puiSource[uiSource + 2]))
				{
					uiWrong++;
				}
			}
			else
			{
				if ((puiDest[uiDest] != 0) || (puiDest[uiDest + 1] != 0) || (puiDest[uiDest + 2] != 0))
				{
					uiWrong++;
				}
			}
		}
	}
	AssertSize(0, uiWrong);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertDrawCel(Ptr<CImage> pDest, Ptr<CImage> pSource, int32 iDestX, int32 iDestY, CInt32Rect* pcClip)
{
	AssertDrawCelPixels(pDest, 3, pSource, 3, false, iDestX, iDestY, pcClip);
}


//////////////////////////////////////////////////////////////////////////
//
// A black image with opacity to draw anti-aliased shapes on, and a draw
// set to opaque magenta.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> BeginShapeImage(CImageDraw* pcDraw, int iWidth, int iHeight)
{
	Ptr<CImage>			pImage;
	CImageColourARGB	cColour;

	pImage = OMalloc<CImage>(iWidth, iHeight, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP);
	pImage->Black();

	pcDraw->Init(&pImage);
	cColour.Init(1.0f, 1.0f, 0.0f, 1.0f);
	pcDraw->SetColour(&cColour);
	cColour.Kill();
	return pImage;
}


//////////////////////////////////////////////////////////////////////////
//
// The opacity of the whole image in pixels.
//
//////////////////////////////////////////////////////////////////////////
float64 SumShapeOpacity(Ptr<CImage> pImage)
{
	int		x;
	int		y;
	int32	iSum;

	iSum = 0;
	for (y = 0; y < (int)pImage->GetHeight(); y++)
	{
		for (x = 0; x < (int)pImage->GetWidth(); x++)
		{
			iSum += GetCircleOpacity(pImage, x, y);
		}
	}
	return iSum / 255.0;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the image is the same mirrored left to right and top to bottom,
// and within one about its diagonal (if bDiagonal and it is square); the
// coverage rasteriser cuts edges at rows and columns differently.
//
//////////////////////////////////////////////////////////////////////////
bool IsShapeSymmetric(Ptr<CImage> pImage, bool bDiagonal)
{
	int		x;
	int		y;
	int		iWidth;
	int		iHeight;
	bool	bSymmetric;

	iWidth = pImage->GetWidth();
	iHeight = pImage->GetHeight();
	bSymmetric = true;
	for (y = 0; y < iHeight; y++)
	{
		for (x = 0; x < iWidth; x++)
		{
			bSymmetric &= GetCircleOpacity(pImage, x, y) == GetCircleOpacity(pImage, iWidth - 1 - x, y);
			bSymmetric &= GetCircleOpacity(pImage, x, y) == GetCircleOpacity(pImage, x, iHeight - 1 - y);
			if (bDiagonal && (iWidth == iHeight))
			{
				bSymmetric &= abs(GetCircleOpacity(pImage, x, y) - GetCircleOpacity(pImage, y, x)) <= 1;
			}
		}
	}
	return bSymmetric;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDrawShapes(void)
{
	CImageDraw			cDraw;
	Ptr<CImage>			pImage;
	CVectorPath			cPath;
	SFloat64Vec2		asPoints[3];
	SFloat64Vec2		sOrigin;
	SLogConfig			sLogConfig;
	int					x;
	int					y;
	bool				bInside;
	float64				fArea;

	//A circle covers its area and is symmetric about its middle, which is on a pixel corner.
	pImage = BeginShapeImage(&cDraw, 32, 32);
	AssertTrue(cDraw.DrawCircle(16, 16, 10));
	cDraw.Kill();
	fArea = SumShapeOpacity(pImage);
	AssertTrue(fabs(fArea - N_PI_64 * 100) < 1);
	AssertTrue(IsShapeSymmetric(pImage, true));
	AssertInt(255, GetCircleOpacity(pImage, 16, 16));
	AssertInt(255, GetCircleChannel(pImage, 16, 16, 0));
	AssertInt(0, GetCircleChannel(pImage, 16, 16, 1));
	AssertInt(0, GetCircleOpacity(pImage, 0, 0));

	//And in the middle of a pixel.
	pImage = BeginShapeImage(&cDraw, 21, 21);
	AssertTrue(cDraw.DrawCircle(10.5, 10.5, 7.25));
	cDraw.Kill();
	AssertTrue(IsShapeSymmetric(pImage, true));

	//A rectangle on pixel edges fills exactly its pixels.  One half a pixel from an edge half covers that edge.
	pImage = BeginShapeImage(&cDraw, 8, 8);
	AssertTrue(cDraw.DrawRectangle(2, 1, 6, 4));
	cDraw.Kill();
	bInside = true;
	for (y = 0; y < 8; y++)
	{
		for (x = 0; x < 8; x++)
		{
			bInside &= GetCircleOpacity(pImage, x, y) == (((x >= 2) && (x < 6) && (y >= 1) && (y < 4)) ? 255 : 0);
		}
	}
	AssertTrue(bInside);
	pImage = BeginShapeImage(&cDraw, 8, 8);
	AssertTrue(cDraw.DrawRectangle(2.5, 1, 6, 4));
	cDraw.Kill();
	AssertTrue(abs(GetCircleOpacity(pImage, 2, 2) - 128) <= 1);
	AssertInt(255, GetCircleOpacity(pImage, 3, 2));
	AssertInt(0, GetCircleOpacity(pImage, 1, 2));

	//A rounded rectangle has full sides and anti-aliased corners.
	pImage = BeginShapeImage(&cDraw, 16, 16);
	AssertTrue(cDraw.DrawRoundedRectangle(2, 2, 14, 14, 4));
	cDraw.Kill();
	AssertTrue(IsShapeSymmetric(pImage, true));
	AssertInt(255, GetCircleOpacity(pImage, 2, 8));
	AssertInt(255, GetCircleOpacity(pImage, 8, 2));
	AssertInt(0, GetCircleOpacity(pImage, 2, 2));
	fArea = SumShapeOpacity(pImage);
	AssertTrue(fabs(fArea - (144 - 4 * (16 - N_PI_64 * 4))) < 0.5);

	//A ring leaves its middle alone.  A sector is the part of the circle between its angles; 0 is right and 90 up.
	pImage = BeginShapeImage(&cDraw, 32, 32);
	AssertTrue(cDraw.DrawRing(16, 16, 12, 8, 0, 360));
	cDraw.Kill();
	AssertTrue(IsShapeSymmetric(pImage, true));
	AssertInt(0, GetCircleOpacity(pImage, 16, 16));
	AssertInt(255, GetCircleOpacity(pImage, 26, 16));
	AssertTrue(fabs(SumShapeOpacity(pImage) - N_PI_64 * (144 - 64)) < 1);
	pImage = BeginShapeImage(&cDraw, 32, 32);
	AssertTrue(cDraw.DrawSector(16, 16, 12, 0, 90));
	cDraw.Kill();
	AssertInt(255, GetCircleOpacity(pImage, 20, 12));
	AssertInt(0, GetCircleOpacity(pImage, 12, 12));
	AssertInt(0, GetCircleOpacity(pImage, 20, 20));
	AssertTrue(fabs(SumShapeOpacity(pImage) - N_PI_64 * 36) < 0.5);

	//A stroke is a line with round ends.
	pImage = BeginShapeImage(&cDraw, 16, 16);
	AssertTrue(cDraw.DrawStroke(4, 8, 12, 8, 2));
	cDraw.Kill();
	AssertInt(255, GetCircleOpacity(pImage, 8, 7));
	AssertInt(255, GetCircleOpacity(pImage, 8, 8));
	AssertInt(0, GetCircleOpacity(pImage, 8, 6));
	AssertTrue(IsShapeSymmetric(pImage, false));
	AssertTrue(fabs(SumShapeOpacity(pImage) - (16 + N_PI_64)) < 0.1);

	//A polygon.
	pImage = BeginShapeImage(&cDraw, 16, 16);
	asPoints[0].Init(2, 2);
	asPoints[1].Init(14, 2);
	asPoints[2].Init(2, 14);
	AssertTrue(cDraw.DrawPolygon(asPoints, 3));
	cDraw.Kill();
	AssertTrue(fabs(SumShapeOpacity(pImage) - 72) < 0.1);
	AssertInt(255, GetCircleOpacity(pImage, 3, 3));
	AssertInt(0, GetCircleOpacity(pImage, 12, 12));

	//A path in other units with y up: millimetres at 2 pixels to the millimetre with the image's top left at 0, 8 mm.
	//Its second part has exposure off and erases a hole.
	pImage = BeginShapeImage(&cDraw, 16, 16);
	cPath.Init();
	cPath.BeginPart(true);
	asPoints[0].Init(1, 1);
	asPoints[1].Init(7, 7);
	AddVectorRectangle(&cPath, &asPoints[0], &asPoints[1]);
	cPath.BeginPart(false);
	asPoints[0].Init(4, 4);
	cPath.AddCircle(&asPoints[0], 1, false);
	sOrigin.Init(0, 8);
	AssertTrue(cDraw.DrawPath(&cPath, &sOrigin, 2, true));
	cPath.Kill();
	cDraw.Kill();
	AssertInt(255, GetCircleOpacity(pImage, 2, 2));
	AssertInt(255, GetCircleOpacity(pImage, 13, 13));
	AssertInt(0, GetCircleOpacity(pImage, 1, 1));
	AssertInt(0, GetCircleOpacity(pImage, 7, 7));
	AssertInt(0, GetCircleOpacity(pImage, 8, 8));
	AssertTrue(IsShapeSymmetric(pImage, true));
	AssertTrue(fabs(SumShapeOpacity(pImage) - (144 - N_PI_64 * 4)) < 0.5);

	//Shapes outside the image or empty draw nothing.
	pImage = BeginShapeImage(&cDraw, 8, 8);
	AssertTrue(cDraw.DrawCircle(-20, -20, 4));
	AssertTrue(cDraw.DrawRectangle(5, 5, 5, 7));
	cDraw.Kill();
	AssertTrue(SumShapeOpacity(pImage) == 0);

	//An image without opacity can't show the anti-aliased edge.
	pImage = OMalloc<CImage>(8, 8, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	cDraw.Init(&pImage);
	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cDraw.DrawCircle(4, 4, 3));
	gcLogger.SetConfig(&sLogConfig);
	cDraw.Kill();

	pImage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDrawCelClipping(void)
{
	CImageDraw		cDraw;
	Ptr<CImage>		pSource;
	Ptr<CImage>		pDest;
	Ptr<CImageCel>	pCel;
	CInt32Rect		cClip;
	int32			aiPositions[10][2] = { { 2, 2 }, { -2, -1 }, { 7, 5 }, { -3, 5 }, { 7, -2 }, { -6, 0 }, { 10, 3 }, { 0, -4 }, { 4, 8 }, { -20, -20 } };
	uint32			i;

	//The source is 6x4 and the destination 10x8 so the cel can hang off every edge and corner and fall right outside.
	pSource = CreateDrawCelSourceImage(6, 4);
	pCel = OMalloc<CImageCel>(pSource);
	pDest = OMalloc<CImage>(10, 8, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);

	cDraw.Init(&pDest);
	for (i = 0; i < 10; i++)
	{
		pDest->Black();
		AssertTrue(cDraw.DrawCel(aiPositions[i][0], aiPositions[i][1], pCel));
		AssertDrawCel(pDest, pSource, aiPositions[i][0], aiPositions[i][1], NULL);
	}

	//A clip rectangle limits drawing further still.  Its right and bottom are exclusive.
	cClip.Init(3, 2, 7, 5);
	for (i = 0; i < 10; i++)
	{
		pDest->Black();
		AssertTrue(cDraw.DrawCel(aiPositions[i][0], aiPositions[i][1], pCel, &cClip));
		AssertDrawCel(pDest, pSource, aiPositions[i][0], aiPositions[i][1], &cClip);
	}

	//A clip rectangle partly outside the destination is limited to the destination.
	cClip.Init(-5, 6, 4, 20);
	pDest->Black();
	AssertTrue(cDraw.DrawCel(-1, 5, pCel, &cClip));
	AssertDrawCel(pDest, pSource, -1, 5, &cClip);

	//An empty clip rectangle draws nothing.
	cClip.Init(4, 4, 4, 6);
	pDest->Black();
	AssertTrue(cDraw.DrawCel(2, 2, pCel, &cClip));
	AssertDrawCel(pDest, pSource, 2, 2, &cClip);

	cDraw.Kill();
	pCel = NULL;
	pSource = NULL;
	pDest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDrawCelMismatchedFormat(void)
{
	CImageDraw		cDraw;
	Ptr<CImage>		pSource;
	Ptr<CImage>		pDest;
	Ptr<CImageCel>	pCel;
	SLogConfig		sLogConfig;
	bool			bResult;

	//A cel can only be drawn onto an image with the same colour order and colour bits.
	pSource = CreateDrawCelSourceImage(6, 4);
	pCel = OMalloc<CImageCel>(pSource);
	pDest = OMalloc<CImage>(10, 8, CFT_RGB, CCO_BGR, CRGB_24bit, ARGB_None);
	pDest->Black();

	cDraw.Init(&pDest);
	sLogConfig = gcLogger.SetSilent();
	bResult = cDraw.DrawCel(2, 2, pCel);
	gcLogger.SetConfig(&sLogConfig);
	AssertFalse(bResult);
	AssertDrawCel(pDest, pSource, 100, 100, NULL);

	cDraw.Kill();
	pCel = NULL;
	pSource = NULL;
	pDest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDrawCelClippingTransparentGaps(void)
{
	CImageDraw		cDraw;
	Ptr<CImage>		pSource;
	Ptr<CImage>		pDest;
	Ptr<CImageCel>	pCel;
	CInt32Rect		acClips[4];
	int32			aiPositions[6][2] = { { 0, 0 }, { 2, 3 }, { -3, 1 }, { 4, -1 }, { 6, 6 }, { -1, -1 } };
	uint32			i;
	uint32			j;

	//A cel with transparent gaps is blitted as several segments per row.  The narrow clips leave
	//some segments of a row wholly outside the clip while others in the same row are drawn.
	pSource = CreateDrawCelImage(9, 3, CFT_RGBA, ARGB_8bit, 4, true);
	pCel = OMalloc<CImageCel>(pSource);
	pDest = OMalloc<CImage>(10, 8, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);

	acClips[0].Init(3, 0, 6, 8);
	acClips[1].Init(5, 0, 6, 8);
	acClips[2].Init(0, 1, 10, 2);
	acClips[3].Init(2, 2, 9, 7);

	cDraw.Init(&pDest);
	for (i = 0; i < 6; i++)
	{
		pDest->Black();
		AssertTrue(cDraw.DrawCel(aiPositions[i][0], aiPositions[i][1], pCel));
		AssertDrawCelPixels(pDest, 3, pSource, 4, true, aiPositions[i][0], aiPositions[i][1], NULL);

		for (j = 0; j < 4; j++)
		{
			pDest->Black();
			AssertTrue(cDraw.DrawCel(aiPositions[i][0], aiPositions[i][1], pCel, &acClips[j]));
			AssertDrawCelPixels(pDest, 3, pSource, 4, true, aiPositions[i][0], aiPositions[i][1], &acClips[j]);
		}
	}

	cDraw.Kill();
	pCel = NULL;
	pSource = NULL;
	pDest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDrawCelRGBX(void)
{
	CImageDraw		cDraw;
	Ptr<CImage>		pSource;
	Ptr<CImage>		pDest;
	Ptr<CImageCel>	pCel;
	CInt32Rect		cClip;

	//RGBX has alpha bits for its ignored channel but no opacity channel and must still be drawable.
	pSource = CreateDrawCelImage(6, 4, CFT_RGBX, ARGB_8bit, 4, false);
	pCel = OMalloc<CImageCel>(pSource);
	pDest = OMalloc<CImage>(10, 8, CFT_RGBX, CCO_RGB, CRGB_24bit, ARGB_8bit);

	cDraw.Init(&pDest);

	pDest->Black();
	AssertTrue(cDraw.DrawCel(2, 1, pCel));
	AssertDrawCelPixels(pDest, 4, pSource, 4, false, 2, 1, NULL);

	pDest->Black();
	AssertTrue(cDraw.DrawCel(-2, 5, pCel));
	AssertDrawCelPixels(pDest, 4, pSource, 4, false, -2, 5, NULL);

	cClip.Init(3, 2, 7, 4);
	pDest->Black();
	AssertTrue(cDraw.DrawCel(2, 1, pCel, &cClip));
	AssertDrawCelPixels(pDest, 4, pSource, 4, false, 2, 1, &cClip);

	cDraw.Kill();
	pCel = NULL;
	pSource = NULL;
	pDest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDrawCelDestAlphaRowEnds(void)
{
	CImageDraw		cDraw;
	Ptr<CImage>		pSource;
	Ptr<CImage>		pDest;
	Ptr<CImageCel>	pCel;
	uint8			auiOpaque[4] = { 200, 100, 50, 255 };
	uint8			auiTranslucent[4] = { 60, 120, 180, 128 };
	uint8			auiClear[4] = { 0, 0, 0, 0 };
	uint8			auiBackground[4] = { 10, 20, 30, 100 };
	uint8*			apuiRows[2][8] = { { auiOpaque, auiOpaque, auiClear, auiClear, auiTranslucent, auiTranslucent, auiOpaque, auiOpaque },
									   { auiTranslucent, auiTranslucent, auiOpaque, auiOpaque, auiClear, auiClear, auiTranslucent, auiTranslucent } };
	uint8*			puiSource;
	uint8*			puiDest;
	int32			x;
	int32			y;

	//A cel with opacity is split into opaque and translucent segments.  Drawn onto an image with its own opacity each
	//segment must also write that opacity.  Row 0 ends with an opaque segment and row 1 with a translucent one, and
	//both kinds also appear in the middle of a row.  The same source pixel drawn over the same background must give
	//the same result wherever its segment is in the row.
	pSource = OMalloc<CImage>(8, 2, CFT_RGBA, CCO_RGB, CRGB_24bit, ARGB_8bit);
	pDest = OMalloc<CImage>(8, 2, CFT_RGBA, CCO_RGB, CRGB_24bit, ARGB_8bit);
	puiSource = (uint8*)pSource->GetData();
	puiDest = (uint8*)pDest->GetData();
	for (y = 0; y < 2; y++)
	{
		for (x = 0; x < 8; x++)
		{
			memcpy(&puiSource[(x + y * 8) * 4], apuiRows[y][x], 4);
			memcpy(&puiDest[(x + y * 8) * 4], auiBackground, 4);
		}
	}
	pCel = OMalloc<CImageCel>(pSource);

	cDraw.Init(&pDest);
	AssertTrue(cDraw.DrawCel(0, 0, pCel));
	cDraw.Kill();

	//Transparent pixels leave the background alone.
	AssertMemory(auiBackground, &puiDest[(2 + 0 * 8) * 4], 4);
	AssertMemory(auiBackground, &puiDest[(4 + 1 * 8) * 4], 4);

	//Opaque pixels replace the background, opacity included.
	AssertMemory(auiOpaque, &puiDest[(0 + 0 * 8) * 4], 4);
	AssertMemory(auiOpaque, &puiDest[(2 + 1 * 8) * 4], 4);
	AssertMemory(auiOpaque, &puiDest[(6 + 0 * 8) * 4], 4);  //Ends row 0.
	AssertMemory(auiOpaque, &puiDest[(7 + 0 * 8) * 4], 4);

	//Translucent pixels must blend the same way in the middle of row 0, at the start of row 1 and at the end of row 1.
	AssertMemory(&puiDest[(4 + 0 * 8) * 4], &puiDest[(0 + 1 * 8) * 4], 4);
	AssertMemory(&puiDest[(4 + 0 * 8) * 4], &puiDest[(6 + 1 * 8) * 4], 4);  //Ends row 1.
	AssertMemory(&puiDest[(4 + 0 * 8) * 4], &puiDest[(7 + 1 * 8) * 4], 4);

	//Translucent pixels combine their opacity with the background's: 128 + 100 * (255 - 128) / 255.
	AssertInt(178, puiDest[(4 + 0 * 8) * 4 + 3]);
	AssertInt(178, puiDest[(0 + 1 * 8) * 4 + 3]);
	AssertInt(178, puiDest[(7 + 1 * 8) * 4 + 3]);

	pCel = NULL;
	pSource = NULL;
	pDest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertDrawCelOpacity(Ptr<CImage> pDest, int32 iLeft, int32 iTop, int32 iRight, int32 iBottom, CInt32Rect* pcClip)
{
	uint8*	puiDest;
	int32	x;
	int32	y;
	bool	bDrawn;
	uint8	uiExpected;
	uint32	uiWrong;
	int32	iHeight;
	int32	iWidth;

	//The fourth byte of an RGBA destination must be opaque where the cel was drawn and still zero everywhere else.
	puiDest = (uint8*)pDest->GetData();
	uiWrong = 0;
	iHeight = pDest->GetImageHeight();
	iWidth = pDest->GetImageWidth();
	for (y = 0; y < iHeight; y++)
	{
		for (x = 0; x < iWidth; x++)
		{
			bDrawn = (x >= iLeft) && (x < iRight) && (y >= iTop) && (y < iBottom);
			bDrawn &= (pcClip == NULL) || ((x >= pcClip->msTopLeft.x) && (x < pcClip->msBottomRight.x) && (y >= pcClip->msTopLeft.y) && (y < pcClip->msBottomRight.y));
			uiExpected = bDrawn ? 255 : 0;
			if (puiDest[(x + y * iWidth) * 4 + 3] != uiExpected)
			{
				uiWrong++;
			}
		}
	}
	AssertSize(0, uiWrong);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDrawCelOpaqueOntoDestOpacity(void)
{
	CImageDraw		cDraw;
	Ptr<CImage>		pSource;
	Ptr<CImage>		pDest;
	Ptr<CImageCel>	pCel;
	CInt32Rect		cClip;

	//An RGB cel has no opacity of its own.  Drawn onto an RGBA image the colour must be copied unchanged and the
	//destination opacity set to opaque, not written over one of the colours.
	pSource = CreateDrawCelSourceImage(6, 4);
	pCel = OMalloc<CImageCel>(pSource);
	pDest = OMalloc<CImage>(10, 8, CFT_RGBA, CCO_RGB, CRGB_24bit, ARGB_8bit);

	cDraw.Init(&pDest);

	memset(pDest->GetData(), 0, pDest->GetByteSize());
	AssertTrue(cDraw.DrawCel(2, 1, pCel));
	AssertDrawCelPixels(pDest, 4, pSource, 3, false, 2, 1, NULL);
	AssertDrawCelOpacity(pDest, 2, 1, 8, 5, NULL);

	memset(pDest->GetData(), 0, pDest->GetByteSize());
	AssertTrue(cDraw.DrawCel(-3, 6, pCel));
	AssertDrawCelPixels(pDest, 4, pSource, 3, false, -3, 6, NULL);
	AssertDrawCelOpacity(pDest, -3, 6, 3, 10, NULL);

	cClip.Init(3, 2, 7, 4);
	memset(pDest->GetData(), 0, pDest->GetByteSize());
	AssertTrue(cDraw.DrawCel(2, 1, pCel, &cClip));
	AssertDrawCelPixels(pDest, 4, pSource, 3, false, 2, 1, &cClip);
	AssertDrawCelOpacity(pDest, 2, 1, 8, 5, &cClip);

	cDraw.Kill();
	pCel = NULL;
	pSource = NULL;
	pDest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertDrawCelTranslucentPixels(Ptr<CImage> pDest, size uiRed, size uiGreen, size uiBlue, size uiOpacity)
{
	uint8*	puiData;
	size	i;
	uint8	auiColour[3] = { 200, 100, 50 };
	uint8	auiAlpha[4] = { 0, 64, 128, 255 };
	size	uiExpected;

	//Drawn onto transparent black a straight alpha source must leave its colour multiplied by its opacity, rounded.
	puiData = (uint8*)pDest->GetData();
	for (i = 0; i < 4; i++)
	{
		uiExpected = ((uint32)auiColour[0] * auiAlpha[i] + 127) / 255;
		AssertInt((int)uiExpected, puiData[i * 4 + uiRed]);
		uiExpected = ((uint32)auiColour[1] * auiAlpha[i] + 127) / 255;
		AssertInt((int)uiExpected, puiData[i * 4 + uiGreen]);
		uiExpected = ((uint32)auiColour[2] * auiAlpha[i] + 127) / 255;
		AssertInt((int)uiExpected, puiData[i * 4 + uiBlue]);
		AssertInt(auiAlpha[i], puiData[i * 4 + uiOpacity]);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDrawCelTranslucentBGROntoDestOpacity(void)
{
	CImageDraw		cDraw;
	Ptr<CImage>		pSource;
	Ptr<CImage>		pConverted;
	Ptr<CImage>		pDest;
	Ptr<CImageCel>	pCel;
	uint8*			puiData;
	size			i;
	uint8			auiAlpha[4] = { 0, 64, 128, 255 };

	//A font atlas is A,R,G,B.  Converted to the WinGDI order it is A,B,G,R and is drawn onto a B,G,R,A canvas.
	pSource = OMalloc<CImage>(4, 1, PT_uint8, IMAGE_OPACITY, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	puiData = (uint8*)pSource->GetData();
	for (i = 0; i < 4; i++)
	{
		puiData[i * 4 + 0] = auiAlpha[i];
		puiData[i * 4 + 1] = 200;
		puiData[i * 4 + 2] = 100;
		puiData[i * 4 + 3] = 50;
	}

	pDest = OMalloc<CImage>(4, 1, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP);
	memset(pDest->GetData(), 0, pDest->GetByteSize());
	pCel = OMalloc<CImageCel>(pSource);
	cDraw.Init(&pDest);
	AssertTrue(cDraw.DrawCel(0, 0, pCel));
	AssertDrawCelTranslucentPixels(pDest, 0, 1, 2, 3);
	cDraw.Kill();

	pConverted = CImageColourConverter::ConvertColour(pSource, CCO_BGR);
	AssertInt(CCO_BGR, pConverted->GetColourOrder());
	pDest = OMalloc<CImage>(4, 1, PT_uint8, IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, IMAGE_OPACITY, CHANNEL_STOP);
	memset(pDest->GetData(), 0, pDest->GetByteSize());
	pCel = OMalloc<CImageCel>(pConverted);
	cDraw.Init(&pDest);
	AssertTrue(cDraw.DrawCel(0, 0, pCel));
	AssertDrawCelTranslucentPixels(pDest, 2, 1, 0, 3);
	cDraw.Kill();

	pCel = NULL;
	pConverted = NULL;
	pSource = NULL;
	pDest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDrawCelAlignment(void)
{
	CImageDraw		cDraw;
	Ptr<CImage>		pSource;
	Ptr<CImage>		pDest;
	Ptr<CImageCel>	pCel;
	CInt32Rect		cClip;
	int32			aiPositions[6][2] = { { 8, 6 }, { 6, 4 }, { 3, 2 }, { 12, 9 }, { 15, 3 }, { 7, 13 } };
	uint32			i;

	//A cel aligned to its bottom right is drawn with its bottom right corner at the position given.  The cel must be
	//both clipped and drawn where its alignment puts it, else it can be drawn outside the destination image.
	pSource = CreateDrawCelSourceImage(6, 4);
	pCel = OMalloc<CImageCel>(pSource, 0, 0, 6, 4, 0, 0, 0, 0, SUB_IMAGE_ALIGNMENT_RIGHT | SUB_IMAGE_ALIGNMENT_BOTTOM);
	pDest = OMalloc<CImage>(10, 8, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);

	cDraw.Init(&pDest);
	for (i = 0; i < 6; i++)
	{
		pDest->Black();
		AssertTrue(cDraw.DrawCel(aiPositions[i][0], aiPositions[i][1], pCel));
		AssertDrawCel(pDest, pSource, aiPositions[i][0] - 6, aiPositions[i][1] - 4, NULL);
	}

	cClip.Init(3, 2, 7, 5);
	for (i = 0; i < 6; i++)
	{
		pDest->Black();
		AssertTrue(cDraw.DrawCel(aiPositions[i][0], aiPositions[i][1], pCel, &cClip));
		AssertDrawCel(pDest, pSource, aiPositions[i][0] - 6, aiPositions[i][1] - 4, &cClip);
	}

	//Centred, the middle of the cel is at the position given.
	pCel = OMalloc<CImageCel>(pSource, 0, 0, 6, 4, 0, 0, 0, 0, SUB_IMAGE_ALIGNMENT_CENTER_HORIZONTAL | SUB_IMAGE_ALIGNMENT_CENTER_VERTICAL);
	pDest->Black();
	AssertTrue(cDraw.DrawCel(5, 4, pCel));
	AssertDrawCel(pDest, pSource, 2, 2, NULL);

	pDest->Black();
	AssertTrue(cDraw.DrawCel(9, 1, pCel));
	AssertDrawCel(pDest, pSource, 6, -1, NULL);

	cDraw.Kill();
	pCel = NULL;
	pSource = NULL;
	pDest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDrawBlitterContextInit(void)
{
	CImageBlitterContext	cContext;

	//Init with no images must clear every field, not just the first few bytes.
	memset(&cContext, 0xAB, sizeof(CImageBlitterContext));
	cContext.Init();

	AssertSize(0, cContext.miSourcePixelStride);
	AssertSize(0, cContext.miDestPixelStride);
	AssertSize(0, cContext.miSourceWidth);
	AssertSize(0, cContext.miDestWidth);
	AssertSize(0, cContext.miSourceRowStride);
	AssertSize(0, cContext.miDestRowStride);
	AssertSize(0, cContext.muiSourceColourOffset);
	AssertSize(0, cContext.muiDestColourOffset);
	AssertSize(0, cContext.muiDestAlphaOffset);
	AssertSize(0, cContext.muiSourceAlphaOffset);
	AssertSize(0, cContext.muiColourWidth);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CreateDrawCelTranslucentImage(int32 iWidth, int32 iHeight)
{
	Ptr<CImage>		pImage;
	uint8*			puiData;
	int32			x;
	int32			y;
	uint8			auiAlpha[8] = { 255, 255, 0, 128, 40, 255, 0, 200 };

	//Opaque, transparent and translucent runs of different lengths in every row.
	pImage = CreateDrawCelImage(iWidth, iHeight, CFT_RGBA, ARGB_8bit, 4, false);
	puiData = (uint8*)pImage->GetData();
	for (y = 0; y < iHeight; y++)
	{
		for (x = 0; x < iWidth; x++)
		{
			puiData[(x + y * iWidth) * 4 + 3] = auiAlpha[(x + 3 * y) % 8];
		}
	}
	return pImage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void FillDrawCelBackground(Ptr<CImage> pDest, uint32 uiDestStride, uint8* puiBackground)
{
	uint8*	puiDest;
	uint32	ui;
	uint32	uiNumPixels;
	uint32	iWidth;

	puiDest = (uint8*)pDest->GetData();
	iWidth = pDest->GetImageWidth();
	uiNumPixels = iWidth * pDest->GetHeight();
	for (ui = 0; ui < uiNumPixels; ui++)
	{
		memcpy(&puiDest[ui * uiDestStride], puiBackground, uiDestStride);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertDrawCelSized(Ptr<CImage> pDest, uint32 uiDestStride, uint8* puiBackground, Ptr<CImageCel> pCel, uint32 uiSourceStride, int32 iLeft, int32 iTop, int32 iWidth, int32 iHeight, CInt32Rect* pcClip)
{
	CSubImage*		pcSubImage;
	Ptr<CImage>		pSource;
	uint8*			puiDest;
	uint8*			puiSource;
	uint8*			puiSourcePixel;
	uint8			auiExpected[4];
	int32			x;
	int32			y;
	int32			xCel;
	int32			yCel;
	bool			bInside;
	uint16			uiAlpha;
	uint32			i;
	uint32			uiWrong;
	int32			iDestHeight;
	int32			iDestWidth;

	//Every destination pixel is worked out on its own: the cel pixel under its centre, (2x + 1) * full / (2 * sized),
	//drawn over the background.  Pixels outside the sized cel, the clip, or the cel's image keep the background.
	pcSubImage = pCel->GetSubImage();
	pSource = pCel->GetSourceImage();
	puiDest = (uint8*)pDest->GetData();
	puiSource = (uint8*)pSource->GetData();
	uiWrong = 0;
	iDestHeight = pDest->GetImageHeight();
	iDestWidth = pDest->GetImageWidth();
	for (y = 0; y < iDestHeight; y++)
	{
		for (x = 0; x < iDestWidth; x++)
		{
			memcpy(auiExpected, puiBackground, uiDestStride);

			bInside = (x >= iLeft) && (x < iLeft + iWidth) && (y >= iTop) && (y < iTop + iHeight);
			bInside &= (pcClip == NULL) || ((x >= pcClip->msTopLeft.x) && (x < pcClip->msBottomRight.x) && (y >= pcClip->msTopLeft.y) && (y < pcClip->msBottomRight.y));
			if (bInside)
			{
				xCel = ((2 * (x - iLeft) + 1) * pcSubImage->GetFullWidth()) / (2 * iWidth) - pcSubImage->msOffsetTopLeft.x;
				yCel = ((2 * (y - iTop) + 1) * pcSubImage->GetFullHeight()) / (2 * iHeight) - pcSubImage->msOffsetTopLeft.y;
				if ((xCel >= 0) && (xCel < pcSubImage->GetImageWidth()) && (yCel >= 0) && (yCel < pcSubImage->GetImageHeight()))
				{
					puiSourcePixel = &puiSource[((pcSubImage->mcImageRect.msTopLeft.x + xCel) + (pcSubImage->mcImageRect.msTopLeft.y + yCel) * pSource->GetWidth()) * uiSourceStride];
					uiAlpha = (uiSourceStride == 4) ? puiSourcePixel[3] : 255;
					if (uiAlpha == 255)
					{
						memcpy(auiExpected, puiSourcePixel, 3);
						if (uiDestStride == 4)
						{
							auiExpected[3] = 255;
						}
					}
					else if (uiAlpha != 0)
					{
						for (i = 0; i < 3; i++)
						{
							auiExpected[i] = (uint8)((puiSourcePixel[i] * uiAlpha + auiExpected[i] * (255 - uiAlpha) + 127) / 255);
						}
						if (uiDestStride == 4)
						{
							auiExpected[3] = (uint8)(uiAlpha + (auiExpected[3] * (255 - uiAlpha) + 127) / 255);
						}
					}
				}
			}

			if (memcmp(&puiDest[(x + y * iDestWidth) * uiDestStride], auiExpected, uiDestStride) != 0)
			{
				uiWrong++;
			}
		}
	}
	AssertSize(0, uiWrong);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertDrawCelSizes(Ptr<CImageCel> pCel, uint32 uiSourceStride, Ptr<CImage> pDest, uint32 uiDestStride)
{
	CImageDraw		cDraw;
	CInt32Rect		acClips[3];
	CInt32Rect*		pcClip;
	uint8			auiBackground[4] = { 5, 6, 7, 9 };
	int32			aiSizes[13][2] = { { 12, 8 }, { 3, 2 }, { 7, 5 }, { 6, 9 }, { 13, 4 }, { 6, 2 }, { 1, 1 }, { 25, 3 }, { 2, 11 }, { 6, 4 }, { 19, 14 }, { 0, 5 }, { 4, -1 } };
	int32			aiPositions[6][2] = { { 2, 2 }, { -3, -2 }, { 15, 11 }, { -30, -30 }, { 0, 0 }, { 9, -4 } };
	uint32			uiSize;
	uint32			uiPosition;
	uint32			uiClip;
	int32			x;
	int32			y;
	int32			iWidth;
	int32			iHeight;
	bool			bResult;

	//Every size is drawn through the same blitter so its sized tables must be rebuilt whenever the size changes.
	//A size of 6x4 is the cel's own and the zero and negative sizes draw nothing.
	acClips[1].Init(3, 2, 9, 7);
	acClips[2].Init(5, 0, 6, 16);

	cDraw.Init(&pDest);
	for (uiSize = 0; uiSize < 13; uiSize++)
	{
		iWidth = aiSizes[uiSize][0];
		iHeight = aiSizes[uiSize][1];
		for (uiPosition = 0; uiPosition < 6; uiPosition++)
		{
			x = aiPositions[uiPosition][0];
			y = aiPositions[uiPosition][1];
			for (uiClip = 0; uiClip < 3; uiClip++)
			{
				pcClip = (uiClip == 0) ? NULL : &acClips[uiClip];
				FillDrawCelBackground(pDest, uiDestStride, auiBackground);
				bResult = cDraw.DrawCel(x, y, iWidth, iHeight, pCel, pcClip);
				AssertTrue(bResult);
				AssertDrawCelSized(pDest, uiDestStride, auiBackground, pCel, uiSourceStride, x, y, iWidth, iHeight, pcClip);
			}
		}
	}
	cDraw.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDrawCelSizedOpaque(void)
{
	Ptr<CImage>		pSource;
	Ptr<CImage>		pDest;
	Ptr<CImageCel>	pCel;

	//The same layout with no destination opacity uses the contiguous blitter, which can only be used one for one.
	pSource = CreateDrawCelSourceImage(6, 4);
	pCel = OMalloc<CImageCel>(pSource);
	pDest = OMalloc<CImage>(20, 16, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
	AssertDrawCelSizes(pCel, 3, pDest, 3);

	//Opaque onto a destination with opacity.
	pDest = OMalloc<CImage>(20, 16, CFT_RGBA, CCO_RGB, CRGB_24bit, ARGB_8bit);
	AssertDrawCelSizes(pCel, 3, pDest, 4);

	//Opaque segments between transparent gaps.  Shrinking can drop a segment entirely.
	pSource = CreateDrawCelImage(6, 4, CFT_RGBA, ARGB_8bit, 4, true);
	pCel = OMalloc<CImageCel>(pSource);
	pDest = OMalloc<CImage>(20, 16, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
	AssertDrawCelSizes(pCel, 4, pDest, 3);

	pCel = NULL;
	pSource = NULL;
	pDest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDrawCelSizedTranslucent(void)
{
	Ptr<CImage>		pSource;
	Ptr<CImage>		pDest;
	Ptr<CImageCel>	pCel;

	//Translucent and opaque segments, without and with destination opacity.
	pSource = CreateDrawCelTranslucentImage(6, 4);
	pCel = OMalloc<CImageCel>(pSource);

	pDest = OMalloc<CImage>(20, 16, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
	AssertDrawCelSizes(pCel, 4, pDest, 3);

	pDest = OMalloc<CImage>(20, 16, CFT_RGBA, CCO_RGB, CRGB_24bit, ARGB_8bit);
	AssertDrawCelSizes(pCel, 4, pDest, 4);

	pCel = NULL;
	pSource = NULL;
	pDest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDrawCelSizedBorders(void)
{
	Ptr<CImage>		pSource;
	Ptr<CImage>		pDest;
	Ptr<CImageCel>	pCel;

	//The cel is the 4x3 part of the source at 1,1 with transparent borders of 1 left, 0 top, 1 right and 1 bottom,
	//so the full cel is 6x4.  The borders are sized with the rest of the cel.
	pSource = CreateDrawCelTranslucentImage(7, 5);
	pCel = OMalloc<CImageCel>(pSource, 1, 1, 5, 4, 1, 0, 1, 1, SUB_IMAGE_ALIGNMENT_LEFT | SUB_IMAGE_ALIGNMENT_TOP);
	AssertInt((int32)6, pCel->GetSubImage()->GetFullWidth());
	AssertInt((int32)4, pCel->GetSubImage()->GetFullHeight());

	pDest = OMalloc<CImage>(20, 16, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
	AssertDrawCelSizes(pCel, 4, pDest, 3);

	pCel = NULL;
	pSource = NULL;
	pDest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDrawCelSizedAlignment(void)
{
	CImageDraw		cDraw;
	Ptr<CImage>		pSource;
	Ptr<CImage>		pDest;
	Ptr<CImageCel>	pCel;
	CInt32Rect		cClip;
	uint8			auiBackground[3] = { 5, 6, 7 };

	//A centred 6x4 cel drawn 12x8 is centred on its position.
	pSource = CreateDrawCelSourceImage(6, 4);
	pCel = OMalloc<CImageCel>(pSource, 0, 0, 6, 4, 0, 0, 0, 0, SUB_IMAGE_ALIGNMENT_CENTER_HORIZONTAL | SUB_IMAGE_ALIGNMENT_CENTER_VERTICAL);
	pDest = OMalloc<CImage>(20, 16, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);

	cDraw.Init(&pDest);

	FillDrawCelBackground(pDest, 3, auiBackground);
	AssertTrue(cDraw.DrawCel(10, 8, 12, 8, pCel));
	AssertDrawCelSized(pDest, 3, auiBackground, pCel, 3, 4, 4, 12, 8, NULL);

	FillDrawCelBackground(pDest, 3, auiBackground);
	AssertTrue(cDraw.DrawCel(1, 15, 12, 8, pCel));
	AssertDrawCelSized(pDest, 3, auiBackground, pCel, 3, -5, 11, 12, 8, NULL);

	//Aligned to its bottom right and shrunk to 3x2.
	pCel = OMalloc<CImageCel>(pSource, 0, 0, 6, 4, 0, 0, 0, 0, SUB_IMAGE_ALIGNMENT_RIGHT | SUB_IMAGE_ALIGNMENT_BOTTOM);
	cClip.Init(0, 0, 9, 7);
	FillDrawCelBackground(pDest, 3, auiBackground);
	AssertTrue(cDraw.DrawCel(10, 8, 3, 2, pCel, &cClip));
	AssertDrawCelSized(pDest, 3, auiBackground, pCel, 3, 7, 6, 3, 2, &cClip);

	cDraw.Kill();
	pCel = NULL;
	pSource = NULL;
	pDest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDrawBlitterCacheOnImage(void)
{
	Ptr<CImage>					pImage;
	Ptr<CImage>					pSource;
	Ptr<CImageCelBlitterCache>	pCache;
	Ptr<CImageCelBlitterCache>	pNextCache;
	Ptr<CImageCel>				pCel;
	uint32						uiObjects;

	//An image keeps the cache of blitters that draw onto it.  The same cache is returned until the image's size or
	//format changes, then a new one is made because blitters only work for the image they were made for.
	uiObjects = (uint32)gcObjects.NumMemoryIndexes();
	pImage = OMalloc<CImage>(10, 8, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
	pSource = CreateDrawCelSourceImage(6, 4);
	pCel = OMalloc<CImageCel>(pSource);

	pCache = pImage->GetBlitterCache();
	AssertTrue(pCache.IsNotNull());
	AssertTrue(pCache->Matches(pImage));
	AssertTrue(pCache->CreateImageBlitter(pCel).IsNotNull());
	AssertTrue(pImage->GetBlitterCache().Object() == pCache.Object());

	pImage->BeginChange();
	pImage->SetSize(12, 8);
	pImage->EndChange();
	pNextCache = pImage->GetBlitterCache();
	AssertTrue(pNextCache.Object() != pCache.Object());
	pCache = pNextCache;

	pImage->ReInit();
	pNextCache = pImage->GetBlitterCache();
	AssertTrue(pNextCache.Object() != pCache.Object());

	//The image, its cache and a blitter in it point at each other, but once nothing else does they are all freed.
	pImage->ReInit(4, 4, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	AssertTrue(pImage->GetBlitterCache()->CreateImageBlitter(pCel).IsNotNull());
	pCache = NULL;
	pNextCache = NULL;
	pImage = NULL;
	pCel = NULL;
	pSource = NULL;
	AssertSize(uiObjects, gcObjects.NumMemoryIndexes());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CreateDrawCel(void)
{
	//A cel and its source image are two objects.
	return OMalloc<CImageCel>(CreateDrawCelSourceImage(6, 4));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CreateDrawDest(void)
{
	return OMalloc<CImage>(10, 8, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDrawBlitterCacheCelFreedFirst(void)
{
	Ptr<CImage>					pDest;
	Ptr<CImageCel>				pCel;
	Ptr<CImageCelBlitterCache>	pCache;
	uint32						uiStart;

	//A blitter does not keep its cel alive, so a cel can be freed while the image it was drawn onto lives on.
	//The cache forgets the cel's blitter and only removes it the next time the cache is used.
	uiStart = (uint32)gcObjects.NumMemoryIndexes();
	pDest = CreateDrawDest();
	pCel = CreateDrawCel();
	pCache = pDest->GetBlitterCache();
	AssertTrue(pCache->CreateImageBlitter(pCel).IsNotNull());
	AssertSize(1, pCel->NumBlitterCaches());
	AssertSize(uiStart + 5, gcObjects.NumMemoryIndexes());  //Destination, cache, blitter, cel and its image.

	pCel = NULL;
	AssertSize(uiStart + 3, gcObjects.NumMemoryIndexes());  //The cel and its image are gone, the forgotten blitter is not yet.
	AssertSize(0, pCache->NumImageBlitters());
	AssertSize(uiStart + 2, gcObjects.NumMemoryIndexes());

	//The cache carries on working.
	pCel = CreateDrawCel();
	AssertTrue(pCache->CreateImageBlitter(pCel).IsNotNull());
	AssertSize(1, pCache->NumImageBlitters());

	pCel = NULL;
	pCache = NULL;
	pDest = NULL;
	AssertSize(uiStart, gcObjects.NumMemoryIndexes());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDrawBlitterCacheDestinationFreedFirst(void)
{
	Ptr<CImage>		pDest;
	Ptr<CImageCel>	pCel;
	uint32			uiStart;

	//When the destination is freed its cache and blitters go with it, and the cel forgets the cache.  The cel can then
	//be drawn onto another image and freed without touching the freed cache.
	uiStart = (uint32)gcObjects.NumMemoryIndexes();
	pDest = CreateDrawDest();
	pCel = CreateDrawCel();
	AssertTrue(pDest->GetBlitterCache()->CreateImageBlitter(pCel).IsNotNull());
	AssertSize(1, pCel->NumBlitterCaches());

	pDest = NULL;
	AssertSize(0, pCel->NumBlitterCaches());
	AssertSize(uiStart + 2, gcObjects.NumMemoryIndexes());  //Just the cel and its image.

	pDest = CreateDrawDest();
	AssertTrue(pDest->GetBlitterCache()->CreateImageBlitter(pCel).IsNotNull());
	AssertSize(1, pCel->NumBlitterCaches());
	pCel = NULL;
	AssertSize(0, pDest->GetBlitterCache()->NumImageBlitters());

	pDest = NULL;
	AssertSize(uiStart, gcObjects.NumMemoryIndexes());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDrawBlitterCacheSharedCel(void)
{
	Ptr<CImage>		pDest1;
	Ptr<CImage>		pDest2;
	Ptr<CImageCel>	pCel;
	uint32			uiStart;

	//One cel drawn onto two images is in both images' caches.
	uiStart = (uint32)gcObjects.NumMemoryIndexes();
	pDest1 = CreateDrawDest();
	pDest2 = CreateDrawDest();
	pCel = CreateDrawCel();
	AssertTrue(pDest1->GetBlitterCache()->CreateImageBlitter(pCel).IsNotNull());
	AssertTrue(pDest2->GetBlitterCache()->CreateImageBlitter(pCel).IsNotNull());
	AssertTrue(pDest2->GetBlitterCache()->CreateImageBlitter(pCel).IsNotNull());  //The same blitter again.
	AssertSize(2, pCel->NumBlitterCaches());
	AssertSize(1, pDest2->GetBlitterCache()->NumImageBlitters());

	pDest1 = NULL;
	AssertSize(1, pCel->NumBlitterCaches());

	pCel = NULL;
	AssertSize(0, pDest2->GetBlitterCache()->NumImageBlitters());

	pDest2 = NULL;
	AssertSize(uiStart, gcObjects.NumMemoryIndexes());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDrawBlitterCacheCelKilled(void)
{
	Ptr<CImage>		pDest;
	Ptr<CImageCel>	pCel;
	Ptr<CImageCel>	pOtherPointer;
	uint32			uiStart;

	//Killing a cel outright, while other pointers still point at it, also makes the caches forget it.
	uiStart = (uint32)gcObjects.NumMemoryIndexes();
	pDest = CreateDrawDest();
	pCel = CreateDrawCel();
	pOtherPointer = pCel;
	AssertTrue(pDest->GetBlitterCache()->CreateImageBlitter(pCel).IsNotNull());

	pCel->Kill();
	AssertSize(0, pDest->GetBlitterCache()->NumImageBlitters());

	pCel = NULL;
	pOtherPointer = NULL;
	pDest = NULL;
	AssertSize(uiStart, gcObjects.NumMemoryIndexes());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDrawBlitterCacheFreedTogether(void)
{
	Ptr<CArrayObject>	pHolder;
	Ptr<CImage>			pDest;
	Ptr<CImageCel>		pCel;
	uint32				i;
	uint32				uiStart;

	//When the only thing holding a destination and a cel is freed, the destination, its cache, the blitter, the cel and
	//its image are all freed in the same kill in whatever order the objects system chooses.  Each must let go of the
	//others without touching one that has already been freed.  A few cels make some be freed before the cache and some after.
	uiStart = (uint32)gcObjects.NumMemoryIndexes();
	pHolder = OMalloc<CArrayObject>();
	pDest = CreateDrawDest();
	pHolder->Add(pDest);
	for (i = 0; i < 4; i++)
	{
		pCel = CreateDrawCel();
		pHolder->Add(pCel);
		AssertTrue(pDest->GetBlitterCache()->CreateImageBlitter(pCel).IsNotNull());
	}
	pCel = NULL;
	AssertSize(4, pDest->GetBlitterCache()->NumImageBlitters());
	pDest = NULL;
	AssertSize(uiStart + 1 + 2 + 4 + 4 * 2, gcObjects.NumMemoryIndexes());  //Holder, destination and cache, four blitters, four cels and their images.

	pHolder = NULL;
	AssertSize(uiStart, gcObjects.NumMemoryIndexes());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDraw(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestImageDrawBox1();
	TestImageDrawBox2();
	TestImageDrawLine();
	TestImageDrawLineExtents();
	TestImageDrawSemicircle();
	TestImageDrawShapes();
	TestImageDrawCelClipping();
	TestImageDrawCelMismatchedFormat();
	TestImageDrawCelClippingTransparentGaps();
	TestImageDrawCelRGBX();
	TestImageDrawCelDestAlphaRowEnds();
	TestImageDrawCelOpaqueOntoDestOpacity();
	TestImageDrawCelTranslucentBGROntoDestOpacity();
	TestImageDrawCelAlignment();
	TestImageDrawBlitterContextInit();
	TestImageDrawCelSizedOpaque();
	TestImageDrawCelSizedTranslucent();
	TestImageDrawCelSizedBorders();
	TestImageDrawCelSizedAlignment();
	TestImageDrawBlitterCacheOnImage();
	TestImageDrawBlitterCacheCelFreedFirst();
	TestImageDrawBlitterCacheDestinationFreedFirst();
	TestImageDrawBlitterCacheSharedCel();
	TestImageDrawBlitterCacheCelKilled();
	TestImageDrawBlitterCacheFreedTogether();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

