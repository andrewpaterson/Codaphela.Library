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
#ifndef __COMPONENT_INPUT_NUM_KEY_H__
#define __COMPONENT_INPUT_NUM_KEY_H__
#include "BaseLib/ArrayTemplate.h"
#include "InputLib/InputSourceDesc.h"


class CComponentInputNumKey
{
public:
	CInputSourceDesc*	pcSource;
	CInputSourceDesc*	pcYesNumLockRemap;
	CInputSourceDesc*	pcNoNumLockRemap;

	void Init(CInputSourceDesc*	pcSource, CInputSourceDesc* pcYesNumLockRemap, CInputSourceDesc* pcNoNumLockRemap);
	void Kill(void);
};


typedef CArrayTemplate<CComponentInputNumKey>	CArrayComponentInputNumKey;



#endif // __COMPONENT_INPUT_NUM_KEY_H__

