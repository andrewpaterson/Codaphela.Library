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
#include "BaseLib/Logger.h"
#include "StandardLib/ClassDefines.h"
#include "ComponentInput.h"
#include "CommandSet.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCommandSet::Init(void)
{
	PreInit();
	maCommands.Init();
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCommandSet::Class(void)
{
	M_Embedded(maCommands);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCommandSet::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
// A command set isn't saved; the application makes its commands each run.
//
//////////////////////////////////////////////////////////////////////////
bool CCommandSet::Save(CObjectWriter* pcFile) { return false; }
bool CCommandSet::Load(CObjectReader* pcFile) { return false; }


//////////////////////////////////////////////////////////////////////////
//
// NULL, logged, if the set has a command of pCommand's name already.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CCommand> CCommandSet::Add(Ptr<CCommand> pCommand)
{
	if (Find(pCommand->GetName()).IsNotNull())
	{
		gcLogger.Error2(__METHOD__, " There is a command named [", pCommand->GetName(), "] already.", NULL);
		return NULL;
	}
	maCommands.Add(pCommand);
	return pCommand;
}


//////////////////////////////////////////////////////////////////////////
//
// NULL if there is none.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CCommand> CCommandSet::Find(const char* szName)
{
	size	i;

	for (i = 0; i < maCommands.Size(); i++)
	{
		if (StringCompare(maCommands.Get(i)->GetName(), szName) == 0)
		{
			return maCommands.Get(i);
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The command bound to szKeys (named as CComponentInput names them, as an
// event's are), NULL if there is none.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CCommand> CCommandSet::FindKeys(const char* szKeys)
{
	size	i;

	if (StrEmpty(szKeys))
	{
		return NULL;
	}
	for (i = 0; i < maCommands.Size(); i++)
	{
		if (StringCompare(maCommands.Get(i)->GetKeys(), szKeys) == 0)
		{
			return maCommands.Get(i);
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Binds every command's keys in pcInput as CIC_Command.  False if any
// couldn't be bound.
//
//////////////////////////////////////////////////////////////////////////
bool CCommandSet::BindKeys(CComponentInput* pcInput)
{
	size	i;
	bool	bResult;

	bResult = true;
	for (i = 0; i < maCommands.Size(); i++)
	{
		if (!StrEmpty(maCommands.Get(i)->GetKeys()))
		{
			bResult &= pcInput->BindKeys(maCommands.Get(i)->GetKeys(), CIC_Command);
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Runs the command named szName.  False if there is none or it can't run
// now.
//
//////////////////////////////////////////////////////////////////////////
bool CCommandSet::Run(const char* szName)
{
	Ptr<CCommand>	pCommand;

	pCommand = Find(szName);
	return pCommand.IsNotNull() && pCommand->Run();
}


//////////////////////////////////////////////////////////////////////////
//
// Runs the command bound to szKeys.  False if there is none or it can't
// run now, so the keys go on.
//
//////////////////////////////////////////////////////////////////////////
bool CCommandSet::RunKeys(const char* szKeys)
{
	Ptr<CCommand>	pCommand;

	pCommand = FindKeys(szKeys);
	return pCommand.IsNotNull() && pCommand->Run();
}


size CCommandSet::NumCommands(void) { return maCommands.Size(); }
Ptr<CCommand> CCommandSet::GetCommand(size uiIndex) { return maCommands.Get(uiIndex); }

