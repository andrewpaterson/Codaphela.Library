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
void CNamedDefine::Define(CExternalString* pcName, CDefine* pcSource, CPPTokenMemory* pcTokens)
{
	CPPLine	cLine;

	memcpy(&mcReplacement, &cLine, sizeof(CPPLine));

	mszName.Init();
	mszName.AppendSubString(pcName->msz, pcName->miLen);
	miFlags = pcSource->IsBacketed() ? NAMED_DEFINE_FLAGS_BRACKETED : 0;
	maszArguments.Init();
	maszArguments.Copy(pcSource->GetArguments());
	mcReplacement.Init(pcSource->GetReplacement()->Line(), pcSource->GetReplacement()->Column());
	mcReplacement.Copy(pcSource->GetReplacement(), pcTokens);
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
	maszArguments.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedDefine::Kill(void)
{
	mszName.Kill();
	maszArguments.Kill();
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
		if (mcReplacement.Equals(pcOther->GetReplacement()))
		{
			if (miFlags == pcOther->miFlags)
			{
				if (maszArguments.Equals(pcOther->GetArguments()))
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
		for (i = 0; i < maszArguments.NumElements(); i++)
		{
			psz = maszArguments.Get(i);
			sz.Append(psz->Text());

			if (i != maszArguments.NumElements()-1)
			{
				sz.Append(", ");
			}
		}
		sz.Append(')');
	}

	if (mcReplacement.TokenLength() > 0)
	{
		sz.Append(' ');
		mcReplacement.Print(&sz);
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedDefine::IsUndeffed(void)
{
	return FixBool(miFlags & NAMED_DEFINE_FLAGS_UNDEFFED);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CNamedDefine::GetName(void)
{
	return mszName.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CNamedDefine::GetNameLength(void)
{
	return mszName.Length();
}





//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayChars* CNamedDefine::GetArguments(void)
{
	return &maszArguments;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPLine* CNamedDefine::GetReplacement(void)
{
	return &mcReplacement;
}

