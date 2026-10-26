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
#ifndef __SPLITTER_H__
#define __SPLITTER_H__
#include "BaseLib/ArrayChars.h"
#include "Pin.h"


//Logicim's Splitter: a trace port for each bit of the bus at its start and one for each fan out at its end.
class CSplitter : public CPassive
{
CONSTRUCTABLE(CSplitter);
protected:
	CArrayTracePortPtr	mapcStartPorts;
	CArrayTracePortPtr	mapcEndPorts;

public:
			void			Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, CArrayChars* paszStartPortNames, CArrayChars* paszEndPortNames);
			void			Class(void) override;
			void			Free(void) override;

	virtual const char*		GetType(void);
			size			NumStartPorts(void);
			CTracePort*		GetStartPort(size uiIndex);
			size			NumEndPorts(void);
			CTracePort*		GetEndPort(size uiIndex);

protected:
			void			AddTracePorts(CArrayTracePortPtr* papcPorts, CArrayChars* paszPortNames);
};


#endif // __SPLITTER_H__

