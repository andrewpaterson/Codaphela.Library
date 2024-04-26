/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela GameLib

Codaphela GameLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela GameLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela GameLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __SPRITE_H__
#define __SPRITE_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Chars.h"
#include "BaseLib/GeometricTypes.h"
#include "GameObject.h"


class CSprite : public CGameObject
{
CONSTRUCTABLE(CSprite);
public:
	SFloat2 					msPosition;
	SFloat2 					msVelocity;
	SFloat2 					msPerceivedVelocity;
	BOOL						mbPositionTicked;
	CChars						mszName;
	int							miLayer;
	int							miCollisionBits;

	int 						miCelFrame;
	int 						miCollisionFrame;
	int 						miLinkFrame;

	CSprite*					mpcParent;
	int							miLinkIndexOnParent;

	void 			Init(void);
	void 			Kill(void);
};


#endif // __SPRITE_H__

