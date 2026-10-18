#include "ImageRowBlitterFactory.h"


CImageRowBlitterFactory	gcImageRowBlitterFactory;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseImageRowBlitter* CImageRowBlitterFactory::CreateImageRowBlitterContiguous(void)
{
	return &mcImageRowBlitterContiguous;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseImageRowBlitter* CImageRowBlitterFactory::CreateImageRowBlitterByteAlignedOpaque(void)
{
	return &mcImageRowBlitterByteAlignedOpaque;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseImageRowBlitter* CImageRowBlitterFactory::CreateImageRowBlitterByteAlignedOpaqueDestAlpha(void)
{
	return &mcImageRowBlitterByteAlignedOpaqueDestAlpha;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseImageRowBlitter* CImageRowBlitterFactory::CreateImageRowBlitterRGBByteAlphaByteTranslucent(void)
{
	return &mcImageRowBlitterRGBByteAlphaByteTranslucent;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseImageRowBlitter* CImageRowBlitterFactory::CreateImageRowBlitterRGBByteAlphaByteTranslucentDestAlpha(void)
{
	return &mcImageRowBlitterRGBByteAlphaByteTranslucentDestAlpha;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseImageRowSizingBlitter* CImageRowBlitterFactory::CreateImageRowSizingBlitterByteAlignedOpaque(void)
{
	return &mcImageRowSizingBlitterByteAlignedOpaque;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseImageRowSizingBlitter* CImageRowBlitterFactory::CreateImageRowSizingBlitterByteAlignedOpaqueDestAlpha(void)
{
	return &mcImageRowSizingBlitterByteAlignedOpaqueDestAlpha;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseImageRowSizingBlitter* CImageRowBlitterFactory::CreateImageRowSizingBlitterRGBByteAlphaByteTranslucent(void)
{
	return &mcImageRowSizingBlitterRGBByteAlphaByteTranslucent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseImageRowSizingBlitter* CImageRowBlitterFactory::CreateImageRowSizingBlitterRGBByteAlphaByteTranslucentDestAlpha(void)
{
	return &mcImageRowSizingBlitterRGBByteAlphaByteTranslucentDestAlpha;
}


CBaseImageRowSizingBlitter* CImageRowBlitterFactory::CreateImageRowSizingBlitter(CBaseImageRowBlitter* pcRowBlitter)
{
	if ((pcRowBlitter == &mcImageRowBlitterContiguous) || (pcRowBlitter == &mcImageRowBlitterByteAlignedOpaque))
	{
		return &mcImageRowSizingBlitterByteAlignedOpaque;
	}
	else if (pcRowBlitter == &mcImageRowBlitterByteAlignedOpaqueDestAlpha)
	{
		return &mcImageRowSizingBlitterByteAlignedOpaqueDestAlpha;
	}
	else if (pcRowBlitter == &mcImageRowBlitterRGBByteAlphaByteTranslucent)
	{
		return &mcImageRowSizingBlitterRGBByteAlphaByteTranslucent;
	}
	else if (pcRowBlitter == &mcImageRowBlitterRGBByteAlphaByteTranslucentDestAlpha)
	{
		return &mcImageRowSizingBlitterRGBByteAlphaByteTranslucentDestAlpha;
	}
	else
	{
		return NULL;
	}
}

