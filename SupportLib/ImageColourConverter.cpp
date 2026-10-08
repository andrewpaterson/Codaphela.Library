#include "ImageCopier.h"
#include "ImageColourConverter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CImageColourConverter::ConvertFormat(Ptr<CImage> pImage, CColourFormatHelper* pcHelper)
{
	ValidatePtr(pImage);

	Ptr<CImage>				pNewImage;
	int						iWidth;
	int						iHeight;

	if (pImage->Matches(pcHelper))
	{
		return pImage;
	}

	iWidth = pImage->GetWidth();
	iHeight = pImage->GetHeight();

	pNewImage = OMalloc<CImage>(iWidth, iHeight, pcHelper);

	CImageCopier::Copy(pImage, pNewImage);

	return pNewImage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CImageColourConverter::ConvertColour(Ptr<CImage> pImage, EColourOrder eOrder)
{
	ValidatePtr(pImage);

	Ptr<CImage>				pNewImage;
	int						iWidth;
	int						iHeight;
	EColourOrder			eImageOrder;
	CArrayChannel			asNewChannels;
	CChannel*				pcChannel;
	size					iNumChannels;
	size					iChannelIndex;
	size					iDiffuseIndex;
	SChannel*				psChannel;
	CColourFormatHelper		cHelper;
	bool					bAddedColours;
	size					eChannel;

	eImageOrder = pImage->GetColourOrder();
	if (eImageOrder == CCO_Unknown)
	{
		return NULL;
	}

	cHelper.Init(CFT_RGB, eOrder, CRGB_Unknown, ARGB_Unknown);

	bAddedColours = false;
	asNewChannels.Init();
	iNumChannels = pImage->NumChannels();
	for (iChannelIndex = 0; iChannelIndex < iNumChannels; iChannelIndex++)
	{
		pcChannel = pImage->GetChannelAtIndex(iChannelIndex);
		if (CHANNEL_PURPOSE(pcChannel->iChannel) == IP_Diffuse)
		{
			if (!bAddedColours)
			{
				for (iDiffuseIndex = 0; iDiffuseIndex < 3; iDiffuseIndex++)
				{
					eChannel = (size)cHelper.GetColourChannel(iDiffuseIndex);
					psChannel = asNewChannels.Add();
					psChannel->eType = pcChannel->eType;
					psChannel->iChannel = eChannel;
					psChannel->bReverse = false;
				}
				bAddedColours = true;
			}
		}
		else
		{
			psChannel = asNewChannels.Add();
			psChannel->eType = pcChannel->eType;
			psChannel->iChannel = pcChannel->iChannel;
			psChannel->bReverse = false;
		}
	}


	if (pImage->Matches(&asNewChannels))
	{
		asNewChannels.Kill();
		return pImage;
	}

	iWidth = pImage->GetWidth();
	iHeight = pImage->GetHeight();

	pNewImage = OMalloc<CImage>(iWidth, iHeight, &asNewChannels);
	asNewChannels.Kill();

	CImageCopier::Copy(pImage, pNewImage);

	return pNewImage;
}

