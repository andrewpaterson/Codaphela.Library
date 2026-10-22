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
#ifndef __65_DATA_OPERATION_H__
#define __65_DATA_OPERATION_H__
#include "BaseLib/PrimitiveTypes.h"
#include "65Operation.h"


#define BYTE_LANE_LOWER		0x01
#define BYTE_LANE_UPPER		0x02
#define BYTE_LANES_BOTH		(BYTE_LANE_LOWER | BYTE_LANE_UPPER)


//Every bus cycle has exactly one data operation.  It says what the cycle does on the bus.
class C65DataOperation : public C65Operation
{
protected:
	bool	mbOpcode;
	bool	mbProgramAddress;
	bool	mbDataAddress;
	bool	mbRead;
	bool	mbNotVectorPull;

public:
			void	Init(bool bOpcode, bool bProgramAddress, bool bDataAddress, bool bRead, bool bNotVectorPull);

			bool	IsData(void) override;
			bool	IsOpcode(void);
			bool	IsProgramAddress(void);
			bool	IsDataAddress(void);
			bool	IsRead(void);
			bool	IsNotVectorPull(void);
	virtual uint8	GetByteLanes(C65C64* pcCPU);
};


#endif // __65_DATA_OPERATION_H__

