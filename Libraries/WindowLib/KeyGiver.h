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
#ifndef __KEY_GIVER_H__
#define __KEY_GIVER_H__
#include "BaseLib/Chars.h"
#include "Command.h"
#include "CommandSet.h"
#include "Dialog.h"


class CComponentInput;


//What watches dialogs for an application's scripted checks (the HCP65 IDE's CIDEScript): a dialog as it's shown, and
//stops when it closes.
class CDialogWatcher
{
public:
	virtual void	WatchDialog(Ptr<CDialog> pDialog) = 0;
	virtual void	StopWatchingDialog(void) = 0;
};


//Where the keys being given are: the keymap page's copies until OK, or the commands themselves (CCommandSetKeys).  Each
//list is a command's keys, with a caption for it in messages ("Debug: Start Debugging").
class CKeyListSource
{
public:
	virtual size				NumKeyLists(void) = 0;
	virtual CCommand*			GetKeyListCommand(size uiIndex) = 0;
	virtual CCommandKeyList*	GetKeyList(size uiIndex) = 0;
	virtual void				AppendKeyListCaption(size uiIndex, CChars* pszDest) = 0;
};


//The commands of a command set as the key lists given to: their own keys, changed there and then.
class CCommandSetKeys : public CKeyListSource
{
protected:
	CCommandSet*	mpcCommands;  //Not held.

public:
	void				Init(CCommandSet* pcCommands);
	size				NumKeyLists(void) override;
	CCommand*			GetKeyListCommand(size uiIndex) override;
	CCommandKeyList*	GetKeyList(size uiIndex) override;
	void				AppendKeyListCaption(size uiIndex, CChars* pszDest) override;
	size				Find(CCommand* pcCommand);
};


//What the clash pop up is answered with without being shown (by a check, or a keymap page's scripted edits).
enum EKeyGiverAnswer
{
	KGA_Ask,
	KGA_Give,
	KGA_Cancel,
};


//Gives a command keys in one of its contexts, as Settings' Keymap and LogiCim's component chooser both do, so the rules
//and what's said are the same everywhere (see CheckCommandKeys).  A key belongs to one command in each context: given
//keys another command has there, it asks first ("Ctrl+Alt+A is File: Open Project.  Give it to AND Gate instead?") and
//moves them.  A key a Global command has, given in another context, shadows it there, and Global keys another command
//has in an inner context are shadowed by it there, which are said but allowed.  It
//only changes the key lists: binding them and saving them is for whoever gave them.
class CKeyGiver
{
protected:
	CKeyListSource*		mpcSource;  //Not held.
	CComponentInput*	mpcInput;  //Not held.
	Ptr<CWindow>		mpOwner;  //The clash pop up's, NULL to give without asking.
	SDialogLook*		mpsLook;
	CDialogWatcher*		mpcWatcher;  //NULL for none.
	EKeyGiverAnswer		meAnswer;

public:
	void	Init(CKeyListSource* pcSource, CComponentInput* pcInput);
	void	Kill(void);
	void	SetDialog(Ptr<CWindow> pOwner, SDialogLook* psLook, CDialogWatcher* pcWatcher);
	void	SetAnswer(EKeyGiverAnswer eAnswer);

	bool	Describe(size uiIndex, const char* szContext, const char* szKeys, CChars* pszMessage);
	bool	Give(size uiIndex, const char* szContext, const char* szKeys, CChars* pszMessage);
	size	FindOwner(size uiExcept, const char* szContext, const char* szKeys);

protected:
	bool	AskToMove(size uiIndex, size uiOwner, const char* szKeys);
	void	AppendShadowers(size uiIndex, const char* szKeys, CChars* pszMessage);
};


bool	CheckCommandKeys(CCommand* pcCommand, const char* szContext, CCommandKeyList* pcKeys, const char* szKeys, CComponentInput* pcInput, CChars* pszMessage);
void	AppendContextText(CChars* pszDest, const char* szContext);


#endif // __KEY_GIVER_H__

