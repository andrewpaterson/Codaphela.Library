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
bool CTileMapGenerator::AddTileGenerator(char* szTileGridSource, int iMapLayer, int iCelType, CTileColourSource* pcSource)
{
	Ptr<CTileCelGenerator>	pCelGenerator;
	Ptr<CTileGridSource>	pTileGridSource;
	Ptr<CTileLayer>			pTileLayer;

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
	if (pSource.IsNull())
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
		return szPatternConstantName;
	}
	return NULL;
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
			Ptr<CTileCelBrush> pBrush = OMalloc<CTileCelBrush>(pCel, iCelType, szPatternName);
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
	SInt32Vec2			sOffset(0, 0);

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
	}
		mpMap->AddLayer(pTileLayer);

	return NULL;
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

