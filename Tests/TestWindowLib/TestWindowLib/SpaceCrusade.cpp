#include "SupportLib/ImageColourConverter.h"
#include "TestLib/Assert.h"
#include "SpaceCrusade.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AddSpacePatterns(Ptr<CTileMapGenerator> pTileMapGenerator, Ptr<CArrayImageCel> paBackgroundCels)
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

	pTileMapGenerator->AddTileBrush(paBackgroundCels, 23, SCCT_Shadow, "ShadowSingle");
	pTileMapGenerator->AddTileBrush(paBackgroundCels, 24, SCCT_Shadow, "ShadowUpLeft");
	pTileMapGenerator->AddTileBrush(paBackgroundCels, 25, SCCT_Shadow, "ShadowUp");
	pTileMapGenerator->AddTileBrush(paBackgroundCels, 27, SCCT_Shadow, "ShadowDown");
	pTileMapGenerator->AddTileBrush(paBackgroundCels, 28, SCCT_Shadow, "ShadowDiag");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AddSpaceSources(Ptr<CTileMapGenerator> pTileMapGenerator)
{
	CTileCelColourSource*		pcWallColour;
	CTileCelColourSource*		pcFloorColour;
	CTileCelColourSource*		pcSpaceColour;
	CTileCelColourSource*		pcDoorColour;

	pcWallColour = pTileMapGenerator->AddColourSource(138, 138, 138);
	pcFloorColour = pTileMapGenerator->AddColourSource(107, 73, 14);
	pcSpaceColour = pTileMapGenerator->AddColourSource(0, 0, 0);
	pcDoorColour = pTileMapGenerator->AddColourSource(71, 71, 71);

	pTileMapGenerator->AddTileGenerator("FloorPlan", SCML_Floor, SCCT_Space, pcSpaceColour);
	pTileMapGenerator->AddTileGenerator("FloorPlan", SCML_Floor, SCCT_Wall, pcWallColour);
	pTileMapGenerator->AddTileGenerator("FloorPlan", SCML_Floor, SCCT_Door, pcDoorColour);
	pTileMapGenerator->AddTileGenerator("FloorPlan", SCML_Floor, SCCT_Floor, pcFloorColour);
	pTileMapGenerator->AddTileGenerator("FloorPlan", SCML_Shadow, SCCT_Shadow, pcFloorColour);
	pTileMapGenerator->SetEdgeSource(pcSpaceColour);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> ReadSpaceImage(char* szDirectory, char* szFilename)
{
	CFileUtil			cFileUtil;
	CChars				szInputFilename;
	Ptr<CImage>			pImage;

	szInputFilename.Init();
	cFileUtil.CurrentDirectory(&szInputFilename);
	cFileUtil.AppendToPath(&szInputFilename, "Input");
	if (!StrEmpty(szDirectory))
	{
		cFileUtil.AppendToPath(&szInputFilename, szDirectory);
	}
	cFileUtil.AppendToPath(&szInputFilename, szFilename);
	AssertTrue(cFileUtil.Exists(szInputFilename.Text()));

	pImage = ReadImage(szInputFilename.Text(), IT_Unknown, true);
	AssertTrue(pImage.IsNotNull());
	szInputFilename.Kill();

	return pImage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CArrayImageCel> ReadSpaceCels(char* szDirectory, char* szFilename, int iColumnCount, int iRowCount, CColourFormatHelper* pcFormat)
{
	Ptr<CImage>				pImage;
	CImageDivider			cImageDivider;
	CImageDividerNumbers	cNumbers;
	Ptr<CArrayImageCel>		pCels;

	pImage = ReadSpaceImage(szDirectory, szFilename);
	pImage = CImageColourConverter::ConvertColour(pImage, pcFormat->meOrder);

	cNumbers.InitGeneral(-1, -1, iColumnCount, iRowCount, 0, 0, 0, 0);
	cImageDivider.Init(pImage, NULL);
	cImageDivider.GenerateFromNumbers(&cNumbers);
	pCels = cImageDivider.GetDestImageCels();

	AssertTrue(pCels.IsNotNull());
	AssertSize((size)(iColumnCount * iRowCount), pCels->NumElements());

	AssertSize(2, pCels.NumStackFroms());
	AssertSize(0, pCels.NumHeapFroms());

	cImageDivider.Kill();

	AssertSize(1, pCels.NumStackFroms());
	AssertSize(0, pCels.NumHeapFroms());

	return pCels;
}

