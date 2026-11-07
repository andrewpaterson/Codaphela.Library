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
void CCommand::Init(const char* szName, const char* szCaption, const char* szKeys, Ptr<CImageCel> pIcon, SMemberCall* psRun)
{
	PreInit();
	mszName.Init(szName);
	mszCaption.Init(szCaption);
	mszKeys.Init();
	if (!StrEmpty(szKeys))
	{
		CComponentInput::NameKeys(szKeys, &mszKeys);
	}
	mpIcon = pIcon;
	msRun = *psRun;
	msEnabled.Clear();
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
	U_String(mszKeys);
	M_Pointer(mpIcon);
	U_Data(SMemberCall, msRun);
	U_Data(SMemberCall, msEnabled);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCommand::Free(void)
{
	mszKeys.Kill();
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
// False, and nothing is run, if the command can't run now.
//
//////////////////////////////////////////////////////////////////////////
bool CCommand::Run(void)
{
	if (!IsEnabled())
	{
		return false;
	}
	msRun.Call(NULL);
	return true;
}


char* CCommand::GetName(void) { return mszName.Text(); }
char* CCommand::GetCaption(void) { return mszCaption.Text(); }
char* CCommand::GetKeys(void) { return mszKeys.Text(); }
Ptr<CImageCel> CCommand::GetIcon(void) { return mpIcon; }

