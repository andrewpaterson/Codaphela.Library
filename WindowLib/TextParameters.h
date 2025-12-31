#ifndef __TEXT_PARAMETERS_H__
#define __TEXT_PARAMETERS_H__
/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/Int2.h"
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"
#include "SupportLib/ColourARGB32.h"
#include "SupportLib/Font.h"


class CWindow;
//Rename CTextParameters.  Its more of a Font.
class CTextParameters : public CObject
{
CONSTRUCTABLE(CTextParameters); 
DESTRUCTABLE(CTextParameters);
protected:
	Ptr<CFont>				mpFont;
	Ptr<CWindow>			mpWindow;
	int16					miTabSpaceCount;  //tab width = miTabSpaceCount * miSpaceWidth.

	ARGB32					msColour;

public:
	Ptr<CTextParameters>	Init(Ptr<CFont> pcFont, Ptr<CWindow> pWindow);
	void 					Free(void);
	void					Class(void);

	bool					Load(CObjectReader* pcFile);
	bool					Save(CObjectWriter* pcFile);

	void					SetFont(CFont* pcFont);
	CGlyph*					GetGlyph(uint16 c);
	int16					GetAscent(void);
	int16					GetDescent(void);
	
	Ptr<CFont>				GetFont(void);

	int16					GetTabSpaceCount(void);
	int16					GetSpaceWidth(void);
};


#endif // __TEXT_PARAMETERS_H__

