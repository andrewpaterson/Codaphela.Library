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
#ifndef __DEFINITION_H__
#define __DEFINITION_H__
#include "CoreLib/TextParser.h"
#include "BaseLib/ArrayInt.h"
#include "ElementTypes.h"
#include "NodeConstructor.h"


class CDefinition
{
public:
	SParseState		msParserState;  //Unnecessary after parsing
	CChars			mszName;  //Not strictly necessay.  Ever.
	CArrayInt		mcElements;
	BOOL			mbComplete;  //Unnecessary after parsing
	int				miNumber;

	void 	Init(char* szName, int iNumber);
	void 	Kill(void);
	void 	Copy(CDefinition* pcDefinition);
	int		NextElementPosition(void);

	void 	AddCharacter(char c);
	void 	AddUnknownIdentifier(void);
	void 	AddUnknownIdentifier(int iEnumerationId);
	void 	AddPrimitiveType(EElement eType);
	void 	AddText(char* szString);
	void	AddNode(int iPos, CNodeConstructor* pcNode);
	void 	AddDefinition(int iDefinitionNum);
	void    AddEnd(void);
	void	SetFirstNodeName(char* szNodeName);

	void	Dump(void);
	void	RecurseDump(int iNextElement, int iNumElements, int iLevel);
};


#endif //__DEFINITION_H__
