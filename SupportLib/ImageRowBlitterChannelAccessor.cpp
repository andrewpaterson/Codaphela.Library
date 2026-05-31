#include "BaseLib/TypeNames.h"
#include "ImageRowBlitterChannelAccessor.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterChannelAccessor::Init(CColourFormatHelper* pcSourceFormatHelper, CColourFormatHelper* pcDestFormatHelper)
{
	PreInit();

	//CChannelsAccessor*			pcAccessor;
	//CChannelsAccessorCreator	cCreator;
	//size						uiNumChannels;
	//size						uiChannelIndex;
	//EChannel					eChannel;
	//EPrimitiveType				eType;
	//CColourFormatHelper			cFormatHelper;

	//uiNumChannels = cFormatHelper.GetNumChannels();

	//cCreator.Init(pSourceImage->GetChannels());
	//for (uiChannelIndex = 0; uiChannelIndex < uiNumChannels; uiChannelIndex++)
	//{
	//	eChannel = cFormatHelper.GetChannel(uiChannelIndex);
	//	eType = cFormatHelper.GetType(uiChannelIndex);
	//	if ((eChannel == IMAGE_CHANNEL_UNKNOWN) || (eType == PT_Undefined))
	//	{
	//		return false;
	//	}

	//	cCreator.AddAccess(eChannel, eType);
	//}
	// 
	//pcAccessor = cCreator.Create();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterChannelAccessor::Free(void)
{
	CBaseImageRowBlitter::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterChannelAccessor::Class(void)
{
	CBaseImageRowBlitter::Class();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageRowBlitterChannelAccessor::Save(CObjectWriter* pcFile)
{
	return CBaseImageRowBlitter::Save(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageRowBlitterChannelAccessor::Load(CObjectReader* pcFile)
{
	return CBaseImageRowBlitter::Load(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterChannelAccessor::Copy(CImageBlitterContext* pcContext, size iDestX, size iDestY, size iSourceXLeft, size iSourceXRight, size iSourceY)
{
	void*	pvSource;
	void*	pvDest;
	size	x;

	pvSource = RemapSinglePointer(pcContext->mpvSource, iSourceY * pcContext->miSourceRowStride + iSourceXLeft * pcContext->miSourcePixelStride);

	pvDest = RemapSinglePointer(pcContext->mpvDest, iDestY * pcContext->miDestRowStride + iDestX * pcContext->miDestPixelStride);

	for (x = iSourceXLeft; x <= iSourceXRight; x++)
	{
		ga_memcpy_fast[muiColourWidth](pvDest, pvSource);

		pvSource = RemapSinglePointer(pvSource, pcContext->miSourcePixelStride);
		pvDest = RemapSinglePointer(pvSource, pcContext->miDestPixelStride);
	}
}

