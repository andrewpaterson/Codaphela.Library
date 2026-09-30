#ifndef __TILE_MAP_GENERATOR_H__
#define __TILE_MAP_GENERATOR_H__
#include "BaseLib/EnumeratorVoid.h"
#include "TileCelBrush.h"
#include "TileMapPattern.h"
#include "TileCelType.h"
#include "TileColourSource.h"
#include "TileGridSource.h"
#include "TileCelGenerator.h"
#include "TileMap.h"


class CRandom;
class CImageAccessor;
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
	Ptr<CTileMap>				mpMap;
	SSizeVec2					msCelSize;

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

	SSizeVec2				GetMapSize(void);
	SSizeVec2				GetCelSize(void);

	bool					Generate(void);
	bool					Generate(int iSeed);
	Ptr<CTileMap>			GetMap(void);

protected:
	char*					AddPatternConstantName(char* szPatternName);
	CTileMapPattern*		GetPatternConstantName(int iCelType, char* szPatternConstantName);

	char*					AddSourceConstantName(char* szSourceName);
	Ptr<CTileGridSource>	GetSourceConstantName(char* szSourceConstantName);

	Ptr<CTileLayer>			GetTileLayer(int iIdentifier);
	Ptr<CTileLayer>			AddTileLayer(CPointer pTileMap, const char* szTileType, int iIdentifier);

	bool					Generate(CRandom* pcRandom);
	bool					ValidatePatterns(void);
	bool					GenerateCels(CTileCelGenerator* pcGenerator, CRandom* pcRandom);
	Ptr<CTileCelBrush>		ChooseBrush(CTileCelGenerator* pcGenerator, CImageAccessor* pcAccessor, int x, int y, CRandom* pcRandom);
	Ptr<CTileCelBrush>		ChooseBrush(int iCelType, char* szPatternConstantName, CRandom* pcRandom);
	bool					MatchPattern(CTileMapPattern* pcPattern, CTileCelGenerator* pcGenerator, CImageAccessor* pcAccessor, int x, int y);
	bool					MatchCelType(CTileCelType* pcCelType, char* szSourceConstantName, CImageAccessor* pcAccessor, int x, int y);
	bool					HasCelType(char* szSourceConstantName, CImageAccessor* pcAccessor, int x, int y, int iCelType);
	ARGB32					GetColour(CImageAccessor* pcAccessor, int x, int y);
};


#endif // __TILE_MAP_GENERATOR_H__

