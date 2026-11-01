#include "BaseLib/GlobalDataTypesIO.h"
#include "SupportLib/ImageWriter.h"
#include "SupportLib/Maps.h"
#include "SupportLib/TileLayerCel.h"
#include "SupportLib/ImageWriter.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"
#include "TestReadImage.h"
#include "SupportAssert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CArrayImageCel> TestMapsReadSpaceCrusadeCels(char* szFilename, int iColumnCount, int iRowCount)
{
	return TestReadCels("SpaceCrusade", szFilename, iColumnCount, iRowCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapsTinySpriteMap(void)
{
	char				szDirectory[] = "Output" _FS_ "MapsSpriteMap";
	CIndexTreeMemory	cMemory;
	CFileUtil			cFileUtil;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	ObjectsInit();
	{
		CMaps						cMaps;
		CImageDivider				cImageDivider;
		CChars						szInputFilename;
		CChars						szOutputFilename;
		Ptr<CArrayImageCel>			pCels;
		Ptr<CSpriteMap>				pSpriteMap;
		Ptr<CImageCel>				pCel1;
		Ptr<CImageCel>				pCel2;
		Ptr<CImageCel>				pCel3;
		Ptr<CImageCel>				pCel4;
		CSubImage*					pcSubImage;
		Ptr<CImage>					pWholeMap;
		bool						bWritten;
		size						uiSprite;

		ImageChannelDescriptorInit();

		pCels = TestReadCels("MapsSpriteMap", "TinySprites.png", 2, 2);

		AssertSize(4, pCels->Size());
		for (uiSprite = 0; uiSprite < 4; uiSprite++)
		{
			pCel1 = pCels->Get(uiSprite);
			AssertTrue(pCel1.IsNotNull());

			pcSubImage = pCel1->GetSubImage();
			AssertSize(4, pcSubImage->GetFullWidth());
			AssertSize(4, pcSubImage->GetFullHeight());
			AssertSize(2, pcSubImage->GetImageWidth());
			AssertSize(2, pcSubImage->GetImageHeight());
		}

		pCel1 = pCels->Get(0);
		pCel2 = pCels->Get(1);
		pCel3 = pCels->Get(2);
		pCel4 = pCels->Get(3);

		cMaps.Init(NULL, NULL);

		pSpriteMap = OMalloc<CSpriteMap>();
		cMaps.AddMap(pSpriteMap);

		pSpriteMap->AddSprite(pCel1, 0, 0);
		pSpriteMap->AddSprite(pCel2, 4, 0);
		pSpriteMap->AddSprite(pCel3, 0, 4);
		pSpriteMap->AddSprite(pCel4, 4, 4);

		AssertSize(4, pSpriteMap->NumSprites());

		pWholeMap = pSpriteMap->WriteToImage();
		AssertTrue(pWholeMap.IsNotNull());

		szOutputFilename.Init(szDirectory);
		cFileUtil.AppendToPath(&szOutputFilename, "TestTinyMap.png");
		bWritten = WriteImage(pWholeMap, szOutputFilename.Text());
		AssertTrue(bWritten);

		cMaps.Kill();

		ImageChannelDescriptorKill();

		szInputFilename.Init(szOutputFilename);
		szInputFilename.Replace("Output", "Input");

		AssertFile(szInputFilename, szOutputFilename);

		szOutputFilename.Kill();
		szInputFilename.Kill();
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapsSpriteMapSharedCels(void)
{
	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CArrayImageCel>			pCels;
		Ptr<CSpriteMap>				pSpriteMap;
		Ptr<CImageCel>				pCel1;
		Ptr<CImageCel>				pCel2;
		Ptr<CImage>					pViewport;
		Ptr<CImageCelBlitterCache>	pCache;
		Ptr<CImage>					pWholeMap;
		CInt32Rect					cViewportRect;
		SSprite*					psSprite;

		ImageChannelDescriptorInit();

		pRoot = ORoot();
		pCels = TestReadCels("MapsSpriteMap", "TinySprites.png", 2, 2);
		pRoot->Add(pCels);
		pCel1 = pCels->Get(0);
		pCel2 = pCels->Get(1);

		pSpriteMap = OMalloc<CSpriteMap>();
		pRoot->Add(pSpriteMap);

		AssertFalse(pSpriteMap->AddSprite(NULL, 0, 0));
		AssertTrue(pSpriteMap->AddSprite(pCel1, 0, 0));
		AssertTrue(pSpriteMap->AddSprite(pCel2, 4, 0));
		AssertTrue(pSpriteMap->AddSprite(pCel1, 0, 4));
		AssertTrue(pSpriteMap->AddSprite(pCel1, 4, 4));

		AssertSize(4, pSpriteMap->NumSprites());
		AssertSize(2, pSpriteMap->NumCels());
		AssertPointer(pCel1.Object(), pSpriteMap->GetCel(0).Object());
		AssertPointer(pCel2.Object(), pSpriteMap->GetCel(1).Object());
		psSprite = pSpriteMap->GetSprite(2);
		AssertSize(0, psSprite->uiCel);
		AssertInt((int32)0, psSprite->sPosition.x);
		AssertInt((int32)4, psSprite->sPosition.y);
		AssertSize(1, pSpriteMap->GetSprite(1)->uiCel);
		AssertNull(pSpriteMap->GetSprite(4));

		pWholeMap = pSpriteMap->WriteToImage();
		AssertTrue(pWholeMap.IsNotNull());
		AssertInt((int32)8, pWholeMap->GetWidth());
		AssertInt((int32)8, pWholeMap->GetHeight());

		pViewport = pSpriteMap->CreateViewportImage();
		pSpriteMap->SetViewport(pViewport);
		AssertFalse(pSpriteMap->HasCelBlitters());
		pCache = pSpriteMap->CreateBlitterCache();
		pSpriteMap->SetBlitterCache(pCache);
		AssertFalse(pSpriteMap->HasCelBlitters());
		cViewportRect.Init(0, 0, 8, 8);
		AssertFalse(pSpriteMap->Blit(&cViewportRect, false));
		AssertTrue(pSpriteMap->Blit(&cViewportRect, true));
		AssertTrue(pSpriteMap->HasCelBlitters());
		AssertSize(2, pCache->NumImageBlitters());
		AssertTrue(pSpriteMap->Blit(&cViewportRect, false));

		pSpriteMap->ClearCelBlitters();
		AssertFalse(pSpriteMap->HasCelBlitters());
		AssertTrue(pSpriteMap->CreateCelBlitters());
		AssertTrue(pSpriteMap->HasCelBlitters());
		AssertSize(2, pCache->NumImageBlitters());

		pSpriteMap->Clear();
		AssertSize(0, pSpriteMap->NumSprites());
		AssertSize(0, pSpriteMap->NumCels());
		AssertTrue(pSpriteMap->AddSprite(pCel2, 0, 0));
		AssertSize(1, pSpriteMap->NumCels());
		AssertPointer(pCel2.Object(), pSpriteMap->GetCel(0).Object());

		pCache = NULL;
		pViewport = NULL;
		pWholeMap = NULL;
		pRoot->Remove(pSpriteMap);
		pRoot->Remove(pCels);
		pSpriteMap = NULL;
		pCels = NULL;
		pCel1 = NULL;
		pCel2 = NULL;
		pRoot = NULL;

		ImageChannelDescriptorKill();
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapsTileMap(void)
{
	char				szDirectory[] = "Output" _FS_ "MapsGenerate";
	CIndexTreeMemory	cMemory;
	CFileUtil			cFileUtil;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	ObjectsInit();
	{
		CMaps					cMaps;
		CImageDivider			cImageDivider;
		CImageDividerNumbers	cNumbers;
		Ptr<CArrayImageCel>		pacBackgroundCels;
		CChars					szInputFilename;
		CChars					szOutputFilename;
		Ptr<CImage>				pBackground;
		Ptr<CTileMap>			pTileMap;
		Ptr<CTileLayerCel>		pCelLayer;
		Ptr<CImageCel>			pCel;
		CSubImage*				pcSubImage;
		bool					b1, b2, b3, b4;
		Ptr<CImage>				pWholeMap;
		bool					bWritten;
		size					uiTile;

		ImageChannelDescriptorInit();

		pBackground = TestReadImage("SpaceCrusade", "Tiles.png");

		cNumbers.InitGeneral(-1, -1, 10, 3, 0, 0, 0, 0);
		cImageDivider.Init(&pBackground, NULL);
		cImageDivider.GenerateFromNumbers(&cNumbers);
		pacBackgroundCels = cImageDivider.GetDestImageCels();
		AssertSize(30, pacBackgroundCels->NumElements());

		pCel = pacBackgroundCels->Get(0);
		pcSubImage = pCel->GetSubImage();
		AssertSize(16, pcSubImage->GetFullWidth());
		AssertSize(16, pcSubImage->GetFullHeight());

		for (uiTile = 0; uiTile < 30; uiTile++)
		{
			pCel = pacBackgroundCels->Get(uiTile);
			AssertTrue(pCel.IsNotNull());
		}

		cMaps.Init(NULL, NULL);
		
		pTileMap = OMalloc<CTileMap>();
		cMaps.AddMap(pTileMap);
		pCelLayer = OMalloc<CTileLayerCel>(pTileMap, "Cel", SSizeVec2(4, 4), SSizeVec2(16, 16), 0, SInt32Vec2(0, 0));
		b1 = pCelLayer->SetTiles(0, 0, pacBackgroundCels,  7, 5, 5, 6);
		b2 = pCelLayer->SetTiles(0, 1, pacBackgroundCels, 19, 2, 2, 4);
		b3 = pCelLayer->SetTiles(0, 2, pacBackgroundCels, 18, 2, 3, 4);
		b4 = pCelLayer->SetTiles(0, 3, pacBackgroundCels,  9, 5, 5, 8);
		AssertTrue(b1);
		AssertTrue(b2);
		AssertTrue(b3);
		AssertTrue(b4);

		szOutputFilename.Init(szDirectory);
		cFileUtil.AppendToPath(&szOutputFilename, "TestTileLayer.png");

		pWholeMap = pCelLayer->WriteToImage();
		AssertTrue(pWholeMap.IsNotNull());

		bWritten = WriteImage(pWholeMap, szOutputFilename.Text(), IT_PNG);
		AssertTrue(bWritten);

		cMaps.Kill();

		ImageChannelDescriptorKill();

		szInputFilename.Init(szOutputFilename);
		szInputFilename.Replace("Output", "Input");

		AssertFile(szInputFilename, szOutputFilename);

		szOutputFilename.Kill();
		szInputFilename.Kill();
	}
	ObjectsKill(false);

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapsSpriteMap(void)
{
	char				szDirectory[] = "Output" _FS_ "MapsSpriteMap";
	CIndexTreeMemory	cMemory;
	CFileUtil			cFileUtil;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	ObjectsInit();
	{
		CMaps						cMaps;
		CChars						szInputFilename;
		CChars						szOutputFilename;
		Ptr<CArrayImageCel>			pSoulSuckerCels;
		Ptr<CArrayImageCel>			pBloodAngelsCels;
		Ptr<CArrayImageCel>			pGretchinCels;
		Ptr<CSpriteMap>				pSpriteMap;
		Ptr<CImageCel>				pCel1;
		Ptr<CImageCel>				pCel2;
		Ptr<CImageCel>				pCel3;
		Ptr<CImageCel>				pCel4;
		Ptr<CImageCel>				pCel5;
		Ptr<CImageCel>				pCel6;
		CSubImage*					pcSubImage;
		Ptr<CImage>					pWholeMap;
		bool						bWritten;
		size						uiSprite;

		ImageChannelDescriptorInit();

		pSoulSuckerCels = TestMapsReadSpaceCrusadeCels("SoulSucker.png", 8, 1);
		pBloodAngelsCels = TestMapsReadSpaceCrusadeCels("BloodAngels.png", 8, 1);
		pGretchinCels = TestMapsReadSpaceCrusadeCels("Gretchin.png", 8, 1);

		for (uiSprite = 0; uiSprite < 8; uiSprite++)
		{
			pCel1 = pSoulSuckerCels->Get(uiSprite);
			AssertTrue(pCel1.IsNotNull());
		}
		for (uiSprite = 0; uiSprite < 8; uiSprite++)
		{
			pCel1 = pBloodAngelsCels->Get(uiSprite);
			AssertTrue(pCel1.IsNotNull());
		}
		for (uiSprite = 0; uiSprite < 8; uiSprite++)
		{
			pCel1 = pGretchinCels->Get(uiSprite);
			AssertTrue(pCel1.IsNotNull());
		}

		pCel1 = pSoulSuckerCels->Get(7);
		pcSubImage = pCel1->GetSubImage();
		AssertSize(16, pcSubImage->GetFullWidth());
		AssertSize(16, pcSubImage->GetFullHeight());
		AssertSize(15, pcSubImage->GetImageWidth());
		AssertSize(14, pcSubImage->GetImageHeight());

		pCel2 = pSoulSuckerCels->Get(2);
		pcSubImage = pCel2->GetSubImage();
		AssertSize(16, pcSubImage->GetFullWidth());
		AssertSize(16, pcSubImage->GetFullHeight());
		AssertSize(16, pcSubImage->GetImageWidth());
		AssertSize(13, pcSubImage->GetImageHeight());

		pCel3 = pBloodAngelsCels->Get(3);
		pcSubImage = pCel3->GetSubImage();
		AssertSize(16, pcSubImage->GetFullWidth());
		AssertSize(16, pcSubImage->GetFullHeight());
		AssertSize(16, pcSubImage->GetImageWidth());
		AssertSize(16, pcSubImage->GetImageHeight());

		pCel4 = pBloodAngelsCels->Get(0);
		pcSubImage = pCel4->GetSubImage();
		AssertSize(16, pcSubImage->GetFullWidth());
		AssertSize(16, pcSubImage->GetFullHeight());
		AssertSize(16, pcSubImage->GetImageWidth());
		AssertSize(13, pcSubImage->GetImageHeight());

		pCel5 = pGretchinCels->Get(6);
		pcSubImage = pCel5->GetSubImage();
		AssertSize(16, pcSubImage->GetFullWidth());
		AssertSize(16, pcSubImage->GetFullHeight());
		AssertSize(13, pcSubImage->GetImageWidth());
		AssertSize(16, pcSubImage->GetImageHeight());

		pCel6 = pGretchinCels->Get(4);
		pcSubImage = pCel6->GetSubImage();
		AssertSize(16, pcSubImage->GetFullWidth());
		AssertSize(16, pcSubImage->GetFullHeight());
		AssertSize(15, pcSubImage->GetImageWidth());
		AssertSize(13, pcSubImage->GetImageHeight());

		AssertPointer(&pCel1->GetSourceImage(), &pCel2->GetSourceImage());
		AssertPointer(&pCel3->GetSourceImage(), &pCel4->GetSourceImage());
		AssertPointer(&pCel5->GetSourceImage(), &pCel6->GetSourceImage());

		AssertNotPointer(&pCel1->GetSourceImage(), &pCel3->GetSourceImage());
		AssertNotPointer(&pCel3->GetSourceImage(), &pCel5->GetSourceImage());
		AssertNotPointer(&pCel5->GetSourceImage(), &pCel1->GetSourceImage());

		cMaps.Init(NULL, NULL);

		pSpriteMap = OMalloc<CSpriteMap>();
		cMaps.AddMap(pSpriteMap);

		pSpriteMap->AddSprite(pCel1, 18, 29);
		pSpriteMap->AddSprite(pCel2, 2, 36);
		pSpriteMap->AddSprite(pCel3, 31, 9);
		pSpriteMap->AddSprite(pCel4, 39, 27);
		pSpriteMap->AddSprite(pCel5, 11, 3);
		pSpriteMap->AddSprite(pCel6, 25, 19);

		AssertSize(6, pSpriteMap->NumSprites());
		
		pWholeMap = pSpriteMap->WriteToImage();
		AssertTrue(pWholeMap.IsNotNull());

		szOutputFilename.Init(szDirectory);
		cFileUtil.AppendToPath(&szOutputFilename, "TestSpriteMap.png");
		bWritten = WriteImage(pWholeMap, szOutputFilename.Text());
		AssertTrue(bWritten);

		cMaps.Kill();

		ImageChannelDescriptorKill();

		szInputFilename.Init(szOutputFilename);
		szInputFilename.Replace("Output", "Input");

		AssertFile(szInputFilename, szOutputFilename);

		szOutputFilename.Kill();
		szInputFilename.Kill();
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMaps(void)
{
	BeginTests();

	DataIOInit();

	TestMapsTinySpriteMap();
	TestMapsSpriteMapSharedCels();
	TestMapsTileMap();
	TestMapsSpriteMap();

	DataIOKill();

	TestStatistics();
}

