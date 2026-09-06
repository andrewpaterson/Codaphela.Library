#ifndef __TILE_MAP_GENERATOR_H__
#define __TILE_MAP_GENERATOR_H__
#include "BaseLib/EnumeratorVoid.h"
#include "TileCelBrush.h"
#include "TileMapPattern.h"
#include "TileCelType.h"
#include "TileColourSource.h"
#include "TileGridSource.h"
#include "TileCelGenerator.h"


class CTileMapGenerator : public CObject
{
CONSTRUCTABLE(CTileMapGenerator);
DESTRUCTABLE(CTileMapGenerator);
protected:
	CArrayTileMapPattern		macTileMapPatterns;
	CArray<CTileCelBrush>		maTileCelBrushes;
	CEnumeratorVoid				meszPatternNames;
	CEnumeratorVoid				meszSourceNames;
	CArrayTileCelType			macTileCelTypes;
	CArray<CTileGridSource>		maTileGridSources;
	CArrayTileColourSource		macTileColourSources;
	CArray<CTileCelGenerator>	maTileCelGenerators;

public:
	void					Init(void);
	void					Free(void);
	void					Class(void);

	bool					Load(CObjectReader* pcFile);
	bool					Save(CObjectWriter* pcFile);

	bool					AddTileGenerator(char* szTileGridSource, int iMapLayer, int iCelType, CTileColourSource* pcSource);

	bool					AddPattern(char* szTileGridSource, int iCelType, char* szPatternName, char* szPatternChars);
	CTileMapPattern*		GetPattern(int iCelType, char* szPatternName);

	bool					AddTileGridSource(char* szSourceName, Ptr<CImage> pSourceImage);
	Ptr<CTileGridSource>	GetSource(char* szSourceName);

	bool					AddTileBrush(Ptr<CArrayImageCel> pCels, size iCelIndex, int iCelType, char* szPatternName, size iWeight = 1, int mapOffsetX = 0, int mapOffsetY = 0);

	bool					AddCelType(char szPatternChar, int iCelType);
	bool					AddCelType(char szPatternChar, int iCelType1, int iCelType2);
	bool					AddNegativeCelType(char szPatternChar, int iCelType);
	bool					AddNegativeCelType(char szPatternChar, int iCelType1, int iCelType2);
	CTileCelType*			GetCelType(char szPatternChar);

	CTileColourSource*		AddColourSource(uint8 iRed, uint8 iGreen, uint8 iBlue);

protected:
	char*					AddPatternConstantName(char* szPatternName);
	CTileMapPattern*		GetPatternConstantName(int iCelType, char* szPatternConstantName);

	char*					AddSourceConstantName(char* szSourceName);
	Ptr<CTileGridSource>	GetSourceConstantName(char* szSourceConstantName);
};


#endif // __TILE_MAP_GENERATOR_H__

