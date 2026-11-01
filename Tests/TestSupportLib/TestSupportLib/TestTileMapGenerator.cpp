#include "BaseLib/GlobalDataTypesIO.h"
#include "SupportLib/ImageWriter.h"
#include "SupportLib/Maps.h"
#include "SupportLib/TileLayerCel.h"
#include "SupportLib/TileMapGenerator.h"
#include "SupportLib/ImageWriter.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"
#include "TestReadImage.h"
#include "SupportAssert.h"


enum ESpaceCrusadeCelType
{
	SCCT_Invalid,
	SCCT_Space,
	SCCT_Wall,
	SCCT_Floor,
	SCCT_Door,
	SCCT_Shadow,
};


enum ESpaceCrusadeMapLayer
{
	SCML_Invalid,
	SCML_Floor,
	SCML_Shadow,
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTileMapGeneratorAddPatterns(Ptr<CTileMapGenerator> pTileMapGenerator, Ptr<CArrayImageCel> paBackgroundCels, bool bIncludeShadows)
{
	//Blocks must be rectangular with each row ending with a newline.  Spaces are removed before processing.
	// .: any block 
	//  e.g. W ->  SCCT_Wall
	//  e.g. w -> !SCCT_Wall

	pTileMapGenerator->AddCelType(			'W', SCCT_Wall);
	pTileMapGenerator->AddNegativeCelType(	'w', SCCT_Wall);
	pTileMapGenerator->AddCelType(			'C', SCCT_Wall, SCCT_Door);
	pTileMapGenerator->AddNegativeCelType(	'c', SCCT_Wall, SCCT_Door);
	pTileMapGenerator->AddCelType(			'D', SCCT_Door);
	pTileMapGenerator->AddCelType(			'F', SCCT_Floor);

	pTileMapGenerator->AddPattern("FloorPlan", SCCT_Space, "Single",		" . . . \n"
																			" . . . \n"
																			" . . . \n");

	pTileMapGenerator->AddPattern("FloorPlan", SCCT_Floor, "Single",		" . . . \n"
																			" . F . \n"
																			" . . . \n");

	pTileMapGenerator->AddPattern("FloorPlan", SCCT_Wall, "Single",			" . c . \n"
																			" c F c \n"
																			" . c . \n");

	pTileMapGenerator->AddPattern("FloorPlan", SCCT_Wall, "Star",			" . C . \n"
																			" C W C \n"
																			" . C . \n");

	pTileMapGenerator->AddPattern("FloorPlan", SCCT_Wall, "UpDown",			" . C . \n"
																			" c W c \n"
																			" . C . \n");

	pTileMapGenerator->AddPattern("FloorPlan", SCCT_Wall, "LeftRight",		" . c . \n"
																			" C W C \n"
																			" . c . \n");

	pTileMapGenerator->AddPattern("FloorPlan", SCCT_Wall, "UpRight",		" . C . \n"
																			" c W C \n"
																			" . c . \n");

	pTileMapGenerator->AddPattern("FloorPlan", SCCT_Wall, "DownRight",		" . c . \n"
																			" c W C \n"
																			" . C . \n");

	pTileMapGenerator->AddPattern("FloorPlan", SCCT_Wall, "DownLeft",		" . c . \n"
																			" C W c \n"
																			" . C . \n");

	pTileMapGenerator->AddPattern("FloorPlan", SCCT_Wall, "UpLeft",			" . C . \n"
																			" C W c \n"
																			" . c . \n");

	pTileMapGenerator->AddPattern("FloorPlan", SCCT_Wall, "LeftRightDown",	" . c . \n"
																			" C W C \n"
																			" . C . \n");

	pTileMapGenerator->AddPattern("FloorPlan", SCCT_Wall, "UpDownLeft",		" . C . \n"
																			" C W c \n"
																			" . C . \n");

	pTileMapGenerator->AddPattern("FloorPlan", SCCT_Wall, "UpDownRight",	" . C . \n"
																			" c W C \n"
																			" . C . \n");

	pTileMapGenerator->AddPattern("FloorPlan", SCCT_Wall, "LeftRightUp",	" . C . \n"
																			" C W C \n"
																			" . c . \n");

	pTileMapGenerator->AddPattern("FloorPlan", SCCT_Door, "DoorDown",		" . W . \n"
																			" . D . \n"
																			" . D . \n");

	pTileMapGenerator->AddPattern("FloorPlan", SCCT_Door, "DoorUp",			" . D . \n"
																			" . D . \n"
																			" . W . \n");

	pTileMapGenerator->AddPattern("FloorPlan", SCCT_Door, "DoorLeft",		" . . . \n"
																			" W D D \n"
																			" . . . \n");

	pTileMapGenerator->AddPattern("FloorPlan", SCCT_Door, "DoorRight",		" . . . \n"
																			" D D W \n"
																			" . . . \n");

	pTileMapGenerator->AddPattern("FloorPlan", SCCT_Shadow, "ShadowLeft",	" C w . \n"
																			" W F . \n"
																			" . . . \n");
	if (bIncludeShadows)
	{
		pTileMapGenerator->AddPattern("FloorPlan", SCCT_Shadow, "ShadowUp",		" C W . \n"
																				" w F . \n"
																				" . . . \n");

		pTileMapGenerator->AddPattern("FloorPlan", SCCT_Shadow, "ShadowSingle",	" W w . \n"
																				" w F . \n"
																				" . . . \n");

		pTileMapGenerator->AddPattern("FloorPlan", SCCT_Shadow, "ShadowUpLeft",	" W W . \n"
																				" W F . \n"
																				" . . . \n");

		pTileMapGenerator->AddPattern("FloorPlan", SCCT_Shadow,	"ShadowDown",	" w w . \n"
																				" W F . \n"
																				" . . . \n");

		pTileMapGenerator->AddPattern("FloorPlan", SCCT_Shadow,	"ShadowDiag",	" w W . \n"
																				" w F . \n"
																				" . . . \n");
	}

	pTileMapGenerator->AddTileBrush(paBackgroundCels, 26, SCCT_Space,	"Single");
	pTileMapGenerator->AddTileBrush(paBackgroundCels,  0, SCCT_Wall,	"Single");
	pTileMapGenerator->AddTileBrush(paBackgroundCels,  0, SCCT_Wall,	"Star");
	pTileMapGenerator->AddTileBrush(paBackgroundCels,  4, SCCT_Wall,	"UpDown");
	pTileMapGenerator->AddTileBrush(paBackgroundCels,  5, SCCT_Wall,	"LeftRight");
	pTileMapGenerator->AddTileBrush(paBackgroundCels,  6, SCCT_Wall,	"DownLeft");
	pTileMapGenerator->AddTileBrush(paBackgroundCels,  7, SCCT_Wall,	"DownRight");
	pTileMapGenerator->AddTileBrush(paBackgroundCels,  8, SCCT_Wall,	"UpLeft");
	pTileMapGenerator->AddTileBrush(paBackgroundCels,  9, SCCT_Wall,	"UpRight");
	pTileMapGenerator->AddTileBrush(paBackgroundCels, 10, SCCT_Wall,	"LeftRightDown");
	pTileMapGenerator->AddTileBrush(paBackgroundCels, 11, SCCT_Wall,	"UpDownLeft");
	pTileMapGenerator->AddTileBrush(paBackgroundCels, 12, SCCT_Wall,	"UpDownRight");
	pTileMapGenerator->AddTileBrush(paBackgroundCels, 13, SCCT_Wall,	"LeftRightUp");
	pTileMapGenerator->AddTileBrush(paBackgroundCels,  1, SCCT_Floor,	"Single", 2);
	pTileMapGenerator->AddTileBrush(paBackgroundCels,  2, SCCT_Floor,	"Single", 200);
	pTileMapGenerator->AddTileBrush(paBackgroundCels,  3, SCCT_Floor,	"Single", 1);
	pTileMapGenerator->AddTileBrush(paBackgroundCels, 14, SCCT_Door,	"DoorUp");
	pTileMapGenerator->AddTileBrush(paBackgroundCels, 15, SCCT_Door,	"DoorDown");
	pTileMapGenerator->AddTileBrush(paBackgroundCels, 16, SCCT_Door,	"DoorLeft");
	pTileMapGenerator->AddTileBrush(paBackgroundCels, 17, SCCT_Door,	"DoorRight");
	pTileMapGenerator->AddTileBrush(paBackgroundCels, 22, SCCT_Shadow,	"ShadowLeft");
	if (bIncludeShadows)
	{
		pTileMapGenerator->AddTileBrush(paBackgroundCels, 23, SCCT_Shadow, "ShadowSingle");
		pTileMapGenerator->AddTileBrush(paBackgroundCels, 24, SCCT_Shadow, "ShadowUpLeft");
		pTileMapGenerator->AddTileBrush(paBackgroundCels, 25, SCCT_Shadow, "ShadowUp");
		pTileMapGenerator->AddTileBrush(paBackgroundCels, 27, SCCT_Shadow, "ShadowDown");
		pTileMapGenerator->AddTileBrush(paBackgroundCels, 28, SCCT_Shadow, "ShadowDiag");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTileMapGenerator3x3ImageGenerate(void)
{
	char				szDirectory[] = "Output" _FS_ "TileMapGenerator3x3Generate";
	CIndexTreeMemory	cMemory;
	CFileUtil			cFileUtil;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	ObjectsInit();
	{
		Ptr<CMaps>					pMaps;
		CImageDivider				cImageDivider;
		CChars						szInputFilename;
		CChars						szOutputFilename;
		Ptr<CRoot>					pRoot;
		Ptr<CTileMapGenerator>		pTileMapGenerator;
		Ptr<CArrayImageCel>			paBackgroundCels;
		CTileCelColourSource*		pcWallColour;
		CTileCelColourSource*		pcFloorColour;
		CTileCelColourSource*		pcSpaceColour;
		CTileCelColourSource*		pcDoorColour;
		Ptr<CImage>					pFloorPlanImage;
		Ptr<CTileMap>				pTileMap;
		Ptr<CImage>					pDestImage;
		Ptr<CImageCelBlitterCache>	pBlitterCache;
		SSizeVec2					sMapSize;
		SSizeVec2					sCelSize;
		bool						bResult;
		bool						bWritten;
		CRandom						cRandom;

		pFloorPlanImage = TestReadImage("SpaceCrusade", "SmallPlan.png");

		cRandom.Init(2345);
		pRoot = ORoot();
		pTileMapGenerator = OMalloc<CTileMapGenerator>(&cRandom);
		pRoot->Add(pTileMapGenerator);

		pTileMapGenerator->AddTileGridSource("FloorPlan", pFloorPlanImage);

		paBackgroundCels = TestReadCels("SpaceCrusade", "Tiles.png", 10, 3);

		TestTileMapGeneratorAddPatterns(pTileMapGenerator, paBackgroundCels, false);

		pcWallColour = pTileMapGenerator->AddColourSource(138, 138, 138);
		pcFloorColour = pTileMapGenerator->AddColourSource(107, 73, 14);
		pcSpaceColour = pTileMapGenerator->AddColourSource(0, 0, 0);
		pcDoorColour = pTileMapGenerator->AddColourSource(71, 71, 71);

		pTileMapGenerator->AddTileGenerator("FloorPlan", SCML_Floor, SCCT_Space,	pcSpaceColour);
		pTileMapGenerator->AddTileGenerator("FloorPlan", SCML_Floor, SCCT_Wall,		pcWallColour);
		pTileMapGenerator->AddTileGenerator("FloorPlan", SCML_Floor, SCCT_Door,		pcDoorColour);
		pTileMapGenerator->AddTileGenerator("FloorPlan", SCML_Floor, SCCT_Floor,	pcFloorColour);
		pTileMapGenerator->SetEdgeSource(pcSpaceColour);

		sMapSize = pTileMapGenerator->GetMapSize();
		sCelSize = pTileMapGenerator->GetCelSize();

		pTileMap = pTileMapGenerator->Generate();

		pDestImage = OMalloc<CImage>((uint32)(sMapSize.x * sCelSize.x), (uint32)(sMapSize.y * sCelSize.y), PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pBlitterCache = OMalloc<CImageCelBlitterCache>(pDestImage);

		pMaps = OMalloc<CMaps>(pBlitterCache, pDestImage);;
		pRoot->Add(pMaps);
		pMaps->AddMap(pTileMap);

		pMaps->SetViewportPosition(0, 0);

		bResult = pMaps->CreateCelBlitters();
		AssertTrue(bResult);

		bResult = pMaps->Blit(false);
		AssertTrue(bResult);

		pRoot->Remove(pMaps);
		pRoot->Remove(pTileMapGenerator);
		pMaps = NULL;
		pTileMapGenerator = NULL;

		szOutputFilename.Init(szDirectory);
		cFileUtil.AppendToPath(&szOutputFilename, "TileMapGenerator3x3Generate");
		szOutputFilename.Append(".png");
		bWritten = WriteImage(pDestImage, szOutputFilename.Text(), IT_PNG);
		AssertTrue(bWritten);

		szInputFilename.Init(szOutputFilename);
		szInputFilename.Replace("Output", "Input");

		AssertFile(szInputFilename, szOutputFilename);

		szOutputFilename.Kill();
		szInputFilename.Kill();
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTileMapGenerator40x40ImageGenerate(void)
{
	char				szDirectory[] = "Output" _FS_ "TileMapGenerator40x40Generate";
	CIndexTreeMemory	cMemory;
	CFileUtil			cFileUtil;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	ObjectsInit();
	{
		Ptr<CMaps>					pMaps;
		CImageDivider				cImageDivider;
		CChars						szInputFilename;
		CChars						szOutputFilename;
		Ptr<CRoot>					pRoot;
		Ptr<CTileMapGenerator>		pTileMapGenerator;
		Ptr<CArrayImageCel>			paBackgroundCels;
		CTileCelColourSource*		pcWallColour;
		CTileCelColourSource*		pcFloorColour;
		CTileCelColourSource*		pcSpaceColour;
		CTileCelColourSource*		pcDoorColour;
		Ptr<CImage>					pFloorPlanImage;
		Ptr<CTileMap>				pTileMap;
		Ptr<CImage>					pDestImage;
		Ptr<CImageCelBlitterCache>	pBlitterCache;
		SSizeVec2					sMapSize;
		SSizeVec2					sCelSize;
		bool						bResult;
		bool						bWritten;
		CRandom						cRandom;

		pFloorPlanImage = TestReadImage("SpaceCrusade", "FloorPlan.png");

		cRandom.Init(2345);

		pRoot = ORoot();
		pTileMapGenerator = OMalloc<CTileMapGenerator>(&cRandom);
		pRoot->Add(pTileMapGenerator);

		pTileMapGenerator->AddTileGridSource("FloorPlan", pFloorPlanImage);

		paBackgroundCels = TestReadCels("SpaceCrusade", "Tiles.png", 10, 3);

		TestTileMapGeneratorAddPatterns(pTileMapGenerator, paBackgroundCels, true);

		pcWallColour = pTileMapGenerator->AddColourSource(138, 138, 138);
		pcFloorColour = pTileMapGenerator->AddColourSource(107, 73, 14);
		pcSpaceColour = pTileMapGenerator->AddColourSource(0, 0, 0);
		pcDoorColour = pTileMapGenerator->AddColourSource(71, 71, 71);

		pTileMapGenerator->AddTileGenerator("FloorPlan", SCML_Floor,	SCCT_Space,  pcSpaceColour);
		pTileMapGenerator->AddTileGenerator("FloorPlan", SCML_Floor,	SCCT_Wall,	 pcWallColour);
		pTileMapGenerator->AddTileGenerator("FloorPlan", SCML_Floor,	SCCT_Door,	 pcDoorColour);
		pTileMapGenerator->AddTileGenerator("FloorPlan", SCML_Floor,	SCCT_Floor,  pcFloorColour);
		pTileMapGenerator->AddTileGenerator("FloorPlan", SCML_Shadow,	SCCT_Shadow, pcFloorColour);
		pTileMapGenerator->SetEdgeSource(pcSpaceColour);

		sMapSize = pTileMapGenerator->GetMapSize();
		sCelSize = pTileMapGenerator->GetCelSize();

		//Validating every object each time one is added is too slow for the thousands of tiles and their blitters; validate
		//once afterwards.
		gcObjects.DisableValidation();
		pTileMap = pTileMapGenerator->Generate();

		pDestImage = OMalloc<CImage>((uint32)(sMapSize.x * sCelSize.x), (uint32)(sMapSize.y * sCelSize.y), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		pDestImage->White();
		pBlitterCache = OMalloc<CImageCelBlitterCache>(pDestImage);
		
		pMaps = OMalloc<CMaps>(pBlitterCache, pDestImage);;
		pRoot->Add(pMaps);
		pMaps->AddMap(pTileMap);

		pMaps->SetViewportPosition(0, 0);

		bResult = pMaps->CreateCelBlitters();
		AssertTrue(bResult);

		bResult = pMaps->Blit(false);
		AssertTrue(bResult);
		gcObjects.EnableValidation();
		gcObjects.ValidateObjectsConsistency();

		pRoot->Remove(pMaps);
		pRoot->Remove(pTileMapGenerator);
		pMaps = NULL;
		pTileMapGenerator = NULL;

		szOutputFilename.Init(szDirectory);
		cFileUtil.AppendToPath(&szOutputFilename, "TileMapGenerator40x40Generate");
		szOutputFilename.Append(".png");
		bWritten = WriteImage(pDestImage, szOutputFilename.Text(), IT_PNG);
		AssertTrue(bWritten);

		szInputFilename.Init(szOutputFilename);
		szInputFilename.Replace("Output", "Input");

		AssertFile(szInputFilename, szOutputFilename);

		szOutputFilename.Kill();
		szInputFilename.Kill();
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTileMapGenerator7x5StringGenerate(void)
{
	char				szDirectory[] = "Output" _FS_ "TileMapGenerator7x5StringGenerate";
	CIndexTreeMemory	cMemory;
	CFileUtil			cFileUtil;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	ObjectsInit();
	{
		Ptr<CMaps>					pMaps;
		CImageDivider				cImageDivider;
		CChars						szInputFilename;
		CChars						szOutputFilename;
		Ptr<CRoot>					pRoot;
		Ptr<CTileMapGenerator>		pTileMapGenerator;
		Ptr<CArrayImageCel>			paBackgroundCels;
		CTileCelCharSource*			pcWallColour;
		CTileCelCharSource*			pcFloorColour;
		CTileCelCharSource*			pcSpaceColour;
		CTileCelCharSource*			pcDoorColour;
		Ptr<CTileMap>				pTileMap;
		Ptr<CImage>					pDestImage;
		Ptr<CImageCelBlitterCache>	pBlitterCache;
		SSizeVec2					sMapSize;
		SSizeVec2					sCelSize;
		bool						bResult;
		bool						bWritten;
		CRandom						cRandom;

		cRandom.Init(2345);

		pRoot = ORoot();
		pTileMapGenerator = OMalloc<CTileMapGenerator>(&cRandom);
		pRoot->Add(pTileMapGenerator);

		CArrayChars		aszArrayString;

		aszArrayString.Init("  WWW  ",
							"WWW.WWW",
							"W.....W",
							"WWW.WWW",
							"  WWW  ",
							NULL);

		pTileMapGenerator->AddTileGridSource("FloorPlan", &aszArrayString);

		paBackgroundCels = TestReadCels("SpaceCrusade", "Tiles.png", 10, 3);

		TestTileMapGeneratorAddPatterns(pTileMapGenerator, paBackgroundCels, true);

		pcWallColour = pTileMapGenerator->AddCharSource('W');
		pcFloorColour = pTileMapGenerator->AddCharSource('.');
		pcSpaceColour = pTileMapGenerator->AddCharSource(' ');
		pcDoorColour = pTileMapGenerator->AddCharSource('D');

		pTileMapGenerator->AddTileGenerator("FloorPlan", SCML_Floor,	SCCT_Space,  pcSpaceColour);
		pTileMapGenerator->AddTileGenerator("FloorPlan", SCML_Floor,	SCCT_Wall,	 pcWallColour);
		pTileMapGenerator->AddTileGenerator("FloorPlan", SCML_Floor,	SCCT_Door,	 pcDoorColour);
		pTileMapGenerator->AddTileGenerator("FloorPlan", SCML_Floor,	SCCT_Floor,  pcFloorColour);
		pTileMapGenerator->AddTileGenerator("FloorPlan", SCML_Shadow,	SCCT_Shadow, pcFloorColour);
		pTileMapGenerator->SetEdgeSource(pcSpaceColour);

		sMapSize = pTileMapGenerator->GetMapSize();
		sCelSize = pTileMapGenerator->GetCelSize();

		pTileMap = pTileMapGenerator->Generate();

		pDestImage = OMalloc<CImage>((uint32)(sMapSize.x * sCelSize.x), (uint32)(sMapSize.y * sCelSize.y), PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pDestImage->White();
		pBlitterCache = OMalloc<CImageCelBlitterCache>(pDestImage);
		
		pMaps = OMalloc<CMaps>(pBlitterCache, pDestImage);;
		pRoot->Add(pMaps);
		pMaps->AddMap(pTileMap);

		pMaps->SetViewportPosition(0, 0);

		bResult = pMaps->CreateCelBlitters();
		AssertTrue(bResult);

		bResult = pMaps->Blit(false);
		AssertTrue(bResult);

		pRoot->Remove(pMaps);
		pRoot->Remove(pTileMapGenerator);
		pMaps = NULL;
		pTileMapGenerator = NULL;
		aszArrayString.Kill();

		szOutputFilename.Init(szDirectory);
		cFileUtil.AppendToPath(&szOutputFilename, "TileMapGenerator7x5StringGenerate");
		szOutputFilename.Append(".png");
		bWritten = WriteImage(pDestImage, szOutputFilename.Text(), IT_PNG);
		AssertTrue(bWritten);

		szInputFilename.Init(szOutputFilename);
		szInputFilename.Replace("Output", "Input");

		AssertFile(szInputFilename, szOutputFilename);

		szOutputFilename.Kill();
		szInputFilename.Kill();
	}
	ObjectsFlush();
	ObjectsKill(false);
}


struct SDoor
{
	SSizeVec2	sPosition;
	char*		szDoorDirection;
};


typedef CArrayTemplate<SDoor> CArrayDoor;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DoorCallback(Ptr<CTileCelBrush> pBrush, size xPosition, size yPosition, void* pvData)
{
	CArrayDoor*		pasDoors;
	SDoor*			psDoor;

	pasDoors = (CArrayDoor*)pvData;
	psDoor = pasDoors->Add();
	psDoor->szDoorDirection = pBrush->GetPatternName();
	psDoor->sPosition.Init(xPosition, yPosition);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTileMapGenerator11x11StringGenerate(void)
{
	char				szDirectory[] = "Output" _FS_ "TileMapGenerator11x11StringGenerate";
	CIndexTreeMemory	cMemory;
	CFileUtil			cFileUtil;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	ObjectsInit();
	{
		Ptr<CMaps>					pMaps;
		CImageDivider				cImageDivider;
		CChars						szInputFilename;
		CChars						szOutputFilename;
		Ptr<CRoot>					pRoot;
		Ptr<CTileMapGenerator>		pTileMapGenerator;
		Ptr<CArrayImageCel>			paBackgroundCels;
		CTileCelCharSource*			pcWallColour;
		CTileCelCharSource*			pcFloorColour;
		CTileCelCharSource*			pcSpaceColour;
		CTileCelCharSource*			pcDoorColour;
		Ptr<CTileMap>				pTileMap;
		Ptr<CImage>					pDestImage;
		Ptr<CImageCelBlitterCache>	pBlitterCache;
		Ptr<CTileCelBrush>			pBrush;
		SSizeVec2					sMapSize;
		SSizeVec2					sCelSize;
		bool						bResult;
		bool						bWritten;
		CRandom						cRandom;
		CArrayDoor					asDoors;
		SDoor*						psDoor;

		cRandom.Init(2345);

		pRoot = ORoot();
		pTileMapGenerator = OMalloc<CTileMapGenerator>(&cRandom);
		pRoot->Add(pTileMapGenerator);

		CArrayChars		aszArrayString;

		aszArrayString.Init("             ",
							" WWWWWWWWWWW ",
							" W....W....W ",
							" W....D....W ",
							" W....D....W ",
							" W....W....W ",
							" WWDDWWWDDWW ",
							" W....W....W ",
							" W....D....W ",
							" W....D....W ",
							" W....W....W ",
							" WWWWWWWWWWW ",
							"             ",
							NULL);

		pTileMapGenerator->AddTileGridSource("FloorPlan", &aszArrayString);

		paBackgroundCels = TestReadCels("SpaceCrusade", "Tiles.png", 10, 3);

		TestTileMapGeneratorAddPatterns(pTileMapGenerator, paBackgroundCels, true);

		pcWallColour = pTileMapGenerator->AddCharSource('W');
		pcFloorColour = pTileMapGenerator->AddCharSource('.');
		pcSpaceColour = pTileMapGenerator->AddCharSource(' ');
		pcDoorColour = pTileMapGenerator->AddCharSource('D');

		pTileMapGenerator->AddTileGenerator("FloorPlan", SCML_Floor,	SCCT_Space,  pcSpaceColour);
		pTileMapGenerator->AddTileGenerator("FloorPlan", SCML_Floor,	SCCT_Wall,	 pcWallColour);
		pTileMapGenerator->AddTileGenerator("FloorPlan", SCML_Floor,	SCCT_Door,	 pcDoorColour);
		pTileMapGenerator->AddTileGenerator("FloorPlan", SCML_Floor,	SCCT_Floor,  pcFloorColour);
		pTileMapGenerator->AddTileGenerator("FloorPlan", SCML_Shadow,	SCCT_Shadow, pcFloorColour);
		pTileMapGenerator->SetEdgeSource(pcSpaceColour);

		sMapSize = pTileMapGenerator->GetMapSize();
		sCelSize = pTileMapGenerator->GetCelSize();

		asDoors.Init();
		pBrush = pTileMapGenerator->GetTileBrush(SCCT_Door, "DoorUp");
		pBrush->SetCallback(DoorCallback, &asDoors);
		pBrush = pTileMapGenerator->GetTileBrush(SCCT_Door, "DoorRight");
		pBrush->SetCallback(DoorCallback, &asDoors);

		pTileMap = pTileMapGenerator->Generate();

		AssertSize(4, asDoors.NumElements());
		psDoor = asDoors.Get(0);
		AssertSize(6, psDoor->sPosition.x);
		AssertSize(4, psDoor->sPosition.y);
		AssertString("DoorUp", psDoor->szDoorDirection);

		psDoor = asDoors.Get(1);
		AssertSize(4, psDoor->sPosition.x);
		AssertSize(6, psDoor->sPosition.y);
		AssertString("DoorRight", psDoor->szDoorDirection);

		psDoor = asDoors.Get(2);
		AssertSize(9, psDoor->sPosition.x);
		AssertSize(6, psDoor->sPosition.y);
		AssertString("DoorRight", psDoor->szDoorDirection);

		psDoor = asDoors.Get(3);
		AssertSize(6, psDoor->sPosition.x);
		AssertSize(9, psDoor->sPosition.y);
		AssertString("DoorUp", psDoor->szDoorDirection);

		asDoors.Kill();

		pDestImage = OMalloc<CImage>((uint32)(sMapSize.x * sCelSize.x), (uint32)(sMapSize.y * sCelSize.y), PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pDestImage->White();
		pBlitterCache = OMalloc<CImageCelBlitterCache>(pDestImage);
		
		pMaps = OMalloc<CMaps>(pBlitterCache, pDestImage);;
		pRoot->Add(pMaps);
		pMaps->AddMap(pTileMap);

		pMaps->SetViewportPosition(0, 0);

		bResult = pMaps->CreateCelBlitters();
		AssertTrue(bResult);

		bResult = pMaps->Blit(false);
		AssertTrue(bResult);

		aszArrayString.Kill();
		pRoot->Remove(pMaps);
		pRoot->Remove(pTileMapGenerator);
		pMaps = NULL;
		pTileMapGenerator = NULL;

		szOutputFilename.Init(szDirectory);
		cFileUtil.AppendToPath(&szOutputFilename, "TileMapGenerator11x11StringGenerate");
		szOutputFilename.Append(".png");
		bWritten = WriteImage(pDestImage, szOutputFilename.Text(), IT_PNG);
		AssertTrue(bWritten);

		szInputFilename.Init(szOutputFilename);
		szInputFilename.Replace("Output", "Input");

		AssertFile(szInputFilename, szOutputFilename);

		szOutputFilename.Kill();
		szInputFilename.Kill();
	}
	ObjectsFlush();
	ObjectsKill(false);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTileMapGenerator(void)
{
	BeginTests();

	DataIOInit();

	TestTileMapGenerator3x3ImageGenerate();
	TestTileMapGenerator40x40ImageGenerate();

	TestTileMapGenerator7x5StringGenerate();
	TestTileMapGenerator11x11StringGenerate();

	DataIOKill();

	TestStatistics();
}

