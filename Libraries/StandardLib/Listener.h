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
#ifndef __LISTENER_H__
#define __LISTENER_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/ArrayTemplateEmbeddedPtr.h"


class CListenerCall;
typedef CArrayTemplateEmbeddedPtr<CListenerCall, 2>	CArrayEmbeddedListenerCallPtr;


//A listener knows the calls it is registered with and a call knows its listeners.  Whichever goes first unhooks itself
//from the other so neither is left holding a pointer to freed memory.
//
//A listener that is destroyed with its C++ destructor (on the stack, a member of a plain class) unhooks itself.  One
//whose memory is freed without its destructor running (an object or an unknown) must call RemoveFromListenerCalls in
//its Kill or Free.
class CListener : public CConstructable
{
CONSTRUCTABLE(CListener)
private:
	CArrayEmbeddedListenerCallPtr	mapcCalls;

public:
			CListener(void);
			CListener(const CListener& cOther);
			~CListener(void);
	CListener&	operator=(const CListener& cOther);

	void	RemoveFromListenerCalls(void);
	size	NumListenerCalls(void);

protected:
	friend class CListenerCall;
	void	AddListenerCall(CListenerCall* pcCall);
	void	RemoveListenerCall(CListenerCall* pcCall);
};


#endif // __LISTENER_H__

