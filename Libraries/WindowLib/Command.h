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
#include "BaseLib/Chars.h"
#include "StandardLib/Object.h"
#include "StandardLib/MemberCall.h"
#include "SupportLib/ImageCel.h"
#include "ComponentInputCommand.h"


//Something the application does, like Build or Step Over, that its keys, a menu item and a tool button can all ask
//for: its name (how it is found, by scripts too), caption and icon, the keys bound to it (see
//CComponentInput::BindKeys), what it runs and whether it can run now.  Keys reach it when nothing focused takes them
//(see CCommandSet), so a focused component's own keys come first.  Several commands can run one function that is
//given the command run (a table of an application's actions, say).  The functions' object isn't held by the command;
//it must outlive it.
class CCommand : public CObject
{
CONSTRUCTABLE(CCommand);
DESTRUCTABLE(CCommand);
protected:
	CChars					mszName;
	CChars					mszCaption;
	CChars					mszKeys;  //Named as CComponentInput names them, empty for none.
	Ptr<CImageCel>			mpIcon;  //NULL for none.
	SMemberCall				msRun;
	SMemberCall				msEnabled;  //Not set: the command can always run.
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
	bool					Run(void);
	void					SetStandardCommand(EComponentInputCommand eStandard);
	EComponentInputCommand	GetStandardCommand(void);

	char*				GetName(void);
	char*				GetCaption(void);
	char*				GetKeys(void);
	Ptr<CImageCel>		GetIcon(void);

protected:
	void				Init(const char* szName, const char* szCaption, const char* szKeys, Ptr<CImageCel> pIcon, SMemberCall* psRun, bool bRunAsks);
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


#endif // __COMMAND_H__

