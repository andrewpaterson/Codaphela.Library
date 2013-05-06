/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LanguageLib

Codaphela LanguageLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LanguageLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LanguageLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __DEFINITION_ELEMENT_H__
#define __DEFINITION_ELEMENT_H__
#include "ElementTypes.h"
#include "Meta.h"


class CDefinition;


struct SDefinitionElement
{
	EElement	eET_Definition;  //This int must always have the value ET_Definition.
	int			iDefinitionNum;

	void Init(int iDefinitionNum)
	{
		eET_Definition = ET_Definition;
		this->iDefinitionNum = iDefinitionNum;
	}

	int Size(void)
	{
		return 2;
	}

	void String(CChars* szString)
	{
		CDefinition* pcDefinition;
			
		pcDefinition = gpcCurrentMeta->GetDefinition(iDefinitionNum);

		szString->Init("DEFINITION(");
		szString->Append(pcDefinition->mszName);
		szString->Append(", ");
		szString->Append(iDefinitionNum);
		szString->Append(')');
	}
};


#endif //__DEFINITION_ELEMENT_H__