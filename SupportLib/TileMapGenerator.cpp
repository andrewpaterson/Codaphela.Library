#include "BaseLib/Logger.h"
#include "TileLayerCel.h"
#include "TileMapGenerator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileMapGenerator::Init(void)
{
	Init(NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileMapGenerator::Init(CRandom* pcRandom)
{
	PreInit();
	macTileMapPatterns.Init();
	maTileCelBrushes.Init();
	meszPatternNames.Init();
	meszSourceNames.Init();
	macTileCelTypes.Init();
	maTileGridSources.Init();
	mllcTileSources.Init();
	maTileCelGenerators.Init();
	mpMap = OMalloc<CTileMap>();
	msCelSize.Init(0, 0);
	if (pcRandom)
	{
		mpcRandom = pcRandom;
	}
	else
	{
		mcRandom.Init();
		mpcRandom = NULL;
	}
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileMapGenerator::Free(void)
{
	size				i;
	size				iNumElements;
	CTileMapPattern*	pcPattern;
	CTileCelType*		pcCelType;
	CTileCelSource*		pcSource;

	if (mpcRandom)
	{
		mpcRandom = NULL;
	}
	else
	{
		mcRandom.Kill();
	}

	iNumElements = macTileMapPatterns.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcPattern = macTileMapPatterns.Get(i);
		pcPattern->Kill();
	}
	macTileMapPatterns.Kill();

	iNumElements = macTileCelTypes.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcCelType = macTileCelTypes.Get(i);
		pcCelType->Kill();
	}
	macTileCelTypes.Kill();

	pcSource = (CTileCelSource*)mllcTileSources.GetHead();
	while (pcSource)
	{
		pcSource->Kill();
		pcSource = (CTileCelSource*)mllcTileSources.GetNext(pcSource);
	}
	mllcTileSources.Kill();
	
	meszPatternNames.Kill();
	meszSourceNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileMapGenerator::Class(void)
{
	U_Data(CArrayTileMapPattern, macTileMapPatterns);
	M_Embedded(maTileCelBrushes);
	U_Data(CEnumeratorVoid, meszPatternNames);
	U_Data(CEnumeratorVoid, meszSourceNames);
	U_Data(CArrayTileCelType, macTileCelTypes);
	M_Embedded(maTileGridSources);
	U_Data(CLinkedListBlock, mllcTileSources);
	M_Embedded(maTileCelGenerators);
	M_Pointer(mpMap);
	U_Data(SSizeVec2, msCelSize);
	U_Pointer(mpcEdgeSource);
	U_Data(CRandom, mcRandom);
	U_Pointer(mpcRandom);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapGenerator::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapGenerator::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapGenerator::AddTileGenerator(char* szTileGridSource, int iMapLayer, int iCelType, CTileCelSource* pcSource)
{
	Ptr<CTileCelGenerator>	pCelGenerator;
	Ptr<CTileGridSource>	pTileGridSource;
	Ptr<CTileLayer>			pTileLayer;

	if (pcSource == NULL)
	{
		return false;
	}

	pTileGridSource = GetSource(szTileGridSource);
	if (pTileGridSource.IsNull())
	{
		return false;
	}

	pTileLayer = GetTileLayer(iMapLayer);
	if (pTileLayer.IsNull())
	{
		pTileLayer = AddTileLayer(mpMap, "Graphics", iMapLayer);
		if (pTileLayer.IsNull())
		{
			return false;
		}
	}

	pCelGenerator = OMalloc<CTileCelGenerator>(pTileGridSource, pTileLayer, iCelType, pcSource);
	if (pCelGenerator.IsNull())
	{
		return false;
	}

	maTileCelGenerators.Add(pCelGenerator);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapGenerator::AddTileGridSource(char* szSourceName, Ptr<CImage> pSourceImage)
{
	Ptr<CTileGridImageSource>	pSource;
	char*						szSourceConstantName;

	szSourceConstantName = AddSourceConstantName(szSourceName);
	if (!szSourceConstantName)
	{
		return false;
	}

	pSource = GetSourceConstantName(szSourceConstantName);
	if (pSource.IsNotNull())
	{
		return false;
	}

	pSource = OMalloc<CTileGridImageSource>(szSourceConstantName, pSourceImage);
	if (pSource.IsNotNull())
	{
		maTileGridSources.Add(pSource);
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapGenerator::AddTileGridSource(char* szSourceName, CArrayChars* pszSourceString)
{
	Ptr<CTileGridStringSource>	pSource;
	char*						szSourceConstantName;

	szSourceConstantName = AddSourceConstantName(szSourceName);
	if (!szSourceConstantName)
	{
		return false;
	}

	pSource = GetSourceConstantName(szSourceConstantName);
	if (pSource.IsNotNull())
	{
		return false;
	}

	pSource = OMalloc<CTileGridStringSource>(szSourceConstantName, pszSourceString);
	if (pSource.IsNotNull())
	{
		maTileGridSources.Add(pSource);
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CTileMapGenerator::AddPatternConstantName(char* szPatternName)
{
	char* szPatternConstantName;

	szPatternConstantName = meszPatternNames.GetName(szPatternName);
	if (!szPatternConstantName)
	{
		meszPatternNames.Add(szPatternName);
		szPatternConstantName = meszPatternNames.GetName(szPatternName);
	}
	return szPatternConstantName;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CTileMapGenerator::AddSourceConstantName(char* szSourceName)
{
	char* szSourceConstantName;

	szSourceConstantName = meszSourceNames.GetName(szSourceName);
	if (!szSourceConstantName)
	{
		meszSourceNames.Add(szSourceName);
		szSourceConstantName = meszSourceNames.GetName(szSourceName);
		return szSourceConstantName;
	}
	return NULL;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapGenerator::AddPattern(char* szTileGridSource, int iCelType, char* szPatternName, char* szPatternChars)
{
	CTileMapPattern*		pcPattern;
	bool					bResult;
	char*					szPatternConstantName;
	Ptr<CTileGridSource>	pTileGridSource;

	szPatternConstantName = AddPatternConstantName(szPatternName);
	if (!szPatternConstantName)
	{
		return false;
	}

	pcPattern = GetPatternConstantName(iCelType, szPatternConstantName);
	if (pcPattern)
	{
		return false;
	}

	pTileGridSource = GetSource(szTileGridSource);
	if (pTileGridSource.IsNull())
	{
		return false;
	}

	pcPattern = macTileMapPatterns.Add();
	bResult = pcPattern->Init(pTileGridSource->GetConstantName(), iCelType, szPatternConstantName, szPatternChars);
	
	if (bResult)
	{
		return true;
	}
	else
	{
		macTileMapPatterns.Remove(pcPattern);
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapGenerator::AddTileBrush(Ptr<CArrayImageCel> pCels, size iCelIndex, int iCelType, char* szPatternName, size iWeight, int mapOffsetX, int mapOffsetY)
{
	CTileMapPattern*	pcPattern;
	CSubImage*			pcSubImage;
	size				iWidth;
	size				iHeight;

	pcPattern = GetPattern(iCelType, szPatternName);
	if (pcPattern)
	{
		Ptr<CImageCel> pCel = pCels->Get(iCelIndex);
		if (pCel.IsNotNull())
		{
			Ptr<CTileCelBrush> pBrush = OMalloc<CTileCelBrush>(pCel, (size)iCelType, pcPattern->GetConstantName(), iWeight);
			if (pBrush.IsNotNull())
			{
				pcSubImage = pCel->GetSubImage();
				iWidth = (size)pcSubImage->GetFullWidth();
				iHeight = (size)pcSubImage->GetFullHeight();
				msCelSize.Maximise(iWidth, iHeight);

				maTileCelBrushes.Add(pBrush);
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTileMapPattern* CTileMapGenerator::GetPatternConstantName(int iCelType, char* szPatternConstantName)
{
	size				i;
	size				iNumElements;
	CTileMapPattern*	pcPattern;

	iNumElements = macTileMapPatterns.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcPattern = macTileMapPatterns.Get(i);
		if (pcPattern->IsNamed(iCelType, szPatternConstantName))
		{
			return pcPattern;
		}
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CTileGridSource> CTileMapGenerator::GetSourceConstantName(char* szSourceConstantName)
{
	size					i;
	size					iNumElements;
	Ptr<CTileGridSource>	pSource;

	iNumElements = maTileGridSources.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pSource = maTileGridSources.Get(i);
		if (pSource->IsNamed(szSourceConstantName))
		{
			return pSource;
		}
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTileMapPattern* CTileMapGenerator::GetPattern(int iCelType, char* szPatternName)
{
	char*	szPatternConstantName;

	if (StrEmpty(szPatternName))
	{
		return NULL;
	}

	szPatternConstantName = meszPatternNames.GetName(szPatternName);
	return GetPatternConstantName(iCelType, szPatternConstantName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CTileGridSource> CTileMapGenerator::GetSource(char* szTileGridSource)
{
	char*	szSourceConstantName;

	if (StrEmpty(szTileGridSource))
	{
		return NULL;
	}

	szSourceConstantName = meszSourceNames.GetName(szTileGridSource);
	return GetSourceConstantName(szSourceConstantName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileMapGenerator::SetEdgeSource(CTileCelSource* pcSource)
{
	mpcEdgeSource = pcSource;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapGenerator::AddCelType(char szPatternChar, int iCelType)
{
	CTileCelType*	pcTileCelType;

	pcTileCelType = GetPatternCelType(szPatternChar);
	if (pcTileCelType == NULL)
	{
		pcTileCelType = macTileCelTypes.Add();
		if (pcTileCelType)
		{
			pcTileCelType->Init(szPatternChar, iCelType);
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		if (!pcTileCelType->IsNegative())
		{
			pcTileCelType->AddCelType(iCelType);
			return true;
		}
		else
		{
			return false;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapGenerator::AddCelType(char szPatternChar, int iCelType1, int iCelType2)
{
	bool bResult;

	bResult = AddCelType(szPatternChar, iCelType1);
	bResult &= AddCelType(szPatternChar, iCelType2);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapGenerator::AddNegativeCelType(char szPatternChar, int iCelType)
{
	CTileCelType* pcTileCelType;

	pcTileCelType = GetPatternCelType(szPatternChar);
	if (pcTileCelType == NULL)
	{
		pcTileCelType = macTileCelTypes.Add();
		if (pcTileCelType)
		{
			pcTileCelType->Init(szPatternChar, true);
			pcTileCelType->AddCelType(iCelType);
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		if (pcTileCelType->IsNegative())
		{
			pcTileCelType->AddCelType(iCelType);
			return true;
		}
		else
		{
			return false;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapGenerator::AddNegativeCelType(char szPatternChar, int iCelType1, int iCelType2)
{
	bool bResult;

	bResult = AddNegativeCelType(szPatternChar, iCelType1);
	bResult &= AddNegativeCelType(szPatternChar, iCelType2);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTileCelType* CTileMapGenerator::GetPatternCelType(char szPatternChar)
{
	size				i;
	size				iNumElements;
	CTileCelType*		pcTileCelType;

	iNumElements = macTileCelTypes.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcTileCelType = macTileCelTypes.Get(i);
		if (pcTileCelType->IsPattern(szPatternChar))
		{
			return pcTileCelType;
		}
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTileCelColourSource* CTileMapGenerator::AddColourSource(uint8 iRed, uint8 iGreen, uint8 iBlue)
{
	CTileCelColourSource*	pcSource;

	pcSource = (CTileCelColourSource*)mllcTileSources.InsertAfterTail(sizeof(CTileCelColourSource));
	New(pcSource);
	if (pcSource)
	{
		pcSource->Init(iRed, iGreen, iBlue);
	}

	return pcSource;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTileCelCharSource* CTileMapGenerator::AddCharSource(char c)
{
	CTileCelCharSource*		pcSource;

	pcSource = (CTileCelCharSource*)mllcTileSources.InsertAfterTail(sizeof(CTileCelCharSource));
	New(pcSource);
	if (pcSource)
	{
		pcSource->Init(c);
	}

	return pcSource;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CTileLayer> CTileMapGenerator::GetTileLayer(int iIdentifier)
{
	return mpMap->GetTileLayer(iIdentifier);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CTileLayer> CTileMapGenerator::AddTileLayer(CPointer pTileMap, const char* szTileType, int iIdentifier)
{
	Ptr<CTileLayer>		pTileLayer;
	SSizeVec2			sMapSize;
	SSizeVec2			sCelSize;
	SIntVec2			sOffset(0, 0);

	pTileLayer = mpMap->GetTileLayer(iIdentifier);
	if (pTileLayer.IsNotNull())
	{
		return NULL;
	}

	sMapSize = GetMapSize();
	sCelSize = GetCelSize();


	pTileLayer = OMalloc<CTileLayerCel>(pTileMap, szTileType, sMapSize, sCelSize, iIdentifier, sOffset);
	if (pTileLayer.IsNotNull())
	{
		mpMap->AddLayer(pTileLayer);
	}

	return pTileLayer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SSizeVec2 CTileMapGenerator::GetMapSize(void)
{
	size					i;
	size					iNumElements;
	Ptr<CTileGridSource>	pSource;
	SSizeVec2				sSize;
	SSizeVec2				sFullSize(0, 0);

	iNumElements = maTileGridSources.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pSource = maTileGridSources.Get(i);
		sSize = pSource->GetSize();
		sFullSize.Maximise(sSize);
	}

	return sFullSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SSizeVec2 CTileMapGenerator::GetCelSize(void)
{
	return msCelSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CTileMap> CTileMapGenerator::GetMap(void)
{
	return mpMap;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CTileCelBrush> CTileMapGenerator::GetTileBrush(int iCelType, char* szPatternName)
{
	size				iNumElements;
	size				i;
	Ptr<CTileCelBrush>	pBrush;
	char*				szConstantPatternName;

	szConstantPatternName = meszPatternNames.GetName(szPatternName);

	iNumElements = maTileCelBrushes.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pBrush = maTileCelBrushes.Get(i);
		if (pBrush->IsFor(iCelType, szConstantPatternName))
		{
			return pBrush;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CTileMap> CTileMapGenerator::Generate(void)
{
	size					i;
	size					iNumElements;
	Ptr<CTileCelGenerator>	pGenerator;
	Ptr<CTileGridSource>	pSource;
	bool					bResult;

	bResult = ValidatePatterns();
	if (!bResult)
	{
		return NULL;
	}

	iNumElements = maTileGridSources.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pSource = maTileGridSources.Get(i);
		bResult = pSource->StartGeneration();
		if (!bResult)
		{
			return false;
		}

		bResult = GenerateCels(&pSource);
		if (!bResult)
		{
			pSource->StopGeneration();
			return NULL;
		}
		pSource->StopGeneration();
	}

	return mpMap;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapGenerator::ValidatePatterns(void)
{
	size				i;
	size				iNumElements;
	CTileMapPattern*	pcPattern;
	size				x;
	size				y;
	size				iWidth;
	size				iHeight;
	char				c;
	char				szChar[2];

	iNumElements = macTileMapPatterns.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcPattern = macTileMapPatterns.Get(i);

		iWidth = pcPattern->GetWidth();
		iHeight = pcPattern->GetHeight();
		for (y = 0; y < iHeight; y++)
		{
			for (x = 0; x < iWidth; x++)
			{
				c = pcPattern->GetChar(x, y);

				if ((c != '.') && (GetPatternCelType(c) == NULL))
				{
					szChar[0] = c;
					szChar[1] = '\0';
					return gcLogger.Error2(__METHOD__, " Pattern [", pcPattern->GetConstantName(), "] uses undefined cel type [", szChar, "].", NULL);
				}
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapGenerator::GenerateCels(Ptr<CTileGridSource> pSource)
{
	int						x;
	int						y;
	int						iWidth;
	int						iHeight;
	bool					bResult;
	SSizeVec2				sSize;
	char*					szSourceConstantName;

	sSize = pSource->GetSize();
	iWidth = sSize.x;
	iHeight = sSize.y;
	szSourceConstantName = pSource->GetConstantName();

	//Every generator for this source that matches a cel writes a tile to its own layer.

	for (y = 0; y < iHeight; y++)
	{
		for (x = 0; x < iWidth; x++)
		{
			bResult = GenerateCels(szSourceConstantName, x, y);
			if (!bResult)
			{
				return false;
			}
		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapGenerator::GenerateCels(char* szConstantSouceName, size x, size y)
{
	Ptr<CTileLayerCel>		pCelLayer;
	Ptr<CTileCelBrush>		pBrush;
	Ptr<CTileCelGenerator>	pGenerator;
	size					iNumGenerators;
	size					i;
	bool					bMatches;
	bool					bResult;
	SSizeVec2				sSize;

	//Every generator for this source that matches a cel writes a tile to its own layer.
	iNumGenerators = maTileCelGenerators.NumElements();

	for (i = 0; i < iNumGenerators; i++)
	{
		pGenerator = maTileCelGenerators.Get(i);
		if (!pGenerator->GetTileGridSource()->IsNamed(szConstantSouceName))
		{
			continue;
		}

		bMatches = pGenerator->Matches(x, y);
		if (bMatches)
		{
			pBrush = CalculateBrush(pGenerator, x, y);
			if (pBrush.IsNotNull())
			{
				pCelLayer = pGenerator->GetTileLayer();

				bResult = pCelLayer->SetTile(x, y, pBrush->GetCel());
				if (!bResult)
				{
					return false;
				}

				pBrush->Callback(x, y);
			}
		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CTileCelBrush> CTileMapGenerator::CalculateBrush(Ptr<CTileCelGenerator> pGenerator, int x, int y)
{
	size				i;
	size				iNumElements;
	CTileMapPattern*	pcPattern;
	Ptr<CTileCelBrush>	pBrush;
	int					iCelType;
	char*				szSourceConstantName;

	//The first pattern (in the order added) that matches and has a brush is used.
	iCelType = pGenerator->GetCelType();
	szSourceConstantName = pGenerator->GetTileGridSource()->GetConstantName();
	iNumElements = macTileMapPatterns.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcPattern = macTileMapPatterns.Get(i);
		if ((pcPattern->GetType() == (size)iCelType) && pcPattern->IsSource(szSourceConstantName))
		{
			if (MatchPattern(pcPattern, pGenerator, x, y))
			{
				pBrush = ChooseBrush(iCelType, pcPattern->GetConstantName());
				if (pBrush.IsNotNull())
				{
					return pBrush;
				}
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CTileCelBrush> CTileMapGenerator::ChooseBrush(int iCelType, char* szPatternConstantName)
{
	size				i;
	size				iNumElements;
	Ptr<CTileCelBrush>	pBrush;
	size				iTotalWeight;
	size				iChoice;

	iTotalWeight = 0;
	iNumElements = maTileCelBrushes.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pBrush = maTileCelBrushes.Get(i);
		if (pBrush->IsFor((size)iCelType, szPatternConstantName))
		{
			iTotalWeight += pBrush->GetWeight();
		}
	}

	if (iTotalWeight == 0)
	{
		return NULL;
	}

	iChoice = (size)GetRandom()->Next(0, iTotalWeight - 1);
	for (i = 0; i < iNumElements; i++)
	{
		pBrush = maTileCelBrushes.Get(i);
		if (pBrush->IsFor((size)iCelType, szPatternConstantName))
		{
			if (iChoice < pBrush->GetWeight())
			{
				return pBrush;
			}
			iChoice -= pBrush->GetWeight();
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapGenerator::MatchPattern(CTileMapPattern* pcPattern, Ptr<CTileCelGenerator> pGenerator, int x, int y)
{
	Ptr<CTileGridSource>	pTileGridSource;
	Ptr<CTileLayer>			pTileLayer;
	size					iBlockX;
	size					iBlockY;
	size					iWidth;
	size					iHeight;
	size					px;
	size					py;
	int						iX;
	int						iY;
	char					cPatternCharacter;
	int						iGeneratorCelType;
	int						iMapLayer;
	char*					szSourceConstantName;
	CTileCelType*			pcCelType;
	SSizeVec2				sSourceSize;
	bool					bEdge;

	iBlockX = pcPattern->GetBX();
	iBlockY = pcPattern->GetBY();

	iGeneratorCelType = pGenerator->GetCelType();
	pTileGridSource = pGenerator->GetTileGridSource();
	pTileLayer = pGenerator->GetTileLayer();
	szSourceConstantName = pTileGridSource->GetConstantName();
	iMapLayer = pTileLayer->GetIdentifier();
	sSourceSize = pTileGridSource->GetSize();

	iWidth = pcPattern->GetWidth();
	iHeight = pcPattern->GetHeight();
	for (py = 0; py < iHeight; py++)
	{
		for (px = 0; px < iWidth; px++)
		{
			cPatternCharacter = pcPattern->GetChar(px, py);
			iX = x + (int)px - (int)iBlockX;
			iY = y + (int)py - (int)iBlockY;

			if (cPatternCharacter == '.')
			{
				continue;
			}
			else
			{
				bEdge = ((iX < 0) || (iY < 0) ||
					(iX >= (int)sSourceSize.x) || (iY >= (int)sSourceSize.y));

				pcCelType = GetPatternCelType(cPatternCharacter);
				if (!MatchCelType(pcCelType, szSourceConstantName, iX, iY, bEdge))
				{
					return false;
				}
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapGenerator::MatchCelType(CTileCelType* pcCelType, char* szSourceConstantName, int x, int y, bool bEdge)
{
	size	i;
	size	iNumElements;
	bool	bHasCelType;

	if (pcCelType == NULL)
	{
		return false;
	}

	//Positive: matches if the cel has any of the cel types.  Negative: matches if the cel has none of them.
	iNumElements = pcCelType->NumCelTypes();
	for (i = 0; i < iNumElements; i++)
	{
		bHasCelType = MatchCelType(szSourceConstantName, pcCelType->GetCelType(i), x, y, bEdge);
		if (bHasCelType)
		{
			return !pcCelType->IsNegative();
		}
	}
	return pcCelType->IsNegative();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapGenerator::MatchCelType(char* szSourceConstantName, int iCelType, int x, int y, bool bEdge)
{
	size					i;
	size					iNumElements;
	Ptr<CTileCelGenerator>	pGenerator;

	//A cel has a cel type if any generator of that cel type for the same source matches it.
	iNumElements = maTileCelGenerators.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pGenerator = maTileCelGenerators.Get(i);
		if ((pGenerator->GetCelType() == iCelType) && pGenerator->GetTileGridSource()->IsNamed(szSourceConstantName))
		{
			if (!bEdge)
			{
				if (pGenerator->Matches(x, y))
				{
					return true;
				}
			}
			else
			{
				if (pGenerator->Matches(mpcEdgeSource))
				{
					return true;
				}
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CRandom* CTileMapGenerator::GetRandom(void)
{
	if (mpcRandom)
	{
		return mpcRandom;
	}
	else
	{
		return &mcRandom;
	}
}

