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
#ifndef __INPUT_H__
#define __INPUT_H__
#include "BaseLib/ArrayTemplate.h"
#include "StandardLib/Unknown.h"
#include "InputDevices.h"
#include "InputActions.h"
#include "WinInput.h"
#include "ProgramInput.h"
#include "InputDeviceValue.h"


class CInput : public CUnknown
{
CONSTRUCTABLE(CInput);
protected:
	CInputDevices		mcDevices;
	CInputActions		mcActions;
	unsigned int		muiSequence;  //Sequence is updated everytime CInput::Update is called.  Not every event.

	CWinInput			mcWinInput;  //Fix this later.
	CProgramInput		mcProgramInput;

public:
	void 				Init(void);
	void 				Kill(void);
	BOOL 				Read(char* szDirectory = NULL);
	void 				AddProgramInput(void);
	void 				AddWinInput(HWND hWnd, BOOL bExclusive);

	void 				Update(void);
	void 				Step(void);
	void 				ClearStateFlags(void);
	void 				UpdateRestEvents(void);
	void 				CallListeners(void);
	void 				ClearEvents(void);
	void 				SortEvents(void);
	void 				UpdateVirtualDevicesEvents(void);
	void 				DumpEvents(void);
	CInputCategory*		GetCategory(char* szName);

	CWinInput*			GetWinInput(void);
	CProgramInput*		GetProgramInput(void);
	CInputDevices*		GetDevices(void);
	CInputActions*		GetActions(void);
};


#endif // !__INPUT_H__

