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
#ifndef __INPUT_VIRTUAL_DEVICE_SOURCE_H__
#define __INPUT_VIRTUAL_DEVICE_SOURCE_H__
#include "StandardLib/Unknown.h"
#include "StandardLib/SetType.h"
#include "InputDeviceValue.h"
#include "InputSourceDesc.h"


enum EInputDevice
{
	EID_Any = -1,		//Special case.  If any source is 'any' then every source must be 'any'.
	EID_Device1 = 0,	//Count into the number of devices of description to initialise this virtual device with.
	EID_Device2,		//Eg: 4 wired XBox controllers would be devices interger values 0 ... 3.
	EID_Device3,		//Eg: 2 Mouses, a logitech and a microsoft mouse would have different descriptions and both be device value 0.
	EID_Device4,
	//...
};


class CInputDevice;
class CInputVirtualDeviceSource : public CUnknown
{
CONSTRUCTABLE(CInputVirtualDeviceSource);
protected:
	SInputDeviceValueSource		msSource;

public:
	void 						Init(CInputDevice* pcSourceDevice, CInputSourceDesc* pcSourceDesc);
	void 						Kill(void);
	BOOL 						Equals(SInputDeviceValueSource* psOther);
	CInputSourceDesc*			GetSourceDesc(void);
	SInputDeviceValueSource*	GetSource(void);
	CInputDevice* 				GetSourceDevice(void);
};


typedef CSetType<CInputVirtualDeviceSource>	CSetInputVirtualDeviceSource;


#endif // __INPUT_VIRTUAL_DEVICE_SOURCE_H__

