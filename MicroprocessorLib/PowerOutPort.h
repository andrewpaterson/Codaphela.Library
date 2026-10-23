/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela MicroprocessorLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MicroprocessorLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MicroprocessorLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __POWER_OUT_PORT_H__
#define __POWER_OUT_PORT_H__
#include "Port.h"


//Logicim's PowerOutPort: drives its trace with a fixed voltage, as a power supply does.
class CPowerOutPort : public CPort
{
CONSTRUCTABLE(CPowerOutPort);
protected:
	float32		mfVoltage;

public:
			void	Init(const char* szName, CPortHolder* pcHolder, float32 fVoltage);

	virtual float32	GetVoltageOut(int64 iTime);
	virtual void	AppendDescription(CChars* psz);
};


#endif // __POWER_OUT_PORT_H__

