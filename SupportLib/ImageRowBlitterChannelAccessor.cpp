#include "BaseLib/TypeNames.h"
#include "ImageRowBlitterChannelAccessor.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterChannelAccessor::Init(Ptr<CImage> pSource, Ptr<CImage> pDest, CColourFormatHelper* pcSourceFormatHelper, CColourFormatHelper* pcDestFormatHelper)
{
	PreInit();

	EPrimitiveType	eType;
	size			uiTypeSize;

	CBaseImageRowBlitter::Init(pSource, pDest);

	muiSourceColourOffset = pcSourceFormatHelper->GetFirstColourIndex();
	muiDestColourOffset = pcDestFormatHelper->GetFirstColourIndex();

	eType = pcSourceFormatHelper->GetColourType(0);
	uiTypeSize = gcTypeNames.GetByteSize(eType);

	muiSourceColourOffset *= uiTypeSize;
	muiDestColourOffset *= uiTypeSize;

	muiColourWidth = 3 * uiTypeSize;

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

	U_Size(muiSourceColourOffset);
	U_Size(muiDestColourOffset);
	U_Size(muiColourWidth);
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
void CImageRowBlitterChannelAccessor::Copy(size iDestX, size iDestY, size iSourceXLeft, size iSourceXRight, size iSourceY)
{
	void*	pvSource;
	void*	pvDest;
	size	x;

	pvSource = mpSource->GetData();
	pvSource = RemapSinglePointer(pvSource, iSourceY * miSourceRowStride + iSourceXLeft * miSourcePixelStride);

	pvDest = mpDest->GetData();
	pvDest = RemapSinglePointer(pvDest, iDestY * miDestRowStride + iDestX * miDestPixelStride);

	for (x = iSourceXLeft; x <= iSourceXRight; x++)
	{
		ga_memcpy_fast[muiColourWidth](pvDest, pvSource);

		pvSource = RemapSinglePointer(pvSource, miSourcePixelStride);
		pvDest = RemapSinglePointer(pvSource, miDestPixelStride);
	}
}

