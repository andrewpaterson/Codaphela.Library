#ifndef __TILE_MAP_GENERATOR_H__
#define __TILE_MAP_GENERATOR_H__
#include "BaseLib/StdRandom.h"
#include "BaseLib/EnumeratorVoid.h"
#include "TileCelBrush.h"
#include "TileMapPattern.h"
#include "TileCelType.h"
#include "TileColourSource.h"
#include "TileGridSource.h"
#include "TileGridImageSource.h"
#include "TileGridStringSource.h"
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
	CLinkedListTileColourSource	mllcTileColourSources;	//Linked list so the CTileColourSource pointers held by generators are not moved.
	CArray<CTileCelGenerator>	maTileCelGenerators;
	Ptr<CTileMap>				mpMap;
	SSizeVec2					msCelSize;

	CRandom						mcRandom;
	CRandom*					mpcRandom;

public:
	void					Init(void);
	void					Init(CRandom* pcRandom);
	void					Free(void);
	void					Class(void);

	bool					Load(CObjectReader* pcFile);
	bool					Save(CObjectWriter* pcFile);

	bool					AddTileGenerator(char* szTileGridSource, int iMapLayer, int iCelType, CTileColourSource* pcSource);

	bool					AddPattern(char* szTileGridSource, int iCelType, char* szPatternName, char* szPatternChars);
	CTileMapPattern*		GetPattern(int iCelType, char* szPatternName);

	bool					AddTileGridSource(char* szSourceName, Ptr<CImage> pSourceImage);
	bool					AddTileGridSource(char* szSourceName, CArrayChars pszSourceString);
	Ptr<CTileGridSource>	GetSource(char* szSourceName);

	bool					AddTileBrush(Ptr<CArrayImageCel> pCels, size iCelIndex, int iCelType, char* szPatternName, size iWeight = 1, int mapOffsetX = 0, int mapOffsetY = 0);

	bool					AddCelType(char szPatternChar, int iCelType);
	bool					AddCelType(char szPatternChar, int iCelType1, int iCelType2);
	bool					AddNegativeCelType(char szPatternChar, int iCelType);
	bool					AddNegativeCelType(char szPatternChar, int iCelType1, int iCelType2);

	CTileCelType*			GetPatternCelType(char szPatternChar);

	CTileColourSource*		AddColourSource(uint8 iRed, uint8 iGreen, uint8 iBlue);

	SSizeVec2				GetMapSize(void);
	SSizeVec2				GetCelSize(void);

	Ptr<CTileMap>			Generate(void);
	Ptr<CTileMap>			GetMap(void);

protected:
	bool					GenerateCels(Ptr<CTileGridSource> pSource);
	bool					GenerateCels(char* szConstantSouceName, size x, size y);

	char*					AddPatternConstantName(char* szPatternName);
	CTileMapPattern*		GetPatternConstantName(int iCelType, char* szPatternConstantName);

	char*					AddSourceConstantName(char* szSourceName);
	Ptr<CTileGridSource>	GetSourceConstantName(char* szSourceConstantName);

	Ptr<CTileLayer>			GetTileLayer(int iIdentifier);
	Ptr<CTileLayer>			AddTileLayer(CPointer pTileMap, const char* szTileType, int iIdentifier);

	CRandom*				GetRandom(void);

	bool					ValidatePatterns(void);
	Ptr<CTileCelBrush>		CalculateBrush(Ptr<CTileCelGenerator> pGenerator, int x, int y);
	Ptr<CTileCelBrush>		ChooseBrush(int iCelType, char* szPatternConstantName);
	bool					MatchPattern(CTileMapPattern* pcPattern, Ptr<CTileCelGenerator> pGenerator, int x, int y);
	bool					MatchCelType(CTileCelType* pcCelType, char* szSourceConstantName, int x, int y);
	bool					HasCelType(char* szSourceConstantName, int iCelType, int x, int y);
};


#endif // __TILE_MAP_GENERATOR_H__

