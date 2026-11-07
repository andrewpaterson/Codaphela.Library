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
#include "WindowLib/RoundedBoxCanvasDraw.h"
#include "WindowLib/ProgressBarDraw.h"
#include "WindowLib/ScrollPane.h"
#include "WindowLib/GridContainer.h"
#include "WindowLib/TextArea.h"
#include "WindowLib/WindowTheme.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/AssertFile.h"
#include "TestLib/AssertGeometric.h"
#include "TestRefWindowCanvasDraw.h"
#include "BorderCanvasDraw.h"
#include "BoxFillCanvasDraw.h"
#include "TickTestRefWindow.h"
#include "NoImageRefWindowFactory.h"
#include "DarkForest.h"
#include "TickTestCanvas.h"
#include "DataTestRefWindow.h"
#include "SpaceCrusade.h"


Ptr<CWindowTheme> CreateTestTheme(Ptr<CFont> pFont);




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

		//The native canvas's image was held by its canvas, so went with the window (it used to be left in the root's
		//free objects).
		AssertSize(0, pRoot->NumObjects());

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
		CColourFormatHelper		cFormat;

		cNativeFactory.Init(&gcMemoryAllocator, 24, 16, szDirectory);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Mismatched Format", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pDraw = OMalloc<CBorderCanvasDraw>(Set32BitColour(1.0f, 0, 0));
		cFormat.Init(CFT_RGB, eCanvasOrder, CRGB_24bit, ARGB_None);
		pCanvas = OMalloc<CCanvas>(pTestWindow, pDraw, &cFormat);
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
		CColourFormatHelper		cFormat;

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
		pTransparent->GetColourFormat(&cFormat);
		AssertInt(CFT_RGBA, cFormat.meFormat);
		AssertInt(CCO_RGB, cFormat.meOrder);
		AssertInt(CRGB_24bit, cFormat.meColourBits);
		AssertInt(ARGB_8bit, cFormat.meAlphaBits);

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
// A window made from another's colour format, as a dialog is, has the	//
// same format, and so do canvases made without one.					//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestCanvasWindowColourFormat(void)
{
	CWinRefWindowFactory	cNativeFactory;

	ObjectsInit();
	{
		Ptr<CWindow>			pOwner;
		Ptr<CWindow>			pCopy;
		Ptr<CCanvas>			pCanvas;
		Ptr<CTickTestRefWindow>	pTick;
		SDataTestRefWindow		cData;
		CPointer				pNull;
		CColourFormatHelper		cFormat;
		CColourFormatHelper		cCopyFormat;

		cNativeFactory.Init(&gcMemoryAllocator, 12, 4, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pOwner = OMalloc<CWindow>("Owner", &cNativeFactory, pTick, pNull, CFT_RGBX, CCO_BGR, CRGB_24bit, ARGB_8bit);
		ORoot()->Add(pOwner);
		pOwner->GetColourFormat(&cFormat);
		AssertInt(CFT_RGBX, cFormat.meFormat);
		AssertInt(CCO_BGR, cFormat.meOrder);
		AssertInt(CRGB_24bit, cFormat.meColourBits);
		AssertInt(ARGB_8bit, cFormat.meAlphaBits);
		AssertInt(CCO_BGR, pOwner->GetColourOrder());

		pCopy = OMalloc<CWindow>("Copy", &cNativeFactory, pTick, pNull, &cFormat);
		ORoot()->Add(pCopy);
		pCopy->GetColourFormat(&cCopyFormat);
		AssertInt(CFT_RGBX, cCopyFormat.meFormat);
		AssertInt(CCO_BGR, cCopyFormat.meOrder);
		AssertInt(CRGB_24bit, cCopyFormat.meColourBits);
		AssertInt(ARGB_8bit, cCopyFormat.meAlphaBits);

		pCanvas = OMalloc<CCanvas>(pCopy, pNull);
		AssertInt(CFT_RGBX, pCanvas->GetColourFormat());
		AssertInt(CCO_BGR, pCanvas->GetColourOrder());
		AssertInt(CRGB_24bit, pCanvas->GetColourBits());
		AssertInt(ARGB_8bit, pCanvas->GetAlphaBits());

		pCanvas = NULL;
		ORoot()->Remove(pCopy);
		ORoot()->Remove(pOwner);
		pCopy = NULL;
		pOwner = NULL;
		pTick = NULL;
	}
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
// A canvas still held after its window is killed frees its native		//
// canvas through the factory that made it, not through the window.		//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestCanvasOutlivesKilledWindow(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "CanvasOutlivesKilledWindow";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	ObjectsInit();
	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CFillContainer>		pWindowFill;
		Ptr<CBoxFillCanvasDraw>	pBlueDraw;
		Ptr<CCanvas>			pCanvas;
		Ptr<CTickTestRefWindow>	pTick;
		SDataTestRefWindow		cData;

		cNativeFactory.Init(&gcMemoryAllocator, 12, 4, szDirectory);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Killed Window", &cNativeFactory, pTick, (CPointer)NULL, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pWindowFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pWindowFill);

		pBlueDraw = OMalloc<CBoxFillCanvasDraw>(Set32BitColour((uint8)0, 0, 255));
		pCanvas = OMalloc<CCanvas>(pTestWindow, pBlueDraw);
		pCanvas->SetDesiredSize(4, 4);
		pWindowFill->AddComponent(pCanvas);

		pTestWindow->Show();
		AssertTrue(pCanvas->GetImageOrNull().IsNotNull());

		pWindowFill = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow->Kill();
		AssertTrue(pTestWindow.IsNull());
		AssertTrue(pCanvas.IsNotNull());

		pCanvas = NULL;
		pBlueDraw = NULL;
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
// An image draw onto a canvas without an image of its own blits into	//
// one of the canvas's format.  Made transparent, the canvas has an		//
// opacity channel, and so must that image: around the cel the canvas	//
// stays clear, not opaque black.										//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestCanvasImageDrawFollowsTransparency(void)
{
	CNoImageRefWindowFactory	cNativeFactory;
	CFileUtil					cFileUtil;
	char						szDirectory[] = "Output" _FS_ "CanvasImageDrawFollowsTransparency";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	ObjectsInit();
	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CFillContainer>		pWindowFill;
		Ptr<CImage>				pImage;
		Ptr<CImageCanvasDraw>	pDraw;
		Ptr<CCanvas>			pCanvas;
		Ptr<CTickTestRefWindow>	pTick;
		SDataTestRefWindow		cData;
		uint8*					puiPixels;

		cNativeFactory.Init(&gcMemoryAllocator, 12, 4, szDirectory);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Image Draw Transparency", &cNativeFactory, pTick, (CPointer)NULL, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pWindowFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pWindowFill);

		pImage = OMalloc<CImage>(2, 2, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		pImage->Black();
		pDraw = OMalloc<CImageCanvasDraw>(OMalloc<CImageCel>(pImage));
		pCanvas = OMalloc<CCanvas>(pTestWindow, pDraw);
		pCanvas->SetDesiredSize(4, 4);
		pCanvas->SetAlignment(CSH_Left, CSV_Top);
		pWindowFill->AddComponent(pCanvas);

		pTestWindow->Show();
		AssertTrue(pCanvas->GetImageOrNull().IsNull());

		AssertTrue(pCanvas->SetTransparent(true));
		pTestWindow->Paint();

		puiPixels = pCanvas->GetPixelData();
		AssertInt(255, puiPixels[(0 + 0 * 4) * 4 + 3]);
		AssertInt(0, puiPixels[(3 + 3 * 4) * 4 + 3]);

		pCanvas = NULL;
		pDraw = NULL;
		pImage = NULL;
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
// Counts the native canvases asked for one canvas.						//
//																		//
//////////////////////////////////////////////////////////////////////////
class CCountingCanvasFactory : public CWinRefWindowFactory
{
CONSTRUCTABLE(CCountingCanvasFactory);
public:
	CCanvas*	mpcCounted;
	int			miCreates;

	CNativeCanvas* CreateNativeCanvas(CCanvas* pcCanvas) override
	{
		if (pcCanvas == mpcCounted)
		{
			miCreates++;
		}
		return CWinRefWindowFactory::CreateNativeCanvas(pcCanvas);
	}
};


//////////////////////////////////////////////////////////////////////////
//																		//
// A canvas with no area has no native canvas, and the factory isn't	//
// asked for one on every paint.  It gets one when it has an area and	//
// loses it again when it hasn't.										//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestCanvasZeroAreaHasNoNativeCanvas(void)
{
	CCountingCanvasFactory	cFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "CanvasZeroArea";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	ObjectsInit();
	cFactory.Init(&gcMemoryAllocator, 16, 16, szDirectory);
	cFactory.mpcCounted = NULL;
	cFactory.miCreates = 0;
	{
		Ptr<CWindow>				pWindow;
		Ptr<CFillContainer>			pFill;
		Ptr<CCanvas>				pCanvas;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;

		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pWindow = OMalloc<CWindow>("Zero Area", &cFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);

		pFill = OMalloc<CFillContainer>(pWindow);
		pWindow->SetContainer(pFill);
		pCanvas = OMalloc<CCanvas>(pWindow, OMalloc<CBoxFillCanvasDraw>(Set32BitColour(1.0f, 0, 0)));
		pCanvas->SetDesiredSize(0, 0);
		pCanvas->SetAlignment(CSH_Left, CSV_Top);
		pFill->AddComponent(pCanvas);
		cFactory.mpcCounted = &pCanvas;

		pWindow->Paint();
		pWindow->Paint();
		pWindow->Paint();
		AssertInt(0, cFactory.miCreates);
		AssertNull(pCanvas->GetNativeCanvas());
		AssertTrue(pCanvas->GetImageOrNull().IsNull());

		pCanvas->SetDesiredSize(4, 3);
		pWindow->Paint();
		pWindow->Paint();
		AssertInt(1, cFactory.miCreates);
		AssertNotNull(pCanvas->GetNativeCanvas());

		pCanvas->SetDesiredSize(0, 3);
		pWindow->Paint();
		pWindow->Paint();
		AssertInt(1, cFactory.miCreates);
		AssertNull(pCanvas->GetNativeCanvas());

		pCanvas = NULL;
		pFill = NULL;
		ORoot()->Remove(pWindow);
		pWindow = NULL;
		pTick = NULL;
	}
	AssertTrue(ORoot()->RemoveAll());
	cFactory.Kill();
	ObjectsFlush();
	ObjectsKill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//																		//
// The canvas's own drawing is cut to its native canvas.  An outline	//
// cut by its edge loses only the sides that are off it.					//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestCanvasDrawBoxCutToCanvas(void)
{
	CWinRefWindowFactory	cFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "CanvasDrawBoxCut";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	ObjectsInit();
	cFactory.Init(&gcMemoryAllocator, 16, 16, szDirectory);
	{
		Ptr<CWindow>				pWindow;
		Ptr<CFillContainer>			pFill;
		Ptr<CCanvas>				pCanvas;
		Ptr<CImage>					pImage;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		CInt32Rect					cRect;
		uint8*						puiData;
		int32						x;
		int32						y;
		size						uiWrong;
		bool						bRed;
		bool						bGreen;

		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pWindow = OMalloc<CWindow>("Box Cut", &cFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);

		pFill = OMalloc<CFillContainer>(pWindow);
		pWindow->SetContainer(pFill);
		pCanvas = OMalloc<CCanvas>(pWindow, OMalloc<CBoxFillCanvasDraw>(Set32BitColour(0.0f, 0, 0)));
		pCanvas->SetDesiredSize(8, 8);
		pCanvas->SetAlignment(CSH_Left, CSV_Top);
		pFill->AddComponent(pCanvas);
		pWindow->Paint();

		cRect.Init(-2, -2, 5, 5);
		pCanvas->DrawBox(&cRect, false, Set32BitColour(1.0f, 0, 0));
		cRect.Init(6, 6, 20, 20);
		pCanvas->DrawBox(&cRect, true, Set32BitColour(0, 1.0f, 0));
		pCanvas->DrawPixel(-1, 3, Set32BitColour(0, 0, 1.0f));
		pCanvas->DrawPixel(8, 3, Set32BitColour(0, 0, 1.0f));
		pCanvas->DrawPixel(3, 8, Set32BitColour(0, 0, 1.0f));

		pImage = pCanvas->GetImageOrNull();
		AssertNotNull(&pImage);
		AssertInt(8, pImage->GetWidth());
		AssertInt(8, pImage->GetHeight());
		puiData = (uint8*)pImage->GetData();
		uiWrong = 0;
		for (y = 0; y < 8; y++)
		{
			for (x = 0; x < 8; x++)
			{
				bRed = ((x == 5) && (y <= 5)) || ((y == 5) && (x <= 5));
				bGreen = (x >= 6) && (y >= 6);
				if ((puiData[(x + y * 8) * 3] != (bRed ? 255 : 0)) ||
					(puiData[(x + y * 8) * 3 + 1] != (bGreen ? 255 : 0)) ||
					(puiData[(x + y * 8) * 3 + 2] != 0))
				{
					uiWrong++;
				}
			}
		}
		AssertSize(0, uiWrong);

		pImage = NULL;
		pCanvas = NULL;
		pFill = NULL;
		ORoot()->Remove(pWindow);
		pWindow = NULL;
		pTick = NULL;
	}
	AssertTrue(ORoot()->RemoveAll());
	cFactory.Kill();
	ObjectsFlush();
	ObjectsKill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//																		//
// What a canvas, made by CreateClippedContent, is drawn as in a		//
// scroll pane.															//
//																		//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> ReadLabelFont(void);
Ptr<CScrollBarImages> CreateScrollBarImages(void);
Ptr<CWindowTheme> CreateScrollPaneTheme(Ptr<CScrollBarImages> pImages);

enum EClippedContent
{
	CC_Fill,
	CC_Solid,
	CC_Border,
	CC_Pixels,
	CC_Image,
	CC_StretchedImage,
	CC_RoundedBox,
	CC_ProgressBar,
	CC_TextArea,
};

#define CLIPPED_CONTENT_WIDTH	40
#define CLIPPED_CONTENT_HEIGHT	30
#define CLIPPED_VIEW_WIDTH		24
#define CLIPPED_VIEW_HEIGHT		16


Ptr<CCanvas> CreateClippedContent(Ptr<CWindow> pWindow, EClippedContent eContent, SDataTestRefWindow* pcData)
{
	CColourFormatHelper		cHelper;
	SInsets					sInsets;
	Ptr<CCanvas>			pCanvas;
	Ptr<CTextArea>			pTextArea;

	pWindow->GetColourFormat(&cHelper);
	switch (eContent)
	{
	case CC_Fill:
		pCanvas = OMalloc<CCanvas>(pWindow, OMalloc<CBoxFillCanvasDraw>(Set32BitColour(0.25f, 0.5f, 0.75f)));
		break;
	case CC_Solid:
		pCanvas = OMalloc<CCanvas>(pWindow, OMalloc<CClearCanvasDraw>(Set32BitColour(0.25f, 0.5f, 0.75f)));
		break;
	case CC_Border:
		pCanvas = OMalloc<CCanvas>(pWindow, OMalloc<CBorderCanvasDraw>(Set32BitColour(1.0f, 0, 0)));
		break;
	case CC_Pixels:
		pcData->iTick = 0;
		pCanvas = OMalloc<CCanvas>(pWindow, OMalloc<CTestRefWindowCanvasDraw>(pcData));
		break;
	case CC_Image:
		pCanvas = OMalloc<CCanvas>(pWindow, OMalloc<CImageCanvasDraw>(ReadDarkForestCel("Wolf.png", &cHelper)));
		break;
	case CC_StretchedImage:
		pCanvas = OMalloc<CCanvas>(pWindow, OMalloc<CImageCanvasDraw>(ReadDarkForestCel("Wolf.png", &cHelper), true));
		break;
	case CC_RoundedBox:
		sInsets.Init(3, 3);
		pCanvas = OMalloc<CCanvas>(pWindow, OMalloc<CRoundedBoxCanvasDraw>(Set32BitColour(0, 1.0f, 0.5f), 6, &sInsets));
		pCanvas->SetTransparent(true);
		break;
	case CC_ProgressBar:
		pCanvas = OMalloc<CCanvas>(pWindow, OMalloc<CProgressBarDraw>(Set32BitColour(0.5f, 0.5f, 0.5f), Set32BitColour(1.0f, 1.0f, 0)));
		break;
	case CC_TextArea:
		pTextArea = OMalloc<CTextArea>(pWindow, CreateTestTheme(Ptr<CFont>()), ReadLabelFont(), "The quick brown\nfox jumps over\nthe lazy dog");
		return pTextArea;
	}
	pCanvas->SetDesiredSize(CLIPPED_CONTENT_WIDTH, CLIPPED_CONTENT_HEIGHT);
	return pCanvas;
}


//////////////////////////////////////////////////////////////////////////
//																		//
// A canvas in a scroll pane has a native canvas only the size of the	//
// view, and draws in the window just as the same canvas drawn whole	//
// does, wherever it is scrolled to.									//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestCanvasClippedContent(EClippedContent eContent)
{
	CWinRefWindowFactory	cWholeFactory;
	CWinRefWindowFactory	cScrolledFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "CanvasClipped";
	SInt32Vec2				asScrolls[] = { { 0, 0 }, { 10, 7 }, { 16, 14 }, { 3, 12 }, { 0, 0 } };

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	ObjectsInit();
	cWholeFactory.Init(&gcMemoryAllocator, 64, 48, szDirectory);
	cScrolledFactory.Init(&gcMemoryAllocator, CLIPPED_VIEW_WIDTH, CLIPPED_VIEW_HEIGHT, szDirectory);
	{
		Ptr<CWindow>				pWhole;
		Ptr<CWindow>				pScrolled;
		Ptr<CFillContainer>			pFill;
		Ptr<CScrollPane>			pScrollPane;
		Ptr<CCanvas>				pWholeContent;
		Ptr<CCanvas>				pContent;
		Ptr<CScrollBar>				pBar;
		Ptr<CImage>					pWholeImage;
		Ptr<CImage>					pImage;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		SInt32Vec2					sContentSize;
		SInt32Vec2					sScroll;
		SInt32Vec2					sVec;
		CInt32Rect					acBars[2];
		size						uiBars;
		size						uiBar;
		size						uiScroll;
		uint8*						puiWhole;
		uint8*						puiData;
		int32						x;
		int32						y;
		size						uiWrong;
		size						uiCompared;
		bool						bUnderBar;

		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);

		//The content drawn whole, at its top left, as the reference.  The windows are cleared before each paint as what is
		//behind a transparent canvas shows through it.
		pWhole = OMalloc<CWindow>("Whole", &cWholeFactory, pTick, OMalloc<CClearCanvasDraw>(Set32BitColour(0.0f, 0, 0)), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWhole);
		pFill = OMalloc<CFillContainer>(pWhole);
		pWhole->SetContainer(pFill);
		pWholeContent = CreateClippedContent(pWhole, eContent, &cData);
		pWholeContent->SetAlignment(CSH_Left, CSV_Top);
		pFill->AddComponent(pWholeContent);
		pWhole->Paint();
		sContentSize = pWholeContent->GetActualSize();
		AssertTrue(sContentSize.x > CLIPPED_VIEW_WIDTH);
		AssertTrue(sContentSize.y > CLIPPED_VIEW_HEIGHT);
		pWholeImage = pWhole->GetCanvas()->GetImageOrNull();
		puiWhole = (uint8*)pWholeImage->GetData();

		pScrolled = OMalloc<CWindow>("Scrolled", &cScrolledFactory, pTick, OMalloc<CClearCanvasDraw>(Set32BitColour(0.0f, 0, 0)), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pScrolled);
		pFill = OMalloc<CFillContainer>(pScrolled);
		pScrolled->SetContainer(pFill);
		pScrollPane = OMalloc<CScrollPane>(pScrolled, CreateScrollPaneTheme(CreateScrollBarImages()));
		pFill->AddComponent(pScrollPane);
		pContent = CreateClippedContent(pScrolled, eContent, &cData);
		pContent->SetAlignment(CSH_Left, CSV_Top);
		pScrollPane->SetContent(pContent);

		for (uiScroll = 0; uiScroll < sizeof(asScrolls) / sizeof(SInt32Vec2); uiScroll++)
		{
			pScrollPane->SetScrollPosition(asScrolls[uiScroll].x, asScrolls[uiScroll].y);
			pScrolled->Paint();
			sScroll = pScrollPane->GetScrollPosition();
			AssertInt(asScrolls[uiScroll].x, sScroll.x);
			AssertInt(asScrolls[uiScroll].y, sScroll.y);

			//Only the view's worth of the content shows.  A solid colour fills it in the window without a native canvas.
			sVec = pContent->GetNativeOrigin();
			AssertInt32Vec2(sScroll.x, sScroll.y, &sVec);
			sVec = pContent->GetNativeSize();
			AssertInt32Vec2(CLIPPED_VIEW_WIDTH, CLIPPED_VIEW_HEIGHT, &sVec);
			if (eContent == CC_Solid)
			{
				AssertNull(pContent->GetNativeCanvas());
			}
			else
			{
				AssertInt(CLIPPED_VIEW_WIDTH, pContent->GetImageOrNull()->GetWidth());
				AssertInt(CLIPPED_VIEW_HEIGHT, pContent->GetImageOrNull()->GetHeight());
			}

			//The scroll bars are drawn over the content, so what is under them isn't compared.
			uiBars = 0;
			pBar = pScrollPane->GetScrollBar(SBO_Horizontal);
			if (pBar->IsShown())
			{
				acBars[uiBars].Init(pBar->GetPosition(), SInt32Vec2(pBar->GetPosition().x + pBar->GetActualSize().x, pBar->GetPosition().y + pBar->GetActualSize().y));
				uiBars++;
			}
			pBar = pScrollPane->GetScrollBar(SBO_Vertical);
			if (pBar->IsShown())
			{
				acBars[uiBars].Init(pBar->GetPosition(), SInt32Vec2(pBar->GetPosition().x + pBar->GetActualSize().x, pBar->GetPosition().y + pBar->GetActualSize().y));
				uiBars++;
			}

			pImage = pScrolled->GetCanvas()->GetImageOrNull();
			puiData = (uint8*)pImage->GetData();
			uiWrong = 0;
			uiCompared = 0;
			for (y = 0; y < CLIPPED_VIEW_HEIGHT; y++)
			{
				for (x = 0; x < CLIPPED_VIEW_WIDTH; x++)
				{
					bUnderBar = false;
					for (uiBar = 0; uiBar < uiBars; uiBar++)
					{
						bUnderBar |= acBars[uiBar].ContainsPoint(x, y);
					}
					if (!bUnderBar)
					{
						uiCompared++;
						if (memcmp(&puiData[(x + y * CLIPPED_VIEW_WIDTH) * 3], &puiWhole[((x + sScroll.x) + (y + sScroll.y) * 64) * 3], 3) != 0)
						{
							uiWrong++;
						}
					}
				}
			}
			AssertTrue(uiCompared > (CLIPPED_VIEW_WIDTH * CLIPPED_VIEW_HEIGHT) / 2);
			AssertSize(0, uiWrong);
		}

		pImage = NULL;
		pWholeImage = NULL;
		pBar = NULL;
		pContent = NULL;
		pWholeContent = NULL;
		pScrollPane = NULL;
		pFill = NULL;
		ORoot()->Remove(pScrolled);
		pScrolled = NULL;
		ORoot()->Remove(pWhole);
		pWhole = NULL;
		pTick = NULL;
	}
	AssertTrue(ORoot()->RemoveAll());
	cScrolledFactory.Kill();
	cWholeFactory.Kill();
	ObjectsFlush();
	ObjectsKill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//																		//
// Of two canvases in a scroll pane, the one scrolled out of sight has	//
// no native canvas, and gets one back when it is scrolled into sight.	//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestCanvasScrolledOutOfSight(void)
{
	CWinRefWindowFactory	cFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "CanvasOutOfSight";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	ObjectsInit();
	cFactory.Init(&gcMemoryAllocator, CLIPPED_VIEW_WIDTH, CLIPPED_VIEW_HEIGHT, szDirectory);
	{
		Ptr<CWindow>				pWindow;
		Ptr<CFillContainer>			pFill;
		Ptr<CScrollPane>			pScrollPane;
		Ptr<CGridContainer>			pGrid;
		Ptr<CCanvas>				pTop;
		Ptr<CCanvas>				pBottom;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		SInt32Vec2					sVec;

		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pWindow = OMalloc<CWindow>("Out Of Sight", &cFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);
		pFill = OMalloc<CFillContainer>(pWindow);
		pWindow->SetContainer(pFill);
		pScrollPane = OMalloc<CScrollPane>(pWindow, CreateScrollPaneTheme(CreateScrollBarImages()));
		pFill->AddComponent(pScrollPane);

		pGrid = OMalloc<CGridContainer>(pWindow);
		pTop = OMalloc<CCanvas>(pWindow, OMalloc<CBoxFillCanvasDraw>(Set32BitColour(1.0f, 0, 0)));
		pTop->SetDesiredSize(CLIPPED_VIEW_WIDTH, 20);
		pBottom = OMalloc<CCanvas>(pWindow, OMalloc<CBoxFillCanvasDraw>(Set32BitColour(0, 0, 1.0f)));
		pBottom->SetDesiredSize(CLIPPED_VIEW_WIDTH, 20);
		pGrid->AddComponent(pTop, 0, 0);
		pGrid->AddComponent(pBottom, 0, 1);
		pGrid->SetDesiredSize(CLIPPED_VIEW_WIDTH, 40);
		pScrollPane->SetContent(pGrid);

		pWindow->Paint();
		AssertNotNull(pTop->GetNativeCanvas());
		sVec = pTop->GetNativeSize();
		AssertInt32Vec2(CLIPPED_VIEW_WIDTH, CLIPPED_VIEW_HEIGHT, &sVec);
		AssertNull(pBottom->GetNativeCanvas());

		pScrollPane->SetScrollPosition(0, 24);
		pWindow->Paint();
		AssertNull(pTop->GetNativeCanvas());
		AssertNotNull(pBottom->GetNativeCanvas());
		sVec = pBottom->GetNativeOrigin();
		AssertInt32Vec2(0, 4, &sVec);
		sVec = pBottom->GetNativeSize();
		AssertInt32Vec2(CLIPPED_VIEW_WIDTH, 16, &sVec);

		pScrollPane->SetScrollPosition(0, 10);
		pWindow->Paint();
		sVec = pTop->GetNativeSize();
		AssertInt32Vec2(CLIPPED_VIEW_WIDTH, 10, &sVec);
		sVec = pBottom->GetNativeSize();
		AssertInt32Vec2(CLIPPED_VIEW_WIDTH, 6, &sVec);

		pTop = NULL;
		pBottom = NULL;
		pGrid = NULL;
		pScrollPane = NULL;
		pFill = NULL;
		ORoot()->Remove(pWindow);
		pWindow = NULL;
		pTick = NULL;
	}
	AssertTrue(ORoot()->RemoveAll());
	cFactory.Kill();
	ObjectsFlush();
	ObjectsKill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
Ptr<CCanvas> CreateFillCanvas(Ptr<CWindow> pWindow, ARGB32 sColour, bool bSolid)
{
	if (bSolid)
	{
		return OMalloc<CCanvas>(pWindow, OMalloc<CClearCanvasDraw>(sColour));
	}
	else
	{
		return OMalloc<CCanvas>(pWindow, OMalloc<CBoxFillCanvasDraw>(sColour));
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
// A scroll pane over a grid of fills (one holding a smaller fill of its	//
// own), a transparent rounded box and a border, drawn at each of		//
// asScrolls into auiWindows.  With bSolid the fills are CClearCanvasDraw	//
// canvases and have no native canvas.									//
//																		//
//////////////////////////////////////////////////////////////////////////
#define SOLID_WINDOW_WIDTH	32
#define SOLID_WINDOW_HEIGHT	24
#define SOLID_SCROLLS		3

void DrawSolidFills(bool bSolid, uint8 auiWindows[SOLID_SCROLLS][SOLID_WINDOW_WIDTH * SOLID_WINDOW_HEIGHT * 3])
{
	CWinRefWindowFactory	cFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "CanvasSolidFills";
	SInt32Vec2				asScrolls[SOLID_SCROLLS] = { { 0, 0 }, { 8, 16 }, { 3, 5 } };

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	ObjectsInit();
	cFactory.Init(&gcMemoryAllocator, SOLID_WINDOW_WIDTH, SOLID_WINDOW_HEIGHT, szDirectory);
	{
		Ptr<CWindow>				pWindow;
		Ptr<CFillContainer>			pFill;
		Ptr<CScrollPane>			pScrollPane;
		Ptr<CGridContainer>			pGrid;
		Ptr<CCanvas>				pRed;
		Ptr<CCanvas>				pGreen;
		Ptr<CCanvas>				pBlue;
		Ptr<CCanvas>				pRounded;
		Ptr<CCanvas>				pBorder;
		Ptr<CFillContainer>			pRedFill;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		SInsets						sInsets;
		size						uiScroll;

		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pWindow = OMalloc<CWindow>("Solid Fills", &cFactory, pTick, OMalloc<CClearCanvasDraw>(Set32BitColour(0.0f, 0, 0)), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);
		pFill = OMalloc<CFillContainer>(pWindow);
		pWindow->SetContainer(pFill);
		pScrollPane = OMalloc<CScrollPane>(pWindow, CreateScrollPaneTheme(CreateScrollBarImages()));
		pFill->AddComponent(pScrollPane);

		pGrid = OMalloc<CGridContainer>(pWindow);
		pGrid->SetDesiredSize(40, 40);
		pRed = CreateFillCanvas(pWindow, Set32BitColour(1.0f, 0, 0), bSolid);
		pRed->SetDesiredSize(20, 20);
		pRedFill = OMalloc<CFillContainer>(pWindow);
		pRed->SetContainer(pRedFill);
		pBlue = CreateFillCanvas(pWindow, Set32BitColour(0, 0, 1.0f), bSolid);
		pBlue->SetDesiredSize(6, 6);
		pRedFill->AddComponent(pBlue);
		pGreen = CreateFillCanvas(pWindow, Set32BitColour(0, 1.0f, 0), bSolid);
		pGreen->SetDesiredSize(20, 20);
		sInsets.Init(2, 2);
		pRounded = OMalloc<CCanvas>(pWindow, OMalloc<CRoundedBoxCanvasDraw>(Set32BitColour(1.0f, 1.0f, 0), 5, &sInsets));
		pRounded->SetTransparent(true);
		pRounded->SetDesiredSize(20, 20);
		pBorder = OMalloc<CCanvas>(pWindow, OMalloc<CBorderCanvasDraw>(Set32BitColour(0, 1.0f, 1.0f)));
		pBorder->SetDesiredSize(20, 20);
		pGrid->AddComponent(pRed, 0, 0);
		pGrid->AddComponent(pGreen, 1, 0);
		pGrid->AddComponent(pRounded, 0, 1);
		pGrid->AddComponent(pBorder, 1, 1);
		pScrollPane->SetContent(pGrid);

		for (uiScroll = 0; uiScroll < SOLID_SCROLLS; uiScroll++)
		{
			pScrollPane->SetScrollPosition(asScrolls[uiScroll].x, asScrolls[uiScroll].y);
			pWindow->Paint();
			memcpy(auiWindows[uiScroll], pWindow->GetCanvas()->GetImageOrNull()->GetData(), SOLID_WINDOW_WIDTH * SOLID_WINDOW_HEIGHT * 3);

			//A fill copied from a native canvas has one while it shows.
			AssertBool(!bSolid && pRed->HasArea(), pRed->GetNativeCanvas() != NULL);
			AssertBool(!bSolid && pBlue->HasArea(), pBlue->GetNativeCanvas() != NULL);
			AssertNotNull(pBorder->GetNativeCanvas());
		}

		pRedFill = NULL;
		pBorder = NULL;
		pRounded = NULL;
		pBlue = NULL;
		pGreen = NULL;
		pRed = NULL;
		pGrid = NULL;
		pScrollPane = NULL;
		pFill = NULL;
		ORoot()->Remove(pWindow);
		pWindow = NULL;
		pTick = NULL;
	}
	AssertTrue(ORoot()->RemoveAll());
	cFactory.Kill();
	ObjectsFlush();
	ObjectsKill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//																		//
// A canvas of one opaque colour fills its part of the window itself,		//
// and the window is the same as when it is drawn into its own canvas	//
// and copied.															//
//																		//
//////////////////////////////////////////////////////////////////////////
void TestCanvasSolidFillsDrawnIntoWindow(void)
{
	uint8	auiSolid[SOLID_SCROLLS][SOLID_WINDOW_WIDTH * SOLID_WINDOW_HEIGHT * 3];
	uint8	auiCopied[SOLID_SCROLLS][SOLID_WINDOW_WIDTH * SOLID_WINDOW_HEIGHT * 3];
	size	uiScroll;

	DrawSolidFills(true, auiSolid);
	DrawSolidFills(false, auiCopied);
	for (uiScroll = 0; uiScroll < SOLID_SCROLLS; uiScroll++)
	{
		AssertMemory(auiCopied[uiScroll], auiSolid[uiScroll], SOLID_WINDOW_WIDTH * SOLID_WINDOW_HEIGHT * 3);
	}
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
	TestCanvasWindowColourFormat();
	TestCanvasOutlivesKilledWindow();
	TestCanvasImageDrawFollowsTransparency();
	TestCanvasZeroAreaHasNoNativeCanvas();
	TestCanvasDrawBoxCutToCanvas();
	TestCanvasClippedContent(CC_Fill);
	TestCanvasClippedContent(CC_Solid);
	TestCanvasClippedContent(CC_Border);
	TestCanvasClippedContent(CC_Pixels);
	TestCanvasClippedContent(CC_Image);
	TestCanvasClippedContent(CC_StretchedImage);
	TestCanvasClippedContent(CC_RoundedBox);
	TestCanvasClippedContent(CC_ProgressBar);
	TestCanvasClippedContent(CC_TextArea);
	TestCanvasScrolledOutOfSight();
	TestCanvasSolidFillsDrawnIntoWindow();

	TestStatistics();
}

