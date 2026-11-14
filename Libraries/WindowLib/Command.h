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
#ifndef __COMMAND_H__
#define __COMMAND_H__
#include "BaseLib/ArrayChars.h"
#include "BaseLib/Chars.h"
#include "StandardLib/Object.h"
#include "StandardLib/MemberCall.h"
#include "SupportLib/ImageCel.h"
#include "ComponentInputCommand.h"


class CComponentInput;


//Keys and the context each is bound in (see CComponentInput::AddContext), as a command keeps them: "R" in "Circuit",
//"Control+R" in "Global".  The keys are named as CComponentInput::NameKeys names them.
class CCommandKeyList
{
protected:
	CArrayChars	maszContexts;
	CArrayChars	maszKeys;

public:
	void	Init(void);
	void	Kill(void);

	void	Clear(void);
	void	Copy(CCommandKeyList* pcSource);
	bool	Add(const char* szContext, const char* szKeys);
	bool	Remove(const char* szContext, const char* szKeys);
	size	Find(const char* szContext, const char* szKeys);
	bool	Contains(const char* szContext, const char* szKeys);
	bool	ContainsKeys(const char* szKeys);
	bool	HasSameKeys(CCommandKeyList* pcOther);
	size	NumKeys(void);
	char*	GetKeys(size uiIndex);
	char*	GetContext(size uiIndex);
};


//Something the application does, like Build or Step Over, that its keys, a menu item and a tool button can all ask
//for: its name (how it is found, by scripts too), caption and icon, the keys bound to it (see
//CComponentInput::BindKeys), what it runs and whether it can run now.  Keys reach it when nothing focused takes them
//(see CCommandSet), so a focused component's own keys come first.  Several commands can run one function that is
//given the command run (a table of an application's actions, say).  The functions' object isn't held by the command;
//it must outlive it.
//
//Its keys are each bound in a context (LogiCim's Rotate Left has R over the circuit, and could have Control+R
//everywhere): those it may have keys in are its contexts (AllowContext), the first, Global unless it's told otherwise
//(SetContext), being where keys given without one go.  They run it, or give its standard command when it stands for
//one a window's or an editor's parts take (SetGivesStandard: Copy, Close Document).  A command can have several keys,
//the first Global one being what a menu shows.  It remembers those it started with, so a user's changes can be told
//from them and undone, and those it last bound, so they can be unbound.  Keys a project keeps rather than the user's
//keymap (a subcircuit's, in LogiCim) are SetKeptByKeymap(false).
//
//A command that turns something on and off (Group by Project) says which it is (SetCheckedCheck): a toolbar shows its
//button on while it is.
class CCommand : public CObject
{
CONSTRUCTABLE(CCommand);
DESTRUCTABLE(CCommand);
protected:
	CChars					mszName;
	CChars					mszCaption;
	CArrayChars				maszContexts;  //Where it may have keys, the first where keys without a context go.
	CCommandKeyList			mcKeys;
	CCommandKeyList			mcDefaultKeys;  //The keys it was made with.
	CCommandKeyList			mcBoundKeys;  //The keys given to a CComponentInput by Bind.
	bool					mbGivesStandard;  //Its keys give meStandard rather than running it.
	bool					mbKeptByKeymap;  //Its keys are the user's keymap's (CCommandSet::ReadKeys and WriteKeys).
	Ptr<CImageCel>			mpIcon;  //NULL for none.
	SMemberCall				msRun;
	SMemberCall				msEnabled;  //Not set: the command can always run.
	SMemberCall				msChecked;  //Not set: the command isn't one that is on or off.
	EComponentInputCommand	meStandard;  //The component command it stands for (an Edit menu's copy), whose keys it shows; CIC_Ignored if none.
	bool					mbRunAsks;  //msRun returns whether it did anything, which isn't needed.

public:
	template<class M, class O>
	void				Init(const char* szName, const char* szCaption, const char* szKeys, Ptr<CImageCel> pIcon, O* pcObject, void(M::*fRun)(void));
	template<class M, class O>
	void				Init(const char* szName, const char* szCaption, const char* szKeys, Ptr<CImageCel> pIcon, O* pcObject, bool(M::*fRun)(void));
	template<class M, class O>
	void				Init(const char* szName, const char* szCaption, const char* szKeys, Ptr<CImageCel> pIcon, O* pcObject, void(M::*fRun)(CCommand*));
	void				Class(void) override;
	void				Free(void) override;
	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	template<class M, class O>
	void					SetEnabledCheck(O* pcObject, bool(M::*fIsEnabled)(void));
	bool					IsEnabled(void);
	template<class M, class O>
	void					SetCheckedCheck(O* pcObject, bool(M::*fIsChecked)(void));
	bool					IsCheckable(void);
	bool					IsChecked(void);
	bool					Run(void);
	void					SetStandardCommand(EComponentInputCommand eStandard);
	EComponentInputCommand	GetStandardCommand(void);
	void					SetGivesStandard(bool bGivesStandard);
	bool					IsGivingStandard(void);

	char*				GetName(void);
	char*				GetCaption(void);
	void				SetContext(const char* szContext);
	void				AllowContext(const char* szContext);
	bool				IsContextAllowed(const char* szContext);
	size				NumContexts(void);
	char*				GetContext(size uiIndex);
	char*				GetFirstContext(void);
	void				SetKeptByKeymap(bool bKept);
	bool				IsKeptByKeymap(void);

	char*				GetKeys(void);
	size				NumKeys(void);
	char*				GetKeys(size uiIndex);
	char*				GetKeysContext(size uiIndex);
	bool				HasKeys(const char* szKeys, const char* szContext = NULL);
	void				AddDefaultKeys(const char* szKeys, const char* szContext = NULL);
	bool				AddKeys(const char* szKeys, const char* szContext = NULL);
	void				RemoveKeys(const char* szKeys, const char* szContext = NULL);
	void				SetKeys(CArrayChars* paszKeys);
	void				SetKeys(CCommandKeyList* pcKeys);
	void				ClearKeys(void);
	void				ResetKeys(void);
	bool				HasDefaultKeys(void);
	size				NumDefaultKeys(void);
	char*				GetDefaultKeys(size uiIndex);
	char*				GetDefaultKeysContext(size uiIndex);
	CCommandKeyList*	GetKeyList(void);
	bool				Bind(CComponentInput* pcInput);
	void				Unbind(CComponentInput* pcInput);
	Ptr<CImageCel>		GetIcon(void);
	bool				AppendKeysText(CChars* pszDest, CComponentInput* pcInput);

protected:
	void				Init(const char* szName, const char* szCaption, const char* szKeys, Ptr<CImageCel> pIcon, SMemberCall* psRun, bool bRunAsks);
	const char*			ToContext(const char* szContext);
	bool				NameKeys(const char* szKeys, CChars* pszDest);
};


//////////////////////////////////////////////////////////////////////////
//
// szKeys as CComponentInput::BindKeys reads them, NULL or "" for none.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class O>
void CCommand::Init(const char* szName, const char* szCaption, const char* szKeys, Ptr<CImageCel> pIcon, O* pcObject, void(M::*fRun)(void))
{
	SMemberCall	sRun;

	sRun.Init(pcObject, fRun);
	Init(szName, szCaption, szKeys, pIcon, &sRun, false);
}


//////////////////////////////////////////////////////////////////////////
//
// A command whose function says whether it did anything, as many of an
// application's do; what it says isn't needed.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class O>
void CCommand::Init(const char* szName, const char* szCaption, const char* szKeys, Ptr<CImageCel> pIcon, O* pcObject, bool(M::*fRun)(void))
{
	SMemberCall	sRun;

	sRun.Init(pcObject, fRun);
	Init(szName, szCaption, szKeys, pIcon, &sRun, true);
}


//////////////////////////////////////////////////////////////////////////
//
// A command whose function is given the command, so several commands can
// share it.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class O>
void CCommand::Init(const char* szName, const char* szCaption, const char* szKeys, Ptr<CImageCel> pIcon, O* pcObject, void(M::*fRun)(CCommand*))
{
	SMemberCall	sRun;

	sRun.Init(pcObject, fRun);
	Init(szName, szCaption, szKeys, pIcon, &sRun, false);
}


//////////////////////////////////////////////////////////////////////////
//
// fIsEnabled says whether the command can run now; asked before it runs
// and as its menu items and tool buttons are shown.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class O>
void CCommand::SetEnabledCheck(O* pcObject, bool(M::*fIsEnabled)(void))
{
	msEnabled.Init(pcObject, fIsEnabled);
}


//////////////////////////////////////////////////////////////////////////
//
// fIsChecked says whether what the command turns on and off is on.  The
// object isn't held by the command; it must outlive it.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class O>
void CCommand::SetCheckedCheck(O* pcObject, bool(M::*fIsChecked)(void))
{
	msChecked.Init(pcObject, fIsChecked);
}


#endif // __COMMAND_H__

