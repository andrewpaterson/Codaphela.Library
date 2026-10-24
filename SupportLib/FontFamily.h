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

** ------------------------------------------------------------------------ **/
#ifndef __FONT_FAMILY_H__
#define __FONT_FAMILY_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplate.h"
#include "StandardLib/Array.h"
#include "ColourFormat.h"
#include "TrueTypeFace.h"
#include "FontColours.h"
#include "TextStyle.h"


//The face, size and turn of a CFontColours in the family.  The size is in 1/64 pixels.
struct SFontFamilySize
{
	EFontStyle	eStyle;
	int32		iSize64;
	EFontTurn	eTurn;
};


typedef CArrayTemplate<SFontFamilySize>	CArrayFontFamilySize;


//The faces (regular, bold, italic and bold italic) of a typeface in any size.  Each face is a TrueType font and each
//size of a face is made the first time it is asked for, in white in the family's colour order, and kept as a
//CFontColours that makes the colours.  A size is lazy: its glyphs are only rasterised (and coloured) as they are
//first asked for.  A style with no face is an error.  A family can instead be given fonts already
//made, each in only the one size it was made for.
class CFontFamily : public CObject
{
CONSTRUCTABLE(CFontFamily);
DESTRUCTABLE(CFontFamily);
protected:
	CChars					mszName;
	CChars					mszCharacters;		//UTF8, the characters every size has.
	float32					mfDefaultPixelSize;
	EColourOrder			meColourOrder;		//CCO_Unknown leaves the rasteriser's order.
	int32					miImageWidth;
	CArray<CTrueTypeFace>	mapFaces;			//By EFontStyle.  NULL where the family has no face.
	CArray<CFontColours>	mapColours;			//In the same order as masSizes.
	CArrayFontFamilySize	masSizes;

public:
	void				Init(const char* szName, const char* szCharacters, float32 fDefaultPixelSize, EColourOrder eColourOrder = CCO_Unknown);
	void				Init(Ptr<CFontColours> pRegular);
	void				Class(void) override;
	void 				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	bool				SetFace(EFontStyle eStyle, const char* szTrueTypeFilename);
	bool				HasFace(EFontStyle eStyle);
	void				AddFont(EFontStyle eStyle, float32 fPixelSize, Ptr<CFontColours> pColours);

	Ptr<CFontColours>	GetColours(EFontStyle eStyle, float32 fPixelSize, EFontTurn eTurn = FT_Upright);
	Ptr<CFont>			GetFont(STextStyle* psStyle);
	Ptr<CFont>			GetRunFont(STextStyle* psStyle, int16* piBaselineShift);

	char*				GetFamilyName(void);  //Not GetName, that is the object's name.
	float32				GetDefaultPixelSize(void);
	size				NumSizes(void);

protected:
	int32				GetSizeKey(float32 fPixelSize);
	Ptr<CFontColours>	Generate(EFontStyle eStyle, float32 fPixelSize, EFontTurn eTurn);
};


const char* FontStyleToString(EFontStyle eStyle);


#endif // __FONT_FAMILY_H__

