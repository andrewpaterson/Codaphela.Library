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
#ifndef __POWER_IN_PORT_H__
#define __POWER_IN_PORT_H__
#include "Port.h"


//Logicim's PowerInPort: a chip's VCC or GND, which reads its trace's voltage.
class CPowerInPort : public CPort
{
CONSTRUCTABLE(CPowerInPort);
public:
			void	Init(EPortType eType, const char* szName, CPortHolder* pcHolder);

	virtual bool	IsPowerIn(void);
			float32	GetVoltageIn(int64 iTime);
};


#endif // __POWER_IN_PORT_H__

