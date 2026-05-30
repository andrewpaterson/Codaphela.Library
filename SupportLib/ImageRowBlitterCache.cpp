#include "ImageRowBlitterCache.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterCache::Init(void)
{
	PreInit();

	mMapBlitters.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterCache::Class(void)
{
	M_Embedded(mMapBlitters);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterCache::Free(void)
{
}
	

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageRowBlitterCache::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageRowBlitterCache::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageRowBlitterCacheValue> CImageRowBlitterCache::GetOrCreateImageRowBlitterCacheValue(Ptr<CImage> pSource)
{
	Ptr<CImageRowBlitterCacheValue>		pValue;

	pValue = mMapBlitters.Get(pSource);
	if (pValue.IsNull())
	{
		pValue = OMalloc<CImageRowBlitterCacheValue>(pSource);
		mMapBlitters.Put(pSource, pValue);
	}

	return pValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseImageRowBlitter* CImageRowBlitterCache::CreateImageRowBlitterContiguous(Ptr<CImage> pSource, Ptr<CImage> pDest)
{
	CImageRowBlitterContiguous*			pcRowBlitter;
	Ptr<CImageRowBlitterCacheValue>		pValue;

	pValue = GetOrCreateImageRowBlitterCacheValue(pSource);

	pcRowBlitter = (CImageRowBlitterContiguous*)pValue->Get(pDest, CLASS_NAME(CImageRowBlitterContiguous));
	if (pcRowBlitter)
	{
		return pcRowBlitter;
	}

	pcRowBlitter = UMalloc(CImageRowBlitterContiguous);
	pcRowBlitter->Init(pSource, pDest);
	
	pValue->Add(pDest, pcRowBlitter);
	return pcRowBlitter;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseImageRowBlitter* CImageRowBlitterCache::CreateImageRowBlitterByteAlignedOpaque(Ptr<CImage> pSource, Ptr<CImage> pDest, CColourFormatHelper* pcSourceFormatHelper, CColourFormatHelper* pcDestFormatHelper)
{
	CImageRowBlitterByteAlignedOpaque*	pcRowBlitter;
	Ptr<CImageRowBlitterCacheValue>		pValue;

	pValue = GetOrCreateImageRowBlitterCacheValue(pSource);

	pcRowBlitter = (CImageRowBlitterByteAlignedOpaque*)pValue->Get(pDest, CLASS_NAME(CImageRowBlitterByteAlignedOpaque));
	if (pcRowBlitter)
	{
		return pcRowBlitter;
	}

	pcRowBlitter = UMalloc(CImageRowBlitterByteAlignedOpaque);
	pcRowBlitter->Init(pSource, pDest, pcSourceFormatHelper, pcDestFormatHelper);
	
	pValue->Add(pDest, pcRowBlitter);
	return pcRowBlitter;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseImageRowBlitter* CImageRowBlitterCache::CreateImageRowBlitterRGBByteAlphaByteTranslucent(Ptr<CImage> pSource, Ptr<CImage> pDest, CColourFormatHelper* pcSourceFormatHelper, CColourFormatHelper* pcDestFormatHelper)
{
	CImageRowBlitterRGBByteAlphaByteTranslucent*	pcRowBlitter;
	Ptr<CImageRowBlitterCacheValue>					pValue;

	pValue = GetOrCreateImageRowBlitterCacheValue(pSource);

	pcRowBlitter = (CImageRowBlitterRGBByteAlphaByteTranslucent*)pValue->Get(pDest, CLASS_NAME(CImageRowBlitterRGBByteAlphaByteTranslucent));
	if (pcRowBlitter)
	{
		return pcRowBlitter;
	}

	pcRowBlitter = UMalloc(CImageRowBlitterRGBByteAlphaByteTranslucent);
	pcRowBlitter->Init(pSource, pDest, pcSourceFormatHelper, pcDestFormatHelper);
	
	pValue->Add(pDest, pcRowBlitter);
	return pcRowBlitter;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageRowBlitterCache::FreeImageRowBlitter(CBaseImageRowBlitter* pcRowBlitter)
{
	Ptr<CImageRowBlitterCacheValue>		pValue;
	
	pValue = mMapBlitters.Get(pcRowBlitter->GetSource());
	if (pValue)
	{
		return pValue->Remove(pcRowBlitter);
	}
	return false;
}

