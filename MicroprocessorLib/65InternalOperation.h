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
#ifndef __65_INTERNAL_OPERATION_H__
#define __65_INTERNAL_OPERATION_H__
#include "65DataOperation.h"


//A cycle that does not use the bus.  RWB is high.
class C65InternalOperation : public C65DataOperation
{
CONSTRUCTABLE(C65InternalOperation);
public:
	void	Init(void);

	void	Execute(C65C64* pcCPU) override;
	bool	IsInitialSide(void) override;
	void	Print(CChars* psz) override;
};


#endif // __65_INTERNAL_OPERATION_H__

