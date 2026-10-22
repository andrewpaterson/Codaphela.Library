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
#ifndef __65_CONSTANT_OFFSET_H__
#define __65_CONSTANT_OFFSET_H__
#include "65AddressOffset.h"


//A constant added to the other offsets.
class C65ConstantOffset : public C65AddressOffset
{
CONSTRUCTABLE(C65ConstantOffset);
protected:
	int32	miOffset;
public:
	void	Init(int32 iOffset);

	uint32	GetOffset(C65C64* pcCPU) override;
	void	Print(CChars* psz) override;
	bool	IsConstantOffset(void) override;
};


#endif // __65_CONSTANT_OFFSET_H__

