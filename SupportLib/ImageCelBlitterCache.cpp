#include "StandardLib/ClassDefines.h"
#include "ImageCelBlitterCache.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelBlitterCache::Init(Ptr<CImage> pDestImage)
{
	PreInit();

	maBlitters.Init();
	mapcCels.Init();
	mpDestImage = pDestImage;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelBlitterCache::Class(void)
{
	M_Embedded(maBlitters);
	U_Data(CArrayImageCelPtr, mapcCels);
	M_Pointer(mpDestImage);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelBlitterCache::Free(void)
{
	size			i;
	CImageCel*		pcCel;

	//maBlitters has already been freed so only the cels are touched.  A cel still listed has not been freed yet,
	//because a freed cel sets its entry to NULL.
	for (i = 0; i < mapcCels.NumElements(); i++)
	{
		pcCel = *mapcCels.Get(i);
		if (pcCel)
		{
			pcCel->ForgetBlitterCache(this);
		}
	}
	mapcCels.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCelBlitterCache::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCelBlitterCache::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageBlitter> CImageCelBlitterCache::CreateImageBlitter(Ptr<CImageCel> pCel)
{
	return CreateImageBlitter(pCel, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageBlitter> CImageCelBlitterCache::CreateSizingImageBlitter(Ptr<CImageCel> pCel)
{
	return CreateImageBlitter(pCel, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageBlitter> CImageCelBlitterCache::CreateImageBlitter(Ptr<CImageCel> pCel, bool bAllowSizing)
{
	Ptr<CImageBlitter>	pImageBlitter;
	CImageCel*			pcCel;
	bool				bValid;

	ValidatePtr(pCel);

	pImageBlitter = GetImageBlitter(pCel);
	if (pImageBlitter.IsNotNull())
	{
		if (!pImageBlitter->AllowsSizing())
		{
			pImageBlitter->EnableSizing(&gcImageRowBlitterFactory);
		}
		return pImageBlitter;
	}

	pImageBlitter = OMalloc<CImageBlitter>(pCel, mpDestImage);
	if (!pImageBlitter)
	{
		return NULL;
	}

	bValid = pImageBlitter->Configure(&gcImageRowBlitterFactory, bAllowSizing);
	if (!bValid)
	{
		return NULL;
	}

	pcCel = (CImageCel*)pCel.Object();
	maBlitters.Add(pImageBlitter);
	mapcCels.Add(&pcCel);
	pcCel->AddBlitterCache(this);

	return pImageBlitter;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageBlitter> CImageCelBlitterCache::GetImageBlitter(Ptr<CImageCel> pCel)
{
	size	uiIndex;

	RemoveForgottenBlitters();
	uiIndex = FindCel((CImageCel*)pCel.Object());
	if (uiIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		return NULL;
	}
	return maBlitters.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCelBlitterCache::HasImageBlitter(Ptr<CImageCel> pCel)
{
	return GetImageBlitter(pCel).IsNotNull();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CImageCelBlitterCache::NumImageBlitters(void)
{
	RemoveForgottenBlitters();
	return mapcCels.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCelBlitterCache::Matches(Ptr<CImage> pImage)
{
	if (mpDestImage == pImage)
	{
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
void CImageCelBlitterCache::RemoveCelDuringFree(CImageCel* pcCel)
{
	size			uiIndex;
	CImageCel**		ppcCel;

	//The cel is being freed.  Its blitter is not removed here because that would free the blitter while the cel is
	//still being freed, so it is only marked and removed the next time this cache is used.
	uiIndex = FindCel(pcCel);
	if (uiIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		ppcCel = mapcCels.Get(uiIndex);
		*ppcCel = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CImageCelBlitterCache::FindCel(CImageCel* pcCel)
{
	if (pcCel == NULL)
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}
	return mapcCels.Find(&pcCel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCelBlitterCache::RemoveForgottenBlitters(void)
{
	size	i;

	i = mapcCels.NumElements();
	while (i > 0)
	{
		i--;
		if (*mapcCels.Get(i) == NULL)
		{
			mapcCels.RemoveAt(i);
			maBlitters.RemoveAt(i);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CImageCelBlitterCache::GetDestImage(void) { return mpDestImage; }

