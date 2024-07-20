/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#ifndef __IMAGE_MODIFIER_STACK_H__
#define __IMAGE_MODIFIER_STACK_H__
#include "StandardLib/ArrayUnknown.h"
#include "StandardLib/Unknowns.h"
#include "Image.h"
#include "ImageModifier.h"


class CImageModifierStack : public CUnknown
{
CONSTRUCTABLE(CImageModifierStack);
protected:
	CArrayUnknown	macModifiers;  //Image | Modifier 0 | Modifier 1 | etc...
	Ptr<CImage>		mpcImage;

public:
	void 			Init(Ptr<CImage> pcImage = NULL);
	void 			Kill(void);

	void 			AddModifier(CImageModifier* pcModifier);
	template<typename M>
	M*				AddModifier(void);

	void			SetImage(Ptr<CImage> pcImage);  //SetImage and ApplyAll make the stack act on multiple images.  There should be a better way of doing this.
	void			ApplyAll(void);  //Better way.  CImageModifierStack has a list of images.  But then the cache is broken.  Need a cached image.

	int				NumModifiers(void);
	Ptr<CImage>		GetImage(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<typename M>
M* CImageModifierStack::AddModifier(void)
{
	M*	pv;

	if (mpcUnknownsThisIn != NULL)
	{
		pv = mpcUnknownsThisIn->Add<M>();
	}
	else
	{
		pv = UMalloc(M);
	}
	AddModifier(pv);
	return pv;
}


#endif // __IMAGE_MODIFIER_STACK_H__

