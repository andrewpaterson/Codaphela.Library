/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "NamedDefine.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedDefine::Define(CExternalString* pcName, CDefine* pcSource, CMemoryStackExtended* mpcStack)
{
	CPPLine	cLine;

	memcpy(&mcReplacement, &cLine, sizeof(CPPLine));

	mszName.Init();
	mszName.AppendSubString(pcName->msz, pcName->miLen);
	miFlags = pcSource->IsBacketed() ? NAMED_DEFINE_FLAGS_BRACKETED : 0;
	mcArguments.Init();
	mcArguments.Copy(&pcSource->mcArguments);
	mcReplacement.Init(pcSource->mcReplacement.Line(), pcSource->mcReplacement.Column());
	mcReplacement.Copy(&pcSource->mcReplacement, mpcStack);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedDefine::Undef(CExternalString* pcName)
{
	CPPLine	cLine;

	memcpy(&mcReplacement, &cLine, sizeof(CPPLine));

	mszName.Init();
	mszName.AppendSubString(pcName->msz, pcName->miLen);
	miFlags = NAMED_DEFINE_FLAGS_UNDEFFED;
	mcReplacement.Init(-1, -1);
	mcArguments.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedDefine::Kill(void)
{
	mszName.Kill();
	mcArguments.Kill();
	mcReplacement.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedDefine::IsBacketed(void)
{
	return miFlags & NAMED_DEFINE_FLAGS_BRACKETED;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedDefine::Equals(CNamedDefine* pcOther)
{
	if (mszName.Equals(pcOther->mszName))
	{
		if (mcReplacement.Equals(&pcOther->mcReplacement))
		{
			if (miFlags == pcOther->miFlags)
			{
				if (mcArguments.Equals(&pcOther->mcArguments))
				{
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedDefine::Dump(void)
{
	CChars	sz;
	int		i;
	CChars*	psz;

	sz.Init();

	sz.Append(mszName);

	if (IsBacketed())
	{
		sz.Append('(');
		for (i = 0; i < mcArguments.NumElements(); i++)
		{
			psz = mcArguments.Get(i);
			sz.Append(psz->Text());

			if (i != mcArguments.NumElements()-1)
			{
				sz.Append(", ");
			}
		}
		sz.Append(')');
	}

	if (mcReplacement.TokenLength() > 0)
	{
		sz.Append(' ');
		mcReplacement.Append(&sz);
	}
	else
	{
		sz.AppendNewLine();
	}

	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayNamedDefines::Equals(CArrayNamedDefines* pcDefines)
{
	int				i;
	CNamedDefine*	pcThis;
	CNamedDefine*	pcOther;

	if (pcDefines->miUsedElements != miUsedElements)
	{
		return FALSE;
	}

	for (i = 0; i < miUsedElements; i++)
	{
		pcThis = Get(i);
		pcOther = pcDefines->Get(i);

		if (!pcThis->Equals(pcOther))
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayNamedDefines::Dump(void)
{
	int				i;
	CNamedDefine*	pcDefine;
	
	for (i = 0; i < miUsedElements; i++)
	{
		pcDefine = Get(i);
		pcDefine->Dump();
	}
}

