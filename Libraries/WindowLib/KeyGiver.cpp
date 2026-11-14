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
#include "StandardLib/Objects.h"
#include "ComponentInput.h"
#include "MessageDialog.h"
#include "KeyGiver.h"


//////////////////////////////////////////////////////////////////////////
//
// pcCommands isn't held; it must outlive this.
//
//////////////////////////////////////////////////////////////////////////
void CCommandSetKeys::Init(CCommandSet* pcCommands)
{
	mpcCommands = pcCommands;
}


size CCommandSetKeys::NumKeyLists(void) { return mpcCommands->NumCommands(); }
CCommand* CCommandSetKeys::GetKeyListCommand(size uiIndex) { return &mpcCommands->GetCommand(uiIndex); }
CCommandKeyList* CCommandSetKeys::GetKeyList(size uiIndex) { return mpcCommands->GetCommand(uiIndex)->GetKeyList(); }
void CCommandSetKeys::AppendKeyListCaption(size uiIndex, CChars* pszDest) { pszDest->Append(mpcCommands->GetCommand(uiIndex)->GetCaption()); }


//////////////////////////////////////////////////////////////////////////
//
// The index of pcCommand's keys, ARRAY_ELEMENT_NOT_FOUND if it isn't in
// the set.
//
//////////////////////////////////////////////////////////////////////////
size CCommandSetKeys::Find(CCommand* pcCommand)
{
	size	i;

	for (i = 0; i < mpcCommands->NumCommands(); i++)
	{
		if (&mpcCommands->GetCommand(i) == pcCommand)
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
// Gives keys to pcSource's lists, the rules read from pcInput.  Neither
// is held.  It gives without asking until it's given a dialog
// (SetDialog), or an answer.
//
//////////////////////////////////////////////////////////////////////////
void CKeyGiver::Init(CKeyListSource* pcSource, CComponentInput* pcInput)
{
	mpcSource = pcSource;
	mpcInput = pcInput;
	mpOwner = NULL;
	mpsLook = NULL;
	mpcWatcher = NULL;
	meAnswer = KGA_Ask;
}


void CKeyGiver::Kill(void)
{
	mpOwner = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The clash pop up goes over pOwner, as psLook draws dialogs, watched by
// pcWatcher (NULL for none).
//
//////////////////////////////////////////////////////////////////////////
void CKeyGiver::SetDialog(Ptr<CWindow> pOwner, SDialogLook* psLook, CDialogWatcher* pcWatcher)
{
	mpOwner = pOwner;
	mpsLook = psLook;
	mpcWatcher = pcWatcher;
}


void CKeyGiver::SetAnswer(EKeyGiverAnswer eAnswer) { meAnswer = eAnswer; }


//////////////////////////////////////////////////////////////////////////
//
// The list, other than uiExcept, that has szKeys (named as
// CComponentInput names them) in szContext, ARRAY_ELEMENT_NOT_FOUND if
// none has.
//
//////////////////////////////////////////////////////////////////////////
size CKeyGiver::FindOwner(size uiExcept, const char* szContext, const char* szKeys)
{
	size	i;

	for (i = 0; i < mpcSource->NumKeyLists(); i++)
	{
		if ((i != uiExcept) && mpcSource->GetKeyList(i)->Contains(szContext, szKeys))
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether list uiIndex's command can be given szKeys in szContext, and
// what giving them would mean, in pszMessage: refused (see
// CheckCommandKeys), free, moved from another command, shadowing a Global
// command's keys there, or, given Global keys, shadowed by a command that
// has them in another context.
//
//////////////////////////////////////////////////////////////////////////
bool CKeyGiver::Describe(size uiIndex, const char* szContext, const char* szKeys, CChars* pszMessage)
{
	CChars	szName;
	CChars	szShown;
	size	uiOwner;

	if (!CheckCommandKeys(mpcSource->GetKeyListCommand(uiIndex), szContext, mpcSource->GetKeyList(uiIndex), szKeys, mpcInput, pszMessage))
	{
		return false;
	}

	szName.Init();
	szShown.Init();
	CComponentInput::NameKeys(szKeys, &szName);
	CComponentInput::AppendShortcutText(&szShown, szKeys);
	uiOwner = FindOwner(uiIndex, szContext, szName.Text());
	if (uiOwner != ARRAY_ELEMENT_NOT_FOUND)
	{
		pszMessage->Clear();
		pszMessage->Append(szShown);
		pszMessage->Append(" moves here from ");
		mpcSource->AppendKeyListCaption(uiOwner, pszMessage);
		pszMessage->Append(".");
	}
	if (StringCompare(szContext, COMPONENT_INPUT_GLOBAL_CONTEXT) == 0)
	{
		AppendShadowers(uiIndex, szName.Text(), pszMessage);
	}
	else if (uiOwner == ARRAY_ELEMENT_NOT_FOUND)
	{
		uiOwner = FindOwner(uiIndex, COMPONENT_INPUT_GLOBAL_CONTEXT, szName.Text());
		if (uiOwner != ARRAY_ELEMENT_NOT_FOUND)
		{
			pszMessage->Append("  It shadows ");
			mpcSource->AppendKeyListCaption(uiOwner, pszMessage);
			pszMessage->Append(" there.");
		}
	}
	szShown.Kill();
	szName.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Global keys given to list uiIndex's command don't reach it where
// another command has them in an inner context: "  Editor Actions:
// Duplicate keeps it over the Editor." for each.
//
//////////////////////////////////////////////////////////////////////////
void CKeyGiver::AppendShadowers(size uiIndex, const char* szKeys, CChars* pszMessage)
{
	CCommandKeyList*	pcKeys;
	size				i;
	size				j;

	for (i = 0; i < mpcSource->NumKeyLists(); i++)
	{
		if (i == uiIndex)
		{
			continue;
		}
		pcKeys = mpcSource->GetKeyList(i);
		for (j = 0; j < pcKeys->NumKeys(); j++)
		{
			if ((StringCompare(pcKeys->GetKeys(j), szKeys) == 0) && (StringCompare(pcKeys->GetContext(j), COMPONENT_INPUT_GLOBAL_CONTEXT) != 0))
			{
				pszMessage->Append("  ");
				mpcSource->AppendKeyListCaption(i, pszMessage);
				pszMessage->Append(" keeps it");
				AppendContextText(pszMessage, pcKeys->GetContext(j));
				pszMessage->Append(".");
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Gives list uiIndex's command szKeys in szContext, asking first if
// another command has them there, and taking them from it.  pszMessage
// says what was done, or why not.  False if they couldn't be given or
// moving them was cancelled.
//
//////////////////////////////////////////////////////////////////////////
bool CKeyGiver::Give(size uiIndex, const char* szContext, const char* szKeys, CChars* pszMessage)
{
	CChars	szName;
	size	uiOwner;
	bool	bResult;

	if (!Describe(uiIndex, szContext, szKeys, pszMessage))
	{
		return false;
	}
	szName.Init();
	CComponentInput::NameKeys(szKeys, &szName);
	uiOwner = FindOwner(uiIndex, szContext, szName.Text());
	bResult = (uiOwner == ARRAY_ELEMENT_NOT_FOUND) || AskToMove(uiIndex, uiOwner, szKeys);
	if (bResult)
	{
		if (uiOwner != ARRAY_ELEMENT_NOT_FOUND)
		{
			mpcSource->GetKeyList(uiOwner)->Remove(szContext, szName.Text());
		}
		mpcSource->GetKeyList(uiIndex)->Add(szContext, szName.Text());
	}
	else
	{
		pszMessage->Clear();
		pszMessage->Append("Kept ");
		CComponentInput::AppendShortcutText(pszMessage, szKeys);
		pszMessage->Append(" on ");
		mpcSource->AppendKeyListCaption(uiOwner, pszMessage);
		pszMessage->Append(".");
	}
	szName.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The clash pop up: whether to give the keys list uiOwner's command has
// to list uiIndex's.  Answered without being shown if there's an answer,
// or no dialog to show it over (it gives).
//
//////////////////////////////////////////////////////////////////////////
bool CKeyGiver::AskToMove(size uiIndex, size uiOwner, const char* szKeys)
{
	Ptr<CMessageDialog>	pAsk;
	CChars				szMessage;
	bool				bGive;

	if (meAnswer != KGA_Ask)
	{
		return meAnswer == KGA_Give;
	}
	if (mpOwner.IsNull())
	{
		return true;
	}

	szMessage.Init();
	CComponentInput::AppendShortcutText(&szMessage, szKeys);
	szMessage.Append(" is ");
	mpcSource->AppendKeyListCaption(uiOwner, &szMessage);
	szMessage.Append(".  Give it to ");
	szMessage.Append(mpcSource->GetKeyListCommand(uiIndex)->GetCaption());
	szMessage.Append(" instead?");
	pAsk = OMalloc<CMessageDialog>(mpOwner, mpsLook, "Shortcut in Use", szMessage.Text());
	szMessage.Kill();
	pAsk->GetDialog()->SetOKCaption("Give");
	if (mpcWatcher != NULL)
	{
		mpcWatcher->WatchDialog(pAsk->GetDialog());
	}
	bGive = pAsk->Show();
	if (mpcWatcher != NULL)
	{
		mpcWatcher->StopWatchingDialog();
	}
	return bGive;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcCommand, with pcKeys (what it has, or will have), can be given
// szKeys in szContext: not a context it can't have keys in; not keys it
// has there; not keys that type a character unless it's a typing context
// (a canvas's); not keys dialogs and text fields need unless it's an
// editor's.  pszMessage says which ("Ctrl+Alt+I is free.").  Keys
// another command has can be given; whoever gives them says they move.
//
//////////////////////////////////////////////////////////////////////////
bool CheckCommandKeys(CCommand* pcCommand, const char* szContext, CCommandKeyList* pcKeys, const char* szKeys, CComponentInput* pcInput, CChars* pszMessage)
{
	CChars	szShown;
	CChars	szName;
	bool	bResult;

	pszMessage->Clear();
	if (StrEmpty(szKeys))
	{
		pszMessage->Append("Press the keys.");
		return false;
	}

	szShown.Init();
	szName.Init();
	CComponentInput::AppendShortcutText(&szShown, szKeys);
	CComponentInput::NameKeys(szKeys, &szName);
	pszMessage->Append(szShown.Text());
	bResult = false;
	if (!pcCommand->IsContextAllowed(szContext))
	{
		pszMessage->Append(" can't be its shortcut");
		AppendContextText(pszMessage, szContext);
		pszMessage->Append(".");
	}
	else if (pcKeys->Contains(szContext, szName.Text()))
	{
		pszMessage->Append(" is already its shortcut.");
	}
	else if (pcInput->IsTypingKeys(szName.Text()) && !pcInput->IsTypingContext(szContext))
	{
		pszMessage->Append(" types a character: add Ctrl or Alt.");
	}
	else if ((StringCompare(szContext, COMPONENT_INPUT_EDITOR_CONTEXT) != 0) && CComponentInput::IsWidgetCommand(pcInput->GetStandardCommand(szName.Text())))
	{
		pszMessage->Append(" is needed by dialogs and text fields.");
	}
	else
	{
		bResult = true;
		pszMessage->Append(" is free.");
	}
	szName.Kill();
	szShown.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// " over the Circuit" for a context other than Global; nothing for it.
//
//////////////////////////////////////////////////////////////////////////
void AppendContextText(CChars* pszDest, const char* szContext)
{
	if (!StrEmpty(szContext) && (StringCompare(szContext, COMPONENT_INPUT_GLOBAL_CONTEXT) != 0))
	{
		pszDest->Append(" over the ");
		pszDest->Append(szContext);
	}
}

