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
#include <ctype.h>
#include "BaseLib/FileUtil.h"
#include "BaseLib/Logger.h"
#include "BaseLib/XMLFile.h"
#include "BaseLib/XMLWriter.h"
#include "StandardLib/ClassDefines.h"
#include "ComponentInput.h"
#include "CommandSet.h"


#define COMMAND_SET_KEYMAP_TAG		"Keymap"
#define COMMAND_SET_COMMAND_TAG		"Command"
#define COMMAND_SET_KEYS_TAG		"Keys"
#define COMMAND_SET_NAME_ATTRIBUTE	"Name"


//////////////////////////////////////////////////////////////////////////
//
// A tag's text comes with the white space around it in the file.
//
//////////////////////////////////////////////////////////////////////////
static void AddKeysText(Ptr<CCommand> pCommand, const char* szText)
{
	CChars	szKeys;
	size	uiStart;
	size	uiEnd;

	uiStart = 0;
	uiEnd = strlen(szText);
	while ((uiStart < uiEnd) && isspace((unsigned char)szText[uiStart]))
	{
		uiStart++;
	}
	while ((uiEnd > uiStart) && isspace((unsigned char)szText[uiEnd - 1]))
	{
		uiEnd--;
	}
	szKeys.Init(szText, uiStart, uiEnd);
	pCommand->AddKeys(szKeys.Text());
	szKeys.Kill();
}


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

	for (i = 0; i < maCommands.NumElements(); i++)
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
	for (i = 0; i < maCommands.NumElements(); i++)
	{
		if (maCommands.Get(i)->HasKeys(szKeys))
		{
			return maCommands.Get(i);
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Gives pcInput every command's keys as its scope says, in place of those
// given it before, so keys can be changed and bound again and two commands
// can swap theirs.  A command with its keys given by scope replaces the
// standard keys of its standard command.  False if any couldn't be bound.
//
//////////////////////////////////////////////////////////////////////////
bool CCommandSet::BindKeys(CComponentInput* pcInput)
{
	size	i;
	bool	bResult;

	for (i = 0; i < maCommands.NumElements(); i++)
	{
		maCommands.Get(i)->Unbind(pcInput);
	}
	for (i = 0; i < maCommands.NumElements(); i++)
	{
		if (maCommands.Get(i)->GetKeyScope() == CKS_Window)
		{
			pcInput->RemoveStandardKeys(maCommands.Get(i)->GetStandardCommand());
		}
	}

	bResult = true;
	for (i = 0; i < maCommands.NumElements(); i++)
	{
		bResult &= maCommands.Get(i)->Bind(pcInput);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Every command has the keys it was made with, once they're next bound.
//
//////////////////////////////////////////////////////////////////////////
void CCommandSet::ResetKeys(void)
{
	size	i;

	for (i = 0; i < maCommands.NumElements(); i++)
	{
		maCommands.Get(i)->ResetKeys();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The user's keys in szFile, in place of any before, once they're next
// bound.  A key belongs to one command, so a command that kept its own
// gives up any the user gave another; if the file gives two the same, the
// first has it.  Commands the file names that aren't here are ignored.
// False, and each command has what it was made with, if it isn't there or
// isn't a keymap.
//
//////////////////////////////////////////////////////////////////////////
bool CCommandSet::ReadKeys(const char* szFile)
{
	CXMLFile		cFile;
	CFileUtil		cFileUtil;
	CChars			szFileName;
	CChars			szDirectory;
	CChars			szKeys;
	CMarkupTag*		pcRoot;
	CMarkupTag*		pcTag;
	CMarkupTag*		pcKeysTag;
	STagIterator	sIterator;
	STagIterator	sKeysIterator;
	CArrayChars		aszRead;  //The names of the commands the file set, in its order.
	Ptr<CCommand>	pCommand;
	Ptr<CCommand>	pOther;
	char*			szName;
	bool			bResult;
	size			i;
	size			j;
	size			k;

	ResetKeys();
	szFileName.Init();
	szDirectory.Init();
	aszRead.Init();
	cFileUtil.SplitPath(szFile, &szFileName, &szDirectory);
	if (szDirectory.Empty())
	{
		szDirectory.Set(".");
	}
	cFile.Init();
	bResult = cFileUtil.Exists(szFile) && cFile.Read(szFileName.Text(), szDirectory.Text()) && cFile.mcMarkup.mpcDoc;
	pcRoot = bResult ? cFile.mcMarkup.mpcDoc->GetRootTag() : NULL;
	bResult = pcRoot && (StringCompare(pcRoot->GetName(), COMMAND_SET_KEYMAP_TAG) == 0);
	if (bResult)
	{
		for (pcTag = pcRoot->GetTag(COMMAND_SET_COMMAND_TAG, &sIterator); pcTag; pcTag = pcRoot->GetNextTag(&sIterator))
		{
			szName = pcTag->GetStringAttribute(COMMAND_SET_NAME_ATTRIBUTE);
			pCommand = szName ? Find(szName) : Ptr<CCommand>();
			if (pCommand.IsNull() || aszRead.Contains(szName))
			{
				continue;
			}
			aszRead.Add(szName);
			pCommand->ClearKeys();
			for (pcKeysTag = pcTag->GetTag(COMMAND_SET_KEYS_TAG, &sKeysIterator); pcKeysTag; pcKeysTag = pcTag->GetNextTag(&sKeysIterator))
			{
				szKeys.Init();
				pcKeysTag->GetText(&szKeys);
				AddKeysText(pCommand, szKeys.Text());
				szKeys.Kill();
			}
		}
	}
	cFile.Kill();
	szDirectory.Kill();
	szFileName.Kill();

	//Each command the file set, in order, takes its keys from every command that comes after it or kept its own.
	for (i = 0; i < aszRead.NumElements(); i++)
	{
		pCommand = Find(aszRead.GetText(i));
		for (j = 0; j < pCommand->NumKeys(); j++)
		{
			for (k = 0; k < maCommands.NumElements(); k++)
			{
				pOther = maCommands.Get(k);
				if ((&pOther != &pCommand) && (aszRead.GetIndex(pOther->GetName()) > i))
				{
					pOther->RemoveKeys(pCommand->GetKeys(j));
				}
			}
		}
	}
	aszRead.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The commands whose keys differ from those they were made with to
// szFile, its directory made if it isn't there.
//
//////////////////////////////////////////////////////////////////////////
bool CCommandSet::WriteKeys(const char* szFile)
{
	CXMLWriter		cWriter;
	CFileUtil		cFileUtil;
	Ptr<CCommand>	pCommand;
	size			i;
	size			j;
	bool			bResult;

	cFileUtil.TouchDir(szFile, true);
	cWriter.Init();
	cWriter.Begin(COMMAND_SET_KEYMAP_TAG);
	for (i = 0; i < maCommands.NumElements(); i++)
	{
		pCommand = maCommands.Get(i);
		if (!pCommand->HasDefaultKeys())
		{
			cWriter.Begin(COMMAND_SET_COMMAND_TAG);
			cWriter.Attribute(COMMAND_SET_NAME_ATTRIBUTE, pCommand->GetName());
			for (j = 0; j < pCommand->NumKeys(); j++)
			{
				cWriter.Begin(COMMAND_SET_KEYS_TAG);
				cWriter.Text(pCommand->GetKeys(j));
				cWriter.End();
			}
			cWriter.End();
		}
	}
	cWriter.End();
	bResult = cWriter.Write(szFile);
	cWriter.Kill();
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


size CCommandSet::NumCommands(void) { return maCommands.NumElements(); }
Ptr<CCommand> CCommandSet::GetCommand(size uiIndex) { return maCommands.Get(uiIndex); }

