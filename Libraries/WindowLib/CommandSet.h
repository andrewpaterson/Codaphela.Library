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
#ifndef __COMMAND_SET_H__
#define __COMMAND_SET_H__
#include "StandardLib/Array.h"
#include "Command.h"


class CComponentInput;
//An application's commands, given to its window (CWindow::SetCommands).  Their keys are bound (BindKeys) as
//CIC_Command; such a key that nothing focused takes runs the command whose keys it is (see
//CFocus::DispatchUnhandledCommand).  A command is found by its name too, as scripts and menus find it.
class CCommandSet : public CObject
{
CONSTRUCTABLE(CCommandSet);
DESTRUCTABLE(CCommandSet);
protected:
	CArray<CCommand>	maCommands;

public:
	void				Init(void);
	void				Class(void) override;
	void				Free(void) override;
	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	template<class M, class O>
	Ptr<CCommand>		Add(const char* szName, const char* szCaption, const char* szKeys, Ptr<CImageCel> pIcon, O* pcObject, void(M::*fRun)(void));
	Ptr<CCommand>		Add(Ptr<CCommand> pCommand);
	Ptr<CCommand>		Find(const char* szName);
	Ptr<CCommand>		FindKeys(const char* szKeys);
	size				NumCommands(void);
	Ptr<CCommand>		GetCommand(size uiIndex);

	bool				BindKeys(CComponentInput* pcInput);
	bool				Run(const char* szName);
	bool				RunKeys(const char* szKeys);
};


//////////////////////////////////////////////////////////////////////////
//
// A new command, see CCommand::Init.  NULL, logged, if the set has one of
// that name already.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class O>
Ptr<CCommand> CCommandSet::Add(const char* szName, const char* szCaption, const char* szKeys, Ptr<CImageCel> pIcon, O* pcObject, void(M::*fRun)(void))
{
	Ptr<CCommand>	pCommand;

	pCommand = OMalloc<CCommand>(szName, szCaption, szKeys, pIcon, pcObject, fRun);
	return Add(pCommand);
}


#endif // __COMMAND_SET_H__

