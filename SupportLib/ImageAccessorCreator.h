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
#ifndef __IMAGE_ACCESSOR_CREATOR_H__
#define __IMAGE_ACCESSOR_CREATOR_H__
#include "StandardLib/Pointer.h"
#include "StandardLib/ChannelsAccessorCreator.h"
#include "ImageAccessor.h"
#include "ImageColour.h"


class CImageAccessorCreator
{
public:
	static CImageAccessor* CreateEmpty(void);
	static CImageAccessor* Create(CImage* pcImage, int iFirst, ...);
	static CImageAccessor* Create(CImage* pcImage);
	static CImageAccessor* Create(CImage* pcImage, CImage* pcChannels);
	static CImageAccessor* Create(CImage* pcImage, CArrayInt* paiChannels);
	static CImageAccessor* Create(CImage* pcImage, CImageColour* pcColour);

public:
	CChannelsAccessorCreator	mcCreator;
	CImage*						mpcImage;

	void				Init(CImage* pcImage);
	void				Kill(void);
	CImageAccessor*		Create(void);
	CImageAccessor*		CreateAndKill(void);

	void 	AddAccess(int iChannel, EPrimitiveType eType = PT_Undefined);
	void 	AddAccess(int iChannel1, int iChannel2, EPrimitiveType eType = PT_Undefined);
	void	AddAccess(int iChannel1, int iChannel2, int iChannel3, EPrimitiveType eType = PT_Undefined);
	void	AddAccess(int iChannel1, int iChannel2, int iChannel3, int iChannel4, EPrimitiveType eType = PT_Undefined);
	void	AddAccess(CArrayInt* paiChannels, EPrimitiveType eType = PT_Undefined);
	void	AddAccess(CImage* pcChannels);
	void	AddAccess(CImageAccessor* pcChannels);

	void	AddAccess(CImageColour* pcColour);
	void	AddAccessFromIntersectionOf(CArrayInt* paiChannels1, CArrayInt* paiChannels2);  //Hmmm...
};


#endif // __IMAGE_ACCESSOR_CREATOR_H__

