#ifndef __TEXT_RUN_H__
#define __TEXT_RUN_H__
/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MeshLib

Codaphela MeshLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MeshLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MeshLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/Constructable.h"
#include "BaseLib/Int32Vec2.h"
#include "BaseLib/ArrayTemplate.h"
#include "StandardLib/ObjectReader.h"
#include "StandardLib/ObjectWriter.h"
#include "Font.h"
#include "ColourARGB32.h"
#include "TextStyle.h"
#include "TextDrawable.h"


class CText;
class CTextRun
{
CONSTRUCTABLE(CTextRun)
protected:
	CFont*					mpcFont;  //Not a Ptr<CFont> because CTextRun does not extend CObject.
	CText*					mpcText;  //Ditto Ptr<CText>
	CArrayTextElementPtr	mapcText;
	int16					miBaselineShift;  //Pixels up, for superscript.  Down if negative, for subscript.
	uint8					muiDecorations;   //TD_ flags.
	ARGB32					msDecorationColour;

public:
	void			Init(CFont* mpcFont, CText* pcText);
	void 			Kill(void);

	bool			Load(CObjectReader* pcFile);
	bool			Save(CObjectWriter* pcFile);
	CMallocator*	GetMalloc(void);
	
	void			Add(CTextElement* pcElement);
	void			Start(void);
	void			End(void);

	CFont*			GetFont(void);
	void			SetBaselineShift(int16 iBaselineShift);
	int16			GetBaselineShift(void);
	void			SetDecorations(uint8 uiDecorations, ARGB32 sColour);
	uint8			GetDecorations(void);
	ARGB32			GetDecorationColour(void);
	size			NumElements(void);
	CTextElement*	GetElement(size uiIndex);
};


typedef CArrayTemplate<CTextRun>	CArrayTextRun;


#endif // __TEXT_RUN_H__

