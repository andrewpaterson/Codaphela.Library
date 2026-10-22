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
#ifndef __IMAGE_MODIFIER_STACK_H__
#define __IMAGE_MODIFIER_STACK_H__
#include "StandardLib/Object.h"
#include "StandardLib/Objects.h"
#include "StandardLib/Array.h"
#include "Image.h"
#include "ImageModifier.h"


class CImageModifierStack : public CObject
{
CONSTRUCTABLE(CImageModifierStack);
DESTRUCTABLE(CImageModifierStack);
protected:
	CArray<CImageModifier>	maModifiers;  //Image | Modifier 0 | Modifier 1 | etc...
	Ptr<CImage>				mpImage;

public:
	void 					Init(Ptr<CImage> pImage = NULL);
	void					Class(void) override;
	void 					Free(void) override;

	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;

	void 					AddModifier(Ptr<CImageModifier> pModifier);
	template<class M, typename ... Args>
	Ptr<M>					AddModifier(Args ... args);

	void					SetImage(Ptr<CImage> pImage);  //SetImage and ApplyAll make the stack act on multiple images.  There should be a better way of doing this.
	void					ApplyAll(void);  //Better way.  CImageModifierStack has a list of images.  But then the cache is broken.  Need a cached image.

	int						NumModifiers(void);
	Ptr<CImage>				GetImage(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, typename ... Args>
Ptr<M> CImageModifierStack::AddModifier(Args ... args)
{
	Ptr<M>	pModifier;

	pModifier = OMalloc<M>(args ...);
	AddModifier(pModifier);
	return pModifier;
}

#endif // __IMAGE_MODIFIER_STACK_H__

