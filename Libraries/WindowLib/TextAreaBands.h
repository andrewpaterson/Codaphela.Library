/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifndef __TEXT_AREA_BANDS_H__
#define __TEXT_AREA_BANDS_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Int32Vec2.h"


//A band of a text area across its width, iTop to iBottom (exclusive), and a hash of everything drawn in it.
struct STextAreaBand
{
	int32	iTop;
	int32	iBottom;
	uint32	uiHash;
};


typedef CArrayTemplate<STextAreaBand>	CArrayTextAreaBand;


//The part of a text area that shows cut into bands, one for each row from its top to the next row's (so a line's gap
//goes with its last row), each with a hash of what is drawn in it: the row's text and styles and the shapes (the
//backgrounds, selection and lines along the text) crossing the band.  Comparing the bands of what was drawn with the
//bands of what is to be drawn finds what changed (Compare): a band that moved, changed size or changed what is in it is
//drawn again where it was and where it is.
class CTextAreaBands
{
protected:
	CArrayTextAreaBand	masBands;  //Sorted by top, not overlapping.
	bool				mbComplete;  //They are of what was last drawn.

public:
	void			Init(void);
	void			Kill(void);

	void			Clear(void);
	void			Complete(void);
	bool			IsComplete(void);
	void			Add(int32 iTop, int32 iBottom);
	size			NumBands(void);
	STextAreaBand*	Get(size uiIndex);
	size			FindBand(int32 y);
	void			Mix(size uiBand, const void* pvData, size uiBytes);
	void			MixAll(int32 iTop, int32 iBottom, const void* pvData, size uiBytes);
	void			Compare(CTextAreaBands* pcBefore, CArrayInt32Vec2* pasChanged);
	void			Take(CTextAreaBands* pcOther);

protected:
	void			AddChanged(CArrayInt32Vec2* pasChanged, int32 iTop, int32 iBottom);
};


#endif // __TEXT_AREA_BANDS_H__

