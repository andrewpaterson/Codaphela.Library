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
#include "InputLib/InputVirtualDevice.h"
#include "ComponentInputCommand.h"
#include "ComponentInputPrintable.h"
#include "ComponentInputMotion.h"
#include "ComponentInputNumKey.h"


class CComponentInput;
class CComponentInputDevice : public CUnknown
{
CONSTRUCTABLE(CComponentInputDevice);
protected:
	CComponentInput*			mpcComponentInput;

	CInputVirtualDevice*		mpcVirtual;
	CInputDevice*				mpcDevice;

	CArrayTextEditorPrintable	masPrintables;
	CArrayComponentInputCommand	masCommands;
	CArrayComponentInputNumKey	masNumKeys;
	CArrayComponentInputMotion	masMotions;

public:
	virtual void 						Init(CInputDevice* pcDevice, CComponentInput* pcComponentInput) =0;
	void 								InitDevice(CInputVirtualDevice* pcVirtual, CInputDevice* pcDevice, CComponentInput* pcComponentInput);
	void 								Kill(void);

	void 								AddPrintable(CInputSourceDesc* pcSource, char cNoShiftNoCaps, char cYesShiftNoCaps, char cNoShiftYesCaps, char cYesShiftYesCaps);
	virtual CComponentInputPrintable*	GetPrintable(CInputSourceDesc* pcSource);
	void								AddCommand(EComponentInputCommand eCommand, CInputSourceDesc* pcSource, BOOL bControl, BOOL bAlt, BOOL bShift, BOOL bPressed);
	virtual CComponentInputCommand*		GetCommand(CInputSourceDesc* pcSource, BOOL bControl, BOOL bAlt, BOOL bShift, BOOL bPressed);
	void								AddNumKeyRemap(CInputSourceDesc* pcSource, CInputSourceDesc* pcYesNumLockRemap, CInputSourceDesc* pcNoNumLockRemap);
	virtual CComponentInputNumKey*		GetNumKeyRemap(CInputSourceDesc* pcSource);
	void								AddMotion(CInputSourceDesc* pcSource, float fXMultiplier, float fYMultiplier);
	virtual CComponentInputMotion*		GetMotion(CInputSourceDesc* pcSource);

	CInputVirtualDeviceSource*			AddButtonPressedAction(char* szFriendlyName, EComponentInputCommand eCommand, BOOL bControl, BOOL bAlt, BOOL bShift);
	CInputVirtualDeviceSource*			AddButtonPressedAction(EComponentInputCommand eCommand, char* szFriendlyName, BOOL bControl, BOOL bAlt, BOOL bShift);
	CInputVirtualDeviceSource*			AddButtonReleasedAction(char* szFriendlyName, EComponentInputCommand eCommand, BOOL bControl, BOOL bAlt, BOOL bShift);
	CInputVirtualDeviceSource*			AddRangeAction(char* szFriendlyName);
	CInputVirtualDeviceSource*			AddPrintable(char* szFriendlyName, char cNoShiftNoCaps, char cYesShiftNoCaps, char cNoShiftYesCaps, char cYesShiftYesCaps);

	BOOL								HasSpecificAction(CInputVirtualDeviceSource* pcSource, EBasicActionActive eActionType);
};


#endif // __COMPONENT_INPUT_DEVICE_H__

