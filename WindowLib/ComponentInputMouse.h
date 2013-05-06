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
#ifndef __COMPONENT_INPUT_MOUSE_H__
#define __COMPONENT_INPUT_MOUSE_H__
#include "InputLib/Input.h"
#include "ComponentInputDevice.h"


class CComponentInputMouse : public CComponentInputDevice
{
public:
	BASE_FUNCTIONS(CComponentInputMouse);

	CInputVirtualDeviceSource*	mButton1;
	CInputVirtualDeviceSource*	mButton1Released;
	CInputVirtualDeviceSource*	mButton2;
	CInputVirtualDeviceSource*	mButton3;
	CInputVirtualDeviceSource*	mButton3Released;
	CInputVirtualDeviceSource*	mWheelForward;
	CInputVirtualDeviceSource*	mWheelBackward;
	CInputVirtualDeviceSource*	mLeft;
	CInputVirtualDeviceSource*	mRight;
	CInputVirtualDeviceSource*	mUp;
	CInputVirtualDeviceSource*	mDown;

	void 						Init(CInputDevice* pcDevice, CComponentInput* pcComponentInput);
	void						InitMouse(CInput* pcInput);
	void 						Kill(void);
};


#endif // __COMPONENT_INPUT_MOUSE_H__

