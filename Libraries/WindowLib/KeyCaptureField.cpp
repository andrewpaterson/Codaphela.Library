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
#include "BaseLib/StringHelper.h"
#include "StandardLib/ClassDefines.h"
#include "ComponentInput.h"
#include "Window.h"
#include "KeyCaptureField.h"


//The keys that only change what other keys mean, or a lock's state: nothing to bind.
static const char*	gaszKeyCaptureIgnored[] = { "Left Shift", "Right Shift", "Left Control", "Right Control", "Left Alt", "Right Alt", "Caps Lock", "Num Lock" };


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKeyCaptureField::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme)
{
	PreInit();
	CTextField::Init(pWindow, pTheme, "");
	mszKeys.Init();
	mcCaptured.Init();
	AddInputAction(IAF_AnyKeyDown, NULL, this, &CKeyCaptureField::KeyDownAction);
	AddInputAction(IAF_AnyChord, NULL, this, &CKeyCaptureField::ChordAction);
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKeyCaptureField::Class(void)
{
	CTextField::Class();
	U_String(mszKeys);
	U_Unknown(CComponentEvent<CKeyCaptureField>, mcCaptured);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKeyCaptureField::Free(void)
{
	mcCaptured.Kill();
	mszKeys.Kill();
	CTextField::Free();
}


//////////////////////////////////////////////////////////////////////////
//
// A field isn't saved.
//
//////////////////////////////////////////////////////////////////////////
bool CKeyCaptureField::Save(CObjectWriter* pcFile) { return false; }
bool CKeyCaptureField::Load(CObjectReader* pcFile) { return false; }


char* CKeyCaptureField::GetKeys(void) { return mszKeys.Text(); }
CComponentEvent<CKeyCaptureField>* CKeyCaptureField::GetCapturedEvent(void) { return &mcCaptured; }


//////////////////////////////////////////////////////////////////////////
//
// szKeys are shown as a menu shows them, and the captured event is called.
// Keys that can't be read are the same as none.
//
//////////////////////////////////////////////////////////////////////////
void CKeyCaptureField::SetKeys(const char* szKeys)
{
	CChars	szText;

	mszKeys.Clear();
	szText.Init();
	if (!StrEmpty(szKeys) && CComponentInput::NameKeys(szKeys, &mszKeys))
	{
		CComponentInput::AppendShortcutText(&szText, mszKeys.Text());
	}
	SetText(szText.Text());
	szText.Kill();
	mcCaptured.Call(this);
}


//////////////////////////////////////////////////////////////////////////
//
// A key that isn't part of a chord the keyboard knows.  Its modifiers are
// read as it goes down.
//
//////////////////////////////////////////////////////////////////////////
void CKeyCaptureField::KeyDownAction(SComponentInputEvent* psEvent)
{
	CChars				szKeys;
	size				i;
	bool				bControl;
	bool				bAlt;
	bool				bShift;

	for (i = 0; i < sizeof(gaszKeyCaptureIgnored) / sizeof(gaszKeyCaptureIgnored[0]); i++)
	{
		if (StringCompare(psEvent->szName, gaszKeyCaptureIgnored[i]) == 0)
		{
			return;
		}
	}

	bControl = psEvent->bControl;
	bAlt = psEvent->bAlt;
	bShift = psEvent->bShift;
	if (!bControl && !bAlt && !bShift && ((StringCompare(psEvent->szName, "Escape") == 0) || (StringCompare(psEvent->szName, "Enter") == 0) || (StringCompare(psEvent->szName, "Numpad Enter") == 0)))
	{
		psEvent->bPass = true;
		return;
	}

	szKeys.Init();
	szKeys.Append(bControl ? "Control+" : "");
	szKeys.Append(bAlt ? "Alt+" : "");
	szKeys.Append(bShift ? "Shift+" : "");
	szKeys.Append(psEvent->szName);
	SetKeys(szKeys.Text());
	szKeys.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A chord the keyboard knows comes whole, without its keys going down.
//
//////////////////////////////////////////////////////////////////////////
void CKeyCaptureField::ChordAction(SComponentInputEvent* psEvent)
{
	SetKeys(psEvent->szName);
}

