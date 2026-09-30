#include "BaseLib/Logger.h"
#include "BaseLib/StdRandom.h"
#include "ImageAccessorCreator.h"
#include "TileLayerCel.h"
#include "TileMapGenerator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileMapGenerator::Init(void)
{
	PreInit();
	macTileMapPatterns.Init();
	maTileCelBrushes.Init();
	meszPatternNames.Init();
	meszSourceNames.Init();
	macTileCelTypes.Init();
	maTileGridSources.Init();
	macTileColourSources.Init();
	maTileCelGenerators.Init();
	mpMap = OMalloc<CTileMap>();
	msCelSize.Init(0, 0);
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
	CTileColourSource*	pcSource;

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

	iNumElements = macTileColourSources.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcSource = macTileColourSources.Get(i);
		pcSource->Kill();
	}
	macTileColourSources.Kill();
	
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
	U_Data(CArrayTileColourSource, macTileColourSources);
	M_Embedded(maTileCelGenerators);
	M_Pointer(mpMap);
	U_Data(SSizeVec2, msCelSize);
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
Ptr<CTileMap> CTileMapGenerator::Generate(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapGenerator::AddTileGenerator(char* szTileGridSource, int iMapLayer, int iCelType, CTileColourSource* pcSource)
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
	Ptr<CTileGridSource>	pSource;
	char*					szSourceConstantName;

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

	pSource = OMalloc<CTileGridSource>(szSourceConstantName, pSourceImage);
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
bool CTileMapGenerator::AddCelType(char szPatternChar, int iCelType)
{
	CTileCelType*	pcTileCelType;

	pcTileCelType = GetCelType(szPatternChar);
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

	pcTileCelType = GetCelType(szPatternChar);
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
CTileCelType* CTileMapGenerator::GetCelType(char szPatternChar)
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
CTileColourSource* CTileMapGenerator::AddColourSource(uint8 iRed, uint8 iGreen, uint8 iBlue)
{
	CTileColourSource*	pcSource;

	pcSource = macTileColourSources.Add();
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
bool CTileMapGenerator::Generate(void)
{
	CRandom		cRandom;
	bool		bResult;

	cRandom.Init();
	bResult = Generate(&cRandom);
	cRandom.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapGenerator::Generate(int iSeed)
{
	CRandom		cRandom;
	bool		bResult;

	cRandom.Init(iSeed);
	bResult = Generate(&cRandom);
	cRandom.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapGenerator::Generate(CRandom* pcRandom)
{
	size					i;
	size					iNumElements;
	Ptr<CTileCelGenerator>	pGenerator;
	bool					bResult;

	bResult = ValidatePatterns();
	if (!bResult)
	{
		return false;
	}

	iNumElements = maTileCelGenerators.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pGenerator = maTileCelGenerators.Get(i);
		bResult = GenerateCels(&pGenerator, pcRandom);
		if (!bResult)
		{
			return false;
		}
	}
	return true;
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
		if (!pcPattern->FindBlock(&x, &y))
		{
			return gcLogger.Error2(__METHOD__, " Pattern [", pcPattern->GetConstantName(), "] has no 'B' block.", NULL);
		}

		iWidth = pcPattern->GetWidth();
		iHeight = pcPattern->GetHeight();
		for (y = 0; y < iHeight; y++)
		{
			for (x = 0; x < iWidth; x++)
			{
				c = pcPattern->GetChar(x, y);
				if ((c != 'B') && (c != '.') && (c != 'P') && (c != '!') && (GetCelType(c) == NULL))
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
bool CTileMapGenerator::GenerateCels(CTileCelGenerator* pcGenerator, CRandom* pcRandom)
{
	Ptr<CImage>			pImage;
	Ptr<CTileLayerCel>	pCelLayer;
	CImageAccessor*		pcAccessor;
	Ptr<CTileCelBrush>	pBrush;
	int					x;
	int					y;
	int					iWidth;
	int					iHeight;
	bool				bResult;

	pImage = pcGenerator->GetTileGridSource()->GetImage();
	pCelLayer = pcGenerator->GetTileLayer();
	if (pImage.IsNull() || pCelLayer.IsNull())
	{
		return false;
	}

	pcAccessor = CImageAccessorCreator::Create(&pImage, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	if (pcAccessor == NULL)
	{
		return gcLogger.Error2(__METHOD__, " Could not access the RGB channels of tile grid source [", pcGenerator->GetTileGridSource()->GetConstantName(), "].", NULL);
	}

	iWidth = pImage->GetWidth();
	iHeight = pImage->GetHeight();
	for (y = 0; y < iHeight; y++)
	{
		for (x = 0; x < iWidth; x++)
		{
			if (pcGenerator->IsColour(GetColour(pcAccessor, x, y)))
			{
				pBrush = ChooseBrush(pcGenerator, pcAccessor, x, y, pcRandom);
				if (pBrush.IsNotNull())
				{
					bResult = pCelLayer->SetTile(x, y, pBrush->GetCel());
					if (!bResult)
					{
						pcAccessor->Kill();
						return false;
					}
				}
			}
		}
	}

	pcAccessor->Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CTileCelBrush> CTileMapGenerator::ChooseBrush(CTileCelGenerator* pcGenerator, CImageAccessor* pcAccessor, int x, int y, CRandom* pcRandom)
{
	size				i;
	size				iNumElements;
	CTileMapPattern*	pcPattern;
	Ptr<CTileCelBrush>	pBrush;
	int					iCelType;
	char*				szSourceConstantName;

	//The first pattern (in the order added) that matches and has a brush is used.
	iCelType = pcGenerator->GetCelType();
	szSourceConstantName = pcGenerator->GetTileGridSource()->GetConstantName();
	iNumElements = macTileMapPatterns.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcPattern = macTileMapPatterns.Get(i);
		if ((pcPattern->GetType() == (size)iCelType) && pcPattern->IsSource(szSourceConstantName))
		{
			if (MatchPattern(pcPattern, pcGenerator, pcAccessor, x, y))
			{
				pBrush = ChooseBrush(iCelType, pcPattern->GetConstantName(), pcRandom);
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
Ptr<CTileCelBrush> CTileMapGenerator::ChooseBrush(int iCelType, char* szPatternConstantName, CRandom* pcRandom)
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

	iChoice = (size)pcRandom->Next(0, (int)(iTotalWeight - 1));
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
bool CTileMapGenerator::MatchPattern(CTileMapPattern* pcPattern, CTileCelGenerator* pcGenerator, CImageAccessor* pcAccessor, int x, int y)
{
	size			iBlockX;
	size			iBlockY;
	size			iWidth;
	size			iHeight;
	size			px;
	size			py;
	int				iX;
	int				iY;
	char			c;
	int				iCelType;
	char*			szSourceConstantName;

	if (!pcPattern->FindBlock(&iBlockX, &iBlockY))
	{
		return false;
	}

	iCelType = pcGenerator->GetCelType();
	szSourceConstantName = pcGenerator->GetTileGridSource()->GetConstantName();
	iWidth = pcPattern->GetWidth();
	iHeight = pcPattern->GetHeight();
	for (py = 0; py < iHeight; py++)
	{
		for (px = 0; px < iWidth; px++)
		{
			c = pcPattern->GetChar(px, py);
			iX = x + (int)px - (int)iBlockX;
			iY = y + (int)py - (int)iBlockY;

			if ((c == 'B') || (c == '.'))
			{
				continue;
			}
			else if (c == 'P')
			{
				if (!HasCelType(szSourceConstantName, pcAccessor, iX, iY, iCelType))
				{
					return false;
				}
			}
			else if (c == '!')
			{
				if (HasCelType(szSourceConstantName, pcAccessor, iX, iY, iCelType))
				{
					return false;
				}
			}
			else
			{
				if (!MatchCelType(GetCelType(c), szSourceConstantName, pcAccessor, iX, iY))
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
bool CTileMapGenerator::MatchCelType(CTileCelType* pcCelType, char* szSourceConstantName, CImageAccessor* pcAccessor, int x, int y)
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
		bHasCelType = HasCelType(szSourceConstantName, pcAccessor, x, y, pcCelType->GetCelType(i));
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
bool CTileMapGenerator::HasCelType(char* szSourceConstantName, CImageAccessor* pcAccessor, int x, int y, int iCelType)
{
	size					i;
	size					iNumElements;
	Ptr<CTileCelGenerator>	pGenerator;
	ARGB32					uiColour;

	//Cels outside the source have no cel type.
	if (!pcAccessor->IsValid(x, y))
	{
		return false;
	}

	//A cel has every cel type whose generator (on the same source) uses the cel's colour.
	uiColour = GetColour(pcAccessor, x, y);
	iNumElements = maTileCelGenerators.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pGenerator = maTileCelGenerators.Get(i);
		if ((pGenerator->GetCelType() == iCelType) &&
			(pGenerator->GetTileGridSource()->GetConstantName() == szSourceConstantName) &&
			pGenerator->IsColour(uiColour))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ARGB32 CTileMapGenerator::GetColour(CImageAccessor* pcAccessor, int x, int y)
{
	uint8*	puiPixel;

	puiPixel = (uint8*)pcAccessor->Get(x, y);
	return Set32BitColour(puiPixel[0], puiPixel[1], puiPixel[2]);
}

