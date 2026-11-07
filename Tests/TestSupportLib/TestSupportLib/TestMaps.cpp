#include <ctype.h>
#include "BaseLib/GlobalDataTypesIO.h"
#include "SupportLib/ImageWriter.h"
#include "SupportLib/Maps.h"
#include "SupportLib/TileLayerCel.h"
#include "SupportLib/TileTextWriter.h"
#include "SupportLib/ImageCelBlitterCache.h"
#include "SupportLib/SimpleTextReader.h"
#include "SupportLib/TextLayout.h"
#include "SupportLib/MapFontDraw.h"
#include "SupportLib/SpriteMap.h"
#include "SupportLib/GeometryMap.h"
#include "BaseLib/UTF8.h"
#include "SupportLib/FontColours.h"
#include "SupportLib/ColourARGB32.h"
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

		AssertSize(4, pCels->NumElements());
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
// A tile layer row only stores the run of cels from its first tile to its
// last.
//
//////////////////////////////////////////////////////////////////////////
void TestMapsTileLayerRows(void)
{
	ObjectsInit();
	{
		Ptr<CArrayImageCel>		pacCels;
		Ptr<CTileMap>			pTileMap;
		Ptr<CTileLayerCel>		pLayer;
		Ptr<CImageCel>			pCel1;
		Ptr<CImageCel>			pCel2;
		Ptr<CImageCel>			pCel3;
		Ptr<CImageCel>			pCel4;
		Ptr<CImageCel>			pNoCel;
		size					y;

		ImageChannelDescriptorInit();

		pacCels = TestMapsReadSpaceCrusadeCels("Tiles.png", 10, 3);
		pCel1 = pacCels->Get(1);
		pCel2 = pacCels->Get(2);
		pCel3 = pacCels->Get(3);
		pCel4 = pacCels->Get(4);

		pTileMap = OMalloc<CTileMap>();
		pLayer = OMalloc<CTileLayerCel>(pTileMap, "Cel", SSizeVec2(8, 3), SSizeVec2(16, 16), 0, SInt32Vec2(0, 0));
		pTileMap->AddLayer(pLayer);

		//An empty layer stores nothing.
		AssertSize(0, pLayer->NumStoredTiles());
		AssertSize(0, pLayer->NumCels());
		for (y = 0; y < 3; y++)
		{
			AssertSize(pLayer->GetRowStart(y), pLayer->GetRowEnd(y));
		}
		AssertTrue(pLayer->GetTile(3, 1).IsNull());

		//A row grows its run to the right and to the left.
		AssertTrue(pLayer->SetTile(5, 1, pCel1));
		AssertSize(5, pLayer->GetRowStart(1));
		AssertSize(6, pLayer->GetRowEnd(1));
		AssertSize(1, pLayer->NumStoredTiles());

		AssertTrue(pLayer->SetTile(2, 1, pCel2));
		AssertSize(2, pLayer->GetRowStart(1));
		AssertSize(6, pLayer->GetRowEnd(1));
		AssertSize(4, pLayer->NumStoredTiles());
		AssertTrue(pLayer->GetTile(2, 1) == pCel2);
		AssertTrue(pLayer->GetTile(3, 1).IsNull());
		AssertTrue(pLayer->GetTile(5, 1) == pCel1);
		AssertTrue(pLayer->GetTile(1, 1).IsNull());

		AssertTrue(pLayer->SetTile(7, 1, pCel3));
		AssertSize(8, pLayer->GetRowEnd(1));
		AssertSize(6, pLayer->NumStoredTiles());

		//The same cel in many tiles is stored once.
		AssertSize(3, pLayer->NumCels());

		//A row before it moves its cels along; a row after it keeps its place.
		AssertTrue(pLayer->SetTile(0, 0, pCel4));
		AssertTrue(pLayer->SetTile(4, 2, pCel1));
		AssertSize(8, pLayer->NumStoredTiles());
		AssertTrue(pLayer->GetTile(0, 0) == pCel4);
		AssertTrue(pLayer->GetTile(2, 1) == pCel2);
		AssertTrue(pLayer->GetTile(5, 1) == pCel1);
		AssertTrue(pLayer->GetTile(7, 1) == pCel3);
		AssertTrue(pLayer->GetTile(4, 2) == pCel1);
		AssertTrue(pLayer->GetTile(4 + 2 * 8) == pCel1);
		AssertTrue(pLayer->GetTile(7 + 1 * 8) == pCel3);

		//Clearing the ends of a run trims the empty tiles next to them.
		AssertTrue(pLayer->SetTile(2, 1, pNoCel));
		AssertSize(5, pLayer->GetRowStart(1));
		AssertSize(8, pLayer->GetRowEnd(1));
		AssertSize(5, pLayer->NumStoredTiles());

		AssertTrue(pLayer->SetTile(7, 1, pNoCel));
		AssertSize(5, pLayer->GetRowStart(1));
		AssertSize(6, pLayer->GetRowEnd(1));
		AssertSize(3, pLayer->NumStoredTiles());

		//Clearing a tile outside the run changes nothing.
		AssertTrue(pLayer->SetTile(1, 1, pNoCel));
		AssertSize(3, pLayer->NumStoredTiles());

		AssertTrue(pLayer->SetTile(5, 1, pNoCel));
		AssertSize(pLayer->GetRowStart(1), pLayer->GetRowEnd(1));
		AssertSize(2, pLayer->NumStoredTiles());
		AssertTrue(pLayer->GetTile(0, 0) == pCel4);
		AssertTrue(pLayer->GetTile(4, 2) == pCel1);

		//A row that was emptied grows again where its cels belong.
		AssertTrue(pLayer->SetTile(6, 1, pCel2));
		AssertSize(3, pLayer->NumStoredTiles());
		AssertTrue(pLayer->GetTile(6, 1) == pCel2);
		AssertTrue(pLayer->GetTile(4, 2) == pCel1);

		//Tiles outside the layer can't be set.
		AssertFalse(pLayer->SetTile(8, 0, pCel1));
		AssertFalse(pLayer->SetTile(0, 3, pCel1));
		AssertTrue(pLayer->GetTile(8, 0).IsNull());

		pLayer = NULL;
		pTileMap = NULL;
		pacCels = NULL;

		ImageChannelDescriptorKill();
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// A cel added to a tile layer has the same tile value each time until the
// layer is cleared.  Rows set all at once from tile values keep only the
// runs from their first cel to their last, and the rows after them move.
//
//////////////////////////////////////////////////////////////////////////
void TestMapsTileLayerSetRowTiles(void)
{
	ObjectsInit();
	{
		Ptr<CArrayImageCel>		pacCels;
		Ptr<CTileMap>			pTileMap;
		Ptr<CTileLayerCel>		pLayer;
		Ptr<CImageCel>			pNoCel;
		uint32					auiThreeRows[] = { 0, 1, 0, 2,  3, 3 };
		size					auiThreeLengths[] = { 4, 0, 2 };
		uint32					auiLonger[] = { 0, 0, 2, 0, 1, 0 };
		uint32					auiShorter[] = { 2 };
		uint32					auiPastRight[] = { 3, 0, 0, 0, 0, 0, 0, 0, 1 };
		uint32					auiEmptyPastRight[] = { 3, 0, 0, 0, 0, 0, 0, 0, 0 };
		uint32					auiNotACel[] = { 1, 4 };
		size					uiLength;
		size					uiVersion;

		ImageChannelDescriptorInit();

		pacCels = TestMapsReadSpaceCrusadeCels("Tiles.png", 10, 3);
		pTileMap = OMalloc<CTileMap>();
		pLayer = OMalloc<CTileLayerCel>(pTileMap, "Cel", SSizeVec2(8, 3), SSizeVec2(16, 16), 0, SInt32Vec2(0, 0));
		pTileMap->AddLayer(pLayer);

		//A cel's tile value is its index plus one, the same each time; no cel is an empty tile.
		AssertLong(1, pLayer->AddCel(pacCels->Get(1)));
		AssertLong(2, pLayer->AddCel(pacCels->Get(2)));
		AssertLong(3, pLayer->AddCel(pacCels->Get(3)));
		AssertLong(2, pLayer->AddCel(pacCels->Get(2)));
		AssertLong(0, pLayer->AddCel(pNoCel));
		AssertSize(3, pLayer->NumCels());

		//Three rows at once: " 1 2", nothing and "33".
		AssertTrue(pLayer->SetRowTiles(0, 3, auiThreeRows, auiThreeLengths));
		AssertSize(1, pLayer->GetRowStart(0));
		AssertSize(4, pLayer->GetRowEnd(0));
		AssertTrue(pLayer->GetTile(1, 0) == pacCels->Get(1));
		AssertTrue(pLayer->GetTile(2, 0).IsNull());
		AssertTrue(pLayer->GetTile(3, 0) == pacCels->Get(2));
		AssertSize(pLayer->GetRowStart(1), pLayer->GetRowEnd(1));
		AssertTrue(pLayer->GetTile(1, 2) == pacCels->Get(3));
		AssertSize(5, pLayer->NumStoredTiles());

		//A longer row in the middle: the row after it moves along.
		uiLength = 6;
		AssertTrue(pLayer->SetRowTiles(1, 1, auiLonger, &uiLength));
		AssertSize(2, pLayer->GetRowStart(1));
		AssertSize(5, pLayer->GetRowEnd(1));
		AssertTrue(pLayer->GetTile(2, 1) == pacCels->Get(2));
		AssertTrue(pLayer->GetTile(4, 1) == pacCels->Get(1));
		AssertTrue(pLayer->GetTile(3, 0) == pacCels->Get(2));
		AssertTrue(pLayer->GetTile(0, 2) == pacCels->Get(3));
		AssertTrue(pLayer->GetTile(1, 2) == pacCels->Get(3));
		AssertSize(8, pLayer->NumStoredTiles());

		//A shorter first row: the rows after it move back.
		uiLength = 1;
		AssertTrue(pLayer->SetRowTiles(0, 1, auiShorter, &uiLength));
		AssertTrue(pLayer->GetTile(0, 0) == pacCels->Get(2));
		AssertTrue(pLayer->GetTile(3, 0).IsNull());
		AssertTrue(pLayer->GetTile(4, 1) == pacCels->Get(1));
		AssertTrue(pLayer->GetTile(1, 2) == pacCels->Get(3));
		AssertSize(6, pLayer->NumStoredTiles());

		//A cel past the right is cut off, and false; empty tiles past it are nothing to cut.
		uiLength = 9;
		AssertFalse(pLayer->SetRowTiles(2, 1, auiPastRight, &uiLength));
		AssertTrue(pLayer->GetTile(0, 2) == pacCels->Get(3));
		AssertSize(1, pLayer->GetRowEnd(2));
		AssertTrue(pLayer->SetRowTiles(2, 1, auiEmptyPastRight, &uiLength));

		//Rows past the bottom, or a value that isn't one of the layer's cels, change nothing.
		uiLength = 1;
		AssertFalse(pLayer->SetRowTiles(2, 2, auiThreeRows, auiThreeLengths));
		AssertFalse(pLayer->SetRowTiles(3, 1, auiShorter, &uiLength));
		uiLength = 2;
		AssertFalse(pLayer->SetRowTiles(0, 1, auiNotACel, &uiLength));
		AssertTrue(pLayer->GetTile(0, 0) == pacCels->Get(2));
		AssertSize(1, pLayer->GetRowEnd(0));
		AssertSize(5, pLayer->NumStoredTiles());
		AssertTrue(pLayer->SetRowTiles(3, 0, NULL, NULL));

		//Clearing forgets the cels: the values start again.
		uiVersion = pLayer->GetCelVersion();
		pLayer->Clear();
		AssertFalse(uiVersion == pLayer->GetCelVersion());
		AssertSize(0, pLayer->NumCels());
		AssertLong(1, pLayer->AddCel(pacCels->Get(3)));

		pLayer = NULL;
		pTileMap = NULL;
		pacCels = NULL;

		ImageChannelDescriptorKill();
	}
	ObjectsKill(false);
}


Ptr<CFont> CreateWhiteTrueTypeFontFrom(char* szFontName, char* szCharacters, float32 fPixelSize);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool TestMapsIsKeyword(const char* szWord, size uiLength)
{
	static const char*	aszKeywords[] = { "int", "for", "return", NULL };
	size				i;

	for (i = 0; aszKeywords[i] != NULL; i++)
	{
		if ((strlen(aszKeywords[i]) == uiLength) && (memcmp(aszKeywords[i], szWord, uiLength) == 0))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A monospaced font in a tile layer: each character is a tile, the glyph's
// cel (which is already as big as the cell: its advance by the line height)
// from the font of its colour.  Keywords are blue, the rest grey.
//
//////////////////////////////////////////////////////////////////////////
void TestMapsTileLayerText(void)
{
	static const char*	aszLines[] = {	"int Sum(int* aiValues, int iCount)",
										"{",
										"    int iTotal = 0;",
										"    for (int i = 0; i < iCount; i++)",
										"    {",
										"        iTotal += aiValues[i];",
										"    }",
										"    return iTotal;",
										"}",
										NULL };
	char				szDirectory[] = "Output" _FS_ "MapsTileText";
	CFileUtil			cFileUtil;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	ObjectsInit();
	{
		Ptr<CFont>				pWhite;
		Ptr<CFontColours>		pColours;
		Ptr<CFont>				pKeywordFont;
		Ptr<CFont>				pTextFont;
		Ptr<CGlyph>				pGlyph;
		Ptr<CImageCel>			pCel;
		Ptr<CTileMap>			pTileMap;
		Ptr<CTileLayerCel>		pLayer;
		Ptr<CTileTextWriter>	pWriter;
		Ptr<CImage>				pImage;
		CChars					szText;
		CArrayTileTextSpan		asSpans;
		STileTextSpan*			psSpan;
		ARGB32					sKeywordColour;
		ARGB32					sTextColour;
		CChars					szInputFilename;
		CChars					szOutputFilename;
		const char*				szLine;
		size					uiLine;
		size					x;
		size					uiWordStart;
		size					uiLength;
		size					uiCellWidth;
		size					uiCellHeight;
		size					uiStoredTiles;
		size					uiFirst;
		size					uiLast;

		ImageChannelDescriptorInit();

		pWhite = CreateWhiteTrueTypeFontFrom("JetBrainsMonoNL-Regular.ttf", " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", 16.0f);
		AssertTrue(pWhite.IsNotNull());
		AssertTrue(pWhite->IsFixedWidth());

		pColours = OMalloc<CFontColours>(pWhite);
		sKeywordColour = Set32BitColour((uint8)86, (uint8)156, (uint8)214, (uint8)255);
		sTextColour = Set32BitColour((uint8)212, (uint8)212, (uint8)212, (uint8)255);
		pKeywordFont = pColours->GetFont(sKeywordColour);
		pTextFont = pColours->GetFont(sTextColour);

		//A glyph's cel is its advance wide and the line high, so it fills a cell.
		pGlyph = pWhite->GetGlyph((uint8*)"A", 1);
		uiCellWidth = (size)pGlyph->GetFullWidth();
		uiCellHeight = (size)pWhite->GetLineHeight();
		AssertSize(10, uiCellWidth);
		AssertSize((size)pGlyph->GetFullHeight(), uiCellHeight);

		pTileMap = OMalloc<CTileMap>();
		pLayer = OMalloc<CTileLayerCel>(pTileMap, "Text", SSizeVec2(36, 9), SSizeVec2(uiCellWidth, uiCellHeight), 0, SInt32Vec2(0, 0));
		pTileMap->AddLayer(pLayer);

		//The lines in one text, and a span for each keyword.
		szText.Init();
		asSpans.Init();
		for (uiLine = 0; aszLines[uiLine] != NULL; uiLine++)
		{
			if (uiLine > 0)
			{
				szText.Append('\n');
			}
			szLine = aszLines[uiLine];
			uiLength = strlen(szLine);
			for (x = 0; x < uiLength; x++)
			{
				if (((x == 0) || !(isalnum((uint8)szLine[x - 1]) || (szLine[x - 1] == '_'))) && (isalnum((uint8)szLine[x]) || (szLine[x] == '_')))
				{
					uiWordStart = x;
					while ((uiWordStart < uiLength) && (isalnum((uint8)szLine[uiWordStart]) || (szLine[uiWordStart] == '_')))
					{
						uiWordStart++;
					}
					if (TestMapsIsKeyword(&szLine[x], uiWordStart - x))
					{
						psSpan = asSpans.Add();
						psSpan->uiStart = szText.Length() + x;
						psSpan->uiLength = uiWordStart - x;
						psSpan->sColour = sKeywordColour;
					}
				}
			}
			szText.Append(szLine);
		}

		pWriter = OMalloc<CTileTextWriter>(pLayer, pColours, sTextColour);
		AssertTrue(pWriter->IsValid());
		AssertTrue(pWriter->Write(szText.Text(), &asSpans));
		asSpans.Kill();
		szText.Kill();

		uiStoredTiles = 0;
		for (uiLine = 0; aszLines[uiLine] != NULL; uiLine++)
		{
			szLine = aszLines[uiLine];
			uiLength = strlen(szLine);

			//Every glyph fills its cell.
			for (x = 0; x < uiLength; x++)
			{
				pCel = pLayer->GetTile(x, uiLine);
				AssertBool(szLine[x] != ' ', pCel.IsNotNull());
				if (pCel.IsNotNull())
				{
					AssertSize(uiCellWidth, pCel->GetSubImage()->GetFullWidth());
					AssertSize(uiCellHeight, pCel->GetSubImage()->GetFullHeight());
				}
			}

			//A row only keeps the tiles from its first character to its last.
			uiFirst = 0;
			while (szLine[uiFirst] == ' ')
			{
				uiFirst++;
			}
			uiLast = uiLength - 1;
			AssertSize(uiFirst, pLayer->GetRowStart(uiLine));
			AssertSize(uiLast + 1, pLayer->GetRowEnd(uiLine));
			uiStoredTiles += uiLast + 1 - uiFirst;
		}
		AssertSize(uiStoredTiles, pLayer->NumStoredTiles());

		//Each character in each colour is one cel however many tiles show it.
		AssertSize(35, pLayer->NumCels());

		//The spaces between words are empty tiles; the same character in the same colour is the same cel.
		AssertTrue(pLayer->GetTile(3, 0).IsNull());
		AssertTrue(pLayer->GetTile(0, 0) == pKeywordFont->GetGlyph((uint8*)"i", 1)->GetCel());
		AssertTrue(pLayer->GetTile(4, 2) == pLayer->GetTile(0, 0));
		AssertTrue(pLayer->GetTile(4, 0) == pTextFont->GetGlyph((uint8*)"S", 1)->GetCel());
		AssertTrue(pLayer->GetTile(8, 2) == pTextFont->GetGlyph((uint8*)"i", 1)->GetCel());
		AssertFalse(pLayer->GetTile(8, 2) == pLayer->GetTile(0, 0));

		pImage = pLayer->WriteToImage();
		AssertTrue(pImage.IsNotNull());

		szOutputFilename.Init(szDirectory);
		cFileUtil.AppendToPath(&szOutputFilename, "TileText.png");
		AssertTrue(WriteImage(pImage, szOutputFilename.Text(), IT_PNG));

		szInputFilename.Init(szOutputFilename);
		szInputFilename.Replace("Output", "Input");
		AssertFile(szInputFilename.Text(), szOutputFilename.Text());

		szOutputFilename.Kill();
		szInputFilename.Kill();

		pImage = NULL;
		pWriter = NULL;
		pLayer = NULL;
		pTileMap = NULL;
		pGlyph = NULL;
		pCel = NULL;
		pTextFont = NULL;
		pKeywordFont = NULL;
		pColours = NULL;
		pWhite = NULL;

		ImageChannelDescriptorKill();
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// Tabs, carriage returns, cut off text, characters the font hasn't got,
// colour spans and rewriting a row.
//
//////////////////////////////////////////////////////////////////////////
void TestMapsTileTextWriter(void)
{
	ObjectsInit();
	{
		Ptr<CFont>				pWhite;
		Ptr<CFontColours>		pColours;
		Ptr<CFont>				pText;
		Ptr<CFont>				pRed;
		Ptr<CTileMap>			pTileMap;
		Ptr<CTileLayerCel>		pLayer;
		Ptr<CTileLayerCel>		pWrongLayer;
		Ptr<CTileTextWriter>	pWriter;
		Ptr<CTileTextWriter>	pWrongWriter;
		CArrayTileTextSpan		asSpans;
		STileTextSpan*			psSpan;
		ARGB32					sText;
		ARGB32					sRed;
		size					uiCellWidth;
		size					uiCellHeight;

		ImageChannelDescriptorInit();

		pWhite = CreateWhiteTrueTypeFontFrom("JetBrainsMonoNL-Regular.ttf", " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", 16.0f);
		pColours = OMalloc<CFontColours>(pWhite);
		sText = Set32BitColour((uint8)200, (uint8)200, (uint8)200, (uint8)255);
		sRed = Set32BitColour((uint8)255, (uint8)0, (uint8)0, (uint8)255);
		pText = pColours->GetFont(sText);
		pRed = pColours->GetFont(sRed);
		uiCellWidth = pWhite->GetAverageWidth();
		uiCellHeight = (size)pWhite->GetLineHeight();

		pTileMap = OMalloc<CTileMap>();
		pLayer = OMalloc<CTileLayerCel>(pTileMap, "Text", SSizeVec2(8, 3), SSizeVec2(uiCellWidth, uiCellHeight), 0, SInt32Vec2(0, 0));
		pTileMap->AddLayer(pLayer);
		pWriter = OMalloc<CTileTextWriter>(pLayer, pColours, sText, 4);
		AssertTrue(pWriter->IsValid());

		//The layer's cells must be the font's.
		pWrongLayer = OMalloc<CTileLayerCel>(pTileMap, "Wrong", SSizeVec2(8, 3), SSizeVec2(uiCellWidth + 1, uiCellHeight), 1, SInt32Vec2(0, 0));
		pWrongWriter = OMalloc<CTileTextWriter>(pWrongLayer, pColours, sText, 4);
		AssertFalse(pWrongWriter->IsValid());

		//A tab goes to the next tab stop and a carriage return is ignored.
		AssertTrue(pWriter->Write("ab\tc\r\nd"));
		AssertTrue(pLayer->GetTile(0, 0) == pText->GetGlyph((uint8*)"a", 1)->GetCel());
		AssertTrue(pLayer->GetTile(1, 0) == pText->GetGlyph((uint8*)"b", 1)->GetCel());
		AssertTrue(pLayer->GetTile(2, 0).IsNull());
		AssertTrue(pLayer->GetTile(3, 0).IsNull());
		AssertTrue(pLayer->GetTile(4, 0) == pText->GetGlyph((uint8*)"c", 1)->GetCel());
		AssertSize(5, pLayer->GetRowEnd(0));
		AssertTrue(pLayer->GetTile(0, 1) == pText->GetGlyph((uint8*)"d", 1)->GetCel());
		AssertSize(1, pLayer->GetRowEnd(1));
		AssertSize(pLayer->GetRowStart(2), pLayer->GetRowEnd(2));
		AssertSize(6, pLayer->NumStoredTiles());  //Row 0 keeps the empty tiles of its tab.

		//Writing again replaces everything.  A tab from the middle of a stop goes to the next one, which for "abcde\t" is
		//column 8, past the layer, so the f is cut off.
		AssertFalse(pWriter->Write("a\tb\nabcde\tf"));
		AssertTrue(pLayer->GetTile(4, 0) == pText->GetGlyph((uint8*)"b", 1)->GetCel());
		AssertSize(5, pLayer->GetRowEnd(0));
		AssertTrue(pLayer->GetTile(4, 1) == pText->GetGlyph((uint8*)"e", 1)->GetCel());
		AssertTrue(pLayer->GetTile(5, 1).IsNull());
		AssertTrue(pLayer->GetTile(8 - 1, 1).IsNull());
		AssertTrue(pLayer->GetTile(0, 2).IsNull());

		//A trailing new line is an empty line.
		AssertTrue(pWriter->Write("a\nb\n"));
		AssertSize(2, pLayer->NumStoredTiles());

		//What is past the right or the bottom is cut off.
		AssertFalse(pWriter->Write("0123456789"));
		AssertSize(0, pLayer->GetRowStart(0));
		AssertSize(8, pLayer->GetRowEnd(0));
		AssertTrue(pLayer->GetTile(7, 0) == pText->GetGlyph((uint8*)"7", 1)->GetCel());
		AssertFalse(pWriter->Write("a\nb\nc\nd"));
		AssertTrue(pLayer->GetTile(0, 2) == pText->GetGlyph((uint8*)"c", 1)->GetCel());
		AssertSize(3, pLayer->NumStoredTiles());

		//A character the font hasn't got is a '?', one column wide.
		AssertTrue(pWriter->Write("\xC3\xA9!"));
		AssertTrue(pLayer->GetTile(0, 0) == pText->GetGlyph((uint8*)"?", 1)->GetCel());
		AssertTrue(pLayer->GetTile(1, 0) == pText->GetGlyph((uint8*)"!", 1)->GetCel());
		AssertSize(2, pLayer->GetRowEnd(0));

		//Spans colour bytes of the whole text, across lines.
		asSpans.Init();
		psSpan = asSpans.Add();
		psSpan->uiStart = 1;
		psSpan->uiLength = 2;
		psSpan->sColour = sRed;
		psSpan = asSpans.Add();
		psSpan->uiStart = 6;
		psSpan->uiLength = 1;
		psSpan->sColour = sRed;
		AssertTrue(pWriter->Write("abcd\nxy", &asSpans));
		AssertTrue(pLayer->GetTile(0, 0) == pText->GetGlyph((uint8*)"a", 1)->GetCel());
		AssertTrue(pLayer->GetTile(1, 0) == pRed->GetGlyph((uint8*)"b", 1)->GetCel());
		AssertTrue(pLayer->GetTile(2, 0) == pRed->GetGlyph((uint8*)"c", 1)->GetCel());
		AssertTrue(pLayer->GetTile(3, 0) == pText->GetGlyph((uint8*)"d", 1)->GetCel());
		AssertTrue(pLayer->GetTile(0, 1) == pText->GetGlyph((uint8*)"x", 1)->GetCel());
		AssertTrue(pLayer->GetTile(1, 1) == pRed->GetGlyph((uint8*)"y", 1)->GetCel());

		//Rewriting one row: shorter, and coloured from where it is in the text.
		AssertTrue(pWriter->WriteRow(0, "pq", 2, 5, &asSpans));
		AssertSize(2, pLayer->GetRowEnd(0));
		AssertTrue(pLayer->GetTile(0, 0) == pText->GetGlyph((uint8*)"p", 1)->GetCel());
		AssertTrue(pLayer->GetTile(1, 0) == pRed->GetGlyph((uint8*)"q", 1)->GetCel());
		AssertTrue(pLayer->GetTile(1, 1) == pRed->GetGlyph((uint8*)"y", 1)->GetCel());
		AssertSize(4, pLayer->NumStoredTiles());
		asSpans.Kill();

		pWrongWriter = NULL;
		pWrongLayer = NULL;
		pWriter = NULL;
		pLayer = NULL;
		pTileMap = NULL;
		pRed = NULL;
		pText = NULL;
		pColours = NULL;
		pWhite = NULL;

		ImageChannelDescriptorKill();
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// The pixels of pViewport that differ from pReference's at xView, yView
// (black where pReference doesn't reach).
//
//////////////////////////////////////////////////////////////////////////
size TestMapsCountDifferences(Ptr<CImage> pViewport, Ptr<CImage> pReference, int32 xView, int32 yView)
{
	uint8*	puiView;
	uint8*	puiReference;
	int32	iViewWidth;
	int32	iViewHeight;
	int32	iReferenceWidth;
	int32	iReferenceHeight;
	size	uiPixelBytes;
	int32	x, y;
	int32	xReference, yReference;
	size	uiDifferences;
	uint8	auiBlack[16];

	iViewWidth = (int32)pViewport->GetWidth();
	iViewHeight = (int32)pViewport->GetHeight();
	iReferenceWidth = (int32)pReference->GetWidth();
	iReferenceHeight = (int32)pReference->GetHeight();
	uiPixelBytes = (size)(pViewport->GetByteSize() / ((lsize)iViewWidth * (lsize)iViewHeight));
	if ((uiPixelBytes == 0) || (uiPixelBytes > 16) || (uiPixelBytes != (size)(pReference->GetByteSize() / ((lsize)iReferenceWidth * (lsize)iReferenceHeight))))
	{
		return MAX_SIZE;
	}

	memset(auiBlack, 0, sizeof(auiBlack));
	puiView = (uint8*)pViewport->GetData();
	puiReference = (uint8*)pReference->GetData();
	uiDifferences = 0;
	for (y = 0; y < iViewHeight; y++)
	{
		for (x = 0; x < iViewWidth; x++)
		{
			xReference = x + xView;
			yReference = y + yView;
			if ((xReference >= 0) && (xReference < iReferenceWidth) && (yReference >= 0) && (yReference < iReferenceHeight))
			{
				if (memcmp(&puiView[(y * iViewWidth + x) * uiPixelBytes], &puiReference[(yReference * iReferenceWidth + xReference) * uiPixelBytes], uiPixelBytes) != 0)
				{
					uiDifferences++;
				}
			}
			else if (memcmp(&puiView[(y * iViewWidth + x) * uiPixelBytes], auiBlack, uiPixelBytes) != 0)
			{
				uiDifferences++;
			}
		}
	}
	return uiDifferences;
}


//////////////////////////////////////////////////////////////////////////
//
// Blits the maps through the viewport at x, y (making the blitters it
// needs) and checks it shows what the layer written to a new image does.
//
//////////////////////////////////////////////////////////////////////////
void TestMapsAssertViewport(CMaps* pcMaps, Ptr<CImage> pViewport, Ptr<CTileLayerCel> pLayer, int32 x, int32 y)
{
	Ptr<CImage>	pReference;

	pcMaps->SetViewportPosition(x, y);
	pViewport->Black();
	AssertTrue(pcMaps->Blit(true));

	pReference = pLayer->WriteToImage();
	AssertTrue(pReference.IsNotNull());
	AssertSize(0, TestMapsCountDifferences(pViewport, pReference, x, y));
}


//////////////////////////////////////////////////////////////////////////
//
// Rows with runs that start and end at different places, gaps and empty
// rows.
//
//////////////////////////////////////////////////////////////////////////
void TestMapsSetSparseTiles(Ptr<CTileLayerCel> pLayer, Ptr<CArrayImageCel> pacCels)
{
	pLayer->SetTile(2, 0, pacCels->Get(1));
	pLayer->SetTile(3, 0, pacCels->Get(2));
	pLayer->SetTile(5, 0, pacCels->Get(3));
	pLayer->SetTiles(0, 2, pacCels, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13);
	pLayer->SetTile(9, 3, pacCels->Get(14));
	pLayer->SetTile(3, 4, pacCels->Get(15));
	pLayer->SetTile(7, 4, pacCels->Get(16));
}


//////////////////////////////////////////////////////////////////////////
//
// Changing tiles after they have blitters: the blitters stay with their
// tiles as runs grow and shrink, and a changed cel gets a new blitter.
//
//////////////////////////////////////////////////////////////////////////
void TestMapsTileLayerBlitAfterEdits(void)
{
	ObjectsInit();
	{
		CMaps						cMaps;
		Ptr<CArrayImageCel>			pacCels;
		Ptr<CImage>					pViewport;
		Ptr<CImageCelBlitterCache>	pCache;
		Ptr<CTileMap>				pTileMap;
		Ptr<CTileLayerCel>			pLayer;
		Ptr<CImageCel>				pNoCel;

		ImageChannelDescriptorInit();

		pacCels = TestMapsReadSpaceCrusadeCels("Tiles.png", 10, 3);
		pViewport = OMalloc<CImage>(160, 96, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pCache = OMalloc<CImageCelBlitterCache>(pViewport);
		cMaps.Init(pCache, pViewport);

		pTileMap = OMalloc<CTileMap>();
		pLayer = OMalloc<CTileLayerCel>(pTileMap, "Cel", SSizeVec2(10, 6), SSizeVec2(16, 16), 0, SInt32Vec2(0, 0));
		pTileMap->AddLayer(pLayer);
		cMaps.AddMap(pTileMap);

		TestMapsSetSparseTiles(pLayer, pacCels);
		TestMapsAssertViewport(&cMaps, pViewport, pLayer, 0, 0);
		AssertTrue(pLayer->HasCelBlitters());

		//A run grows to the left and to the right of tiles that have blitters.
		AssertTrue(pLayer->SetTile(0, 0, pacCels->Get(20)));
		TestMapsAssertViewport(&cMaps, pViewport, pLayer, 0, 0);
		AssertTrue(pLayer->HasCelBlitters());
		AssertTrue(pLayer->SetTile(9, 4, pacCels->Get(21)));
		TestMapsAssertViewport(&cMaps, pViewport, pLayer, 0, 0);
		AssertTrue(pLayer->HasCelBlitters());

		//A tile's cel changes.
		AssertTrue(pLayer->SetTile(5, 2, pacCels->Get(22)));
		TestMapsAssertViewport(&cMaps, pViewport, pLayer, 0, 0);
		AssertTrue(pLayer->GetBlitter(5, 2)->GetCel() == pacCels->Get(22));

		//A run is trimmed, a row is cleared and an empty row between others is filled.
		AssertTrue(pLayer->SetTile(5, 0, pNoCel));
		TestMapsAssertViewport(&cMaps, pViewport, pLayer, 0, 0);
		AssertTrue(pLayer->HasCelBlitters());
		pLayer->ClearRow(2);
		TestMapsAssertViewport(&cMaps, pViewport, pLayer, 0, 0);
		AssertTrue(pLayer->HasCelBlitters());
		AssertTrue(pLayer->SetTile(1, 1, pacCels->Get(23)));
		AssertTrue(pLayer->SetTile(2, 1, pacCels->Get(24)));
		AssertTrue(pLayer->SetTile(3, 1, pacCels->Get(25)));
		TestMapsAssertViewport(&cMaps, pViewport, pLayer, 0, 0);
		AssertTrue(pLayer->HasCelBlitters());

		//Everything is cleared and set again.
		pLayer->Clear();
		TestMapsAssertViewport(&cMaps, pViewport, pLayer, 0, 0);
		TestMapsSetSparseTiles(pLayer, pacCels);
		TestMapsAssertViewport(&cMaps, pViewport, pLayer, 0, 0);
		AssertTrue(pLayer->HasCelBlitters());

		cMaps.Kill();
		pLayer = NULL;
		pTileMap = NULL;
		pCache = NULL;
		pViewport = NULL;
		pacCels = NULL;

		ImageChannelDescriptorKill();
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// A viewport smaller than the layer, scrolled over it (and past its edges),
// only shows the tiles of the rows' runs it covers.
//
//////////////////////////////////////////////////////////////////////////
void TestMapsTileLayerBlitScrolled(void)
{
	static const int32	aiX[] = { -20, -8, 0, 7, 16, 33, 90, 125, 150 };
	static const int32	aiY[] = { -13, 0, 9, 30, 50, 75, 90 };

	ObjectsInit();
	{
		CMaps						cMaps;
		Ptr<CArrayImageCel>			pacCels;
		Ptr<CImage>					pViewport;
		Ptr<CImageCelBlitterCache>	pCache;
		Ptr<CTileMap>				pTileMap;
		Ptr<CTileLayerCel>			pLayer;
		size						i;
		size						j;

		ImageChannelDescriptorInit();

		pacCels = TestMapsReadSpaceCrusadeCels("Tiles.png", 10, 3);
		pViewport = OMalloc<CImage>(40, 24, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pCache = OMalloc<CImageCelBlitterCache>(pViewport);
		cMaps.Init(pCache, pViewport);

		pTileMap = OMalloc<CTileMap>();
		pLayer = OMalloc<CTileLayerCel>(pTileMap, "Cel", SSizeVec2(10, 6), SSizeVec2(16, 16), 0, SInt32Vec2(0, 0));
		pTileMap->AddLayer(pLayer);
		cMaps.AddMap(pTileMap);
		TestMapsSetSparseTiles(pLayer, pacCels);

		for (j = 0; j < sizeof(aiY) / sizeof(aiY[0]); j++)
		{
			for (i = 0; i < sizeof(aiX) / sizeof(aiX[0]); i++)
			{
				TestMapsAssertViewport(&cMaps, pViewport, pLayer, aiX[i], aiY[j]);
			}
		}

		cMaps.Kill();
		pLayer = NULL;
		pTileMap = NULL;
		pCache = NULL;
		pViewport = NULL;
		pacCels = NULL;

		ImageChannelDescriptorKill();
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// Text in a tile layer looks the same as the same text laid out by
// CTextLayout into a sprite map, with the same fonts.
//
//////////////////////////////////////////////////////////////////////////
void TestMapsTileTextMatchesSpriteText(void)
{
	static const char	szText[] = "int Sum(int* aiValues, int iCount)\n"
								   "{\n"
								   "    int iTotal = 0;\n"
								   "    for (int i = 0; i < iCount; i++)\n"
								   "    {\n"
								   "        iTotal += aiValues[i];\n"
								   "    }\n"
								   "    return iTotal;\n"
								   "}";

	ObjectsInit();
	{
		Ptr<CFont>					pWhite;
		Ptr<CFontColours>			pColours;
		Ptr<CFont>					pKeywordFont;
		Ptr<CFont>					pTextFont;
		ARGB32						sKeywordColour;
		ARGB32						sTextColour;
		CArrayTileTextSpan			asSpans;
		STileTextSpan*				psSpan;
		CArrayTextFontChange		asChanges;
		size						uiLength;
		size						x;
		size						uiEnd;
		size						uiCellWidth;
		size						uiCellHeight;
		CMaps						cTileMaps;
		Ptr<CImage>					pTileViewport;
		Ptr<CTileMap>				pTileMap;
		Ptr<CTileLayerCel>			pLayer;
		Ptr<CTileTextWriter>		pWriter;
		CMaps						cSpriteMaps;
		Ptr<CImage>					pSpriteViewport;
		Ptr<CSpriteMap>				pSpriteMap;
		Ptr<CMapFontDraw>			pFontDraw;
		Ptr<CTextLayout>			pLayout;
		Ptr<CText>					pText;
		CSimpleTextReader			cReader;
		CUTF8						cUTF8;

		ImageChannelDescriptorInit();

		pWhite = CreateWhiteTrueTypeFontFrom("JetBrainsMonoNL-Regular.ttf", " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", 16.0f);
		pColours = OMalloc<CFontColours>(pWhite);
		sKeywordColour = Set32BitColour((uint8)86, (uint8)156, (uint8)214, (uint8)255);
		sTextColour = Set32BitColour((uint8)212, (uint8)212, (uint8)212, (uint8)255);
		pKeywordFont = pColours->GetFont(sKeywordColour);
		pTextFont = pColours->GetFont(sTextColour);
		uiCellWidth = pWhite->GetAverageWidth();
		uiCellHeight = (size)pWhite->GetLineHeight();

		//A span for each keyword, and the font changes the same colours are for the text reader.
		asSpans.Init();
		asChanges.Init();
		uiLength = strlen(szText);
		for (x = 0; x < uiLength; x++)
		{
			if (((x == 0) || !(isalnum((uint8)szText[x - 1]) || (szText[x - 1] == '_'))) && (isalnum((uint8)szText[x]) || (szText[x] == '_')))
			{
				uiEnd = x;
				while ((uiEnd < uiLength) && (isalnum((uint8)szText[uiEnd]) || (szText[uiEnd] == '_')))
				{
					uiEnd++;
				}
				if (TestMapsIsKeyword(&szText[x], uiEnd - x))
				{
					psSpan = asSpans.Add();
					psSpan->uiStart = x;
					psSpan->uiLength = uiEnd - x;
					psSpan->sColour = sKeywordColour;
					asChanges.Add()->Init(x, &pKeywordFont);
					asChanges.Add()->Init(uiEnd, &pTextFont);
				}
			}
		}

		//The text in a tile layer.
		pTileViewport = OMalloc<CImage>(360, 198, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		cTileMaps.Init(OMalloc<CImageCelBlitterCache>(pTileViewport), pTileViewport);
		pTileMap = OMalloc<CTileMap>();
		pLayer = OMalloc<CTileLayerCel>(pTileMap, "Text", SSizeVec2(36, 9), SSizeVec2(uiCellWidth, uiCellHeight), 0, SInt32Vec2(0, 0));
		pTileMap->AddLayer(pLayer);
		cTileMaps.AddMap(pTileMap);
		pWriter = OMalloc<CTileTextWriter>(pLayer, pColours, sTextColour);
		AssertTrue(pWriter->Write(szText, &asSpans));

		//The text laid out into a sprite map.
		pSpriteViewport = OMalloc<CImage>(360, 198, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		cSpriteMaps.Init(OMalloc<CImageCelBlitterCache>(pSpriteViewport), pSpriteViewport);
		pSpriteMap = OMalloc<CSpriteMap>();
		cSpriteMaps.AddMap(pSpriteMap);
		cUTF8.Init(szText);
		cReader.Init(&pTextFont);
		pText = cReader.Read(&cUTF8, true, &asChanges);
		cReader.Kill();
		cUTF8.Kill();
		AssertTrue(pText.IsNotNull());
		pFontDraw = OMalloc<CMapFontDraw>(pSpriteMap);
		pLayout = OMalloc<CTextLayout>();
		pSpriteMap->BeginChange();
		pLayout->Layout(pFontDraw, pText);
		pSpriteMap->EndChange();

		pTileViewport->Black();
		AssertTrue(cTileMaps.Blit(true));
		pSpriteViewport->Black();
		AssertTrue(cSpriteMaps.Blit(true));
		AssertSize(0, TestMapsCountDifferences(pTileViewport, pSpriteViewport, 0, 0));

		//And the text isn't just black.
		pSpriteViewport->Black();
		AssertTrue(TestMapsCountDifferences(pTileViewport, pSpriteViewport, 0, 0) > 1000);

		asChanges.Kill();
		asSpans.Kill();
		cSpriteMaps.Kill();
		cTileMaps.Kill();
		pText->Kill();
		pText = NULL;
		pLayout = NULL;
		pFontDraw = NULL;
		pSpriteMap = NULL;
		pSpriteViewport = NULL;
		pWriter = NULL;
		pLayer = NULL;
		pTileMap = NULL;
		pTileViewport = NULL;
		pTextFont = NULL;
		pKeywordFont = NULL;
		pColours = NULL;
		pWhite = NULL;

		ImageChannelDescriptorKill();
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// The cel of the UTF-8 character szCharacter in pFont (found by code
// point, as CTextLayout and CTileTextWriter find it).
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> TestMapsGlyphCel(Ptr<CFont> pFont, const char* szCharacter)
{
	CUTF8		cUTF8;
	Ptr<CGlyph>	pGlyph;

	cUTF8.Init(szCharacter);
	pGlyph = pFont->GetGlyph(&cUTF8);
	cUTF8.Kill();
	if (pGlyph.IsNull())
	{
		return NULL;
	}
	return pGlyph->GetCel();
}


//////////////////////////////////////////////////////////////////////////
//
// Rows past the bottom, lines with new lines and terminators in them, UTF-8
// characters of 2, 3 and 4 bytes, bytes that aren't UTF-8 and a span that
// starts inside a character.
//
//////////////////////////////////////////////////////////////////////////
void TestMapsTileTextWriterCharacters(void)
{
	ObjectsInit();
	{
		Ptr<CFont>				pWhite;
		Ptr<CFontColours>		pColours;
		Ptr<CFont>				pText;
		Ptr<CTileMap>			pTileMap;
		Ptr<CTileLayerCel>		pLayer;
		Ptr<CTileTextWriter>	pWriter;
		CArrayTileTextSpan		asSpans;
		STileTextSpan*			psSpan;
		ARGB32					sText;
		ARGB32					sRed;
		Ptr<CImageCel>			pQuestion;

		ImageChannelDescriptorInit();

		//The font has e acute (2 bytes) and the euro sign (3 bytes) but no emoji (4 bytes).
		pWhite = CreateWhiteTrueTypeFontFrom("JetBrainsMonoNL-Regular.ttf", "?!abcdx\xC3\xA9\xE2\x82\xAC", 16.0f);
		AssertTrue(pWhite->IsFixedWidth());
		pColours = OMalloc<CFontColours>(pWhite);
		sText = Set32BitColour((uint8)200, (uint8)200, (uint8)200, (uint8)255);
		sRed = Set32BitColour((uint8)255, (uint8)0, (uint8)0, (uint8)255);
		pText = pColours->GetFont(sText);
		pQuestion = pText->GetGlyph((uint8*)"?", 1)->GetCel();

		pTileMap = OMalloc<CTileMap>();
		pLayer = OMalloc<CTileLayerCel>(pTileMap, "Text", SSizeVec2(8, 3), SSizeVec2(pWhite->GetAverageWidth(), (size)pWhite->GetLineHeight()), 0, SInt32Vec2(0, 0));
		pTileMap->AddLayer(pLayer);
		pWriter = OMalloc<CTileTextWriter>(pLayer, pColours, sText, 4);
		AssertTrue(pWriter->IsValid());

		//A row past the bottom takes nothing.
		AssertFalse(pWriter->WriteRow(3, "a", 1));
		AssertTrue(pWriter->WriteRow(3, "", 0));
		AssertSize(0, pLayer->NumStoredTiles());

		//A row stops at a new line or a terminator.
		AssertTrue(pWriter->WriteRow(0, "ab\ncd", 5));
		AssertSize(2, pLayer->GetRowEnd(0));
		AssertTrue(pWriter->WriteRow(1, "ab\0cd", 5));
		AssertSize(2, pLayer->GetRowEnd(1));

		//Each character is a column, however many bytes it is.
		AssertTrue(TestMapsGlyphCel(pText, "\xC3\xA9").IsNotNull());
		AssertTrue(TestMapsGlyphCel(pText, "\xE2\x82\xAC").IsNotNull());
		AssertTrue(pWriter->Write("\xC3\xA9\xE2\x82\xAC\xF0\x9F\x98\x80!"));
		AssertTrue(pLayer->GetTile(0, 0) == TestMapsGlyphCel(pText, "\xC3\xA9"));
		AssertTrue(pLayer->GetTile(1, 0) == TestMapsGlyphCel(pText, "\xE2\x82\xAC"));
		AssertFalse(pLayer->GetTile(0, 0) == pQuestion);
		AssertFalse(pLayer->GetTile(1, 0) == pQuestion);
		AssertTrue(pLayer->GetTile(2, 0) == pQuestion);
		AssertTrue(pLayer->GetTile(3, 0) == pText->GetGlyph((uint8*)"!", 1)->GetCel());
		AssertSize(4, pLayer->GetRowEnd(0));

		//A byte that doesn't start a character, a lead byte cut short and a character cut off by the end of the text are
		//each a '?'.
		AssertTrue(pWriter->Write("\xFF" "a\xC3" "b\xE2\x82"));
		AssertTrue(pLayer->GetTile(0, 0) == pQuestion);
		AssertTrue(pLayer->GetTile(1, 0) == pText->GetGlyph((uint8*)"a", 1)->GetCel());
		AssertTrue(pLayer->GetTile(2, 0) == pQuestion);
		AssertTrue(pLayer->GetTile(3, 0) == pText->GetGlyph((uint8*)"b", 1)->GetCel());
		AssertTrue(pLayer->GetTile(4, 0) == pQuestion);
		AssertTrue(pLayer->GetTile(5, 0) == pQuestion);
		AssertSize(6, pLayer->GetRowEnd(0));

		//A character is the colour of the span its first byte is in: one starting inside e acute doesn't colour it.
		asSpans.Init();
		psSpan = asSpans.Add();
		psSpan->uiStart = 1;
		psSpan->uiLength = 1;
		psSpan->sColour = sRed;
		psSpan = asSpans.Add();
		psSpan->uiStart = 3;
		psSpan->uiLength = 1;
		psSpan->sColour = sRed;
		AssertTrue(pWriter->Write("\xC3\xA9xd", &asSpans));
		AssertTrue(pLayer->GetTile(0, 0) == TestMapsGlyphCel(pText, "\xC3\xA9"));
		AssertTrue(pLayer->GetTile(1, 0) == pText->GetGlyph((uint8*)"x", 1)->GetCel());
		AssertTrue(pLayer->GetTile(2, 0) == pColours->GetFont(sRed)->GetGlyph((uint8*)"d", 1)->GetCel());
		asSpans.Kill();

		pWriter = NULL;
		pLayer = NULL;
		pTileMap = NULL;
		pQuestion = NULL;
		pText = NULL;
		pColours = NULL;
		pWhite = NULL;

		ImageChannelDescriptorKill();
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// The code points of UTF-8 characters, and the byte runs that aren't one.
//
//////////////////////////////////////////////////////////////////////////
void TestMapsTileTextCodePoint(void)
{
	AssertLong('a', TileTextCodePoint("a", 1));
	AssertLong(0xE9, TileTextCodePoint("\xC3\xA9", 2));
	AssertLong(0x20AC, TileTextCodePoint("\xE2\x82\xAC", 3));
	AssertLong(0x1F600, TileTextCodePoint("\xF0\x9F\x98\x80", 4));
	AssertLong(0x10FFFF, TileTextCodePoint("\xF4\x8F\xBF\xBF", 4));

	//A byte that doesn't start a character, more bytes than needed, a surrogate and past U+10FFFF.
	AssertLong(TILE_TEXT_NO_CODE_POINT, TileTextCodePoint("\x80", 1));
	AssertLong(TILE_TEXT_NO_CODE_POINT, TileTextCodePoint("\xC0\xAF", 2));
	AssertLong(TILE_TEXT_NO_CODE_POINT, TileTextCodePoint("\xE0\x80\xAF", 3));
	AssertLong(TILE_TEXT_NO_CODE_POINT, TileTextCodePoint("\xED\xA0\x80", 3));
	AssertLong(TILE_TEXT_NO_CODE_POINT, TileTextCodePoint("\xF4\x90\x80\x80", 4));
}


//////////////////////////////////////////////////////////////////////////
//
// Writing the text again keeps the cels it showed: a character in a colour
// is looked up once and has the same tile value however often it is
// written.  The values are forgotten when the layer is cleared, and the
// cels too when the font changes.  Rows of a text's lines must go down.
//
//////////////////////////////////////////////////////////////////////////
void TestMapsTileTextWriterKeepsCels(void)
{
	ObjectsInit();
	{
		Ptr<CFont>				pWhite;
		Ptr<CFont>				pBigWhite;
		Ptr<CFontColours>		pColours;
		Ptr<CFontColours>		pBigColours;
		Ptr<CFont>				pText;
		Ptr<CFont>				pRed;
		Ptr<CTileMap>			pTileMap;
		Ptr<CTileLayerCel>		pLayer;
		Ptr<CTileTextWriter>	pWriter;
		CArrayTileTextSpan		asSpans;
		STileTextSpan*			psSpan;
		CArrayTileTextLineRows	auiLineRows;
		ARGB32					sText;
		ARGB32					sRed;
		size					uiVersion;

		ImageChannelDescriptorInit();

		pWhite = CreateWhiteTrueTypeFontFrom("JetBrainsMonoNL-Regular.ttf", "?ABCabc\xC3\xA9\xE2\x82\xAC", 16.0f);
		pColours = OMalloc<CFontColours>(pWhite);
		sText = Set32BitColour((uint8)200, (uint8)200, (uint8)200, (uint8)255);
		sRed = Set32BitColour((uint8)255, (uint8)0, (uint8)0, (uint8)255);
		pText = pColours->GetFont(sText);
		pRed = pColours->GetFont(sRed);

		pTileMap = OMalloc<CTileMap>();
		pLayer = OMalloc<CTileLayerCel>(pTileMap, "Text", SSizeVec2(8, 4), SSizeVec2(pWhite->GetAverageWidth(), (size)pWhite->GetLineHeight()), 0, SInt32Vec2(0, 0));
		pTileMap->AddLayer(pLayer);
		pWriter = OMalloc<CTileTextWriter>(pLayer, pColours, sText, 4);

		//"AC" then "ABC": A and C keep their cels, B is the only one added.
		AssertTrue(pWriter->Write("AC"));
		AssertSize(2, pLayer->NumCels());
		AssertTrue(pWriter->Write("ABC"));
		AssertSize(3, pLayer->NumCels());
		AssertTrue(pLayer->GetTile(0, 0) == pText->GetGlyph((uint8*)"A", 1)->GetCel());
		AssertTrue(pLayer->GetTile(1, 0) == pText->GetGlyph((uint8*)"B", 1)->GetCel());
		AssertTrue(pLayer->GetTile(2, 0) == pText->GetGlyph((uint8*)"C", 1)->GetCel());

		//Characters of other pages and the same character in another colour are cels of their own; written again they
		//add nothing.
		asSpans.Init();
		psSpan = asSpans.Add();
		psSpan->uiStart = 1;
		psSpan->uiLength = 1;
		psSpan->sColour = sRed;
		AssertTrue(pWriter->Write("AA\n\xC3\xA9\xE2\x82\xAC", &asSpans));
		AssertSize(6, pLayer->NumCels());
		AssertTrue(pWriter->Write("AA\n\xC3\xA9\xE2\x82\xAC", &asSpans));
		AssertSize(6, pLayer->NumCels());
		AssertTrue(pLayer->GetTile(0, 0) == pText->GetGlyph((uint8*)"A", 1)->GetCel());
		AssertTrue(pLayer->GetTile(1, 0) == pRed->GetGlyph((uint8*)"A", 1)->GetCel());
		AssertTrue(pLayer->GetTile(0, 1) == TestMapsGlyphCel(pText, "\xC3\xA9"));
		AssertTrue(pLayer->GetTile(1, 1) == TestMapsGlyphCel(pText, "\xE2\x82\xAC"));
		AssertSize(4, pLayer->NumStoredTiles());
		asSpans.Kill();

		//A row written alone uses the same cels.
		AssertTrue(pWriter->WriteRow(2, "cab", 3));
		AssertSize(9, pLayer->NumCels());
		AssertTrue(pWriter->WriteRow(3, "abc", 3));
		AssertSize(9, pLayer->NumCels());
		AssertTrue(pLayer->GetTile(1, 3) == pLayer->GetTile(2, 2));

		//The layer cleared, the writer looks its cels up again.
		pLayer->Clear();
		AssertTrue(pWriter->WriteRow(0, "BA", 2));
		AssertSize(2, pLayer->NumCels());
		AssertTrue(pLayer->GetTile(0, 0) == pText->GetGlyph((uint8*)"B", 1)->GetCel());
		AssertTrue(pLayer->GetTile(1, 0) == pText->GetGlyph((uint8*)"A", 1)->GetCel());

		//The same font again forgets nothing; a new font's text has only its own cels.
		uiVersion = pLayer->GetCelVersion();
		pWriter->SetColours(pColours, sRed);
		AssertTrue(pWriter->Write("ABA"));
		AssertTrue(uiVersion == pLayer->GetCelVersion());
		AssertSize(4, pLayer->NumCels());
		AssertTrue(pLayer->GetTile(0, 0) == pRed->GetGlyph((uint8*)"A", 1)->GetCel());

		pBigWhite = CreateWhiteTrueTypeFontFrom("JetBrainsMonoNL-Regular.ttf", "?ABC", 20.0f);
		pBigColours = OMalloc<CFontColours>(pBigWhite);
		pWriter->SetColours(pBigColours, sText);
		AssertTrue(pWriter->Write("AB"));
		AssertFalse(uiVersion == pLayer->GetCelVersion());
		AssertSize(2, pLayer->NumCels());
		AssertTrue(pLayer->GetTile(0, 0) == pBigColours->GetFont(sText)->GetGlyph((uint8*)"A", 1)->GetCel());

		//A line whose row isn't below the line before's is cut off.
		auiLineRows.Init();
		*auiLineRows.Add() = 1;
		*auiLineRows.Add() = 1;
		*auiLineRows.Add() = 3;
		AssertFalse(pWriter->Write("A\nB\nC", NULL, &auiLineRows));
		AssertSize(pLayer->GetRowStart(0), pLayer->GetRowEnd(0));
		AssertTrue(pLayer->GetTile(0, 1) == pBigColours->GetFont(sText)->GetGlyph((uint8*)"A", 1)->GetCel());
		AssertSize(pLayer->GetRowStart(2), pLayer->GetRowEnd(2));
		AssertTrue(pLayer->GetTile(0, 3) == pBigColours->GetFont(sText)->GetGlyph((uint8*)"C", 1)->GetCel());
		AssertSize(2, pLayer->NumStoredTiles());
		auiLineRows.Kill();

		pWriter = NULL;
		pLayer = NULL;
		pTileMap = NULL;
		pRed = NULL;
		pText = NULL;
		pBigColours = NULL;
		pBigWhite = NULL;
		pColours = NULL;
		pWhite = NULL;

		ImageChannelDescriptorKill();
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// The bounds of a sparse layer's pixels are those of the tiles in its
// rows' runs.
//
//////////////////////////////////////////////////////////////////////////
void TestMapsTileLayerImageBounds(void)
{
	ObjectsInit();
	{
		Ptr<CArrayImageCel>		pacCels;
		Ptr<CTileMap>			pTileMap;
		Ptr<CTileLayerCel>		pLayer;
		CInt32Rect				cBounds;
		CInt32Rect				cExpected;
		CInt32Rect				cCel;
		Ptr<CImageCel>			pNoCel;

		ImageChannelDescriptorInit();

		pacCels = TestMapsReadSpaceCrusadeCels("Tiles.png", 10, 3);
		pTileMap = OMalloc<CTileMap>();
		pLayer = OMalloc<CTileLayerCel>(pTileMap, "Cel", SSizeVec2(10, 6), SSizeVec2(16, 16), 0, SInt32Vec2(0, 0));
		pTileMap->AddLayer(pLayer);

		//No tiles have no bounds.
		cExpected.Init();
		AssertTrue(pLayer->GetImageDestBounds(&cBounds));
		AssertTrue(cExpected.Equals(&cBounds));

		//One tile, then tiles in two rows with an empty row above and between them.
		AssertTrue(pLayer->SetTile(4, 1, pacCels->Get(1)));
		pacCels->Get(1)->GetImageDestBounds(4 * 16, 1 * 16, &cExpected);
		AssertTrue(pLayer->GetImageDestBounds(&cBounds));
		AssertTrue(cExpected.Equals(&cBounds));

		AssertTrue(pLayer->SetTile(1, 3, pacCels->Get(2)));
		AssertTrue(pLayer->SetTile(8, 3, pacCels->Get(3)));
		pacCels->Get(2)->GetImageDestBounds(1 * 16, 3 * 16, &cCel);
		cExpected.GrowToContain(&cCel);
		pacCels->Get(3)->GetImageDestBounds(8 * 16, 3 * 16, &cCel);
		cExpected.GrowToContain(&cCel);
		AssertTrue(pLayer->GetImageDestBounds(&cBounds));
		AssertTrue(cExpected.Equals(&cBounds));
		AssertInt((int32)16, cBounds.msTopLeft.x);
		AssertInt((int32)16, cBounds.msTopLeft.y);

		//Taking the ends of the bottom row away shrinks the bounds back to the first tile.
		AssertTrue(pLayer->SetTile(1, 3, pNoCel));
		AssertTrue(pLayer->SetTile(8, 3, pNoCel));
		pacCels->Get(1)->GetImageDestBounds(4 * 16, 1 * 16, &cExpected);
		AssertTrue(pLayer->GetImageDestBounds(&cBounds));
		AssertTrue(cExpected.Equals(&cBounds));

		pLayer = NULL;
		pTileMap = NULL;
		pacCels = NULL;

		ImageChannelDescriptorKill();
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// A layer grows and shrinks keeping the tiles still in it; the columns of
// text go by the tile writer's rules.
//
//////////////////////////////////////////////////////////////////////////
void TestMapsTileLayerResize(void)
{
	ObjectsInit();
	{
		Ptr<CArrayImageCel>		pacCels;
		Ptr<CTileMap>			pTileMap;
		Ptr<CTileLayerCel>		pLayer;

		ImageChannelDescriptorInit();

		pacCels = TestMapsReadSpaceCrusadeCels("Tiles.png", 10, 3);
		pTileMap = OMalloc<CTileMap>();
		pLayer = OMalloc<CTileLayerCel>(pTileMap, "Cel", SSizeVec2(4, 3), SSizeVec2(16, 16), 0, SInt32Vec2(0, 0));
		pTileMap->AddLayer(pLayer);
		AssertTrue(pLayer->SetTile(1, 0, pacCels->Get(1)));
		AssertTrue(pLayer->SetTile(3, 1, pacCels->Get(2)));
		AssertTrue(pLayer->SetTile(0, 2, pacCels->Get(3)));
		AssertTrue(pLayer->SetTile(2, 2, pacCels->Get(4)));

		//Growing keeps every tile, and the new rows and columns can be set.
		pLayer->SetMapSize(SSizeVec2(6, 5));
		AssertInt((int32)6, pLayer->GetLayerSizeX());
		AssertInt((int32)5, pLayer->GetLayerSizeY());
		AssertTrue(pLayer->GetTile(1, 0) == pacCels->Get(1));
		AssertTrue(pLayer->GetTile(3, 1) == pacCels->Get(2));
		AssertTrue(pLayer->GetTile(2, 2) == pacCels->Get(4));
		AssertTrue(pLayer->SetTile(5, 4, pacCels->Get(5)));
		AssertTrue(pLayer->SetTile(5, 0, pacCels->Get(6)));
		AssertTrue(pLayer->GetTile(5, 4) == pacCels->Get(5));
		AssertTrue(pLayer->GetTile(1, 0) == pacCels->Get(1));
		AssertSize(5 + 1 + 3 + 1, pLayer->NumStoredTiles());

		//Shrinking takes away the tiles outside and trims the rows that are left.
		pLayer->SetMapSize(SSizeVec2(2, 3));
		AssertTrue(pLayer->GetTile(1, 0) == pacCels->Get(1));
		AssertSize(2, pLayer->GetRowEnd(0));
		AssertSize(pLayer->GetRowStart(1), pLayer->GetRowEnd(1));
		AssertTrue(pLayer->GetTile(0, 2) == pacCels->Get(3));
		AssertSize(1, pLayer->GetRowEnd(2));
		AssertSize(2, pLayer->NumStoredTiles());
		AssertFalse(pLayer->SetTile(2, 0, pacCels->Get(1)));
		AssertFalse(pLayer->SetTile(0, 3, pacCels->Get(1)));

		//A smaller cel size keeps the tiles where they are in the grid.
		pLayer->SetCelSize(SSizeVec2(8, 8));
		AssertSize(8, pLayer->GetCelSize().x);
		AssertTrue(pLayer->GetTile(0, 2) == pacCels->Get(3));

		pLayer = NULL;
		pTileMap = NULL;
		pacCels = NULL;

		ImageChannelDescriptorKill();
	}
	ObjectsKill(false);

	//Columns: a tab goes to the next stop, a carriage return takes none, a character of any length takes one.
	AssertSize(3, TileTextColumns("abc", 3, 0, 4));
	AssertSize(5, TileTextColumns("a\tb", 3, 0, 4));
	AssertSize(8, TileTextColumns("\t", 1, 4, 4));
	AssertSize(8, TileTextColumns("\t", 1, 7, 4));
	AssertSize(2, TileTextColumns("a\rb", 3, 0, 4));
	AssertSize(2, TileTextColumns("\xC3\xA9\xE2\x82\xAC", 5, 0, 4));
	AssertSize(1, TileTextColumns("a\nbc", 4, 0, 4));
	AssertSize(3, TileTextColumns("\t", 1, 2, 3));
}


//////////////////////////////////////////////////////////////////////////
//
// Maps blitted with a clip draw what they would without one inside it,
// and nothing outside it: tiles and sprites cut at its edges, shapes,
// lines and rectangles cut too, the viewport scrolled.
//
//////////////////////////////////////////////////////////////////////////
void TestMapsClippedBlit(void)
{
	ObjectsInit();
	{
		CMaps						cMaps;
		Ptr<CArrayImageCel>			pacTiles;
		Ptr<CArrayImageCel>			pacSprites;
		Ptr<CImage>					pViewport;
		Ptr<CImageCelBlitterCache>	pCache;
		Ptr<CTileMap>				pTileMap;
		Ptr<CTileLayerCel>			pLayer;
		Ptr<CSpriteMap>				pSpriteMap;
		Ptr<CGeometryMap>			pGeometryMap;
		uint8*						puiFull;
		uint8*						puiData;
		size						uiBytes;
		size						uiPixelBytes;
		CInt32Rect					acClips[2];
		CInt32Rect					cClip;
		int32						x;
		int32						y;
		size						i;
		size						uiInside;
		size						uiWrongInside;
		size						uiDrawnOutside;
		bool						bInside;

		ImageChannelDescriptorInit();

		pacTiles = TestMapsReadSpaceCrusadeCels("Tiles.png", 10, 3);
		pacSprites = TestMapsReadSpaceCrusadeCels("Gretchin.png", 8, 1);
		pViewport = OMalloc<CImage>(160, 96, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pCache = OMalloc<CImageCelBlitterCache>(pViewport);
		cMaps.Init(pCache, pViewport);

		pTileMap = OMalloc<CTileMap>();
		pLayer = OMalloc<CTileLayerCel>(pTileMap, "Cel", SSizeVec2(10, 6), SSizeVec2(16, 16), 0, SInt32Vec2(0, 0));
		pTileMap->AddLayer(pLayer);
		TestMapsSetSparseTiles(pLayer, pacTiles);
		cMaps.AddMap(pTileMap);

		pSpriteMap = OMalloc<CSpriteMap>();
		AssertTrue(pSpriteMap->AddSprite(pacSprites->Get(0), 30, 15));
		AssertTrue(pSpriteMap->AddSprite(pacSprites->Get(3), 95, 50));
		cMaps.AddMap(pSpriteMap);

		pGeometryMap = OMalloc<CGeometryMap>();
		pGeometryMap->AddRectangle(20, 40, 120, 8, Set32BitColour(1.0f, 0, 0, 0.5f));
		pGeometryMap->AddFilledCircle(80.5, 45.5, 22, Set32BitColour(0, 1.0f, 0));
		pGeometryMap->AddLine(0, 0, 150, 90, 3, Set32BitColour(1.0f, 1.0f, 0));
		cMaps.AddMap(pGeometryMap);

		//All of it, scrolled, as it is drawn without a clip.
		cMaps.SetViewportPosition(5, 3);
		pViewport->Black();
		AssertTrue(cMaps.Blit(true));
		uiBytes = (size)pViewport->GetByteSize();
		uiPixelBytes = uiBytes / (160 * 96);
		puiFull = (uint8*)malloc(uiBytes);
		memcpy(puiFull, pViewport->GetData(), uiBytes);

		//Two parts, neither on the tiles' edges, drawn one after the other.
		acClips[0].Init(37, 21, 101, 52);
		acClips[1].Init(3, 60, 49, 95);
		pViewport->Black();
		for (i = 0; i < 2; i++)
		{
			AssertTrue(cMaps.Blit(false, &acClips[i]));
		}

		puiData = (uint8*)pViewport->GetData();
		uiInside = 0;
		uiWrongInside = 0;
		uiDrawnOutside = 0;
		for (y = 0; y < 96; y++)
		{
			for (x = 0; x < 160; x++)
			{
				bInside = acClips[0].ContainsPoint(x, y) || acClips[1].ContainsPoint(x, y);
				i = ((size)y * 160 + (size)x) * uiPixelBytes;
				if (bInside)
				{
					uiInside++;
					if (memcmp(&puiData[i], &puiFull[i], uiPixelBytes) != 0)
					{
						uiWrongInside++;
					}
				}
				else if ((puiData[i] != 0) || (puiData[i + 1] != 0) || (puiData[i + 2] != 0))
				{
					uiDrawnOutside++;
				}
			}
		}
		AssertSize(64 * 31 + 46 * 35, uiInside);
		AssertSize(0, uiWrongInside);
		AssertSize(0, uiDrawnOutside);

		//The clip is in the viewport's pixels, wherever the viewport is.
		cClip.Init(0, 0, 160, 96);
		pViewport->Black();
		AssertTrue(cMaps.Blit(false, &cClip));
		AssertInt(0, memcmp(pViewport->GetData(), puiFull, uiBytes));

		free(puiFull);
		cMaps.Kill();
		pGeometryMap = NULL;
		pSpriteMap = NULL;
		pLayer = NULL;
		pTileMap = NULL;
		pCache = NULL;
		pViewport = NULL;
		pacSprites = NULL;
		pacTiles = NULL;

		ImageChannelDescriptorKill();
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
	TestMapsTileLayerRows();
	TestMapsTileLayerSetRowTiles();
	TestMapsTileLayerText();
	TestMapsTileTextWriter();
	TestMapsTileLayerBlitAfterEdits();
	TestMapsTileLayerBlitScrolled();
	TestMapsClippedBlit();
	TestMapsTileTextMatchesSpriteText();
	TestMapsTileTextWriterCharacters();
	TestMapsTileTextCodePoint();
	TestMapsTileTextWriterKeepsCels();
	TestMapsTileLayerImageBounds();
	TestMapsTileLayerResize();
	TestMapsSpriteMap();

	DataIOKill();

	TestStatistics();
}

