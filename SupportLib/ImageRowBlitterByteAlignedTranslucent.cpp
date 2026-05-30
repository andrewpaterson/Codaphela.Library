#include "BaseLib/TypeNames.h"
#include "ImageRowBlitterByteAlignedTranslucent.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterByteAlignedTranslucent::Init(Ptr<CImage> pSource, Ptr<CImage> pDest, CColourFormatHelper* pcSourceFormatHelper, CColourFormatHelper* pcDestFormatHelper)
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

	muiSourceAlphaOffset = pcSourceFormatHelper->GetAlphaIndex();

	eType = pcSourceFormatHelper->GetAlphaType();
	uiTypeSize = gcTypeNames.GetByteSize(eType);

	muiSourceAlphaOffset *= uiTypeSize;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterByteAlignedTranslucent::Free(void)
{
	CBaseImageRowBlitter::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterByteAlignedTranslucent::Class(void)
{
	CBaseImageRowBlitter::Class();

	U_Size(muiSourceColourOffset);
	U_Size(muiDestColourOffset);
	U_Size(muiSourceAlphaOffset);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageRowBlitterByteAlignedTranslucent::Save(CObjectWriter* pcFile)
{
	return CBaseImageRowBlitter::Save(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageRowBlitterByteAlignedTranslucent::Load(CObjectReader* pcFile)
{
	return CBaseImageRowBlitter::Load(pcFile);
}

