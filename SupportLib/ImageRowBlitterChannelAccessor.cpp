#include "BaseLib/TypeNames.h"
#include "ImageRowBlitterFactory.h"
#include "ImageRowBlitterChannelAccessor.h"


//The start of the accessor based fallback blitter for formats the other row blitters can't copy (see CImageBlitter::InitRowBlitters).
//Until it uses accessors it copies the colour bytes like CImageRowBlitterByteAlignedOpaque.  It is not in the project.
//
//	CChannelsAccessor*			pcAccessor;
//	CChannelsAccessorCreator	cCreator;
//	size						uiNumChannels;
//	size						uiChannelIndex;
//	EChannel					eChannel;
//	EPrimitiveType				eType;
//	CColourFormatHelper			cFormatHelper;
//
//	uiNumChannels = cFormatHelper.GetNumChannels();
//
//	cCreator.Init(pSourceImage->GetChannels());
//	for (uiChannelIndex = 0; uiChannelIndex < uiNumChannels; uiChannelIndex++)
//	{
//		eChannel = cFormatHelper.GetChannel(uiChannelIndex);
//		eType = cFormatHelper.GetType(uiChannelIndex);
//		if ((eChannel == IMAGE_CHANNEL_UNKNOWN) || (eType == PT_Undefined))
//		{
//			return false;
//		}
//
//		cCreator.AddAccess(eChannel, eType);
//	}
//
//	pcAccessor = cCreator.Create();


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterChannelAccessor::Copy(CImageBlitterContext* pcContext, void* pvSource, void* pvDest, size iDestX, size iDestY, size iSourceXLeft, size iSourceXRight, size iSourceY)
{
	size	x;

	pvSource = RemapSinglePointer(pvSource, iSourceY * pcContext->miSourceRowStride + iSourceXLeft * pcContext->miSourcePixelStride + pcContext->muiSourceColourOffset);
	pvDest = RemapSinglePointer(pvDest, iDestY * pcContext->miDestRowStride + iDestX * pcContext->miDestPixelStride + pcContext->muiDestColourOffset);

	for (x = iSourceXLeft; x < iSourceXRight; x++)
	{
		ga_memcpy_fast[pcContext->muiColourWidth](pvDest, pvSource);

		pvSource = RemapSinglePointer(pvSource, pcContext->miSourcePixelStride);
		pvDest = RemapSinglePointer(pvDest, pcContext->miDestPixelStride);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseImageRowSizingBlitter* CImageRowBlitterChannelAccessor::GetSizingBlitter(CImageRowBlitterFactory* pcFactory)
{
	return pcFactory->CreateImageRowSizingBlitterByteAlignedOpaque();
}

