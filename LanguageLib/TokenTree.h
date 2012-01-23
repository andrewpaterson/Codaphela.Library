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
#ifndef __TOKEN_TREE_H__
#define __TOKEN_TREE_H__
#include "BaseLib/EnumeratorVoid.h"
#include "BaseLib/EnumeratorEnumerator.h"
#include "BaseLib/TreeBlock.h"
#include "TokenArray.h"


enum ETokenTreeNodeType
{
	TTNT_Flat,		//int[]
	TTNT_Dynamic,	//CTokenArray
};


enum EEndNode
{
	EN_Parsed,
	EN_Failed,
	EN_InternalError,
};


class CTokenTree
{
public:
	CTreeBlock			mcTree;
	CEnumeratorVoid			mcNodeNames;
	CEnumeratorVoid			mcKeywords;
	CEnumeratorVoid			mcIdentifiers;
	CTokenArray*			mpcCurrentNode;

	void Init(void);
	void Kill(void);
	void Dump(void);

	void BeginTree(void);
	void EndTree(void);
	void BeginNode(SNodeElement* psNode);
	BOOL EndNode(EEndNode eEnd);

	void AddIdentifier(char* sz);
	void AddKeywordText(char* sz);
	void AddKeywordChar(char c);
	void AddInteger(int i);
	void AddReal(float f);
	void AddUnknownText(char* sz);
	void AddUnknownChar(char c);

	BOOL RollBack(void);
	void Finalise(void);
	void FinaliseNode(CTokenArray* pcParent);
	BOOL ContainsFinalNodes(CTokenArray* pcStartNode);
	void KillTokenArrays(CTokenArray* pcStartNode);
};


#endif //__TOKEN_TREE_H__

