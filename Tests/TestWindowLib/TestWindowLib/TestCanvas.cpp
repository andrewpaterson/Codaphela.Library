#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/StdRandom.h"
#include "BaseLib/StringHelper.h"
#include "StandardLib/Objects.h"
#include "StandardLib/FreeObjects.h"
#include "SupportLib/ImageCelBlitterCache.h"
#include "SupportLib/ImageCopier.h"
#include "SupportLib/ImageColourConverter.h"
#include "WindowLib/Window.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/MapsCanvasDraw.h"
#include "WindowLib/ImageCanvasDraw.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/AssertFile.h"
#include "TestLib/AssertGeometric.h"
#include "TestRefWindowCanvasDraw.h"
#include "BorderCanvasDraw.h"
#include "TickTestRefWindow.h"
#include "NoImageRefWindowFactory.h"
#include "DarkForest.h"
#include "TickTestCanvas.h"
#include "DataTestRefWindow.h"
#include "SpaceCrusade.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestCanvasWriteImageRGB(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "CanvasWriteImage";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	ObjectsInit();
	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CCanvas>				pCanvas;
		Ptr<CTickTestCanvas>		pTick;
		SDataTestRefWindow			cData;
		Ptr<CFillContainer>			pFill;
		Ptr<CMapsCanvasDraw>		pDraw;
		Ptr<CRoot>					pRoot;
		CRandom						cRandom;
		Ptr<CImage>					pFloorPlanImage;
		Ptr<CTileMap>				pTileMap;
		Ptr<CTileMapGenerator>		pTileMapGenerator;
		Ptr<CArrayImageCel>			paBackgroundCels;
		Ptr<CMaps>					pMaps;
		CChars						szExpectedDirectory;
		CColourFormatHelper			cHelper;

		pFloorPlanImage = ReadSpaceImage("SpaceCrusade", "FloorPlan.png");

		cRandom.Init(2345);

		cNativeFactory.Init(&gcMemoryAllocator, 320, 200, szDirectory);

		pRoot = ORoot();

		pMaps = OMalloc<CMaps>();

		pTick = OMalloc<CTickTestCanvas>(&cData, pMaps, 25);
		pTestWindow = OMalloc<CWindow>("Space Crusade", &cNativeFactory, pTick, (CPointer)NULL, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		gcObjects.ValidateObjectsConsistency();

		pTestWindow->GetColourFormat(&cHelper);
		pRoot->Add(pTestWindow);

		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);

		pTileMapGenerator = OMalloc<CTileMapGenerator>(&cRandom);
		pRoot->Add(pTileMapGenerator);

		gcObjects.DisableValidation();

		pTileMapGenerator->AddTileGridSource("FloorPlan", pFloorPlanImage);
		paBackgroundCels = ReadSpaceCels("SpaceCrusade", "Tiles.png", 10, 3, &cHelper);
		AddSpacePatterns(pTileMapGenerator, paBackgroundCels);
		AddSpaceSources(pTileMapGenerator);

		pTileMap = pTileMapGenerator->Generate();

		pMaps->AddMap(pTileMap);
		pMaps->SetViewportPosition(0, 0);

		pDraw = OMalloc<CMapsCanvasDraw>(pMaps);
		pCanvas = OMalloc<CCanvas>(pTestWindow, pDraw);
		pFill->AddComponent(pCanvas);

		pTestWindow->Show();

		pRoot->Remove(pTestWindow);
		pTestWindow = NULL;
		pCanvas = NULL;
		pTick = NULL;
		pFill = NULL;
		pDraw = NULL;
		pRoot->RemoveAll();

		gcObjects.EnableValidation();

		cNativeFactory.Kill();

		szExpectedDirectory.Init(szDirectory);
		szExpectedDirectory.Replace("Output", "Input");

		AssertDirectory(szExpectedDirectory.Text(), szDirectory);
		szExpectedDirectory.Kill();
	}
	AssertSize(2, gcObjects.NumMemoryIndexes());  //Root and root-set.
	ObjectsFlush();
	ObjectsKill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestCanvasWriteImageBGRX(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "CanvasWriteImage";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	cNativeFactory.Init(&gcMemoryAllocator, 320, 200, szDirectory);

	ObjectsInit();
	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CCanvas>				pCanvas;
		Ptr<CTickTestCanvas>		pTick;
		SDataTestRefWindow			cData;
		Ptr<CFillContainer>			pFill;
		Ptr<CMapsCanvasDraw>		pDraw;
		Ptr<CRoot>					pRoot;
		CRandom						cRandom;
		Ptr<CImage>					pFloorPlanImage;
		Ptr<CTileMap>				pTileMap;
		Ptr<CTileMapGenerator>		pTileMapGenerator;
		Ptr<CArrayImageCel>			paBackgroundCels;
		Ptr<CMaps>					pMaps;
		CChars						szExpectedDirectory;
		CColourFormatHelper			cHelper;

		pFloorPlanImage = ReadSpaceImage("SpaceCrusade", "FloorPlan.png");

		cRandom.Init(2345);

		pRoot = ORoot();

		pMaps = OMalloc<CMaps>();

		pTick = OMalloc<CTickTestCanvas>(&cData, pMaps, 25);
		pTestWindow = OMalloc<CWindow>("Space Crusade", &cNativeFactory, pTick, (CPointer)NULL, CFT_RGBX, CCO_BGR, CRGB_24bit, ARGB_8bit);
		gcObjects.ValidateObjectsConsistency();

		pTestWindow->GetColourFormat(&cHelper);
		pRoot->Add(pTestWindow);

		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);

		pTileMapGenerator = OMalloc<CTileMapGenerator>(&cRandom);
		pRoot->Add(pTileMapGenerator);

		gcObjects.DisableValidation();

		pTileMapGenerator->AddTileGridSource("FloorPlan", pFloorPlanImage);
		paBackgroundCels = ReadSpaceCels("SpaceCrusade", "Tiles.png", 10, 3, &cHelper);
		AddSpacePatterns(pTileMapGenerator, paBackgroundCels);
		AddSpaceSources(pTileMapGenerator);

		pTileMap = pTileMapGenerator->Generate();

		pMaps->AddMap(pTileMap);
		pMaps->SetViewportPosition(0, 0);

		pDraw = OMalloc<CMapsCanvasDraw>(pMaps);
		pCanvas = OMalloc<CCanvas>(pTestWindow, pDraw);
		pFill->AddComponent(pCanvas);

		pTestWindow->Show();

		pRoot->Remove(pTestWindow);
		pTestWindow = NULL;
		pCanvas = NULL;
		pTick = NULL;
		pFill = NULL;
		pDraw = NULL;
		pRoot->RemoveAll();

		gcObjects.EnableValidation();

		szExpectedDirectory.Init(szDirectory);
		szExpectedDirectory.Replace("Output", "Input");

		AssertDirectory(szExpectedDirectory.Text(), szDirectory);
		szExpectedDirectory.Kill();
	}
	cNativeFactory.Kill();

	AssertSize(2, gcObjects.NumMemoryIndexes());  //Root and root-set.
	ObjectsFlush();
	ObjectsKill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestCanvasImageConvert2(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "CanvasWriteImage";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	cNativeFactory.Init(&gcMemoryAllocator, 320, 200, szDirectory);

	ObjectsInit();
	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CCanvas>				pCanvas;
		Ptr<CTickTestCanvas>		pTick;
		SDataTestRefWindow			cData;
		Ptr<CFillContainer>			pFill;
		Ptr<CMapsCanvasDraw>		pDraw;
		Ptr<CRoot>					pRoot;
		CRandom						cRandom;
		Ptr<CImage>					pFloorPlanImage;
		Ptr<CTileMap>				pTileMap;
		Ptr<CTileMapGenerator>		pTileMapGenerator;
		Ptr<CArrayImageCel>			paBackgroundCels;
		Ptr<CMaps>					pMaps;
		CColourFormatHelper			cHelper;
		Ptr<CFreeObjects>			pFree;
		CImage*						pcNativeCanvasImage;

		pFloorPlanImage = ReadSpaceImage("SpaceCrusade", "FloorPlan.png");

		cRandom.Init(2345);

		pRoot = ORoot();

		pMaps = OMalloc<CMaps>();

		pTick = OMalloc<CTickTestCanvas>(&cData, pMaps, 25);
		pTestWindow = OMalloc<CWindow>("Space Crusade", &cNativeFactory, pTick, (CPointer)NULL, CFT_RGBX, CCO_BGR, CRGB_24bit, ARGB_8bit);
		gcObjects.ValidateObjectsConsistency();

		pTestWindow->GetColourFormat(&cHelper);
		pRoot->Add(pTestWindow);

		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);

		pTileMapGenerator = OMalloc<CTileMapGenerator>(&cRandom);
		pRoot->Add(pTileMapGenerator);

		gcObjects.DisableValidation();

		pTileMapGenerator->AddTileGridSource("FloorPlan", pFloorPlanImage);
		paBackgroundCels = ReadSpaceCels("SpaceCrusade", "Tiles.png", 10, 3, &cHelper);
		AddSpacePatterns(pTileMapGenerator, paBackgroundCels);
		AddSpaceSources(pTileMapGenerator);

		pTileMap = pTileMapGenerator->Generate();

		pMaps->AddMap(pTileMap);
		pMaps->SetViewportPosition(0, 0);

		pDraw = OMalloc<CMapsCanvasDraw>(pMaps);
		pCanvas = OMalloc<CCanvas>(pTestWindow, pDraw);
		pFill->AddComponent(pCanvas);

		pCanvas->SetActualSize(10, 10);
		pCanvas->CreateNativeCanvas();
		pDraw->Draw(pCanvas);

		pCanvas = NULL;
		pTick = NULL;
		pFill = NULL;
		pDraw = NULL;
		pRoot->Remove(pTileMapGenerator);
		pRoot->Remove(pTestWindow);

		AssertSize(1, pRoot->NumObjects());
		pFree = pRoot->Get(FREE_OBJECTS_NAME);
		AssertTrue(pFree.IsNotNull());

		//The native canvas holds its image and a CImageCel over that image.
		AssertSize(2, pFree->NumObjects());
		if (StringCompare(pFree->UnsafeGetObject(0)->ClassName(), "CImage") == 0)
		{
			pcNativeCanvasImage = (CImage*)pFree->UnsafeGetObject(0);
			AssertString("CImageCel", pFree->UnsafeGetObject(1)->ClassName());
		}
		else
		{
			pcNativeCanvasImage = (CImage*)pFree->UnsafeGetObject(1);
			AssertString("CImageCel", pFree->UnsafeGetObject(0)->ClassName());
		}
		AssertString("CImage", pcNativeCanvasImage->ClassName());

		gcObjects.EnableValidation();
	}

	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();

	AssertSize(2, gcObjects.NumMemoryIndexes());  //Root and root-set.
	ObjectsFlush();
	ObjectsKill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestCanvasImageConvert(void)
{
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "CanvasWriteImage";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CImage>					pFloorPlanImage;
		Ptr<CTileMap>				pTileMap;
		Ptr<CTileMapGenerator>		pTileMapGenerator;
		Ptr<CArrayImageCel>			paBackgroundCels;
		CColourFormatHelper			cHelper;

		pRoot = ORoot();


		pTileMapGenerator = OMalloc<CTileMapGenerator>();
		pRoot->Add(pTileMapGenerator);

		gcObjects.DisableValidation();

		cHelper.Init(CFT_RGBX, CCO_BGR, CRGB_24bit, ARGB_8bit);
		paBackgroundCels = ReadSpaceCels("SpaceCrusade", "Tiles.png", 10, 3, &cHelper);
		AssertSize(30, paBackgroundCels->NumElements());

		AddSpacePatterns(pTileMapGenerator, paBackgroundCels);

		pRoot->RemoveAll();

		gcObjects.EnableValidation();
	}
	AssertSize(2, gcObjects.NumMemoryIndexes());  //Root and root-set.
	ObjectsFlush();
	ObjectsKill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void RenderCanvasDrawFrames(CWinRefWindowFactory* pcFactory, char* szDirectory, size uiNumFrames, SInt32Vec2* asMapsSizes, SInt32Vec2* asImageSizes, SInt32Vec2* asViewports, bool bClearWindow)
{
	CFileUtil	cFileUtil;

	//A maps canvas (a tile map with sprites over it) aligned to the top left and an image canvas aligned to the top right
	//of the same fill container, so they sit side by side.  Frame 0 is shown, then the canvases
	//are given each following size in turn and painted again.  Changing a canvas's size recreates its native canvas.
	//If asViewports is not NULL the maps viewport is moved before each frame too.
	//If bClearWindow is true the window is given a canvas draw that clears it to black before each frame.
	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	ObjectsInit();
	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CFillContainer>			pFill;
		Ptr<CCanvas>				pMapsCanvas;
		Ptr<CCanvas>				pImageCanvas;
		Ptr<CMapsCanvasDraw>		pMapsDraw;
		Ptr<CImageCanvasDraw>		pImageDraw;
		Ptr<CCanvasDraw>			pWindowDraw;
		Ptr<CTickTestRefWindow>	pTick;
		SDataTestRefWindow			cData;
		CRandom						cRandom;
		Ptr<CImage>					pFloorPlanImage;
		Ptr<CTileMap>				pTileMap;
		Ptr<CTileMapGenerator>		pTileMapGenerator;
		Ptr<CArrayImageCel>			paBackgroundCels;
		Ptr<CArrayImageCel>			paSpriteCels;
		Ptr<CSpriteMap>				pSpriteMap;
		Ptr<CMaps>					pMaps;
		CColourFormatHelper			cHelper;
		size						i;

		pcFactory->Init(&gcMemoryAllocator, 48, 32, szDirectory);
		pFloorPlanImage = ReadSpaceImage("SpaceCrusade", "FloorPlan.png");
		cRandom.Init(2345);

		//Allocated, not on the stack, because the window is rooted and stack objects can't be under the root.
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		if (bClearWindow)
		{
			pWindowDraw = OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)0, 0, 0));
		}
		pTestWindow = OMalloc<CWindow>("Canvas Draw Frames", pcFactory, pTick, pWindowDraw, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		//Rooted so that pointing to the window's (and the generator's) many objects doesn't make the objects work out
		//again which of them are still on the stack.
		ORoot()->Add(pTestWindow);
		pTestWindow->GetColourFormat(&cHelper);

		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);

		pTileMapGenerator = OMalloc<CTileMapGenerator>(&cRandom);
		ORoot()->Add(pTileMapGenerator);

		gcObjects.DisableValidation();

		pTileMapGenerator->AddTileGridSource("FloorPlan", pFloorPlanImage);
		paBackgroundCels = ReadSpaceCels("SpaceCrusade", "Tiles.png", 10, 3, &cHelper);
		AddSpacePatterns(pTileMapGenerator, paBackgroundCels);
		AddSpaceSources(pTileMapGenerator);
		pTileMap = pTileMapGenerator->Generate();

		pMaps = OMalloc<CMaps>();
		pMaps->AddMap(pTileMap);
		paSpriteCels = ReadSpaceCels("SpaceCrusade", "SoulSucker.png", 8, 1, &cHelper);
		pSpriteMap = OMalloc<CSpriteMap>();
		pSpriteMap->AddSprite(paSpriteCels->Get(7), 2, 2);
		pSpriteMap->AddSprite(paSpriteCels->Get(2), 9, 7);
		pMaps->AddMap(pSpriteMap);
		pMaps->SetViewportPosition(0, 0);

		pMapsDraw = OMalloc<CMapsCanvasDraw>(pMaps);
		pMapsCanvas = OMalloc<CCanvas>(pTestWindow, pMapsDraw);
		pMapsCanvas->SetAlignment(CSH_Left, CSV_Top);
		pFill->AddComponent(pMapsCanvas);

		pImageDraw = OMalloc<CImageCanvasDraw>(ReadDarkForestCel("Wolf.png", &cHelper));
		pImageCanvas = OMalloc<CCanvas>(pTestWindow, pImageDraw);
		pImageCanvas->SetAlignment(CSH_Right, CSV_Top);
		pFill->AddComponent(pImageCanvas);

		for (i = 0; i < uiNumFrames; i++)
		{
			pMapsCanvas->SetDesiredSize(asMapsSizes[i].x, asMapsSizes[i].y);
			pImageCanvas->SetDesiredSize(asImageSizes[i].x, asImageSizes[i].y);
			if (asViewports)
			{
				pMaps->SetViewportPosition(asViewports[i].x, asViewports[i].y);
			}
			if (i == 0)
			{
				pTestWindow->Show();
			}
			else
			{
				pTestWindow->Paint();
			}
		}

		//The window is unrooted first so that it (and its native canvases' free objects) is freed before RemoveAll.
		ORoot()->Remove(pTestWindow);
		ORoot()->Remove(pTileMapGenerator);
		pTestWindow = NULL;
		pTick = NULL;
		pFill = NULL;
		pMapsCanvas = NULL;
		pImageCanvas = NULL;
		pMapsDraw = NULL;
		pImageDraw = NULL;
		pWindowDraw = NULL;
		pFloorPlanImage = NULL;
		pTileMap = NULL;
		pTileMapGenerator = NULL;
		paBackgroundCels = NULL;
		paSpriteCels = NULL;
		pSpriteMap = NULL;
		pMaps = NULL;
		AssertTrue(ORoot()->RemoveAll());

		gcObjects.EnableValidation();
		pcFactory->Kill();
	}
	AssertSize(2, gcObjects.NumMemoryIndexes());  //Root and root-set.
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestCanvasDrawWithoutNativeImage(void)
{
	CWinRefWindowFactory		cImageFactory;
	CNoImageRefWindowFactory	cNoImageFactory;
	CFileUtil					cFileUtil;
	SInt32Vec2					asMapsSizes[3] = { SInt32Vec2(20, 16), SInt32Vec2(24, 20), SInt32Vec2(24, 20) };
	SInt32Vec2					asImageSizes[3] = { SInt32Vec2(-1, -1), SInt32Vec2(16, 14), SInt32Vec2(16, 14) };
	SInt32Vec2					asViewports[3] = { SInt32Vec2(0, 0), SInt32Vec2(0, 0), SInt32Vec2(3, 2) };

	//Without a native image the canvas draws blit into an image of their own, resize it when the canvas changes size
	//and copy it onto the canvas.  That must give exactly the frames drawing straight into the native image does.
	//The last frame keeps the same size but moves the viewport so the reused image must be drawn into again.
	RenderCanvasDrawFrames(&cImageFactory, "Output" _FS_ "CanvasDrawNativeImage", 3, asMapsSizes, asImageSizes, asViewports, false);
	RenderCanvasDrawFrames(&cNoImageFactory, "Output" _FS_ "CanvasDrawNoNativeImage", 3, asMapsSizes, asImageSizes, asViewports, false);

	AssertFile("Output" _FS_ "CanvasDrawNativeImage" _FS_ "Frame0.png", "Output" _FS_ "CanvasDrawNoNativeImage" _FS_ "Frame0.png");
	AssertFile("Output" _FS_ "CanvasDrawNativeImage" _FS_ "Frame1.png", "Output" _FS_ "CanvasDrawNoNativeImage" _FS_ "Frame1.png");
	AssertFile("Output" _FS_ "CanvasDrawNativeImage" _FS_ "Frame2.png", "Output" _FS_ "CanvasDrawNoNativeImage" _FS_ "Frame2.png");

	cFileUtil.RemoveDir("Output" _FS_ "CanvasDrawNativeImage");
	cFileUtil.RemoveDir("Output" _FS_ "CanvasDrawNoNativeImage");
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestCanvasDrawResize(void)
{
	CWinRefWindowFactory	cResizedFactory;
	CWinRefWindowFactory	cFreshFactory;
	CWinRefWindowFactory	cUnclearedFactory;
	CFileUtil				cFileUtil;
	SInt32Vec2				asMapsSizes[3] = { SInt32Vec2(20, 16), SInt32Vec2(24, 20), SInt32Vec2(20, 16) };
	SInt32Vec2				asImageSizes[3] = { SInt32Vec2(-1, -1), SInt32Vec2(20, 16), SInt32Vec2(-1, -1) };

	//Resizing a canvas gives it a new image so every blitter made for the old image, the sprites' included, must be remade.
	//A frame after resizing must therefore be exactly the frame drawn at that size from the start.  The window clears
	//itself each frame so nothing is left behind where the canvases were before they shrank.
	//Clearing happens before the canvases are drawn, so a single frame is the same whether the window clears or not.
	RenderCanvasDrawFrames(&cResizedFactory, "Output" _FS_ "CanvasDrawResized", 3, asMapsSizes, asImageSizes, NULL, true);
	RenderCanvasDrawFrames(&cFreshFactory, "Output" _FS_ "CanvasDrawFresh", 1, &asMapsSizes[1], &asImageSizes[1], NULL, true);
	RenderCanvasDrawFrames(&cUnclearedFactory, "Output" _FS_ "CanvasDrawUncleared", 1, &asMapsSizes[1], &asImageSizes[1], NULL, false);

	AssertFile("Output" _FS_ "CanvasDrawFresh" _FS_ "Frame0.png", "Output" _FS_ "CanvasDrawResized" _FS_ "Frame1.png");
	AssertFile("Output" _FS_ "CanvasDrawResized" _FS_ "Frame0.png", "Output" _FS_ "CanvasDrawResized" _FS_ "Frame2.png");
	AssertFile("Output" _FS_ "CanvasDrawUncleared" _FS_ "Frame0.png", "Output" _FS_ "CanvasDrawFresh" _FS_ "Frame0.png");

	cFileUtil.RemoveDir("Output" _FS_ "CanvasDrawResized");
	cFileUtil.RemoveDir("Output" _FS_ "CanvasDrawFresh");
	cFileUtil.RemoveDir("Output" _FS_ "CanvasDrawUncleared");
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size DrawBorderCanvasInFormat(EColourOrder eCanvasOrder)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "CanvasMismatchedFormat";
	size					uiNonBlack;

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	ObjectsInit();
	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CFillContainer>		pFill;
		Ptr<CCanvas>			pCanvas;
		Ptr<CBorderCanvasDraw>	pDraw;
		Ptr<CImage>				pWindowImage;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow		cData;
		SLogConfig				sLogConfig;
		CPointer				pNull;
		uint8*					puiData;
		size					i;

		cNativeFactory.Init(&gcMemoryAllocator, 24, 16, szDirectory);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Mismatched Format", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pDraw = OMalloc<CBorderCanvasDraw>(Set32BitColour(1.0f, 0, 0));
		pCanvas = OMalloc<CCanvas>(pTestWindow, pDraw, CFT_RGB, eCanvasOrder, CRGB_24bit, ARGB_None);
		pFill->AddComponent(pCanvas);

		sLogConfig = gcLogger.SetSilent();
		pTestWindow->Show();
		gcLogger.SetConfig(&sLogConfig);

		pWindowImage = pTestWindow->GetCanvas()->GetImageOrNull();
		puiData = (uint8*)pWindowImage->GetData();
		uiNonBlack = 0;
		for (i = 0; i < pWindowImage->GetByteSize(); i++)
		{
			if (puiData[i] != 0)
			{
				uiNonBlack++;
			}
		}

		pWindowImage = NULL;
		pCanvas = NULL;
		pDraw = NULL;
		pFill = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
	}
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
	ObjectsFlush();
	ObjectsKill();

	cFileUtil.RemoveDir(szDirectory);
	return uiNonBlack;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestCanvasMismatchedFormat(void)
{
	//A canvas in a different format to its window cannot be blitted onto it.  That is an error so nothing is drawn, but it must not crash.
	AssertTrue(DrawBorderCanvasInFormat(CCO_RGB) > 0);
	AssertSize(0, DrawBorderCanvasInFormat(CCO_BGR));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestCanvasNested(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "CanvasNested";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	ObjectsInit();
	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CFillContainer>		pWindowFill;
		Ptr<CCanvas>			pOuter;
		Ptr<CBorderCanvasDraw>	pOuterDraw;
		Ptr<CFillContainer>		pOuterFill;
		Ptr<CCanvas>			pInner;
		Ptr<CImageCanvasDraw>	pInnerDraw;
		Ptr<CImageCel>			pWolf;
		Ptr<CImage>				pWindowImage;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow		cData;
		CColourFormatHelper		cHelper;
		CPointer				pNull;
		SInt32Vec2				sVec;
		uint8*					puiWindow;
		uint8*					puiWolf;
		int32					x;
		int32					y;
		size					uiWrong;

		cNativeFactory.Init(&gcMemoryAllocator, 32, 24, szDirectory);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Nested Canvas", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pTestWindow->GetColourFormat(&cHelper);

		pWindowFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pWindowFill);

		//An outer canvas with a red border holds a container of its own with the wolf canvas centred in it.
		pOuterDraw = OMalloc<CBorderCanvasDraw>(Set32BitColour(1.0f, 0, 0));
		pOuter = OMalloc<CCanvas>(pTestWindow, pOuterDraw);
		pOuter->SetDesiredSize(20, 16);
		pOuter->SetAlignment(CSH_Left, CSV_Top);
		pWindowFill->AddComponent(pOuter);

		pOuterFill = OMalloc<CFillContainer>(pTestWindow);
		pOuter->SetContainer(pOuterFill);

		pWolf = ReadDarkForestCel("Wolf.png", &cHelper);
		pInnerDraw = OMalloc<CImageCanvasDraw>(pWolf);
		pInner = OMalloc<CCanvas>(pTestWindow, pInnerDraw);
		pOuterFill->AddComponent(pInner);

		pTestWindow->Show();

		sVec = pInner->GetPosition();
		AssertInt32Vec2(4, 3, &sVec);
		sVec = pInner->GetActualSize();
		AssertInt32Vec2(12, 10, &sVec);

		//The inner canvas must be drawn over the outer one, not hidden underneath it.
		pWindowImage = pTestWindow->GetCanvas()->GetImageOrNull();
		puiWindow = (uint8*)pWindowImage->GetData();
		puiWolf = (uint8*)pWolf->GetSourceImage()->GetData();
		uiWrong = 0;
		for (y = 0; y < 10; y++)
		{
			for (x = 0; x < 12; x++)
			{
				if (memcmp(&puiWindow[((4 + x) + (3 + y) * 32) * 3], &puiWolf[(x + y * 12) * 3], 3) != 0)
				{
					uiWrong++;
				}
			}
		}
		AssertSize(0, uiWrong);

		//And the outer canvas's red border is still there around it.
		AssertInt(255, puiWindow[0]);
		AssertInt(0, puiWindow[1]);
		AssertInt(0, puiWindow[2]);

		pWindowImage = NULL;
		pWolf = NULL;
		pInner = NULL;
		pInnerDraw = NULL;
		pOuterFill = NULL;
		pOuter = NULL;
		pOuterDraw = NULL;
		pWindowFill = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
	}
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
	ObjectsFlush();
	ObjectsKill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//																		//
// An opaque canvas covers what is behind it.  A transparent one is		//
// blended over it.														//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestCanvasTransparent(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "CanvasTransparent";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	ObjectsInit();
	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CClearCanvasDraw>	pWindowDraw;
		Ptr<CFillContainer>		pWindowFill;
		Ptr<CClearCanvasDraw>	pBlueDraw;
		Ptr<CCanvas>			pOpaque;
		Ptr<CCanvas>			pTransparent;
		Ptr<CCanvas>			pEmpty;
		Ptr<CImage>				pWindowImage;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow		cData;
		uint8*					puiWindow;
		uint8*					puiPixel;

		cNativeFactory.Init(&gcMemoryAllocator, 12, 4, szDirectory);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pWindowDraw = OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)255, 0, 0));
		pTestWindow = OMalloc<CWindow>("Transparent Canvas", &cNativeFactory, pTick, pWindowDraw, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pWindowFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pWindowFill);

		//Both canvases are cleared to half transparent blue.  The opaque one ignores the opacity.
		pBlueDraw = OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)0, 0, 255, 128));
		pOpaque = OMalloc<CCanvas>(pTestWindow, pBlueDraw);
		pOpaque->SetDesiredSize(4, 4);
		pOpaque->SetAlignment(CSH_Left, CSV_Top);
		pWindowFill->AddComponent(pOpaque);

		pTransparent = OMalloc<CCanvas>(pTestWindow, pBlueDraw);
		pTransparent->SetDesiredSize(4, 4);
		pTransparent->SetAlignment(CSH_Center, CSV_Top);
		pWindowFill->AddComponent(pTransparent);

		//Canvases are opaque unless they are made transparent, which gives their image an opacity channel.
		AssertFalse(pTransparent->IsTransparent());
		AssertInt(CFT_RGB, pTransparent->GetColourFormat());
		AssertInt(ARGB_None, pTransparent->GetAlphaBits());
		AssertTrue(pTransparent->SetTransparent(true));
		AssertTrue(pTransparent->IsTransparent());
		AssertInt(CFT_RGBA, pTransparent->GetColourFormat());
		AssertInt(ARGB_8bit, pTransparent->GetAlphaBits());

		//A transparent canvas with nothing drawn on it doesn't change what is behind it.
		pEmpty = OMalloc<CCanvas>(pTestWindow, (CPointer)NULL);
		pEmpty->SetDesiredSize(4, 4);
		pEmpty->SetAlignment(CSH_Right, CSV_Top);
		AssertTrue(pEmpty->SetTransparent(true));
		pWindowFill->AddComponent(pEmpty);

		pTestWindow->Show();

		pWindowImage = pTestWindow->GetCanvas()->GetImageOrNull();
		puiWindow = (uint8*)pWindowImage->GetData();

		puiPixel = &puiWindow[(1 + 2 * 12) * 3];
		AssertInt(0, puiPixel[0]);
		AssertInt(0, puiPixel[1]);
		AssertInt(255, puiPixel[2]);

		//Half the blue over half the red.
		puiPixel = &puiWindow[(5 + 2 * 12) * 3];
		AssertInt(127, puiPixel[0]);
		AssertInt(0, puiPixel[1]);
		AssertInt(128, puiPixel[2]);

		puiPixel = &puiWindow[(9 + 2 * 12) * 3];
		AssertInt(255, puiPixel[0]);
		AssertInt(0, puiPixel[1]);
		AssertInt(0, puiPixel[2]);

		//And opaque again.
		AssertTrue(pTransparent->SetTransparent(false));
		AssertInt(CFT_RGB, pTransparent->GetColourFormat());
		AssertInt(ARGB_None, pTransparent->GetAlphaBits());

		pWindowImage = NULL;
		pEmpty = NULL;
		pTransparent = NULL;
		pOpaque = NULL;
		pBlueDraw = NULL;
		pWindowFill = NULL;
		pWindowDraw = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
	}
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
	ObjectsFlush();
	ObjectsKill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestCanvasResizeReleasesOldCels(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "CanvasResizeReleasesOldCels";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	ObjectsInit();
	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CFillContainer>		pFill;
		Ptr<CCanvas>			pImageCanvas;
		Ptr<CCanvas>			pBorderCanvas;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow		cData;
		CColourFormatHelper		cHelper;
		CPointer				pNull;
		size					i;
		size					uiObjectsAfterFirstPair;

		cNativeFactory.Init(&gcMemoryAllocator, 48, 32, szDirectory);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Resize Releases Old Cels", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pTestWindow->GetColourFormat(&cHelper);

		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);

		pImageCanvas = OMalloc<CCanvas>(pTestWindow, OMalloc<CImageCanvasDraw>(ReadDarkForestCel("Wolf.png", &cHelper)));
		pImageCanvas->SetAlignment(CSH_Left, CSV_Top);
		pFill->AddComponent(pImageCanvas);
		pBorderCanvas = OMalloc<CCanvas>(pTestWindow, OMalloc<CBorderCanvasDraw>(Set32BitColour(1.0f, 0, 0)));
		pBorderCanvas->SetAlignment(CSH_Right, CSV_Top);
		pFill->AddComponent(pBorderCanvas);

		//Every resize gives the two canvases new images and cels, which are blitted onto the window's image through its
		//blitter cache.  The cache must let go of the old cels, so the same sizes always leave the same number of objects.
		pTestWindow->Show();
		uiObjectsAfterFirstPair = 0;
		for (i = 0; i < 8; i++)
		{
			if (i % 2 == 0)
			{
				pImageCanvas->SetDesiredSize(16, 14);
				pBorderCanvas->SetDesiredSize(10, 10);
			}
			else
			{
				pImageCanvas->SetDesiredSize(-1, -1);
				pBorderCanvas->SetDesiredSize(14, 12);
			}
			pTestWindow->Paint();

			if (i == 1)
			{
				uiObjectsAfterFirstPair = gcObjects.NumMemoryIndexes();
			}
			else if (i % 2 == 1)
			{
				AssertSize(uiObjectsAfterFirstPair, gcObjects.NumMemoryIndexes());
			}
		}

		pBorderCanvas = NULL;
		pImageCanvas = NULL;
		pFill = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
	}
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
	ObjectsFlush();
	ObjectsKill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestCanvas(void)
{
	BeginTests();

	TestCanvasImageConvert();
	TestCanvasWriteImageRGB();
	TestCanvasImageConvert2();
	TestCanvasWriteImageBGRX();
	TestCanvasDrawWithoutNativeImage();
	TestCanvasDrawResize();
	TestCanvasMismatchedFormat();
	TestCanvasNested();
	TestCanvasTransparent();
	TestCanvasResizeReleasesOldCels();

	TestStatistics();
}

