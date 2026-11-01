#ifndef __SPACE_CRUSADE_H__
#define __SPACE_CRUSADE_H__
#include "StandardLib/Pointer.h"
#include "SupportLib/ImageReader.h"
#include "SupportLib/ImageCel.h"
#include "SupportLib/ImageDivider.h"
#include "SupportLib/ColourFormat.h"
#include "SupportLib/Maps.h"
#include "SupportLib/TileLayerCel.h"
#include "SupportLib/TileMapGenerator.h"


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


void					AddSpacePatterns(Ptr<CTileMapGenerator> pTileMapGenerator, Ptr<CArrayImageCel> paBackgroundCels);
void					AddSpaceSources(Ptr<CTileMapGenerator> pTileMapGenerator);
Ptr<CImage>				ReadSpaceImage(char* szDirectory, char* szFilename);
Ptr<CArrayImageCel>		ReadSpaceCels(char* szDirectory, char* szFilename, int iColumnCount, int iRowCount, CColourFormatHelper* pcFormat);


#endif // __SPACE_CRUSADE_H__

