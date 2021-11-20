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
#include "DefineArguments.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefineArguments::Init(void)
{
	mcDefineToArguments.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefineArguments::Kill(void)
{
	int		i;

	SDefineArgument*	psDefineArg;

	for (i = 0; i < mcDefineToArguments.NumElements(); i++)
	{
		psDefineArg = mcDefineToArguments.Get(i);
		psDefineArg->mcArguments.Kill();
	}

	mcDefineToArguments.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SDefineArgument* CDefineArguments::Get(int64 lliDefineID)
{
	int					i;
	SDefineArgument*	psDefineArg;

	for (i = 0; i < mcDefineToArguments.NumElements(); i++)
	{
		psDefineArg = mcDefineToArguments.Get(i);
		if (psDefineArg->mlliDefineID == lliDefineID)
		{
			return psDefineArg;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SDefineArgument* CDefineArguments::Add(int64 lliDefineID)
{
	SDefineArgument*	psDefineArg;

	psDefineArg = Get(lliDefineID);
	if (psDefineArg)
	{
		psDefineArg->mcArguments.Kill();
	}
	else
	{
		psDefineArg = mcDefineToArguments.Add();
	}
	psDefineArg->mcArguments.Init();
	psDefineArg->mlliDefineID = lliDefineID;
	return psDefineArg;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefineArguments::Remove(int64 lliDefineID)
{
	int					i;
	SDefineArgument*	psDefineArg;

	for (i = 0; i < mcDefineToArguments.NumElements(); i++)
	{
		psDefineArg = mcDefineToArguments.Get(i);
		if (psDefineArg->mlliDefineID == lliDefineID)
		{
			psDefineArg->mcArguments.Kill();
			mcDefineToArguments.RemoveAt(i, FALSE);
			return;
		}
	}
}

