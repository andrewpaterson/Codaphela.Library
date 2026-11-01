#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/ImageAccessorCreator.h"
#include "SupportLib/ColourARGB32.h"
#include "SupportLib/GeometryMap.h"
#include "SupportLib/Maps.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// Red, green, blue and opacity (255 if the image has none).
//
//////////////////////////////////////////////////////////////////////////
void GetGeometryPixel(Ptr<CImage> pImage, int32 x, int32 y, uint8* puiRGBA)
{
	CImageAccessorCreator	cCreator;
	CImageAccessor*			pcAccessor;

	cCreator.Init(&pImage);
	if (pImage->HasChannel(IMAGE_OPACITY))
	{
		cCreator.AddAccess(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, PT_uint8);
		pcAccessor = cCreator.CreateAndKill();
		pcAccessor->Get(x, y, puiRGBA);
	}
	else
	{
		cCreator.AddAccess(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, PT_uint8);
		pcAccessor = cCreator.CreateAndKill();
		pcAccessor->Get(x, y, puiRGBA);
		puiRGBA[3] = 255;
	}
	pcAccessor->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertGeometryPixel(int iRed, int iGreen, int iBlue, int iOpacity, Ptr<CImage> pImage, int32 x, int32 y)
{
	uint8	auiRGBA[4];

	GetGeometryPixel(pImage, x, y, auiRGBA);
	AssertInt(iRed, auiRGBA[0]);
	AssertInt(iGreen, auiRGBA[1]);
	AssertInt(iBlue, auiRGBA[2]);
	AssertInt(iOpacity, auiRGBA[3]);
}


//////////////////////////////////////////////////////////////////////////
//
// Rectangles are filled with their right and bottom edges excluded.  A
// translucent colour is blended with what is there.
//
//////////////////////////////////////////////////////////////////////////
void TestGeometryMapRectangles(void)
{
	ObjectsInit();
	{
		Ptr<CImage>			pImage;
		Ptr<CGeometryMap>	pMap;
		CInt32Rect			cViewport;

		pImage = OMalloc<CImage>(8, 6, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pImage->Black();
		pMap = OMalloc<CGeometryMap>();
		pMap->SetViewport(pImage);
		AssertTrue(pMap->GetViewport() == pImage);

		pMap->AddRectangle(1, 1, 3, 2, Set32BitColour((uint8)255, (uint8)0, (uint8)0));
		pMap->AddRectangle(0, 0, 0, 5, Set32BitColour((uint8)255, (uint8)0, (uint8)0));
		pMap->AddRectangle(0, 0, 5, -1, Set32BitColour((uint8)255, (uint8)0, (uint8)0));
		AssertSize(1, pMap->NumCommands());
		AssertInt((int32)4, pMap->GetCommand(0)->cBounds.msBottomRight.x);
		AssertInt((int32)3, pMap->GetCommand(0)->cBounds.msBottomRight.y);

		//Half covered green, partly off the image.
		pMap->AddRectangle(5, 3, 10, 10, Set32BitColour((uint8)0, (uint8)255, (uint8)0, (uint8)128));
		AssertSize(2, pMap->NumCommands());

		cViewport.Init(0, 0, 8, 6);
		AssertTrue(pMap->Blit(&cViewport, false));
		AssertGeometryPixel(255, 0, 0, 255, pImage, 1, 1);
		AssertGeometryPixel(255, 0, 0, 255, pImage, 3, 2);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 4, 1);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 1, 3);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 0, 0);
		AssertGeometryPixel(0, 128, 0, 255, pImage, 5, 3);
		AssertGeometryPixel(0, 128, 0, 255, pImage, 7, 5);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 4, 3);

		pMap->Clear();
		AssertSize(0, pMap->NumCommands());

		pMap = NULL;
		pImage = NULL;
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// On an image with opacity cleared to nothing a translucent colour leaves
// premultiplied pixels, as a translucent cel does.
//
//////////////////////////////////////////////////////////////////////////
void TestGeometryMapOpacity(void)
{
	ObjectsInit();
	{
		Ptr<CImage>			pImage;
		Ptr<CGeometryMap>	pMap;
		CInt32Rect			cViewport;

		pImage = OMalloc<CImage>(4, 4, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP);
		pImage->Black();
		AssertGeometryPixel(0, 0, 0, 0, pImage, 0, 0);

		pMap = OMalloc<CGeometryMap>();
		pMap->SetViewport(pImage);
		pMap->AddRectangle(0, 0, 2, 1, Set32BitColour((uint8)255, (uint8)0, (uint8)0));
		pMap->AddRectangle(0, 1, 2, 1, Set32BitColour((uint8)255, (uint8)0, (uint8)0, (uint8)128));
		pMap->AddRectangle(0, 1, 1, 1, Set32BitColour((uint8)0, (uint8)0, (uint8)255, (uint8)128));
		cViewport.Init(0, 0, 4, 4);
		AssertTrue(pMap->Blit(&cViewport, false));

		AssertGeometryPixel(255, 0, 0, 255, pImage, 0, 0);
		AssertGeometryPixel(128, 0, 0, 128, pImage, 1, 1);

		//Half blue over half red: the red is halved, the opacity is 128 + 128 * 127 / 255.
		AssertGeometryPixel(64, 0, 128, 192, pImage, 0, 1);
		AssertGeometryPixel(0, 0, 0, 0, pImage, 2, 0);

		pMap = NULL;
		pImage = NULL;
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// The viewport rectangle's top left is drawn at the image's top left.
// Shapes outside it aren't drawn.
//
//////////////////////////////////////////////////////////////////////////
void TestGeometryMapViewport(void)
{
	ObjectsInit();
	{
		Ptr<CImage>			pImage;
		Ptr<CGeometryMap>	pMap;
		CInt32Rect			cViewport;

		pImage = OMalloc<CImage>(8, 6, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pImage->Black();
		pMap = OMalloc<CGeometryMap>();
		pMap->SetViewport(pImage);
		pMap->AddRectangle(3, 2, 1, 1, Set32BitColour((uint8)255, (uint8)255, (uint8)255));
		pMap->AddRectangle(-20, -20, 5, 5, Set32BitColour((uint8)255, (uint8)255, (uint8)255));
		pMap->AddRectangle(0, 0, 3, 1, Set32BitColour((uint8)255, (uint8)255, (uint8)255));
		pMap->AddRectangle(8, 5, 4, 4, Set32BitColour((uint8)255, (uint8)255, (uint8)255));

		cViewport.Init(2, 1, 10, 7);
		AssertTrue(pMap->Blit(&cViewport, false));
		AssertGeometryPixel(255, 255, 255, 255, pImage, 1, 1);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 0, 0);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 2, 1);
		AssertGeometryPixel(255, 255, 255, 255, pImage, 6, 4);
		AssertGeometryPixel(255, 255, 255, 255, pImage, 7, 5);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 5, 4);

		pMap = NULL;
		pImage = NULL;
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// Colours go to the image's own channels whatever their order.
//
//////////////////////////////////////////////////////////////////////////
void TestGeometryMapColourOrder(void)
{
	ObjectsInit();
	{
		Ptr<CImage>			pImage;
		Ptr<CGeometryMap>	pMap;
		CInt32Rect			cViewport;
		uint8*				puiData;

		pImage = OMalloc<CImage>(2, 1, PT_uint8, IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, CHANNEL_STOP);
		pImage->Black();
		AssertInt(CCO_BGR, pImage->GetColourOrder());
		pMap = OMalloc<CGeometryMap>();
		pMap->SetViewport(pImage);
		pMap->AddRectangle(0, 0, 1, 1, Set32BitColour((uint8)255, (uint8)16, (uint8)0));
		cViewport.Init(0, 0, 2, 1);
		AssertTrue(pMap->Blit(&cViewport, false));
		AssertGeometryPixel(255, 16, 0, 255, pImage, 0, 0);

		puiData = (uint8*)pImage->GetData();
		AssertInt(0, puiData[0]);
		AssertInt(16, puiData[1]);
		AssertInt(255, puiData[2]);

		pMap = NULL;
		pImage = NULL;
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// A geometry map is drawn by CMaps with the other maps, at the maps'
// viewport position.
//
//////////////////////////////////////////////////////////////////////////
void TestGeometryMapInMaps(void)
{
	ObjectsInit();
	{
		Ptr<CImage>			pImage;
		Ptr<CGeometryMap>	pMap;
		Ptr<CMaps>			pMaps;

		pImage = OMalloc<CImage>(8, 6, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pImage->Black();
		pMap = OMalloc<CGeometryMap>();
		AssertTrue(pMap->GetBlockMap() == NULL);
		pMap->AddRectangle(4, 3, 2, 1, Set32BitColour((uint8)0, (uint8)0, (uint8)255));

		pMaps = OMalloc<CMaps>();
		pMaps->AddMap(pMap);
		pMaps->SetCacheAndViewport(pImage->GetBlitterCache(), pImage);
		AssertTrue(pMap->GetViewport() == pImage);
		AssertTrue(pMaps->HasCelBlitters());
		pMaps->SetViewportPosition(1, 1);
		AssertTrue(pMaps->Blit(false));
		AssertGeometryPixel(0, 0, 255, 255, pImage, 3, 2);
		AssertGeometryPixel(0, 0, 255, 255, pImage, 4, 2);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 5, 2);

		pMaps = NULL;
		pMap = NULL;
		pImage = NULL;
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// Lines have a thickness and are centred on points at pixel centres.  A
// square cap reaches half the thickness past each end so a one pixel line
// covers its end pixels exactly, a butt cap stops at the ends and covers
// half of them.
//
//////////////////////////////////////////////////////////////////////////
void TestGeometryMapLines(void)
{
	ObjectsInit();
	{
		Ptr<CImage>			pImage;
		Ptr<CGeometryMap>	pMap;
		CInt32Rect			cViewport;
		ARGB32				sWhite;
		uint8				auiRGBA[4];
		int					x;
		int					y;

		pImage = OMalloc<CImage>(16, 12, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pImage->Black();
		pMap = OMalloc<CGeometryMap>();
		pMap->SetViewport(pImage);
		sWhite = Set32BitColour((uint8)255, (uint8)255, (uint8)255);

		pMap->AddLine(1, 1, 5, 1, 1, sWhite);
		pMap->AddLine(1, 3, 5, 3, 1, sWhite, GLC_Butt);
		pMap->AddLine(9, 1, 9, 4, 3, sWhite);
		pMap->AddLine(1, 5, 5, 5, 0, sWhite);
		pMap->AddLine(1, 5, 1, 5, 2, sWhite, GLC_Butt);
		AssertSize(3, pMap->NumCommands());
		AssertInt(GC_Line, pMap->GetCommand(0)->eType);
		AssertInt(GLC_Square, pMap->GetCommand(0)->eCap);
		AssertInt(GLC_Butt, pMap->GetCommand(1)->eCap);

		//A square capped dot.
		pMap->AddLine(13, 1, 13, 1, 1, sWhite);
		AssertSize(4, pMap->NumCommands());

		cViewport.Init(0, 0, 16, 12);
		AssertTrue(pMap->Blit(&cViewport, false));

		for (x = 1; x <= 5; x++)
		{
			AssertGeometryPixel(255, 255, 255, 255, pImage, x, 1);
			AssertGeometryPixel(0, 0, 0, 255, pImage, x, 0);
			AssertGeometryPixel(0, 0, 0, 255, pImage, x, 2);
		}
		AssertGeometryPixel(0, 0, 0, 255, pImage, 0, 1);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 6, 1);

		AssertGeometryPixel(128, 128, 128, 255, pImage, 1, 3);
		AssertGeometryPixel(255, 255, 255, 255, pImage, 2, 3);
		AssertGeometryPixel(255, 255, 255, 255, pImage, 4, 3);
		AssertGeometryPixel(128, 128, 128, 255, pImage, 5, 3);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 0, 3);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 6, 3);

		//Three thick from row 1 to row 4 with square caps is columns 8 to 10 and rows 0 to 5.
		for (y = 0; y <= 5; y++)
		{
			AssertGeometryPixel(0, 0, 0, 255, pImage, 7, y);
			for (x = 8; x <= 10; x++)
			{
				AssertGeometryPixel(255, 255, 255, 255, pImage, x, y);
			}
			AssertGeometryPixel(0, 0, 0, 255, pImage, 11, y);
		}
		AssertGeometryPixel(0, 0, 0, 255, pImage, 9, 6);

		AssertGeometryPixel(255, 255, 255, 255, pImage, 13, 1);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 12, 1);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 14, 1);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 13, 0);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 13, 2);

		//A diagonal line is the same either side of the diagonal.
		pImage->Black();
		pMap->Clear();
		pMap->AddLine(2, 2, 9, 9, 2, sWhite);
		AssertTrue(pMap->Blit(&cViewport, false));
		AssertGeometryPixel(255, 255, 255, 255, pImage, 5, 5);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 9, 2);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 2, 9);
		for (y = 0; y < 12; y++)
		{
			for (x = 0; x < 12; x++)
			{
				GetGeometryPixel(pImage, x, y, auiRGBA);
				AssertGeometryPixel(auiRGBA[0], auiRGBA[1], auiRGBA[2], 255, pImage, y, x);
			}
		}

		//A round cap is a half circle past the end.
		pImage->Black();
		pMap->Clear();
		pMap->AddLine(3, 5, 10, 5, 5, sWhite, GLC_Round);
		AssertTrue(pMap->Blit(&cViewport, false));
		AssertGeometryPixel(249, 249, 249, 255, pImage, 1, 5);
		AssertGeometryPixel(255, 255, 255, 255, pImage, 2, 5);
		AssertGeometryPixel(255, 255, 255, 255, pImage, 11, 5);
		AssertGeometryPixel(249, 249, 249, 255, pImage, 12, 5);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 0, 5);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 13, 5);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 0, 3);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 13, 7);
		AssertGeometryPixel(255, 255, 255, 255, pImage, 6, 3);
		AssertGeometryPixel(255, 255, 255, 255, pImage, 6, 7);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 6, 8);

		pMap = NULL;
		pImage = NULL;
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// Filled circles, such as trace junctions, and circle outlines with a
// thickness centred on the radius.
//
//////////////////////////////////////////////////////////////////////////
void TestGeometryMapCircles(void)
{
	ObjectsInit();
	{
		Ptr<CImage>			pImage;
		Ptr<CGeometryMap>	pMap;
		CInt32Rect			cViewport;
		ARGB32				sWhite;
		uint8				auiRGBA[4];
		uint8				auiMirror[4];
		int					x;
		int					y;

		pImage = OMalloc<CImage>(24, 12, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pImage->Black();
		pMap = OMalloc<CGeometryMap>();
		pMap->SetViewport(pImage);
		sWhite = Set32BitColour((uint8)255, (uint8)255, (uint8)255);

		pMap->AddFilledCircle(5, 5, 3, sWhite);
		pMap->AddCircle(17, 5, 4, 1, sWhite);
		pMap->AddFilledCircle(5, 5, 0, sWhite);
		pMap->AddCircle(5, 5, 3, 0, sWhite);
		AssertSize(2, pMap->NumCommands());
		AssertInt(GC_FilledCircle, pMap->GetCommand(0)->eType);
		AssertInt(GC_Circle, pMap->GetCommand(1)->eType);
		AssertInt((int32)2, pMap->GetCommand(0)->cBounds.msTopLeft.x);
		AssertInt((int32)9, pMap->GetCommand(0)->cBounds.msBottomRight.x);

		cViewport.Init(0, 0, 24, 12);
		AssertTrue(pMap->Blit(&cViewport, false));

		AssertGeometryPixel(255, 255, 255, 255, pImage, 5, 5);
		AssertGeometryPixel(255, 255, 255, 255, pImage, 4, 4);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 5, 1);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 1, 5);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 2, 2);

		//The filled circle and the ring are the same mirrored across both axes.
		for (y = 0; y <= 5; y++)
		{
			for (x = 0; x <= 5; x++)
			{
				GetGeometryPixel(pImage, x, y, auiRGBA);
				AssertGeometryPixel(auiRGBA[0], auiRGBA[1], auiRGBA[2], 255, pImage, 10 - x, y);
				AssertGeometryPixel(auiRGBA[0], auiRGBA[1], auiRGBA[2], 255, pImage, x, 10 - y);
				AssertGeometryPixel(auiRGBA[0], auiRGBA[1], auiRGBA[2], 255, pImage, 10 - x, 10 - y);

				GetGeometryPixel(pImage, 12 + x, y, auiRGBA);
				GetGeometryPixel(pImage, 22 - x, 10 - y, auiMirror);
				AssertInt(auiRGBA[0], auiMirror[0]);
			}
		}

		AssertGeometryPixel(0, 0, 0, 255, pImage, 17, 5);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 17, 3);
		AssertGeometryPixel(251, 251, 251, 255, pImage, 17, 1);
		AssertGeometryPixel(251, 251, 251, 255, pImage, 21, 5);
		AssertGeometryPixel(251, 251, 251, 255, pImage, 13, 5);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 17, 0);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 22, 5);

		//An outline thicker than the circle is wide fills it.
		pImage->Black();
		pMap->Clear();
		pMap->AddCircle(5, 5, 1, 4, sWhite);
		AssertTrue(pMap->Blit(&cViewport, false));
		AssertGeometryPixel(255, 255, 255, 255, pImage, 5, 5);
		AssertGeometryPixel(255, 255, 255, 255, pImage, 3, 5);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 1, 5);

		pMap = NULL;
		pImage = NULL;
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// Lines and circles blend as rectangles do: a translucent colour on an
// image cleared to nothing leaves premultiplied pixels, and they are
// drawn at the viewport rectangle's position.
//
//////////////////////////////////////////////////////////////////////////
void TestGeometryMapShapeOpacityAndViewport(void)
{
	ObjectsInit();
	{
		Ptr<CImage>			pImage;
		Ptr<CGeometryMap>	pMap;
		CInt32Rect			cViewport;

		pImage = OMalloc<CImage>(8, 8, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP);
		pImage->Black();
		pMap = OMalloc<CGeometryMap>();
		pMap->SetViewport(pImage);
		pMap->AddLine(10, 11, 14, 11, 1, Set32BitColour((uint8)255, (uint8)0, (uint8)0, (uint8)128));
		pMap->AddFilledCircle(13, 14, 1.5, Set32BitColour((uint8)0, (uint8)0, (uint8)255));
		pMap->AddLine(-10, -10, 2, -10, 1, Set32BitColour((uint8)255, (uint8)255, (uint8)255));

		cViewport.Init(8, 8, 16, 16);
		AssertTrue(pMap->Blit(&cViewport, false));
		AssertGeometryPixel(128, 0, 0, 128, pImage, 2, 3);
		AssertGeometryPixel(128, 0, 0, 128, pImage, 6, 3);
		AssertGeometryPixel(0, 0, 0, 0, pImage, 1, 3);
		AssertGeometryPixel(0, 0, 0, 0, pImage, 7, 3);
		AssertGeometryPixel(0, 0, 255, 255, pImage, 5, 6);
		AssertGeometryPixel(0, 0, 0, 0, pImage, 0, 0);

		pMap = NULL;
		pImage = NULL;
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// Polygons are filled with the non-zero rule whichever way they wind.
//
//////////////////////////////////////////////////////////////////////////
void TestGeometryMapPolygons(void)
{
	ObjectsInit();
	{
		Ptr<CImage>			pImage;
		Ptr<CGeometryMap>	pMap;
		CInt32Rect			cViewport;
		ARGB32				sWhite;
		SFloat64Vec2		asSquare[4];
		SFloat64Vec2		asReversed[4];
		int					x;
		int					y;

		pImage = OMalloc<CImage>(16, 8, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pImage->Black();
		pMap = OMalloc<CGeometryMap>();
		pMap->SetViewport(pImage);
		sWhite = Set32BitColour((uint8)255, (uint8)255, (uint8)255);

		//Pixel centres are whole numbers so 1.5 to 5.5 covers pixels 2 to 5 exactly.
		asSquare[0].Init(1.5, 1.5);
		asSquare[1].Init(5.5, 1.5);
		asSquare[2].Init(5.5, 5.5);
		asSquare[3].Init(1.5, 5.5);
		asReversed[0].Init(9.5, 1.5);
		asReversed[1].Init(9.5, 5.5);
		asReversed[2].Init(13.5, 5.5);
		asReversed[3].Init(13.5, 1.5);
		pMap->AddPolygon(asSquare, 4, sWhite);
		pMap->AddPolygon(asReversed, 4, sWhite);
		pMap->AddPolygon(asSquare, 2, sWhite);
		AssertSize(2, pMap->NumCommands());
		AssertInt(GC_Polygon, pMap->GetCommand(0)->eType);
		AssertSize(4, pMap->GetCommand(1)->uiFirstPoint);
		AssertSize(4, pMap->GetCommand(1)->uiNumPoints);
		AssertDouble(9.5, pMap->GetPoint(4)->x, 3);

		cViewport.Init(0, 0, 16, 8);
		AssertTrue(pMap->Blit(&cViewport, false));

		for (y = 0; y < 8; y++)
		{
			for (x = 0; x < 16; x++)
			{
				if ((y >= 2) && (y <= 5) && (((x >= 2) && (x <= 5)) || ((x >= 10) && (x <= 13))))
				{
					AssertGeometryPixel(255, 255, 255, 255, pImage, x, y);
				}
				else
				{
					AssertGeometryPixel(0, 0, 0, 255, pImage, x, y);
				}
			}
		}

		pMap->Clear();
		AssertSize(0, pMap->NumCommands());
	}
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Polylines are stroked with mitred joins and, when open, square caps.
// Where the segments and joins overlap they are drawn once.
//
//////////////////////////////////////////////////////////////////////////
void TestGeometryMapPolylines(void)
{
	ObjectsInit();
	{
		Ptr<CImage>			pImage;
		Ptr<CGeometryMap>	pMap;
		CInt32Rect			cViewport;
		ARGB32				sWhite;
		ARGB32				sGrey;
		SFloat64Vec2		asSquare[4];
		SFloat64Vec2		asCorner[3];
		SFloat64Vec2		asSharp[3];
		int					x;
		int					y;

		pImage = OMalloc<CImage>(24, 12, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pImage->Black();
		pMap = OMalloc<CGeometryMap>();
		pMap->SetViewport(pImage);
		sWhite = Set32BitColour((uint8)255, (uint8)255, (uint8)255);
		sGrey = Set32BitColour((uint8)255, (uint8)255, (uint8)255, (uint8)128);

		//A closed square one pixel thick covers its outline pixels exactly; the mitres fill the corners.
		asSquare[0].Init(2, 2);
		asSquare[1].Init(8, 2);
		asSquare[2].Init(8, 8);
		asSquare[3].Init(2, 8);
		pMap->AddPolyline(asSquare, 4, true, 1, sWhite);
		AssertInt(GC_Polyline, pMap->GetCommand(0)->eType);
		AssertTrue(pMap->GetCommand(0)->bClosed);
		AssertInt((int32)2, pMap->GetCommand(0)->cBounds.msTopLeft.x);
		AssertInt((int32)9, pMap->GetCommand(0)->cBounds.msBottomRight.x);

		//An open corner three pixels thick, translucent.  Its mitre fills the outside corner square and the overlaps
		//aren't blended twice.
		asCorner[0].Init(12, 2);
		asCorner[1].Init(18, 2);
		asCorner[2].Init(18, 8);
		pMap->AddPolyline(asCorner, 3, false, 3, sGrey);

		cViewport.Init(0, 0, 24, 12);
		AssertTrue(pMap->Blit(&cViewport, false));

		for (y = 1; y <= 9; y++)
		{
			for (x = 1; x <= 9; x++)
			{
				if ((x >= 2) && (x <= 8) && (y >= 2) && (y <= 8) && ((x == 2) || (x == 8) || (y == 2) || (y == 8)))
				{
					AssertGeometryPixel(255, 255, 255, 255, pImage, x, y);
				}
				else
				{
					AssertGeometryPixel(0, 0, 0, 255, pImage, x, y);
				}
			}
		}

		//Rows 1 to 3 from the square cap at column 11 to the mitre at column 19, then columns 17 to 19 down to the
		//square cap at row 9.
		for (x = 11; x <= 19; x++)
		{
			AssertGeometryPixel(128, 128, 128, 255, pImage, x, 1);
			AssertGeometryPixel(128, 128, 128, 255, pImage, x, 3);
			AssertGeometryPixel(0, 0, 0, 255, pImage, x, 0);
		}
		AssertGeometryPixel(0, 0, 0, 255, pImage, 10, 2);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 20, 1);
		for (y = 1; y <= 9; y++)
		{
			AssertGeometryPixel(128, 128, 128, 255, pImage, 17, y);
			AssertGeometryPixel(128, 128, 128, 255, pImage, 19, y);
			AssertGeometryPixel(0, 0, 0, 255, pImage, 16, (y > 3) ? y : 4);
		}
		AssertGeometryPixel(0, 0, 0, 255, pImage, 18, 10);

		//A very sharp turn is bevelled; a mitre would reach about 20 pixels past the turn.
		pMap->Clear();
		asSharp[0].Init(0, 0);
		asSharp[1].Init(20, 1);
		asSharp[2].Init(0, 2);
		pMap->AddPolyline(asSharp, 3, false, 2, sWhite);
		AssertTrue(pMap->GetCommand(0)->cBounds.msBottomRight.x <= 22);

		//A polyline that goes nowhere is a square dot.
		pMap->Clear();
		pImage->Black();
		asSharp[0].Init(3, 3);
		asSharp[1].Init(3, 3);
		pMap->AddPolyline(asSharp, 2, false, 1, sWhite);
		AssertTrue(pMap->Blit(&cViewport, false));
		AssertGeometryPixel(255, 255, 255, 255, pImage, 3, 3);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 2, 3);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 4, 3);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 3, 2);
		AssertGeometryPixel(0, 0, 0, 255, pImage, 3, 4);
	}
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGeometryMap(void)
{
	BeginTests();

	DataIOInit();

	TestGeometryMapRectangles();
	TestGeometryMapOpacity();
	TestGeometryMapViewport();
	TestGeometryMapColourOrder();
	TestGeometryMapInMaps();
	TestGeometryMapLines();
	TestGeometryMapCircles();
	TestGeometryMapPolygons();
	TestGeometryMapPolylines();
	TestGeometryMapShapeOpacityAndViewport();

	DataIOKill();

	TestStatistics();
}

