#include "MultiImageCopier.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMultiImageCopier::Init(Ptr<CImage> pDestImage)
{
	mpDestImage = pDestImage;
	mcMapImageToCopier.Init();
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMultiImageCopier::Init(CMallocator* pcMalloc, Ptr<CImage> pDestImage)
{
	mpDestImage = pDestImage;
	mcMapImageToCopier.Init(pcMalloc);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMultiImageCopier::Kill(void)
{
	SMapIterator	sIter;
	CImage*			pcImage;
	CImageCopier*	pcImageCopier;
	bool			bExists;

	pcImage = NULL;
	pcImageCopier = NULL;

	bExists = mcMapImageToCopier.StartIteration(&sIter, (void**)&pcImage, (void**)&pcImageCopier);
	while (bExists)
	{
		pcImageCopier->Kill();
		bExists = mcMapImageToCopier.Iterate(&sIter, (void**)&pcImage, (void**)&pcImageCopier);
	}

	mcMapImageToCopier.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMultiImageCopier::AddAccessor(Ptr<CImageCel> pCel)
{
	return AddAccessor(pCel->GetSourceImage());
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMultiImageCopier::AddAccessor(Ptr<CImage> pSourceImage)
{
	CImageCopier*				pcCopier;
	bool						bResult;

	pcCopier = (CImageCopier*)mcMapImageToCopier.Get(&pSourceImage);
	if (pcCopier == NULL)
	{
		pcCopier = UMalloc(CImageCopier);
		if (pcCopier)
		{
			bResult = pcCopier->Init(pSourceImage, mpDestImage);
			if (bResult)
			{
				bResult = mcMapImageToCopier.Put(&pSourceImage, pcCopier);
				if (bResult)
				{
					return true;
				}
			}
			UFree(pcCopier);
		}
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMultiImageCopier::Copy(int32 iDestX, int32 iDestY, Ptr<CImageCel> pSourceCel)
{
	CImageCopier*	pcCopier;

	pcCopier = (CImageCopier*)mcMapImageToCopier.Get(&pSourceCel->GetSourceImage());
	if (pcCopier == NULL)
	{
		return false;
	}

	pcCopier->Copy(iDestX, iDestY, pSourceCel);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMultiImageCopier::Copy(int32 iDestX, int32 iDestY, Ptr<CImage> pSourceImage, CRectangle* psSourceRect)
{
	CImageCopier*	pcCopier;

	pcCopier = (CImageCopier*)mcMapImageToCopier.Get(&pSourceImage);
	if (pcCopier == NULL)
	{
		return false;
	}

	pcCopier->Copy(iDestX, iDestY, psSourceRect);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMultiImageCopier::Copy(int32 iDestX, int32 iDestY, Ptr<CImage> pSourceImage, int32 iSourceX1, int32 iSourceY1, int32 iSourceX2, int32 iSourceY2)
{
	CImageCopier*	pcCopier;

	pcCopier = (CImageCopier*)mcMapImageToCopier.Get(&pSourceImage);
	if (pcCopier == NULL)
	{
		return false;
	}

	pcCopier->Copy(iDestX, iDestY, iSourceX1, iSourceY1, iSourceX2, iSourceY2);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMultiImageCopier::Copy(SImageCopy* psCopy, Ptr<CImage> pSourceImage)
{
	CImageCopier*	pcCopier;

	pcCopier = (CImageCopier*)mcMapImageToCopier.Get(&pSourceImage);
	if (pcCopier == NULL)
	{
		return false;
	}

	pcCopier->Copy(psCopy);
	return true;
}

