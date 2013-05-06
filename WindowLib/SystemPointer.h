/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#ifndef __SYSTEM_POINTER_H__
#define __SYSTEM_POINTER_H__
#include "BaseLib/DataTypes.h"
#include "BaseLib/GeometricTypes.h"


class CSystemPointer
{
public:
	SFloat2		msPosition;
	float		mfSpeed;

	void 	Init(void);
	void 	Kill(void);

	void 	Update(void);
	void 	Set(int x, int y);
	void 	Move(int x, int y);
	void 	Set(float x, float y);
	void 	Move(float x, float y);

	int		GetX(void);
	int		GetY(void);
};


#endif // __SYSTEM_POINTER_H__

