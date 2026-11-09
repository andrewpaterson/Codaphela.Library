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
#ifndef __KEY_CAPTURE_FIELD_H__
#define __KEY_CAPTURE_FIELD_H__
#include "TextField.h"


//A field that shows the keys pressed in it rather than typing them: a key or a chord of control, alt and shift with a
//key, as CComponentInput::BindKeys reads them.  It has them before they can mean anything else, so a command bound to
//them doesn't run.  Modifiers pressed alone are waited through.  Plain escape and enter go on, to the dialog's Cancel
//and OK.
class CKeyCaptureField : public CTextField
{
CONSTRUCTABLE(CKeyCaptureField);
DESTRUCTABLE(CKeyCaptureField);
protected:
	CChars							mszKeys;  //Named as CComponentInput names them, empty for none yet.
	CComponentEvent<CKeyCaptureField>	mcCaptured;

public:
	void	Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme);
	void	Class(void) override;
	void	Free(void) override;

	bool	Save(CObjectWriter* pcFile) override;
	bool	Load(CObjectReader* pcFile) override;

	char*	GetKeys(void);
	void	SetKeys(const char* szKeys);

	CComponentEvent<CKeyCaptureField>*	GetCapturedEvent(void);

	void	KeyDownAction(SComponentInputEvent* psEvent);
	void	ChordAction(SComponentInputEvent* psEvent);
};


#endif // __KEY_CAPTURE_FIELD_H__

