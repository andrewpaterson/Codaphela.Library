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
#include "BaseLib/Log.h"
#include "ASCIITree.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CASCIITree::Init(void)
{
	CASCIINode*	pcRoot;

	mcNodes.Init(32, sizeof(CASCIINode));

	pcRoot = (CASCIINode*)mcNodes.Add();
	pcRoot->Init(NULL, -1);
	mpcRoot = pcRoot;
	mcWords.Init(32);
	mbContainsUnusedWords = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CASCIITree::Kill(void)
{
	mpcRoot = NULL;
	mcWords.Kill();
	mcNodes.Kill();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CASCIITree::Add(long long int lliID, char* szText, char* szLastCharInclusive)
{
	int	iResult;

	iResult = AddIndex(lliID, szText, szLastCharInclusive);
	return iResult != -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CASCIITree::AddIndex(char* szText, char* szLastCharInclusive)
{
	return AddIndex(-1, szText, szLastCharInclusive);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CASCIITree::AddIndex(long long int lliID, char* szText, char* szLastCharInclusive)
{
	CASCIINode*		pcNode;
	CASCIINode*		pcWord;
	int				iNode;
	int				iWord;
	int				iLen;

#ifdef DEBUG_ASCII_TREE
#ifdef DEBUG
	gcLogger.Info2("CASCIITree::Add(", LongLongToString(lliID), ", \"", szText, "\");", NULL);
#endif //  DEBUG
#endif // DEBUG_ASCII_TREE

	if (szLastCharInclusive == NULL)
	{
		PrivateGetZeroTerminated(&pcNode, &pcWord, &iNode, &iWord, szText);
		iLen = (int)strlen(szText);
		if (pcNode != NULL)
		{
			if ((iLen-1) == iNode)
			{
				if (pcNode->maiWordEnds[szText[iNode]-ASCII_NODE_START_CHAR] != -1)
				{
					//Already exists.
					return -1;
				}
			}
		}
	}
	else
	{
		PrivateGetLengthTerminated(&pcNode, &pcWord, &iNode, &iWord, szText, szLastCharInclusive);
		iLen = (int)(szLastCharInclusive - szText) + 1;
		if (pcNode != NULL)
		{
			if ((iLen-1) == iNode)
			{
				if (pcNode->maiWordEnds[szText[iNode]-ASCII_NODE_START_CHAR] != -1)
				{
					//Already exists.
					return -1;
				}
			}
		}
	}

	return PrivateAdd(pcNode, iNode, iLen, szText, lliID);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CASCIITree::AddOrGet(char* szText, char* szLastCharInclusive)
{
	return AddOrGet(-1LL, szText, szLastCharInclusive);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CASCIITree::AddOrGet(long long int iID, char* szText, char* szLastCharInclusive)
{
	int		iIndex;

	iIndex = GetIndex(szText, szLastCharInclusive, TRUE);
	if (iIndex == -1)
	{
		return AddIndex(iID, szText, szLastCharInclusive);
	}
	else
	{
		return iIndex;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CASCIITree::PrivateAdd(CASCIINode* pcNode, int iNode, int iLen, char* szText, long long int lliID)
{
	char			c;
	int				i;
	CASCIINode*		pcChild;
	int				iIndex;
	CCharsID*		pcChars;

	if (iLen == 0)
	{
		return -1;
	}

	if (!TestOnlyValidCharacters(szText, iLen))
	{
		return -1;
	}
	
	for (i = iNode; i < iLen-1; i++)
	{
		c = szText[i];
		c -= ASCII_NODE_START_CHAR;

		pcChild = (CASCIINode*)mcNodes.Add();
		pcChild->Init(pcNode, (int)c);
		pcNode->mapcChildren[c] = pcChild;
		pcNode = pcChild;
	}

	c = szText[i];
	c -= ASCII_NODE_START_CHAR;

	iIndex = GetIndexForNew(szText, iLen);
	if (iIndex == mcWords.NumElements())
	{
		pcChars = mcWords.Add(szText, 0, iLen);
	}
	else
	{
		pcChars = mcWords.Get(iIndex);
	}

	pcChars->mlliID = lliID;

	pcNode->maiWordEnds[c] = iIndex;
	return iIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CASCIITree::GetIndexForNew(char* szText, int iLen)
{
	int		iIndex;

	if (mbContainsUnusedWords)
	{
		iIndex = PrivateGetWordIndex(szText, iLen);
		if (iIndex == -1)
		{
			iIndex = mcWords.NumElements();
		}
	}
	else
	{
		iIndex = mcWords.NumElements();
	}
	return iIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CASCIITree::PrivateGetWordIndex(char* szText, int iLen)
{
	int		i;
	CChars* psz;

	for (i = 0; i < mcWords.NumElements(); i++)
	{
		psz = mcWords.Get(i);
		if (psz->Equals(szText, iLen))
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CASCIITree::GetIndex(char* szText, char* szLastCharInclusive, BOOL bExact)
{
	CASCIINode*		pcNode;
	CASCIINode*		pcWord;
	int				iNode;
	int				iWord;
	int				iLen;

	if (szLastCharInclusive == NULL)
	{
		PrivateGetZeroTerminated(&pcNode, &pcWord, &iNode, &iWord, szText);
		if (bExact)
		{
			if (pcNode == NULL)
			{
				return -1;
			}

			iLen = (int)strlen(szText);
			if ((iLen-1) == iNode)
			{
				return pcNode->maiWordEnds[szText[iNode]-ASCII_NODE_START_CHAR];
			}
			return -1;
		}
		else
		{
			if (pcWord == NULL)
			{
				return -1;
			}

			return pcWord->maiWordEnds[szText[iWord]-ASCII_NODE_START_CHAR];
		}
	}
	else
	{
		PrivateGetLengthTerminated(&pcNode, &pcWord, &iNode, &iWord, szText, szLastCharInclusive);
		if (bExact)
		{
			if (pcNode == NULL)
			{
				return -1;
			}

			iLen = (int)(szLastCharInclusive - szText);
			if (iLen == iNode)
			{
				return pcNode->maiWordEnds[szText[iNode]-ASCII_NODE_START_CHAR];
			}
			return -1;
		}
		else
		{
			if (pcWord == NULL)
			{
				return -1;
			}

			return pcWord->maiWordEnds[szText[iWord]-ASCII_NODE_START_CHAR];
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
long long int CASCIITree::Get(char* szText, char* szLastCharInclusive, BOOL bExact)
{
	int			iIndex;
	CCharsID*	pszid;

	iIndex = GetIndex(szText, szLastCharInclusive, bExact);
	pszid = GetCharsID(iIndex);
	if (pszid)
	{
		return pszid->mlliID;
	}
	else
	{
		return -1LL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CASCIITree::Remove(char* szText, char* szLastCharInclusive)
{
	return Remove(ATRS_MoveLastToRemoved, szText, szLastCharInclusive);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CASCIITree::Remove(EASCIITreeRemoveStyle eStyle, char* szText, char* szLastCharInclusive)
{
	CASCIINode*		pcNode;
	CASCIINode*		pcWord;
	int				iNode;
	int				iWord;
	int				iLen;

#ifdef DEBUG_ASCII_TREE
#ifdef DEBUG
	char*	szStyle = NULL;
	switch (eStyle)
	{
	case ATRS_OnlyEndOfWordMarker:
		szStyle = "ATRS_OnlyEndOfWordMarker";
		break;
	case ATRS_EndOfWordMarkerReuseIndex:
		szStyle = "ATRS_EndOfWordMarkerReuseIndex";
		break;
	case ATRS_MoveLastToRemoved:
		szStyle = "ATRS_MoveLastToRemoved";
		break;
	}
	gcLogger.Info2("CASCIITree::Remove(", szStyle, ", \"", szText, "\");", NULL);
#endif // DEBUG
#endif // DEBUG_ASCII_TREE

	if (szText == NULL)
	{
		return FALSE;
	}

	if (szLastCharInclusive == NULL)
	{
		PrivateGetZeroTerminated(&pcNode, &pcWord, &iNode, &iWord, szText);
		iLen = (int)strlen(szText);
		if (pcNode != NULL)
		{
			if ((iLen-1) == iNode)
			{
				if (pcNode->maiWordEnds[szText[iNode]-ASCII_NODE_START_CHAR] != -1)
				{
					return PrivateRemove(eStyle, pcNode, iNode, iLen, szText);
				}
			}
		}
	}
	else
	{
		PrivateGetLengthTerminated(&pcNode, &pcWord, &iNode, &iWord, szText, szLastCharInclusive);
		iLen = (int)(szLastCharInclusive - szText);
		if (pcNode != NULL)
		{
			if (iLen == iNode)
			{
				if (pcNode->maiWordEnds[szText[iNode]-ASCII_NODE_START_CHAR] != -1)
				{
					return PrivateRemove(eStyle, pcNode, iNode, iLen, szText);
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
BOOL CASCIITree::PrivateRemove(EASCIITreeRemoveStyle eStyle, CASCIINode* pcNode, int iNode, int iLen, char* szText)
{
	if (eStyle == ATRS_OnlyEndOfWordMarker)
	{
		pcNode->maiWordEnds[szText[iNode]-ASCII_NODE_START_CHAR] = -1;
		return TRUE;
	}
	else if (eStyle == ATRS_EndOfWordMarkerReuseIndex)
	{
		pcNode->maiWordEnds[szText[iNode]-ASCII_NODE_START_CHAR] = -1;
		mbContainsUnusedWords = TRUE;
		return TRUE;
	}
	else if (eStyle == ATRS_MoveLastToRemoved)
	{
		PrivateMoveLastToRemoved(eStyle, pcNode, iNode, iLen, szText);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CASCIITree::PrivateMoveLastToRemoved(EASCIITreeRemoveStyle eStyle, CASCIINode* pcNode, int iNode, int iLen, char* szText)
{
	CASCIINode* pcParent;
	int			iWordEnd;
	CCharsID*	pszid;
	CCharsID*	pszidEnd;
	int			iEndWord;
	int			iEndNode;
	CASCIINode*	pcEndLastWord;
	CASCIINode*	pcEndLastNode;

	char		c;

	iWordEnd = pcNode->maiWordEnds[szText[iNode] - ASCII_NODE_START_CHAR];

	pcNode->maiWordEnds[szText[iNode] - ASCII_NODE_START_CHAR] = -1;
	pszid = mcWords.Get(iWordEnd);
	pszid->Kill();

	while (pcNode != mpcRoot)
	{
		if (pcNode->IsEmpty())
		{
			pcParent = pcNode->mpcParent;
			pcParent->mapcChildren[pcNode->miParentLetter] = NULL;

			mcNodes.Remove(pcNode);
			pcNode = pcParent;
		}
		else
		{
			break;
		}
	}

	if (iWordEnd < mcWords.NumElements()-1)
	{
		mcWords.Remove(iWordEnd, FALSE);

		pszidEnd = mcWords.Get(iWordEnd);
		PrivateGetZeroTerminated(&pcEndLastNode, &pcEndLastWord, &iEndNode, &iEndWord, pszidEnd->Text());

		c = pszidEnd->GetChar(pszidEnd->Length()-1);
		c -= ASCII_NODE_START_CHAR;
		pcEndLastNode->maiWordEnds[c] = iWordEnd;
	}
	else
	{
		mcWords.Remove(iWordEnd);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CASCIITree::NumElements(void)
{
	return mcWords.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCharsID* CASCIITree::GetCharsID(int iIndex)
{
	return mcWords.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SASCIITreePos* CASCIITree::StackTo(CArrayASCIITreePos* pcStack, CASCIINode* pcNode, int iNode, char* szText)
{
	int					i;
	SASCIITreePos*		psPos;

	if (iNode != -1)
	{
		for (i = iNode; i >= 0; i--)
		{
			psPos = pcStack->Add();
			psPos->iLetter = szText[i]-ASCII_NODE_START_CHAR;
			psPos->pcNode = pcNode;

			pcNode = pcNode->mpcParent;

			if (pcNode == NULL)
			{
				break;
			}
		}
		pcStack->Reverse();	
	}
	else
	{
		psPos = pcStack->Add();
		psPos->iLetter = 0;
		psPos->pcNode = pcNode;
	}

	psPos = pcStack->Tail();
	return psPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CASCIITree::GetBetween(CArrayInt* pcArrayInt, char* szTextZeroTerminatedFirst, char* szTextZeroTerminatedSecond)
{
	CASCIINode*			pcNodeFirst;
	int					iNodeFirst;
	CASCIINode*			pcNodeSecond;
	int					iNodeSecond;
	CASCIINode*			pcWord;
	int					iWord;
	CArrayASCIITreePos	cStack;
	int					iLetterFirst;
	int					iLetterSecond;
	SASCIITreePos*		psPos;

	if (szTextZeroTerminatedFirst)
	{
		PrivateGetZeroTerminated(&pcNodeFirst, &pcWord, &iNodeFirst, &iWord, szTextZeroTerminatedFirst);
		iLetterFirst = szTextZeroTerminatedFirst[iNodeFirst]-ASCII_NODE_START_CHAR;
	}
	else
	{
		pcNodeFirst = mpcRoot;
		iLetterFirst = 0;
		iNodeFirst = -1;
	}

	if (szTextZeroTerminatedSecond)
	{
		PrivateGetZeroTerminated(&pcNodeSecond, &pcWord, &iNodeSecond, &iWord, szTextZeroTerminatedSecond);
		iLetterSecond = szTextZeroTerminatedSecond[iNodeSecond]-ASCII_NODE_START_CHAR;
	}
	else
	{
		pcNodeSecond = mpcRoot;
		iLetterSecond = ASCII_NODE_MAX_CHARS;
	}

	cStack.Init(32);
	psPos = StackTo(&cStack, pcNodeFirst, iNodeFirst, szTextZeroTerminatedFirst);

	PrivateGetBetween(pcArrayInt, psPos, &cStack, pcNodeFirst, pcNodeSecond, iLetterFirst, iLetterSecond);

	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CASCIITree::GetBetween(CArrayInt* pcArrayInt, char* szTextFirst, char* szTextFirstszLastCharInclusive, char* szTextSecond, char* szTextSecondszLastCharInclusive)
{
	CASCIINode*			pcNodeFirst;
	int					iNodeFirst;
	CASCIINode*			pcNodeSecond;
	int					iNodeSecond;
	CASCIINode*			pcWord;
	int					iWord;
	CArrayASCIITreePos	cStack;
	int					iLetterFirst;
	int					iLetterSecond;
	SASCIITreePos*		psPos;

	if ((szTextFirst) && (szTextFirstszLastCharInclusive))
	{
		PrivateGetLengthTerminated(&pcNodeFirst, &pcWord, &iNodeFirst, &iWord, szTextFirst, szTextFirstszLastCharInclusive);
		iLetterFirst = szTextFirst[iNodeFirst]-ASCII_NODE_START_CHAR;
	}
	else
	{
		pcNodeFirst = mpcRoot;
		iLetterFirst = 0;
		iNodeFirst = -1;
	}

	if ((szTextSecond) && (szTextSecondszLastCharInclusive))
	{
		PrivateGetLengthTerminated(&pcNodeSecond, &pcWord, &iNodeSecond, &iWord, szTextSecond, szTextSecondszLastCharInclusive);
		iLetterSecond = szTextSecond[iNodeSecond]-ASCII_NODE_START_CHAR;
	}
	else
	{
		pcNodeSecond = mpcRoot;
		iLetterSecond = ASCII_NODE_MAX_CHARS;
	}

	cStack.Init(32);
	psPos = StackTo(&cStack, pcNodeFirst, iNodeFirst, szTextFirst);

	PrivateGetBetween(pcArrayInt, psPos, &cStack, pcNodeFirst, pcNodeSecond, iLetterFirst, iLetterSecond);

	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CASCIITree::PrivateGetBetween(CArrayInt* pcArrayInt, SASCIITreePos* psPos, CArrayASCIITreePos* pcStack, CASCIINode* pcNodeFirst, CASCIINode* pcNodeSecond, int iLetterFirst, int iLetterSecond)
{
	CASCIINode*			pcChild;
	int					iIndex;

	for (;;)
	{
		for (; psPos->iLetter <= ASCII_NODE_MAX_CHARS; psPos->iLetter++)
		{
			if ((psPos->pcNode == pcNodeSecond) && (psPos->iLetter == iLetterSecond))
			{
				//Function exits here...
				return;
			}

			if (psPos->iLetter == ASCII_NODE_MAX_CHARS)
			{
				continue;
			}

			if (!((psPos->pcNode == pcNodeFirst) && (psPos->iLetter == iLetterFirst)))
			{
				iIndex = psPos->pcNode->maiWordEnds[psPos->iLetter];
				if (iIndex != -1)
				{
					pcArrayInt->Add(iIndex);
				}
			}

			pcChild = psPos->pcNode->mapcChildren[psPos->iLetter];
			if (pcChild != NULL)
			{
				psPos = pcStack->Add();
				psPos->iLetter = 0;
				psPos->pcNode = pcChild;
			}
		}

		pcStack->RemoveTail();
		psPos = pcStack->Tail();
		psPos->iLetter++;
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CASCIITree::PrivateGetZeroTerminated(CASCIINode** ppcLastNode, CASCIINode** ppcLastWord, int* piLastNode, int* piLastWord, char* szText)
{
	int				i;
	CASCIINode*		pcLast;
	CASCIINode*		pcNode;
	CASCIINode*		pcChild;
	unsigned char	c;

	pcLast = NULL;
	pcNode = mpcRoot;
	*piLastWord = -1;
	*ppcLastWord = NULL;

	if (szText == NULL)
	{
		*ppcLastNode = NULL;
		*piLastNode = -1;
		return;
	}

	c = 0;
	for (i = 0;; i++)
	{
		c = szText[i];
		c -= ASCII_NODE_START_CHAR;
		if (c < ASCII_NODE_MAX_CHARS)
		{
			if (pcNode->maiWordEnds[c] != -1)
			{
				*ppcLastWord = pcNode;
				*piLastWord = i;
			}
			pcChild = pcNode->mapcChildren[c];
			if (pcChild)
			{
				pcLast = pcNode;
				pcNode = pcChild;
			}
			else
			{
				*ppcLastNode = pcNode;
				*piLastNode = i;
				return;
			}
		}
		else
		{
			*ppcLastNode = pcLast;
			*piLastNode = i - 1;
			return;
		}
	}	
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CASCIITree::PrivateGetLengthTerminated(CASCIINode** ppcLastNode, CASCIINode** ppcLastWord, int* piLastNode, int* piLastWord, char* szTextStart, char* szTextEndInclusive)
{
	int				i;
	CASCIINode*		pcLast;
	CASCIINode*		pcNode;
	CASCIINode*		pcChild;
	unsigned char	c;
	int				iLen;

	pcLast = NULL;
	pcNode = mpcRoot;
	*piLastWord = -1;
	*ppcLastWord = NULL;

	if ((szTextStart == NULL) || (szTextEndInclusive == NULL))
	{
		*ppcLastNode = NULL;
		*piLastNode = -1;
		return;
	}

	c = 0;
	iLen = (int)(szTextEndInclusive - szTextStart);
	for (i = 0; i <= iLen; i++)
	{
		c = szTextStart[i];
		c -= ASCII_NODE_START_CHAR;

		if (pcNode->maiWordEnds[c] != -1)
		{
			*ppcLastWord = pcNode;
			*piLastWord = i;
		}
		pcChild = pcNode->mapcChildren[c];
		if (pcChild)
		{
			pcLast = pcNode;
			pcNode = pcChild;
		}
		else
		{
			*ppcLastNode = pcNode;
			*piLastNode = i;
			return;
		}
	}
	*ppcLastNode = pcLast;
	*piLastNode = i-1;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CASCIITree::Dump(void)
{
	CChars	sz;

	sz.Init();
	RecurseDump(mpcRoot, &sz, 0);
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CASCIITree::RecurseDump(CASCIINode* pcNode, CChars* psz, int iLevel)
{
	int		i;

	for (i = 0; i < ASCII_NODE_MAX_CHARS; i++)
	{
		if (pcNode->maiWordEnds[i] != -1)
		{
			psz->Append(' ', iLevel);
			psz->Append((char)(i + ASCII_NODE_START_CHAR));
			psz->Append(" [END]");
			psz->AppendNewLine();
		}

		if (pcNode->mapcChildren[i] != NULL)
		{
			psz->Append(' ', iLevel);
			psz->Append((char)(i + ASCII_NODE_START_CHAR));

			psz->AppendNewLine();

			RecurseDump(pcNode->mapcChildren[i], psz, iLevel+1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CASCIITree::DumpWords(void)
{
	mcWords.Dump();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CASCIITree::Dump(CArrayInt* pcArrayInt)
{
	int			i;
	int			iIndex;
	CChars		sz2;
	CCharsID*	psz;

	sz2.Init(1024);
	for (i = 0; i < pcArrayInt->NumElements(); i++)
	{
		iIndex = pcArrayInt->GetValue(i);
		psz = GetCharsID(iIndex);
		if (psz)
		{
			sz2.Append(psz->mlliID);
			sz2.Append(":");
			sz2.Append(psz->Text());
			sz2.AppendNewLine();
		}
	}
	sz2.Dump();
	sz2.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CASCIITree::DumpTree(void)
{
	CArrayInt		aiBetween;

	aiBetween.Init(1024);
	GetBetween(&aiBetween, NULL, NULL);
	Dump(&aiBetween);
	aiBetween.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CASCIITree::IsEmpty(void)
{
	return mpcRoot->IsEmpty();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CASCIITree::Contains(char* szText)
{
	long long int	lli;

	if ((szText == NULL) || (szText[0] == '\0'))
	{
		return FALSE;
	}
	
	lli = Get(szText);
	return lli != -1LL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CASCIITree::IsOnlyValidCharacters(char* szText)
{
	int		i;

	for (i = 0;; i++)
	{
		if ((szText == NULL) || (szText[i] == 0))
		{
			return TRUE;
		}
		else if (szText[i] < ASCII_NODE_START_CHAR)
		{
			return FALSE;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CASCIITree::TestConsistency(void)
{
	if (!TestWordConsistency(mpcRoot))
	{
		return FALSE;
	}

	if (!TestNodeConsistency(mpcRoot, 0))
	{
		return FALSE;
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CASCIITree::TestWordConsistency(CASCIINode* pcNode)
{
	int		i;
	BOOL	bResult;

	if (pcNode)
	{
		for (i = 0; i < ASCII_NODE_MAX_CHARS; i++)
		{
			if ((pcNode->maiWordEnds[i] >= mcWords.mcArray.NumElements()) || (pcNode->maiWordEnds[i] < -1))
			{
				return FALSE;
			}
		}

		for (i = 0; i < ASCII_NODE_MAX_CHARS; i++)
		{
			bResult = TestWordConsistency(pcNode->mapcChildren[i]);
			if (!bResult)
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CASCIITree::TestNodeConsistency(CASCIINode* pcNode, int iLevel)
{
	int		i;
	BOOL	bDetail;

	bDetail = FALSE;
	for (i = 0; i < ASCII_NODE_MAX_CHARS; i++)
	{
		if (pcNode->maiWordEnds[i] != -1)
		{
			bDetail = TRUE;
		}

		if (pcNode->mapcChildren[i] != NULL)
		{
			bDetail = TRUE;
			if (!TestNodeConsistency(pcNode->mapcChildren[i], iLevel+1))
			{
				return FALSE;
			}
		}
	}

	if (!bDetail && iLevel > 0)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CASCIITree::TestOnlyValidCharacters(char* szText, int iLen)
{
	int		i;

	for (i = 0; i < iLen; i++)
	{
		if (szText[i] < ASCII_NODE_START_CHAR)
		{
			gcUserError.Set("ASCIITree cannot add a string with a character 0...32.");
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
long long int CASCIITree::StartIteration(SASCIITreeIter* psIter)
{
	psIter->iLetter = -1;
	psIter->pcNode = NULL;

	if (mpcRoot == NULL)
	{
		return -1LL;
	}
	else
	{
		psIter->pcNode = mpcRoot;
		psIter->iLetter = -1;
		return Iterate(psIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
long long int CASCIITree::Iterate(SASCIITreeIter* psIter)
{
	int				iWordEnd;
	CASCIINode*		psChildNode;
	CCharsID*		pszid;

	for (;;)
	{
		psIter->iLetter++;
		if (psIter->iLetter >= ASCII_NODE_MAX_CHARS)
		{
			//Down.
			psIter->iLetter = psIter->pcNode->miParentLetter;
			psIter->pcNode = psIter->pcNode->mpcParent;
			
			if (psIter->pcNode == NULL)
			{
				return -1LL;
			}
		}
		else
		{
			iWordEnd = psIter->pcNode->maiWordEnds[psIter->iLetter];

			psChildNode = psIter->pcNode->mapcChildren[psIter->iLetter];
			if (psChildNode != NULL)
			{
				psIter->pcNode = psChildNode;
				psIter->iLetter = 0;
			}

			if (iWordEnd != -1)
			{
				pszid = mcWords.Get(iWordEnd);
				return pszid->mlliID;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CASCIITree::GetWord(int iIndex)
{
	CCharsID*	pszid;

	pszid = mcWords.Get(iIndex); 
	if (pszid)
	{
		return pszid->Text();
	}
	else
	{
		return NULL;
	}
}
