/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifndef __WIN_REF_INPUT_H__
#define __WIN_REF_INPUT_H__
#include "InputLib/NativeInput.h"


//The reference system's keyboard and mouse: the standard ones a real platform has (WinInputLib's XML describes them),
//so a program sees the devices it would see on screen, but nothing is ever pressed or moved on them.  A program run on
//the reference screen is driven some other way, by a script say.
class CWinRefInput : public CNativeInput
{
public:
	void	Init(void);
	void	Kill(void) override;
	void	AddDevicesTo(CInputDevices* pcDevices) override;
	void	Update(CInputDevices* pcDevices, uint32 uiSequence) override;

protected:
	void	AddDevice(CInputDevices* pcDevices, const char* szDescription);
};


#endif // __WIN_REF_INPUT_H__

