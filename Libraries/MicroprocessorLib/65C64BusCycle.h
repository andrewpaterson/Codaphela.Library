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
#ifndef __65_BUS_CYCLE_H__
#define __65_BUS_CYCLE_H__
#include "BaseLib/ArrayTemplateEmbeddedPtr.h"
#include "ArrayOwnedPtr.h"
#include "StandardLib/Unknown.h"
#include "65C64AddressOffsetArray.h"
#include "65C64Operation.h"
#include "65C64DataOperation.h"


class C65C64BusCycle : public CUnknown
{
CONSTRUCTABLE(C65C64BusCycle);
protected:
	C65C64AddressOffsetArray	mapcAddressOffsets;
	C65OperationArray		mapcOperations;
	CArrayTemplateEmbeddedPtr<C65C64Operation, 2>	mapcConditions;  //The operations that can skip this cycle, usually none.
	C65C64DataOperation*		mpcDataOperation;
	int16					miCycle;

public:
	void					Init(C65C64AddressOffsetArray* papcAddressOffsets, C65OperationArray* papcOperations);
	void					Kill(void) override;

	uint32					GetAddress(C65C64* pcCPU);
	C65C64DataOperation*		GetDataOperation(void);
	bool					MustExecute(C65C64* pcCPU);
	void					ExecuteInitialSide(C65C64* pcCPU);
	void					ExecuteTrailingSide(C65C64* pcCPU, bool bSkipData);
	bool					IsProgramAddress(void);
	void					ToAddressOffsetString(CChars* psz);
	void					ToOperationString(CChars* psz);
	void					SetCycle(int16 iCycle);
	int16					GetCycle(void);
	C65OperationArray*		GetOperations(void);

protected:
	void					ValidateComponent(void);
};


typedef CArrayOwnedPtr<C65C64BusCycle>	C65BusCycleArray;


#endif // __65_BUS_CYCLE_H__

