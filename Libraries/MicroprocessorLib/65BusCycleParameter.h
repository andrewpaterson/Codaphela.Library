/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __65_BUS_CYCLE_PARAMETER_H__
#define __65_BUS_CYCLE_PARAMETER_H__
#include "BaseLib/Chars.h"
#include "BaseLib/Killable.h"
#include "BaseLib/Refered.h"
#include "StandardLib/Unknown.h"


class C65C64;
class C65BusCycleParameter : public CUnknown, public CRefered
{
public:
	virtual bool IsAddress(void) =0;
	virtual bool IsOperation(void) =0;
};


#endif // __65_BUS_CYCLE_PARAMETER_H__

