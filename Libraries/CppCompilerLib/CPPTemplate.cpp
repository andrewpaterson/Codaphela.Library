/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "CPPTemplate.h"


//Templates and the arguments they are instantiated with.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPTemplateArguments::Init(void)
{
	macBindings.Init();
	mpcOuter = NULL;
	mpcClass = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPTemplateArguments::Kill(void)
{
	macBindings.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A template parameter's binding in these arguments or, if it isn't theirs, in those of the class instance they
// are in, and so on outwards.  NULL if it has none.
//
//////////////////////////////////////////////////////////////////////////
SCPPBinding* CCPPTemplateArguments::Find(CCPPSymbol* pcParameter)
{
	CCPPTemplateArguments*	pcArguments;
	SCPPBinding*			psBinding;
	size					i;

	for (pcArguments = this; pcArguments; pcArguments = pcArguments->mpcOuter)
	{
		for (i = 0; i < pcArguments->macBindings.NumElements(); i++)
		{
			psBinding = pcArguments->macBindings.Get(i);
			if (psBinding->pcParameter == pcParameter)
			{
				return psBinding;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The binding of one of a specialisation's parameters, or NULL if pcParameter is
// not one.
//
//////////////////////////////////////////////////////////////////////////
SCPPBinding* CCPPTemplateArguments::FindOwn(CCPPSymbol* pcParameter)
{
	SCPPBinding*	psBinding;
	size			i;

	if (!pcParameter)
	{
		return NULL;
	}
	for (i = 0; i < macBindings.NumElements(); i++)
	{
		psBinding = macBindings.Get(i);
		if (psBinding->pcParameter == pcParameter)
		{
			return psBinding;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPTemplate::Init(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntax* pcDefinition, bool bMember, CCPPScope* pcNamespace)
{
	mpcSymbol = pcSymbol;
	mpcDeclaration = pcDeclaration;
	mpcDefinition = pcDefinition;
	mbMember = bMember;
	mbMemberTemplate = false;
	mpcClass = NULL;
	mlliName = -1;
	mpcClassDeclaration = NULL;
	mpcDeclarator = NULL;
	mbStatic = false;
	mpcPrimary = this;
	mpcPattern = NULL;
	mpcOwner = NULL;
	mbArguments = false;
	mszArguments.Init();
	mapcInstances.Init();
	mpcNamespace = pcNamespace;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPTemplate::Kill(void)
{
	mapcInstances.Kill();
	mszArguments.Kill();
}

