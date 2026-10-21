/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __COMPONENT_INPUT_LISTENER_H__
#define __COMPONENT_INPUT_LISTENER_H__
#include "StandardLib/Listener.h"
#include "StandardLib/Unknown.h"


//CAction calls listeners through a member function of a CListener so CComponentInput can't be the listener itself.
class CComponentInput;
class CComponentInputListener : public CListener
{
CONSTRUCTABLE(CComponentInputListener);
protected:
	CComponentInput*	mpcComponentInput;

public:
			void Init(CComponentInput* pcComponentInput);
	virtual void ButtonPressed(CUnknown* pcSource, void* pvContext);
	virtual void ButtonReleased(CUnknown* pcSource, void* pvContext);
};


#endif // __COMPONENT_INPUT_LISTENER_H__

