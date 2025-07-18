/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WorldLib

Codaphela WorldLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WorldLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WorldLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft DirectX is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __ANIMATION_CONVERTER_H__
#define __ANIMATION_CONVERTER_H__
#include "SupportLib/Scene.h"
#include "Movement.h"


class CSceneConverter;
class CAnimationConverter
{
protected:
	CSceneConverter* mpcSceneConverter;

public:
	void Init(CSceneConverter* pcSceneConverter);
	void Kill(void);
	bool Convert(CMovement** ppcMovement, CSequence* pcSequence, SMatrix* psMatrix);
	bool IsAnimationConverted(void);
};


#endif //__ANIMATION_CONVERTER_H__

