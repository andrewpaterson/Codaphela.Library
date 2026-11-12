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
#include "Command.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCommand::Init(const char* szName, const char* szCaption, const char* szKeys, Ptr<CImageCel> pIcon, SMemberCall* psRun, bool bRunAsks)
{
	PreInit();
	mszName.Init(szName);
	mszCaption.Init(szCaption);
	maszKeys.Init();
	maszDefaultKeys.Init();
	maszBoundKeys.Init();
	meKeyScope = CKS_Application;
	AddDefaultKeys(szKeys);
	mpIcon = pIcon;
	msRun = *psRun;
	mbRunAsks = bRunAsks;
	msEnabled.Clear();
	msChecked.Clear();
	meStandard = CIC_Ignored;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCommand::Class(void)
{
	U_String(mszName);
	U_String(mszCaption);
	U_Unknown(CArrayChars, maszKeys);
	U_Unknown(CArrayChars, maszDefaultKeys);
	U_Unknown(CArrayChars, maszBoundKeys);
	U_Enum(meKeyScope);
	M_Pointer(mpIcon);
	U_Unknown(SMemberCall, msRun);
	U_Unknown(SMemberCall, msEnabled);
	U_Unknown(SMemberCall, msChecked);
	U_Enum(meStandard);
	U_Bool(mbRunAsks);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCommand::Free(void)
{
	maszBoundKeys.Kill();
	maszDefaultKeys.Kill();
	maszKeys.Kill();
	mszCaption.Kill();
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A command isn't saved; the application makes its commands each run.
//
//////////////////////////////////////////////////////////////////////////
bool CCommand::Save(CObjectWriter* pcFile) { return false; }
bool CCommand::Load(CObjectReader* pcFile) { return false; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCommand::IsEnabled(void)
{
	return !msEnabled.IsSet() || msEnabled.Ask();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the command turns something on and off, and whether it is on.
//
//////////////////////////////////////////////////////////////////////////
bool CCommand::IsCheckable(void) { return msChecked.IsSet(); }
bool CCommand::IsChecked(void) { return msChecked.IsSet() && msChecked.Ask(); }


//////////////////////////////////////////////////////////////////////////
//
// False, and nothing is run, if the command can't run now.
//
//////////////////////////////////////////////////////////////////////////
bool CCommand::Run(void)
{
	if (!IsEnabled())
	{
		return false;
	}
	if (mbRunAsks)
	{
		msRun.Ask();
	}
	else
	{
		msRun.Call(this);  //Only a function given the command takes it.
	}
	return true;
}


void CCommand::SetStandardCommand(EComponentInputCommand eStandard) { meStandard = eStandard; }  //A command without keys of its own shows eStandard's.
EComponentInputCommand CCommand::GetStandardCommand(void) { return meStandard; }
char* CCommand::GetName(void) { return mszName.Text(); }
char* CCommand::GetCaption(void) { return mszCaption.Text(); }
char* CCommand::GetKeys(void) { return (maszKeys.NumElements() > 0) ? maszKeys.GetText(0) : (char*)""; }  //The first of them, those a menu shows.  "" for none.
size CCommand::NumKeys(void) { return maszKeys.NumElements(); }
char* CCommand::GetKeys(size uiIndex) { return maszKeys.GetText(uiIndex); }
size CCommand::NumDefaultKeys(void) { return maszDefaultKeys.NumElements(); }
char* CCommand::GetDefaultKeys(size uiIndex) { return maszDefaultKeys.GetText(uiIndex); }
void CCommand::SetKeyScope(ECommandKeyScope eScope) { meKeyScope = eScope; }  //A scope other than CKS_Application needs a standard command (SetStandardCommand).
ECommandKeyScope CCommand::GetKeyScope(void) { return meKeyScope; }
Ptr<CImageCel> CCommand::GetIcon(void) { return mpIcon; }


//////////////////////////////////////////////////////////////////////////
//
// Whether szKeys, written any way CComponentInput reads, are the command's.
//
//////////////////////////////////////////////////////////////////////////
bool CCommand::HasKeys(const char* szKeys)
{
	CChars	szName;
	bool	bResult;

	szName.Init();
	bResult = !StrEmpty(szKeys) && CComponentInput::NameKeys(szKeys, &szName) && maszKeys.Contains(szName.Text());
	szName.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Keys the command has unless a user says otherwise.  NULL or "" adds
// none, and keys that can't be read aren't added.
//
//////////////////////////////////////////////////////////////////////////
void CCommand::AddDefaultKeys(const char* szKeys)
{
	CChars	szName;

	szName.Init();
	if (!StrEmpty(szKeys) && CComponentInput::NameKeys(szKeys, &szName) && !maszDefaultKeys.Contains(szName.Text()))
	{
		maszDefaultKeys.Add(szName.Text());
		if (!maszKeys.Contains(szName.Text()))
		{
			maszKeys.Add(szName.Text());
		}
	}
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Another keys for the command, once they're next bound.  False if they
// can't be read, or the command has them already.
//
//////////////////////////////////////////////////////////////////////////
bool CCommand::AddKeys(const char* szKeys)
{
	CChars	szName;
	bool	bResult;

	szName.Init();
	bResult = !StrEmpty(szKeys) && CComponentInput::NameKeys(szKeys, &szName) && !maszKeys.Contains(szName.Text());
	if (bResult)
	{
		maszKeys.Add(szName.Text());
	}
	szName.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The command no longer has szKeys, once they're next bound.
//
//////////////////////////////////////////////////////////////////////////
void CCommand::RemoveKeys(const char* szKeys)
{
	CChars	szName;

	szName.Init();
	if (!StrEmpty(szKeys) && CComponentInput::NameKeys(szKeys, &szName) && maszKeys.Contains(szName.Text()))
	{
		maszKeys.Remove(szName.Text());
	}
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCommand::SetKeys(CArrayChars* paszKeys)
{
	size	i;

	ClearKeys();
	for (i = 0; i < paszKeys->NumElements(); i++)
	{
		AddKeys(paszKeys->GetText(i));
	}
}


void CCommand::ClearKeys(void) { maszKeys.ReInit(); }


//////////////////////////////////////////////////////////////////////////
//
// The keys it was made with again, once they're next bound.
//
//////////////////////////////////////////////////////////////////////////
void CCommand::ResetKeys(void)
{
	SetKeys(&maszDefaultKeys);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether its keys are those it was made with, in any order.
//
//////////////////////////////////////////////////////////////////////////
bool CCommand::HasDefaultKeys(void)
{
	size	i;

	if (maszKeys.NumElements() != maszDefaultKeys.NumElements())
	{
		return false;
	}
	for (i = 0; i < maszKeys.NumElements(); i++)
	{
		if (!maszDefaultKeys.Contains(maszKeys.GetText(i)))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Gives pcInput the command's keys, as its scope says.  False if any
// couldn't be bound.
//
//////////////////////////////////////////////////////////////////////////
bool CCommand::Bind(CComponentInput* pcInput)
{
	size	i;
	bool	bResult;
	char*	szKeys;

	bResult = true;
	for (i = 0; i < maszKeys.NumElements(); i++)
	{
		szKeys = maszKeys.GetText(i);
		switch (meKeyScope)
		{
		case CKS_Application:
			bResult &= pcInput->BindKeys(szKeys, CIC_Command);
			break;
		case CKS_Window:
			bResult &= pcInput->BindKeys(szKeys, meStandard);
			break;
		case CKS_Editor:
			bResult &= pcInput->BindEditorKeys(szKeys, meStandard);
			break;
		}
		maszBoundKeys.Add(szKeys);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Takes back from pcInput the keys Bind gave it.
//
//////////////////////////////////////////////////////////////////////////
void CCommand::Unbind(CComponentInput* pcInput)
{
	size	i;

	for (i = 0; i < maszBoundKeys.NumElements(); i++)
	{
		if (meKeyScope == CKS_Editor)
		{
			pcInput->UnbindEditorKeys(maszBoundKeys.GetText(i));
		}
		else
		{
			pcInput->UnbindKeys(maszBoundKeys.GetText(i));
		}
	}
	maszBoundKeys.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// Appends the keys that run the command as a menu shows them: its own,
// or those pcInput binds to the component command it stands for (pcInput
// can be NULL).  False, and nothing appended, if no key runs it.
//
//////////////////////////////////////////////////////////////////////////
bool CCommand::AppendKeysText(CChars* pszDest, CComponentInput* pcInput)
{
	if (maszKeys.NumElements() > 0)
	{
		CComponentInput::AppendShortcutText(pszDest, maszKeys.GetText(0));
		return true;
	}
	if ((meKeyScope == CKS_Application) && (meStandard != CIC_Ignored) && (pcInput != NULL))
	{
		return pcInput->GetShortcutText(meStandard, pszDest);
	}
	return false;
}

