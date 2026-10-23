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
#ifndef __STATE_H__
#define __STATE_H__
#include "BaseLib/Constructable.h"


//Logicim's State: what a CIntegratedCircuit remembers between events, as a flip-flop its output.  Made with NewMalloc
//by the chip's CreateState; the chip owns it.
class CState : public CConstructable
{
CONSTRUCTABLE(CState);
public:
	virtual void	Kill(void);
	virtual bool	IsStateless(void);
};


//Logicim's Stateless: the state of a chip that remembers nothing, as a gate.
class CStateless : public CState
{
CONSTRUCTABLE(CStateless);
public:
	virtual bool	IsStateless(void);
};


#endif // __STATE_H__

