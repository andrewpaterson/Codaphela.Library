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
#ifndef __TOKEN_ARRAY_H__
#define __TOKEN_ARRAY_H__
#include "BaseLib/ArrayInt.h"
#include "NodeElement.h"


class CTokenTree;
class CTokenArray
{
public:
	CArrayInt				mcTokens;
	SNodeElement*			mpsNode;  //The node definition containing the above elements.
	CTokenTree*				mpcTree;
	BOOL					mbFinalised;

	void 	Init(CTokenTree* pcTree, SNodeElement* psNode);
	void 	Kill(void);
	void 	Dump(void);

	void 	AddIdentifier(char* sz);
	void 	AddKeywordText(char* sz);
    void 	AddKeywordChar(char c);
	void 	AddInteger(int i);
	void 	AddReal(float f);
	void 	AddUnknownText(char* sz);
	void 	AddUnknownChar(char c);
	void 	AddNode(void);
	int		GetNodeIndex(int iNum);
	void 	RemoveNode(int iNum);
	void 	ReplaceNode(int iNum, CTokenArray* pcReplacement);

	int*	GrowBy(int iNum);
};


#endif //__TOKEN_ARRAY_H__

/*
struct STokenSingle					//ET_Error, ET_Unknown, ET_Node <- seriously
struct STokenIdentifier				//ET_Identifier
struct STokenKeywordText			//ET_Text
struct STokenKeywordChar			//ET_Char
struct STokenInteger				//PT_IntegerType
struct STokenReal					//PT_RealType
struct STokenUnknownText			//PT_StringType
struct STokenUnknownChar			//PT_CharType
*/

