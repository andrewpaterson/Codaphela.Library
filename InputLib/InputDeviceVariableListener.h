/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela InputLib

Codaphela InputLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela InputLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela InputLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __INPUT_DEVICE_VARIABLE_LISTENER_H__
#define __INPUT_DEVICE_VARIABLE_LISTENER_H__
#include "StandardLib/Listener.h"
#include "StandardLib/Unknown.h"


#define INPUT_DEVICE_VARIABLE_ACTION "Variable Action"


class CInputDeviceVariableListener : public CListener
{
public:
	BASE_FUNCTIONS(CInputDeviceVariableListener);

	virtual void VariableAction(CUnknown* pcSource, void* pvContext) {};
};


#endif // __INPUT_DEVICE_VARIABLE_LISTENER_H__
