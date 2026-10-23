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
#ifndef __CHECK_BOX_IMAGES_H__
#define __CHECK_BOX_IMAGES_H__
#include "StandardLib/Array.h"
#include "SupportLib/ImageCel.h"


//A check box keeps its focus ring while the pointer is over it, so unlike a button's states focus and the pointer are
//shown together.
enum ECheckBoxState
{
	CBS_Normal,
	CBS_MouseOver,
	CBS_Focussed,
	CBS_FocussedMouseOver,

	NUM_CHECK_BOX_STATES
};


//The box of a check box, ticked and not, in each of its states.  Check boxes can share them.  Every cel is the same
//size.
class CCheckBoxImages : public CObject
{
CONSTRUCTABLE(CCheckBoxImages);
DESTRUCTABLE(CCheckBoxImages);
protected:
	CArray<CImageCel>	maCels;  //NUM_CHECK_BOX_STATES unticked then NUM_CHECK_BOX_STATES ticked.

public:
	void			Init(void);
	void			Class(void) override;
	void 			Free(void) override;

	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	void			SetCel(bool bTicked, ECheckBoxState eState, Ptr<CImageCel> pCel);
	Ptr<CImageCel>	GetCel(bool bTicked, ECheckBoxState eState);
};


#endif // __CHECK_BOX_IMAGES_H__

