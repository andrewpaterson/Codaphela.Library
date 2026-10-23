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
#ifndef __PORT_HOLDER_H__
#define __PORT_HOLDER_H__
#include "Port.h"


//Logicim's PortHolder: owns its ports, see CPort.  CPins is the holder of a chip's ports.
class CCircuitComponent;
class CPortHolder
{
protected:
	CArrayPortPtr	mapcPorts;

public:
			void	Init(void);
	virtual void	Kill(void);

			void	AddPort(CPort* pcPort);
			CPort*	GetPort(const char* szName);
			size	NumPorts(void);
			CPort*	GetPort(size uiIndex);

	virtual void	TraceConnected(CTimeline* pcTimeline, CPort* pcPort) =0;
	virtual char*	GetName(void) =0;
	virtual void	AppendDescription(CChars* psz) =0;
	virtual CCircuitComponent*	GetComponent(void) =0;
};


#endif // __PORT_HOLDER_H__

