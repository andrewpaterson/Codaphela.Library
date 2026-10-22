#ifndef __IMAGE_ROW_BLITTER_FACTORY_H__
#define __IMAGE_ROW_BLITTER_FACTORY_H__
/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#include "StandardLib/Pointer.h"
#include "ImageCel.h"
#include "Int32Rect.h"
#include "ImageCopyDimension.h"
#include "ImageRowBlitterContiguous.h"
#include "ImageRowBlitterByteAlignedOpaque.h"
#include "ImageRowBlitterRGBByteAlphaByteTranslucent.h"
#include "ImageRowSizingBlitterByteAlignedOpaque.h"
#include "ImageRowSizingBlitterRGBByteAlphaByteTranslucent.h"


class CImageRowBlitterFactory
{
protected:
	CImageRowBlitterContiguous									mcImageRowBlitterContiguous;
	CImageRowBlitterByteAlignedOpaque							mcImageRowBlitterByteAlignedOpaque;
	CImageRowBlitterByteAlignedOpaqueDestAlpha					mcImageRowBlitterByteAlignedOpaqueDestAlpha;
	CImageRowBlitterRGBByteAlphaByteTranslucent					mcImageRowBlitterRGBByteAlphaByteTranslucent;
	CImageRowBlitterRGBByteAlphaByteTranslucentDestAlpha		mcImageRowBlitterRGBByteAlphaByteTranslucentDestAlpha;

	CImageRowSizingBlitterByteAlignedOpaque						mcImageRowSizingBlitterByteAlignedOpaque;
	CImageRowSizingBlitterByteAlignedOpaqueDestAlpha			mcImageRowSizingBlitterByteAlignedOpaqueDestAlpha;
	CImageRowSizingBlitterRGBByteAlphaByteTranslucent			mcImageRowSizingBlitterRGBByteAlphaByteTranslucent;
	CImageRowSizingBlitterRGBByteAlphaByteTranslucentDestAlpha	mcImageRowSizingBlitterRGBByteAlphaByteTranslucentDestAlpha;

public:
	CBaseImageRowBlitter*			CreateImageRowBlitterContiguous(void);
	CBaseImageRowBlitter*			CreateImageRowBlitterByteAlignedOpaque(void);
	CBaseImageRowBlitter*			CreateImageRowBlitterByteAlignedOpaqueDestAlpha(void);
	CBaseImageRowBlitter*			CreateImageRowBlitterRGBByteAlphaByteTranslucent(void);
	CBaseImageRowBlitter*			CreateImageRowBlitterRGBByteAlphaByteTranslucentDestAlpha(void);

	CBaseImageRowSizingBlitter*		CreateImageRowSizingBlitterByteAlignedOpaque(void);
	CBaseImageRowSizingBlitter*		CreateImageRowSizingBlitterByteAlignedOpaqueDestAlpha(void);
	CBaseImageRowSizingBlitter*		CreateImageRowSizingBlitterRGBByteAlphaByteTranslucent(void);
	CBaseImageRowSizingBlitter*		CreateImageRowSizingBlitterRGBByteAlphaByteTranslucentDestAlpha(void);

	CBaseImageRowSizingBlitter*		CreateImageRowSizingBlitter(CBaseImageRowBlitter* pcRowBlitter);
};


//Every CImageBlitter keeps pointers to the row blitters the factory holds so there is one factory that outlives them all.
extern CImageRowBlitterFactory	gcImageRowBlitterFactory;


typedef CBaseImageRowBlitter* (CImageRowBlitterFactory::*CreateImageRowBlitterFunc)(void);


#endif // __IMAGE_ROW_BLITTER_FACTORY_H__

