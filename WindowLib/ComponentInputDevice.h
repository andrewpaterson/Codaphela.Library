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
#ifndef __COMPONENT_INPUT_DEVICE_H__
#define __COMPONENT_INPUT_DEVICE_H__
#include "WindowInputSource.h"
#include "ComponentInputCommand.h"
#include "ComponentInputPrintable.h"
#include "ComponentInputNumKey.h"
#include "ComponentInputKey.h"


//What the sources of one input device mean to components.  Its sources are
//found by name through CComponentInput's CWindowInputSource.  Sources the
//device doesn't have are skipped so any keyboard or mouse can be used.
class CComponentInput;
class CComponentInputDevice
{
protected:
	CComponentInput*					mpcComponentInput;
	CInputDevice*						mpcDevice;
	CArrayComponentInputPrintable		masPrintables;
	CArrayComponentInputCommand			masCommands;
	CArrayComponentInputNumKey			masNumKeys;
	CArrayComponentInputKey				masKeys;
	bool								mbMouse;

public:
	void						Init(CInputDevice* pcDevice, CComponentInput* pcComponentInput);
	void						Kill(void);

	void						InitKeyboard(void);
	void						InitMouse(void);

	CComponentInputPrintable*	GetPrintable(CInputSourceDesc* pcSource);
	CComponentInputCommand*		GetCommand(CInputSourceDesc* pcSource, bool bPressed);
	bool						IsKeyDown(EComponentInputKey eKey);
	bool						IsMouse(void);
	CComponentInputNumKey*		GetNumKeyRemap(CInputSourceDesc* pcSource);
	CComponentInputKey*			GetKey(CInputSourceDesc* pcSource);
	CInputDevice*				GetDevice(void);

	CInputSourceDesc*			AddCommand(char* szFriendlyName, EComponentInputCommand eCommand, bool bPressed = true);
	CInputSourceDesc*			AddPrintable(char* szFriendlyName, char cNoShiftNoCaps, char cYesShiftNoCaps, char cNoShiftYesCaps, char cYesShiftYesCaps);
	CInputSourceDesc*			AddNumKeyRemap(char* szFriendlyName, char* szYesNumLockRemap, char* szNoNumLockRemap);
	CInputSourceDesc*			AddKey(char* szFriendlyName, EComponentInputKey eKey);

protected:
public:
	CInputSourceDesc*			GetSource(char* szFriendlyName);
};


typedef CArrayTemplate<CComponentInputDevice>	CArrayComponentInputDevice;


#endif // __COMPONENT_INPUT_DEVICE_H__

