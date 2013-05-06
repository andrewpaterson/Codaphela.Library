/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __INPUT_STATE_H__
#define __INPUT_STATE_H__
#include "BaseLib/ArrayTemplate.h"
#include "InputDeviceDesc.h"


#define SOURCE_STATE_VALID			0x01
#define SOURCE_STATE_EMITTED_EVENT	0x02
#define SOURCE_STATE_RESTING		0x04


struct SInputSourceState
{
public:
	CInputSourceDesc*	pcDesc;
	float				fValue;
	int					iFlags;
};


typedef CArrayTemplate<SInputSourceState>	CArrayInputSourceState;


class CInputDeviceValues;
class CInputDevice;
class CInputDeviceState
{
public:
	CArrayInputSourceState	macSourceStates;
	CInputDevice*			mpcDevice;

	void 	Init(void);
	void 	Kill(void);
	BOOL 	Rest(CInputDevice* pcDevice);
	BOOL	GetValue(CInputSourceDesc* pcSourceDesc, float* pfValue);
	BOOL	SetValue(CInputSourceDesc* pcSourceDesc, float fValue);
	BOOL	SetValue(SInputSourceState* psSourceState, float fValue);
	void	ClearFlags(void);
	void	UpdateRestEvents(CInputDeviceValues* pcEvents, unsigned int uiSequence);
};


#endif // __INPUT_STATE_H__

