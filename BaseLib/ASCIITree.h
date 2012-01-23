/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __ASCII_TREE_H__
#define __ASCII_TREE_H__
#include "ASCIINode.h"
#include "FreeList.h"
#include "ArrayString.h"
#include "CharsIDArray.h"


typedef CFreeList<CASCIINode>	CASCIINodes;


enum EASCIITreeRemoveStyle
{
	ATRS_OnlyEndOfWordMarker,
	ATRS_EndOfWordMarkerReuseIndex,
	ATRS_MoveLastToRemoved,
};


struct SASCIITreePos
{
	CASCIINode*		pcNode;
	int				iLetter;
};


typedef CArrayTemplate<SASCIITreePos> CArrayASCIITreePos;


struct SASCIITreeIter
{
	CASCIINode*		pcNode;
	int				iLetter;
};


class CASCIITree
{
public:
	CASCIINodes		mcNodes;
	CASCIINode*		mpcRoot;
	CCharsIDArray	mcWords;
	BOOL			mbContainsUnusedWords;

	void			Init(void);
	void			Kill(void);

	int				Add(char* szText, char* szLastCharInclusive = NULL);
	int				Add(long long int lliID, char* szText, char* szLastCharInclusive = NULL);

	long long int	Get(char* szText, char* szLastCharInclusive = NULL, BOOL bExact = TRUE);

	BOOL			Remove(char* szText, char* szLastCharInclusive = NULL);
	BOOL			Remove(EASCIITreeRemoveStyle eStyle, char* szText, char* szLastCharInclusive = NULL);

	int				NumElements(void);

	long long int	StartIteration(SASCIITreeIter* psIter);
	long long int	Iterate(SASCIITreeIter* psIter);

	BOOL			TestConsistency(void);
	BOOL			IsEmpty(void);

	void			DumpWords(void);
	void			Dump(CArrayInt* pcArrayInt);
	void			DumpTree(void);

//Unprotected for the CPP Parser Lib
	int				AddOrGet(char* szText, char* szLastCharInclusive);
	int				AddOrGet(long long int iID, char* szText, char* szLastCharInclusive);
	int				GetIndex(char* szText, char* szLastCharInclusive = NULL, BOOL bExact = TRUE);
	int				GetIndexForNew(char* szText, int iLen);
	char*			GetWord(int iIndex);
	void			GetBetween(CArrayInt* pcArrayInt, char* szTextZeroTerminatedFirst, char* szTextZeroTerminatedSecond);
	void			PrivateGetZeroTerminated(CASCIINode** ppcLastNode, CASCIINode** ppcLastWord, int* piLastNode, int* piLastWord, char* szText);
	void			PrivateGetLengthTerminated(CASCIINode** ppcLastNode, CASCIINode** ppcLastWord, int* piLastNode, int* piLastWord, char* szTextStart, char* szTextEndInclusive);

protected:
	BOOL			TestConsistency(CASCIINode* pcNode);
	BOOL			TestOnlyValidCharacters(char* szText, int iLen);
	CCharsID*		GetCharsID(int iIndex);

	void			GetBetween(CArrayInt* pcArrayInt, char* szTextFirst, char* szTextFirstszLastCharInclusive, char* szTextSecond, char* szTextSecondszLastCharInclusive);
	SASCIITreePos*	StackTo(CArrayASCIITreePos* pcStack, CASCIINode* pcNode, int iNode, char* szText);

	int				PrivateAdd(CASCIINode* pcNode, int iNode, int iLen, char* szText, long long int lliID);
	BOOL			PrivateRemove(EASCIITreeRemoveStyle eStyle, CASCIINode* pcNode, int iNode, int iLen, char* szText);
	void			PrivateMoveLastToRemoved(EASCIITreeRemoveStyle eStyle, CASCIINode* pcNode, int iNode, int iLen, char* szText);
	int				PrivateGetWordIndex(char* szText, int iLen);
	void			PrivateGetBetween(CArrayInt* pcArrayInt, SASCIITreePos* psPos, CArrayASCIITreePos* pcStack, CASCIINode* pcNodeFirst, CASCIINode* pcNodeSecond, int iLetterFirst, int iLetterSecond);

};


#endif // __ASCII_TREE_H__

