#include "ImageRowBlitterCacheValue.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterCacheValue::Init(Ptr<CImage> pSourceImage)
{
	PreInit();

	mpSourceImage = pSourceImage;
	maDestBlitters.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterCacheValue::Class(void)
{
	M_Pointer(mpSourceImage);
	M_Embedded(maDestBlitters);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterCacheValue::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageRowBlitterCacheValue::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageRowBlitterCacheValue::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CBaseImageRowBlitter> CImageRowBlitterCacheValue::GetCacheDest(Ptr<CImage> pDest, const char* szBlitterClass)
{
	size							ui;
	size							uiNumElements;
	Ptr<CBaseImageRowBlitter>	pDestValue;

	uiNumElements = maDestBlitters.NumElements();
	for (ui = 0; ui < uiNumElements; ui++)
	{
		pDestValue = maDestBlitters.Get(ui);
		if (pDestValue->Is(pDest, szBlitterClass))
		{
			return pDestValue;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CBaseImageRowBlitter> CImageRowBlitterCacheValue::GetCacheDest(Ptr<CBaseImageRowBlitter> pcBlitter)
{
	size						ui;
	size						uiNumElements;
	Ptr<CBaseImageRowBlitter>	pDestValue;

	uiNumElements = maDestBlitters.NumElements();
	for (ui = 0; ui < uiNumElements; ui++)
	{
		pDestValue = maDestBlitters.Get(ui);
		if (pDestValue.IsEqual(pcBlitter))
		{
			return pDestValue;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CBaseImageRowBlitter> CImageRowBlitterCacheValue::Get(Ptr<CImage> pDest, const char* szBlitterClass)
{
	Ptr<CBaseImageRowBlitter>	pDestValue;

	pDestValue = GetCacheDest(pDest, szBlitterClass);
	if (pDestValue.IsNotNull())
	{
		pDestValue->AddUsage();
		return pDestValue;

	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageRowBlitterCacheValue::Add(Ptr<CImage> pDest, Ptr<CBaseImageRowBlitter> pcRowBlitter)
{
	size						ui;
	size						uiNumElements;
	Ptr<CBaseImageRowBlitter>	pDestValue;
	const char*					szBlitterClass;

	szBlitterClass = pcRowBlitter->ClassName();

	uiNumElements = maDestBlitters.NumElements();
	for (ui = 0; ui < uiNumElements; ui++)
	{
		pDestValue = maDestBlitters.Get(ui);
		if (pDestValue->Is(pDest, szBlitterClass))
		{
			return false;
		}
	}

	maDestBlitters.Add(pcRowBlitter);
	pcRowBlitter->AddUsage();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageRowBlitterCacheValue::Remove(Ptr<CBaseImageRowBlitter> pcBlitter)
{
	Ptr<CBaseImageRowBlitter>	pDestValue;
	size						uiUsage;

	pDestValue = GetCacheDest(pcBlitter);
	if (pDestValue)
	{
		uiUsage = pDestValue->RemoveUsage();
		if (uiUsage == 0)
		{
			maDestBlitters.Remove(pDestValue);
			pDestValue = NULL;
			return true;
		}
	}
	return false;
}

