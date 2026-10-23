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
#include "65AddressOffsetArray.h"
#include "65Operation.h"
#include "65DataOperation.h"


class C65BusCycle : public CUnknown
{
CONSTRUCTABLE(C65BusCycle);
protected:
	C65AddressOffsetArray	mapcAddressOffsets;
	C65OperationArray		mapcOperations;
	CArrayTemplateEmbeddedPtr<C65Operation, 2>	mapcConditions;  //The operations that can skip this cycle, usually none.
	C65DataOperation*		mpcDataOperation;
	int16					miCycle;

public:
	void					Init(C65AddressOffsetArray* papcAddressOffsets, C65OperationArray* papcOperations);
	void					Kill(void) override;

	uint32					GetAddress(C65C64* pcCPU);
	C65DataOperation*		GetDataOperation(void);
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
	void					Validate(void);
};


typedef CArrayOwnedPtr<C65BusCycle>	C65BusCycleArray;


#endif // __65_BUS_CYCLE_H__

