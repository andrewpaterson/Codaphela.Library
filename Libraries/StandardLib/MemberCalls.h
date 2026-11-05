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
#ifndef __MEMBER_CALLS_H__
#define __MEMBER_CALLS_H__
#include "BaseLib/ArrayTemplate.h"
#include "MemberCall.h"


//Member functions called one after another with the same argument and value, as an action or a component event calls
//them (see SMemberCall).  The functions' objects aren't held, so an object removes its functions (Remove(this)) before
//it goes.
class CMemberCalls
{
protected:
	CArrayTemplate<SMemberCall>	masCalls;

public:
	void			Init(void);
	void			Kill(void);

	SMemberCall*	Add(void);
	bool			Remove(void* pvObject);
	size			NumCalls(void);

	void			Call(void* pvArgument, void* pvValue);

protected:
	bool			Contains(SMemberCall* psCall);
};


#endif // __MEMBER_CALLS_H__

