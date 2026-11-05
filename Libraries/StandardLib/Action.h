/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

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
#ifndef __ACTION_H__
#define __ACTION_H__
#include "BaseLib/Chars.h"
#include "Unknown.h"
#include "MemberCalls.h"


//Something named that can happen, like a chord being pressed or a timer going off, and the member functions called
//when it does (see CMemberCalls).  A function takes the action and a context that depends on what made it happen,
//like an input chord's event or a timer's STimerData.
//
//The functions' objects aren't held, so an object removes its functions (Remove(this)) before it goes.
class CAction : public CUnknown
{
CONSTRUCTABLE(CAction)
protected:
	CMemberCalls	mcCalls;
	CChars			mszName;

public:
	void 	Init(const char* szName);
	void 	Kill(void);

	template<class M, class O>
	void	Add(O* pcObject, void(M::*fFunction)(CAction* pcAction, void* pvContext));
	bool	Remove(void* pvObject);
	size	NumCalls(void);

	void	Call(void* pvContext);

	bool	Is(const char* szName);
	char*	GetName(void);
};


//////////////////////////////////////////////////////////////////////////
//
// pcObject is usually the object adding it, this.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class O>
void CAction::Add(O* pcObject, void(M::*fFunction)(CAction* pcAction, void* pvContext))
{
	mcCalls.Add()->Init(pcObject, fFunction);
}


#endif // __ACTION_H__

