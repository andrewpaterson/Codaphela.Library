#ifndef __X11_INPUT_H__
#define __X11_INPUT_H__
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
#include "BaseLib/ArrayTemplate.h"
#include "InputLib/NativeInput.h"


//The X keyboard and mouse events are turned into Windows raw input events so that the standard keyboard and mouse
//described by WinInputLib's XML (and CComponentInput's keyboard map) work unchanged.  Laid out as SRawMouseEvent and
//SRawKeyboardEvent in WinInputLib, which follow the 'WinRAW Mouse' and 'WinRAW Keyboard' formats in WindowsFormats.xml.
struct SX11RawMouseEvent
{
	uint16		usFlags;
	uint16		usPadding;
	uint16		usButtonFlags;
	int16		sButtonData;  //The wheel's movement when usButtonFlags has the wheel flag.
	uint32		ulRawButtons;
	int32		lLastX;
	int32		lLastY;
	uint32		ulExtraInformation;
};


struct SX11RawKeyboardEvent
{
	uint16		usMakeCode;
	uint16		usFlags;
	uint16		usReserved;
	uint16		usVKey;
	uint32		uiMessage;
	uint32		uiExtraInformation;
};


struct SX11InputEvent
{
	bool	bKeyboard;
	union
	{
		SX11RawMouseEvent		sMouse;
		SX11RawKeyboardEvent	sKeyboard;
	} uEvent;
};


typedef CArrayTemplate<SX11InputEvent> CArrayX11InputEvent;


class CX11Input : public CNativeInput
{
protected:
	CArrayX11InputEvent		masEvents;
	int						miKeyboard;
	int						miMouse;

public:
	void	Init(void);
	void	Kill(void) override;

	void	AddDevicesTo(CInputDevices* pcDevices) override;
	void	Update(CInputDevices* pcDevices, uint32 uiSequence) override;

	void	Key(unsigned int uiKeyCode, bool bPressed);
	void	MouseButton(unsigned int uiButton, bool bPressed);
	void	MouseMoved(int iDeltaX, int iDeltaY);

protected:
	SX11RawMouseEvent*	AddMouseEvent(void);
};


#endif // __X11_INPUT_H__

