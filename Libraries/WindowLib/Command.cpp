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
void CCommandKeyList::Init(void)
{
	maszContexts.Init();
	maszKeys.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCommandKeyList::Kill(void)
{
	maszKeys.Kill();
	maszContexts.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCommandKeyList::Clear(void)
{
	maszKeys.ReInit();
	maszContexts.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCommandKeyList::Copy(CCommandKeyList* pcSource)
{
	size	i;

	Clear();
	for (i = 0; i < pcSource->NumKeys(); i++)
	{
		Add(pcSource->GetContext(i), pcSource->GetKeys(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// False if they're there already.
//
//////////////////////////////////////////////////////////////////////////
bool CCommandKeyList::Add(const char* szContext, const char* szKeys)
{
	if (Contains(szContext, szKeys))
	{
		return false;
	}
	maszContexts.Add((char*)szContext);
	maszKeys.Add((char*)szKeys);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// False if they weren't there.
//
//////////////////////////////////////////////////////////////////////////
bool CCommandKeyList::Remove(const char* szContext, const char* szKeys)
{
	size	uiIndex;

	uiIndex = Find(szContext, szKeys);
	if (uiIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		return false;
	}
	maszContexts.Remove(uiIndex);
	maszKeys.Remove(uiIndex);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The index of szKeys in szContext, ARRAY_ELEMENT_NOT_FOUND if they
// aren't there.
//
//////////////////////////////////////////////////////////////////////////
size CCommandKeyList::Find(const char* szContext, const char* szKeys)
{
	size	i;

	for (i = 0; i < maszKeys.NumElements(); i++)
	{
		if (maszKeys.Get(i)->Equals(szKeys) && maszContexts.Get(i)->Equals(szContext))
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether it has the same keys in the same contexts, in any order.
//
//////////////////////////////////////////////////////////////////////////
bool CCommandKeyList::HasSameKeys(CCommandKeyList* pcOther)
{
	size	i;

	if (NumKeys() != pcOther->NumKeys())
	{
		return false;
	}
	for (i = 0; i < NumKeys(); i++)
	{
		if (!pcOther->Contains(GetContext(i), GetKeys(i)))
		{
			return false;
		}
	}
	return true;
}


bool CCommandKeyList::Contains(const char* szContext, const char* szKeys) { return Find(szContext, szKeys) != ARRAY_ELEMENT_NOT_FOUND; }
bool CCommandKeyList::ContainsKeys(const char* szKeys) { return maszKeys.Contains((char*)szKeys); }  //In any context.
size CCommandKeyList::NumKeys(void) { return maszKeys.NumElements(); }
char* CCommandKeyList::GetKeys(size uiIndex) { return maszKeys.GetText(uiIndex); }
char* CCommandKeyList::GetContext(size uiIndex) { return maszContexts.GetText(uiIndex); }


//////////////////////////////////////////////////////////////////////////
//
// szKeys (Global keys unless the command is told otherwise, see
// SetContext) are its defaults.
//
//////////////////////////////////////////////////////////////////////////
void CCommand::Init(const char* szName, const char* szCaption, const char* szKeys, Ptr<CImageCel> pIcon, SMemberCall* psRun, bool bRunAsks)
{
	PreInit();
	mszName.Init(szName);
	mszCaption.Init(szCaption);
	maszContexts.Init();
	maszContexts.Add(COMPONENT_INPUT_GLOBAL_CONTEXT);
	mcKeys.Init();
	mcDefaultKeys.Init();
	mcBoundKeys.Init();
	mbGivesStandard = false;
	mbKeptByKeymap = true;
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
	U_Unknown(CArrayChars, maszContexts);
	U_Unknown(CCommandKeyList, mcKeys);
	U_Unknown(CCommandKeyList, mcDefaultKeys);
	U_Unknown(CCommandKeyList, mcBoundKeys);
	U_Bool(mbGivesStandard);
	U_Bool(mbKeptByKeymap);
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
	mcBoundKeys.Kill();
	mcDefaultKeys.Kill();
	mcKeys.Kill();
	maszContexts.Kill();
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
void CCommand::SetGivesStandard(bool bGivesStandard) { mbGivesStandard = bGivesStandard; }  //Its keys give its standard command (SetStandardCommand) for the parts of the window to take.
bool CCommand::IsGivingStandard(void) { return mbGivesStandard; }
char* CCommand::GetName(void) { return mszName.Text(); }
char* CCommand::GetCaption(void) { return mszCaption.Text(); }
size CCommand::NumContexts(void) { return maszContexts.NumElements(); }
char* CCommand::GetContext(size uiIndex) { return maszContexts.GetText(uiIndex); }
char* CCommand::GetFirstContext(void) { return maszContexts.GetText(0); }
void CCommand::SetKeptByKeymap(bool bKept) { mbKeptByKeymap = bKept; }
bool CCommand::IsKeptByKeymap(void) { return mbKeptByKeymap; }
size CCommand::NumKeys(void) { return mcKeys.NumKeys(); }
char* CCommand::GetKeys(size uiIndex) { return mcKeys.GetKeys(uiIndex); }
char* CCommand::GetKeysContext(size uiIndex) { return mcKeys.GetContext(uiIndex); }
size CCommand::NumDefaultKeys(void) { return mcDefaultKeys.NumKeys(); }
char* CCommand::GetDefaultKeys(size uiIndex) { return mcDefaultKeys.GetKeys(uiIndex); }
char* CCommand::GetDefaultKeysContext(size uiIndex) { return mcDefaultKeys.GetContext(uiIndex); }
CCommandKeyList* CCommand::GetKeyList(void) { return &mcKeys; }
void CCommand::ClearKeys(void) { mcKeys.Clear(); }
Ptr<CImageCel> CCommand::GetIcon(void) { return mpIcon; }


//////////////////////////////////////////////////////////////////////////
//
// The only context the command has keys in, in place of those it had;
// before any keys are given.
//
//////////////////////////////////////////////////////////////////////////
void CCommand::SetContext(const char* szContext)
{
	maszContexts.ReInit();
	maszContexts.Add((char*)szContext);
}


//////////////////////////////////////////////////////////////////////////
//
// Another context the command may have keys in.
//
//////////////////////////////////////////////////////////////////////////
void CCommand::AllowContext(const char* szContext)
{
	if (!maszContexts.Contains((char*)szContext))
	{
		maszContexts.Add((char*)szContext);
	}
}


bool CCommand::IsContextAllowed(const char* szContext) { return maszContexts.Contains((char*)szContext); }


//////////////////////////////////////////////////////////////////////////
//
// szContext, or the command's first context for NULL or "".
//
//////////////////////////////////////////////////////////////////////////
const char* CCommand::ToContext(const char* szContext)
{
	return StrEmpty(szContext) ? GetFirstContext() : szContext;
}


//////////////////////////////////////////////////////////////////////////
//
// szKeys named as CComponentInput names them, in pszDest (initialised).
// False if there are none or they can't be read.
//
//////////////////////////////////////////////////////////////////////////
bool CCommand::NameKeys(const char* szKeys, CChars* pszDest)
{
	return !StrEmpty(szKeys) && CComponentInput::NameKeys(szKeys, pszDest);
}


//////////////////////////////////////////////////////////////////////////
//
// The keys a menu shows: the first Global keys, else the first keys in
// any context.  "" for none.
//
//////////////////////////////////////////////////////////////////////////
char* CCommand::GetKeys(void)
{
	size	i;

	for (i = 0; i < mcKeys.NumKeys(); i++)
	{
		if (StringCompare(mcKeys.GetContext(i), COMPONENT_INPUT_GLOBAL_CONTEXT) == 0)
		{
			return mcKeys.GetKeys(i);
		}
	}
	return (mcKeys.NumKeys() > 0) ? mcKeys.GetKeys(0) : (char*)"";
}


//////////////////////////////////////////////////////////////////////////
//
// Whether szKeys, written any way CComponentInput reads, are the
// command's in szContext, or in any context for NULL.
//
//////////////////////////////////////////////////////////////////////////
bool CCommand::HasKeys(const char* szKeys, const char* szContext)
{
	CChars	szName;
	bool	bResult;

	szName.Init();
	bResult = NameKeys(szKeys, &szName) && (szContext ? mcKeys.Contains(szContext, szName.Text()) : mcKeys.ContainsKeys(szName.Text()));
	szName.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Keys the command has in szContext (NULL for its first context) unless
// a user says otherwise.  NULL or "" adds none, and keys that can't be
// read aren't added.
//
//////////////////////////////////////////////////////////////////////////
void CCommand::AddDefaultKeys(const char* szKeys, const char* szContext)
{
	CChars	szName;

	szName.Init();
	if (NameKeys(szKeys, &szName) && mcDefaultKeys.Add(ToContext(szContext), szName.Text()))
	{
		mcKeys.Add(ToContext(szContext), szName.Text());
	}
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Other keys for the command in szContext (NULL for its first context),
// once they're next bound.  False if they can't be read, or the command
// has them there already.
//
//////////////////////////////////////////////////////////////////////////
bool CCommand::AddKeys(const char* szKeys, const char* szContext)
{
	CChars	szName;
	bool	bResult;

	szName.Init();
	bResult = NameKeys(szKeys, &szName) && mcKeys.Add(ToContext(szContext), szName.Text());
	szName.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The command no longer has szKeys in szContext (NULL for its first
// context), once they're next bound.
//
//////////////////////////////////////////////////////////////////////////
void CCommand::RemoveKeys(const char* szKeys, const char* szContext)
{
	CChars	szName;

	szName.Init();
	if (NameKeys(szKeys, &szName))
	{
		mcKeys.Remove(ToContext(szContext), szName.Text());
	}
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// paszKeys, in its first context, in place of its keys.
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


void CCommand::SetKeys(CCommandKeyList* pcKeys) { mcKeys.Copy(pcKeys); }  //In place of its keys.
void CCommand::ResetKeys(void) { mcKeys.Copy(&mcDefaultKeys); }  //The keys it was made with again, once they're next bound.
bool CCommand::HasDefaultKeys(void) { return mcKeys.HasSameKeys(&mcDefaultKeys); }  //Whether its keys are those it was made with, in any order.


//////////////////////////////////////////////////////////////////////////
//
// Gives pcInput the command's keys, each in its context: they give its
// standard command if it gives one, else run it (CIC_Command).  False if
// any couldn't be bound (a context pcInput hasn't).
//
//////////////////////////////////////////////////////////////////////////
bool CCommand::Bind(CComponentInput* pcInput)
{
	size	i;
	bool	bResult;

	bResult = true;
	for (i = 0; i < mcKeys.NumKeys(); i++)
	{
		if (pcInput->BindKeys(mcKeys.GetKeys(i), mbGivesStandard ? meStandard : CIC_Command, mcKeys.GetContext(i)))
		{
			mcBoundKeys.Add(mcKeys.GetContext(i), mcKeys.GetKeys(i));
		}
		else
		{
			bResult = false;
		}
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

	for (i = 0; i < mcBoundKeys.NumKeys(); i++)
	{
		pcInput->UnbindKeys(mcBoundKeys.GetKeys(i), mcBoundKeys.GetContext(i));
	}
	mcBoundKeys.Clear();
}


//////////////////////////////////////////////////////////////////////////
//
// Appends the keys that run the command as a menu shows them (GetKeys),
// or those pcInput binds to the component command it stands for (pcInput
// can be NULL).  False, and nothing appended, if no key runs it.
//
//////////////////////////////////////////////////////////////////////////
bool CCommand::AppendKeysText(CChars* pszDest, CComponentInput* pcInput)
{
	if (mcKeys.NumKeys() > 0)
	{
		CComponentInput::AppendShortcutText(pszDest, GetKeys());
		return true;
	}
	if (!mbGivesStandard && (meStandard != CIC_Ignored) && (pcInput != NULL))
	{
		return pcInput->GetShortcutText(meStandard, pszDest);
	}
	return false;
}

