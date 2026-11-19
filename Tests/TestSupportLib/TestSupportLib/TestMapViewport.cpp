#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Pointer.h"
#include "SupportLib/ImageReader.h"
#include "SupportLib/ImageWriter.h"
#include "SupportLib/ImageDivider.h"
#include "SupportLib/Maps.h"
#include "SupportLib/MapViewport.h"
#include "SupportLib/TileLayerCel.h"
#include "SupportLib/TileLayerBoolean.h"
#include "TestLib/Assert.h"
#include "TestReadImage.h"
#include "SupportAssert.h"
#include "TestLib/AssertFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapViewportBlit(void)
{
	char				szDirectory[] = "Output" _FS_ "MapsViewport";
	CIndexTreeMemory	cMemory;
	CFileUtil			cFileUtil;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	ObjectsInit();
	{
		CMaps						cMaps;
		Ptr<CArrayImageCel>			pacBackgroundCels;
		CChars						szInputFilename;
		CChars						szOutputFilename;
		Ptr<CImage>					pBackground;
		Ptr<CTileMap>				pTileMap;
		Ptr<CTileLayerCel>			pCelLayer;
		Ptr<CImageCel>				pCel;
		bool						bWritten;
		Ptr<CImage>					pDestImage;
		Ptr<CImageCelBlitterCache>	pBlitterCache;
		Ptr<CArrayImageCel>			pSoulSuckerCels;
		Ptr<CArrayImageCel>			pBloodAngelsCels;
		Ptr<CArrayImageCel>			pGretchinCels;
		bool						bResult;
		Ptr<CSpriteMap>				pSpriteMap;
		Ptr<CImageCel>				pCel1;
		Ptr<CImageCel>				pCel2;
		Ptr<CImageCel>				pCel3;
		Ptr<CImageCel>				pCel4;
		Ptr<CImageCel>				pCel5;
		Ptr<CImageCel>				pCel6;
		int32						x, y;

		ImageChannelDescriptorInit();

		pacBackgroundCels = TestReadCels("SpaceCrusade", "Tiles.png", 10, 3);
		pSoulSuckerCels = TestReadCels("SpaceCrusade", "SoulSucker.png", 8, 1);
		pBloodAngelsCels = TestReadCels("SpaceCrusade", "BloodAngels.png", 8, 1);
		pGretchinCels = TestReadCels("SpaceCrusade", "Gretchin.png", 8, 1);
		pCel1 = pSoulSuckerCels->Get(7);
		pCel2 = pSoulSuckerCels->Get(2);
		pCel3 = pBloodAngelsCels->Get(3);
		pCel4 = pBloodAngelsCels->Get(0);
		pCel5 = pGretchinCels->Get(6);
		pCel6 = pGretchinCels->Get(4);

		AssertSize(30, pacBackgroundCels->NumElements());

		pDestImage = OMalloc<CImage>(32, 32, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pBlitterCache = OMalloc<CImageCelBlitterCache>(pDestImage);
		cMaps.Init(pBlitterCache, pDestImage);

		pTileMap = OMalloc<CTileMap>();
		pCelLayer = OMalloc<CTileLayerCel>(pTileMap, "Cel", SSizeVec2(4, 4), SSizeVec2(16, 16), 0, SInt32Vec2(0, 0));
		pCelLayer->SetTiles(0, 0, pacBackgroundCels, 7, 5, 5, 6);
		pCelLayer->SetTiles(0, 1, pacBackgroundCels, 19, 2, 2, 4);
		pCelLayer->SetTiles(0, 2, pacBackgroundCels, 18, 2, 3, 4);
		pCelLayer->SetTiles(0, 3, pacBackgroundCels, 9, 5, 5, 8);
		pTileMap->AddLayer(pCelLayer);

		cMaps.AddMap(pTileMap);

		pSpriteMap = OMalloc<CSpriteMap>();
		cMaps.AddMap(pSpriteMap);

		pSpriteMap->AddSprite(pCel1, 0, 0);
		pSpriteMap->AddSprite(pCel2, 16, 16);
		pSpriteMap->AddSprite(pCel3, 0, 32);
		pSpriteMap->AddSprite(pCel4, 32, 0);
		pSpriteMap->AddSprite(pCel5, 32, 32);
		pSpriteMap->AddSprite(pCel6, 48, 16);

		bResult = cMaps.CreateCelBlitters();
		AssertTrue(bResult);

		for (x = 8; x < 24; x++)
		{
			y = x;
			cMaps.SetViewportPosition(x, y);
			bResult = cMaps.Blit(false);
			AssertTrue(bResult);

			szOutputFilename.Init(szDirectory);
			cFileUtil.AppendToPath(&szOutputFilename, "TestViewportBlit");
			szOutputFilename.Append(x - 8);
			szOutputFilename.Append(".png");
			bWritten = WriteImage(pDestImage, szOutputFilename.Text(), IT_PNG);
			AssertTrue(bWritten);

			szInputFilename.Init(szOutputFilename);
			szInputFilename.Replace("Output", "Input");

			AssertFile(szInputFilename, szOutputFilename);

			szOutputFilename.Kill();
			szInputFilename.Kill();
		}

		cMaps.Kill();

		ImageChannelDescriptorKill();
	}
	ObjectsKill(false);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapViewportBooleanLayer(void)
{
	ObjectsInit();
	{
		CMaps						cMaps;
		Ptr<CArrayImageCel>			pacBackgroundCels;
		Ptr<CArrayImageCel>			pSoulSuckerCels;
		Ptr<CTileMap>				pTileMap;
		Ptr<CTileLayerBoolean>		pBooleanLayer;
		Ptr<CTileLayerCel>			pCelLayer;
		Ptr<CSpriteMap>				pSpriteMap;
		Ptr<CImage>					pDestImage1;
		Ptr<CImage>					pDestImage2;
		Ptr<CImageCelBlitterCache>	pBlitterCache1;
		Ptr<CImageCelBlitterCache>	pBlitterCache2;
		bool						bResult;
		uint8*						puiData;
		size						uiNonZero;
		lsize						i;

		ImageChannelDescriptorInit();

		pacBackgroundCels = TestReadCels("SpaceCrusade", "Tiles.png", 10, 3);
		pSoulSuckerCels = TestReadCels("SpaceCrusade", "SoulSucker.png", 8, 1);

		pDestImage1 = OMalloc<CImage>(32, 32, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pDestImage2 = OMalloc<CImage>(32, 32, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		memset(pDestImage1->GetData(), 0, pDestImage1->GetByteSize());
		memset(pDestImage2->GetData(), 0, pDestImage2->GetByteSize());
		pBlitterCache1 = OMalloc<CImageCelBlitterCache>(pDestImage1);
		cMaps.Init(pBlitterCache1, pDestImage1);

		//A boolean layer has no blitters and must not stop the cel layer beside it from getting them.
		pTileMap = OMalloc<CTileMap>();
		pBooleanLayer = OMalloc<CTileLayerBoolean>(pTileMap, "Boolean", SSizeVec2(4, 4), SSizeVec2(16, 16), 1, SInt32Vec2(0, 0));
		pBooleanLayer->SetTile(1, 1, true);
		pTileMap->AddLayer(pBooleanLayer);
		pCelLayer = OMalloc<CTileLayerCel>(pTileMap, "Cel", SSizeVec2(4, 4), SSizeVec2(16, 16), 0, SInt32Vec2(0, 0));
		pCelLayer->SetTiles(0, 0, pacBackgroundCels, 7, 5, 5, 6);
		pCelLayer->SetTiles(0, 1, pacBackgroundCels, 19, 2, 2, 4);
		pTileMap->AddLayer(pCelLayer);
		cMaps.AddMap(pTileMap);

		pSpriteMap = OMalloc<CSpriteMap>();
		pSpriteMap->AddSprite(pSoulSuckerCels->Get(7), 8, 8);
		cMaps.AddMap(pSpriteMap);

		AssertTrue(pTileMap->UsesCelBlitters());
		AssertFalse(cMaps.HasCelBlitters());
		bResult = cMaps.CreateCelBlitters();
		AssertTrue(bResult);
		AssertTrue(pCelLayer->HasCelBlitters());
		AssertTrue(cMaps.HasCelBlitters());

		cMaps.SetViewportPosition(4, 4);
		bResult = cMaps.Blit(false);
		AssertTrue(bResult);

		puiData = (uint8*)pDestImage1->GetData();
		uiNonZero = 0;
		for (i = 0; i < pDestImage1->GetByteSize(); i++)
		{
			if (puiData[i] != 0)
			{
				uiNonZero++;
			}
		}
		AssertTrue(uiNonZero > 0);

		//Moving to a new cache and image must throw away the blitters made for the old image.
		pBlitterCache2 = OMalloc<CImageCelBlitterCache>(pDestImage2);
		cMaps.SetCacheAndViewport(pBlitterCache2, pDestImage2);
		AssertFalse(cMaps.HasCelBlitters());
		bResult = cMaps.CreateCelBlitters();
		AssertTrue(bResult);
		AssertTrue(cMaps.HasCelBlitters());

		cMaps.SetViewportPosition(4, 4);
		bResult = cMaps.Blit(false);
		AssertTrue(bResult);
		AssertSize(pDestImage1->GetByteSize(), pDestImage2->GetByteSize());
		AssertMemory(pDestImage1->GetData(), pDestImage2->GetData(), pDestImage1->GetByteSize());

		cMaps.Kill();

		ImageChannelDescriptorKill();
	}
	ObjectsKill(false);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapViewport(void)
{
	BeginTests();

	DataIOInit();

	TestMapViewportBlit();
	TestMapViewportBooleanLayer();

	DataIOKill();

	TestStatistics();
}

