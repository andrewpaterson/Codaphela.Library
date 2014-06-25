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
#ifndef __TEMPLATE_ENUMERATOR_H__
#define __TEMPLATE_ENUMERATOR_H__
#include "Define.h"
#include "DataTypes.h"
#include "ArrayInt.h"
#include "LinkListTemplate.h"
#include "StringHelper.h"


#define ENUMERATOR_MANAGED	1


struct SEnumeratorIterator
{
	BOOL		bValid;
	SENode*		psNode;
};


typedef CLinkListTemplate<SENode>  CLinkListSENode;

//Do not confuse the enumerator with a map.  The enumerator is designed for fast access based on an integer (which is not an index)
template<class M>
class __CEnumeratorTemplate
{
public:
	CArrayInt			mcIDArray;
	CLinkListSENode		mcList;
	BOOL				mbCaseSensitive;

public:
	void		Init(void);
	void		Kill(void);
	int			Get(char* szName, M** pvData);
	int			Get(char* szName, int iNameLen, M** pvData);
	int			GetNumMatchingWithKey(char* szName, M* pvKey, int iKeySize);
	int			GetWithKey(char* szName, M* pvKey, int iKeySize, M** pvData, int iMatchNum = 0);
	int			GetWithKey(char* szName, int iNameLen, M* pvKey, int iKeySize, M** pvData, int iMatchNum = 0);
	void		GetWithID(int iID, M** pvData, char** pcName);
	int			NumElements(void);
	void		StartIteration(SEnumeratorIterator* psIterator, char** szName, int* piID, M** pvData);
	void		Iterate(SEnumeratorIterator* psIterator, char** szName, int* piID, M** pvData);
	void		Remove(int iID);
	void		Remove(char* szName);
	void		Swap(SENode* psNode1, SENode* psNode2);
	void		SwapUp(char* szName);
	void		SwapDown(char* szName);
	void		QuickSort(void);
	void		BubbleSort(void);
	void		RecreateIDArray(void);
	void		AllocateNodeData(SENode* psNode, int iLen);

protected:
	void*		Malloc(size_t tSize);
	void*		Realloc(void* pv, size_t iMemSize);
	void		Free(void* pv);

	int			PrivateAddGetNode(char* szName, M* pvData, int iDataSize, int iKeySize, int iNum, BOOL bReplace, SENode** pcThisNode);
	SENode*		PrivateGetWithKey(char* szName, M* pvKey, int iKeySize, SENode* psStartNode);
	SENode*		PrivateGetWithKey(char* szName, int iNameLen, M* pvKey, int iKeySize, SENode* psStartNode);
	int			PrivateGetNextID(int iNum);
	void		PrivateInsertID(SENode* psNode);
	void		PrivateRemoveID(SENode* psNode);
	void		PrivateRemove(SENode* psNode);
	BOOL		PrivateCompare(char* szName, M* pvKey, int iKeySize, SENode* psNode);
	BOOL		PrivateCompare(char* szName, int iNameLen, M* pvKey, int iKeySize, SENode* psNode);
};


template<class M>
class CEnumeratorTemplate : public __CEnumeratorTemplate<M>
{
public:
	int		Add(char* szName, M* pvData, int iNum);
	int		Add(char* szName, M* pvData, int iKeySize, int iNum, BOOL bReplace = TRUE);
	int		Add(char* szName, int iNameLen, M* pvData, int iKeySize, int iNum, BOOL bReplace = TRUE);

	BOOL	WriteEnumeratorTemplate(CFileWriter* pcFileWriter);
	BOOL	ReadEnumeratorTemplate(CFileReader* pcFileReader);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void* __CEnumeratorTemplate<M>::Malloc(size_t tSize)
{
	return malloc(tSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::Free(void* pv)
{
	free(pv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void* __CEnumeratorTemplate<M>::Realloc(void* pv, size_t tSize)
{
	pv = realloc(pv, tSize);
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::Init(void)
{
	mbCaseSensitive = TRUE;
	mcIDArray.Init(4096);
	mcList.Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::Kill(void)
{
	SENode*		psNode;

	psNode = mcList.GetHead();
	while (psNode)
	{
		if (psNode->pvData)
		{
			if (psNode->iFlags & ENUMERATOR_MANAGED)
			{
				Free(psNode->pvData);
				psNode->pvData = NULL;
			}
		}
		if (psNode->szName)
		{
			Free(psNode->szName);
		}
		psNode = mcList.GetNext(psNode);
	}

	mcIDArray.Kill();
	mcList.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::AllocateNodeData(SENode* psNode, int iLen)
{
	psNode->szName = (char*)Malloc(iLen);
	if (psNode->iDataSize)
	{
		psNode->pvData = Malloc(psNode->iDataSize);
	}
	else
	{
		psNode->pvData = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CEnumeratorTemplate<M>::PrivateAddGetNode(char* szName, M* pvData, int iDataSize, int iKeySize, int iNum, BOOL bReplace, SENode** pcThisNode)
{
	SENode*		psNode;
	int			iLen;

	psNode = PrivateGetWithKey(szName, pvData, iKeySize, NULL);

	//Handle the case where the node exists and must be replaced.
	if (psNode && bReplace)
	{
		//Make sure the replaced nodes ID does not have and effect on the new ID.
		PrivateRemoveID(psNode);
		psNode->iNum = -1;
		if (iNum == -1)
		{
			psNode->iNum = PrivateGetNextID(iNum);
		}
		else
		{
			psNode->iNum = iNum;
		}
		PrivateInsertID(psNode);

		//If the user wants to add data to this node.
		if (pvData)
		{
			//If the data sizes are different then free the current node.
			if (psNode->iDataSize != iDataSize)
			{
				if (psNode->iFlags & ENUMERATOR_MANAGED)
				{
					Free(psNode->pvData);
					psNode->pvData = NULL;
				}
			}

			//If no data exists then allocate it.
			if (iDataSize)
			{
				if (psNode->pvData == NULL)
				{
					psNode->pvData = Malloc(iDataSize);
				}
				psNode->iFlags = ENUMERATOR_MANAGED;

				//Copy the data into the nodes data.
				memcpy(psNode->pvData, pvData, iDataSize);
			}
			else
			{
				psNode->iFlags = 0;
				psNode->pvData = pvData;
			}

			psNode->iDataSize = iDataSize;
		}
		//The user doesn't want data on this node.
		else
		{
			//Free any existing data.
			if (psNode->pvData)
			{
				if (psNode->iFlags & ENUMERATOR_MANAGED)
				{
					Free(psNode->pvData);
				}
				psNode->pvData = NULL;
			}
		}
		psNode->iFlags = 0;
		psNode->iDataSize = 0;

		//Return the node just allocated.
		if (pcThisNode)
		{
			*pcThisNode = psNode;
		}
		return psNode->iNum;
	}

	//Handle the case where the node exists but is not being replaced.
	if (psNode)
	{
		//Return the node just allocated.
		if (pcThisNode)
		{
			*pcThisNode = psNode;
		}
		return psNode->iNum;
	}

	//Handle the case where the node does not exist.
	if (psNode == NULL)
	{
		iLen = (int)strlen(szName) + 1;
		psNode = mcList.InsertAfterTail();

		psNode->iFlags = 0;
		psNode->iNum = PrivateGetNextID(iNum);
		psNode->iDataSize = iDataSize;

		AllocateNodeData(psNode, iLen);

		if (iDataSize)
		{
			psNode->iFlags = ENUMERATOR_MANAGED;
			if (pvData)
			{
				memcpy(psNode->pvData, pvData, iDataSize);
			}
		}
		else
		{
			psNode->pvData = pvData;
		}

		strcpy(psNode->szName, szName);

		PrivateInsertID(psNode);

		//Return the node just allocated.
		if (pcThisNode)
		{
			*pcThisNode = psNode;
		}
		return psNode->iNum;
	}

	//Something probably went wrong.
	if (pcThisNode)
	{
		*pcThisNode = NULL;
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::PrivateInsertID(SENode* psNode)
{
	int		iID;

	iID = psNode->iNum;
	mcIDArray.AddRemap(iID, (int)((ENGINE_SIZE_T) psNode));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::PrivateRemoveID(SENode* psNode)
{
	int iID;

	iID = psNode->iNum;
	mcIDArray.RemoveRemap(iID);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
SENode* __CEnumeratorTemplate<M>::PrivateGetWithKey(char* szName, M* pvKey, int iKeySize, SENode* psStartNode)
{
	SENode*		psNode;

	if (psStartNode)
	{
		psNode = psStartNode;
	}
	else
	{
		psNode = mcList.GetHead();
	}
	while (psNode)
	{
		if (PrivateCompare(szName, pvKey, iKeySize, psNode))
		{
			return psNode;
		}
		psNode = mcList.GetNext(psNode);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
SENode* __CEnumeratorTemplate<M>::PrivateGetWithKey(char* szName, int iNameLen, M* pvKey, int iKeySize, SENode* psStartNode)
{
	SENode*		psNode;

	if (psStartNode)
	{
		psNode = psStartNode;
	}
	else
	{
		psNode = mcList.GetHead();
	}
	while (psNode)
	{
		if (PrivateCompare(szName, iNameLen, pvKey, iKeySize, psNode))
		{
			return psNode;
		}
		psNode = mcList.GetNext(psNode);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CEnumeratorTemplate<M>::PrivateGetNextID(int iNum)
{
	if (iNum == -1)
	{
		return mcIDArray.NumElements();
	}
	return iNum;

	//Longer version which might be safer.
	//int			iNextID;
	//SENode*		psNode;
	//psNode = mcList.GetHead();
	//iNextID = 0;
	//while(psNode)
	//{
	//	if (psNode->iNum >= iNextID)
	//	{
	//		iNextID = psNode->iNum + 1;
	//	}
	//	psNode = mcList.GetNext(psNode);
	//}
	//return iNextID;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CEnumeratorTemplate<M>::Get(char* szName, M** pvData)
{
	return GetWithKey(szName, NULL, 0, pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CEnumeratorTemplate<M>::Get(char* szName, int iNameLen, M** pvData)
{
	return GetWithKey(szName, iNameLen, NULL, 0, pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CEnumeratorTemplate<M>::GetNumMatchingWithKey(char* szName, M* pvKey, int iKeySize)
{
	SENode*		psNode;
	int			iNum;
	SENode*		psStart;

	iNum = 0;
	psStart = mcList.GetHead();
	for (;;)
	{
		psNode = PrivateGetWithKey(szName, pvKey, iKeySize, psStart);
		if (psNode)
		{
			iNum++;
			psStart = mcList.GetNext(psNode);
			if (!psStart)
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
	return iNum;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CEnumeratorTemplate<M>::GetWithKey(char* szName, M* pvKey, int iKeySize, M** pvData, int iMatchNum)
{
	SENode*		psNode;
	int			iNum;
	SENode*		psStart;

	iNum = 0;
	psStart = mcList.GetHead();
	for (;;)
	{
		psNode = PrivateGetWithKey(szName, pvKey, iKeySize, psStart);
		if (psNode)
		{
			if (iNum == iMatchNum)
			{
				if (pvData)
				{
					(*pvData) = (M*)psNode->pvData;
				}
				return psNode->iNum;
			}
			iNum++;
			psStart = mcList.GetNext(psNode);
			if (!psStart)
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CEnumeratorTemplate<M>::GetWithKey(char* szName, int iNameLen, M* pvKey, int iKeySize, M** pvData, int iMatchNum)
{
	SENode*		psNode;
	int			iNum;
	SENode*		psStart;

	iNum = 0;
	psStart = mcList.GetHead();
	for (;;)
	{
		psNode = PrivateGetWithKey(szName, iNameLen, pvKey, iKeySize, psStart);
		if (psNode)
		{
			if (iNum == iMatchNum)
			{
				if (pvData)
				{
					(*pvData) = (M*)psNode->pvData;
				}
				return psNode->iNum;
			}
			iNum++;
			psStart = mcList.GetNext(psNode);
			if (!psStart)
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::GetWithID(int iID, M** pvData, char** pcName)
{
	SENode*		psNode;

	if (iID == -1)
	{
		psNode = NULL;
	}
	else
	{
		psNode = (SENode*)(ENGINE_SIZE_T) mcIDArray.SafeGetValue(iID);
		if (psNode == (void*)-1)
		{
			psNode = NULL;
		}
	}

	if (psNode)
	{
		if (pvData)
		{
			(*pvData) = (M*)psNode->pvData;
		}
		if (pcName)
		{
			*pcName = psNode->szName;
		}
	}
	else
	{
		if (pvData)
		{
			*pvData = NULL;
		}
		if (pcName)
		{
			*pcName = NULL;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL __CEnumeratorTemplate<M>::PrivateCompare(char* szName, M* pvKey, int iKeySize, SENode* psNode)
{
	int		iVal;

	if (mbCaseSensitive)
	{
		iVal = strcmp(szName, psNode->szName);
	}
	else
	{
		iVal = StrICmp(szName, psNode->szName);
	}
	if (iVal == 0)
	{
		if (iKeySize)
		{
			//Find where the keys are (always the first members of the user data struct).
			iVal = memcmp(pvKey, psNode->pvData, iKeySize);
		}
		if (iVal == 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL __CEnumeratorTemplate<M>::PrivateCompare(char* szName, int iNameLen, M* pvKey, int iKeySize, SENode* psNode)
{
	int		iVal;

	if (mbCaseSensitive)
	{
		iVal = memcmp(szName, psNode->szName, iNameLen);
	}
	else
	{
		iVal = MemICmp(szName, psNode->szName, iNameLen);
	}
	if (iVal == 0)
	{
		if (iKeySize)
		{
			//Find where the keys are (always the first members of the user data struct).
			iVal = memcmp(pvKey, psNode->pvData, iKeySize);
		}
		if (iVal == 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CEnumeratorTemplate<M>::NumElements(void)
{
	SENode*		psNode;
	int			iCount;

	iCount = 0;
	psNode = mcList.GetHead();
	while (psNode)
	{
		iCount++;
		psNode = mcList.GetNext(psNode);
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::StartIteration(SEnumeratorIterator* psIterator, char** szName, int* piID, M** pvData)
{
	psIterator->psNode = mcList.GetHead();
	if (psIterator->psNode)
	{
		psIterator->bValid = TRUE;
		if (szName)
		{
			*szName = psIterator->psNode->szName;
		}
		if (pvData)
		{
			(*pvData) = (M*)psIterator->psNode->pvData;
		}
		if (piID)
		{
			*piID = psIterator->psNode->iNum;
		}
	}
	else
	{
		psIterator->bValid = FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::Iterate(SEnumeratorIterator* psIterator, char** szName, int* piID, M**pvData)
{
	psIterator->psNode = mcList.GetNext(psIterator->psNode);
	if (psIterator->psNode)
	{
		psIterator->bValid = TRUE;
		if (szName)
		{
			*szName = psIterator->psNode->szName;
		}
		if (pvData)
		{
			(*pvData) = (M*)psIterator->psNode->pvData;
		}
		if (piID)
		{
			*piID = psIterator->psNode->iNum;
		}
	}
	else
	{
		if (szName)
		{
			*szName = NULL;
		}
		psIterator->bValid = FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::PrivateRemove(SENode* psNode)
{
	if (psNode)
	{
		if (psNode->iFlags & ENUMERATOR_MANAGED)
		{
			if (psNode->pvData)
			{
				Free(psNode->pvData);
				psNode->pvData = NULL;
			}
		}
		PrivateRemoveID(psNode);
		mcList.Remove(psNode);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::Remove(int iID)
{
	SENode*		psNode;

	if ((iID >= 0) && (iID < mcIDArray.NumElements()))
	{
		psNode = (SENode*)(ENGINE_SIZE_T) mcIDArray.GetValue(iID);
		PrivateRemove(psNode);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::Remove(char* szName)
{
	SENode*		psNode;

	psNode = PrivateGetWithKey(szName, NULL, 0, NULL);
	PrivateRemove(psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::Swap(SENode* psNode1, SENode* psNode2)
{
	mcList.Swap(psNode1, psNode2);
	PrivateInsertID(psNode2);
	PrivateInsertID(psNode1);

}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::SwapUp(char* szName)
{
	SENode*	psNode1;
	SENode*	psNode2;

	psNode1 = PrivateGetWithKey(szName, NULL, 0, NULL);
	if (psNode1)
	{
		psNode2 = mcList.GetPrev(psNode1);
		if (psNode2)
		{
			Swap(psNode1, psNode2);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::SwapDown(char* szName)
{
	SENode*	psNode1;
	SENode*	psNode2;

	psNode1 = PrivateGetWithKey(szName, NULL, 0, NULL);
	if (psNode1)
	{
		psNode2 = mcList.GetNext(psNode1);
		if (psNode2)
		{
			Swap(psNode1, psNode2);
		}
	}
}


int AlphabeticalComparisionCallbackCaseSensitive(const void* pvNode1, const void* pvNode2);
int AlphabeticalComparisionCallbackCaseInsensitive(const void* pvNode1, const  void* pvNode2);


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::RecreateIDArray(void)
{
	SENode*		psNode;

	mcIDArray.SetArrayValues(-1);

	psNode = mcList.GetHead();
	while (psNode)
	{
		PrivateInsertID(psNode);
		psNode = mcList.GetNext(psNode);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::QuickSort(void)
{

	if (mbCaseSensitive)
	{
		mcList.QuickSort(AlphabeticalComparisionCallbackCaseSensitive);
	}
	else
	{
		mcList.QuickSort(AlphabeticalComparisionCallbackCaseInsensitive);
	}
	RecreateIDArray();
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::BubbleSort(void)
{
	if (mbCaseSensitive)
	{
		mcList.BubbleSort(AlphabeticalComparisionCallbackCaseSensitive);
	}
	else
	{
		mcList.BubbleSort(AlphabeticalComparisionCallbackCaseInsensitive);
	}
	RecreateIDArray();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CEnumeratorTemplate<M>::Add(char* szName, M* pvData, int iNum)
{
	return CEnumeratorTemplate<M>::Add(szName, pvData, 0, iNum, TRUE);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CEnumeratorTemplate<M>::Add(char* szName, M* pvData, int iKeySize, int iNum, BOOL bReplace)
{
	return __CEnumeratorTemplate<M>::PrivateAddGetNode(szName, pvData, sizeof(M), iKeySize, iNum, bReplace, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CEnumeratorTemplate<M>::Add(char* szName, int iNameLen, M* pvData, int iKeySize, int iNum, BOOL bReplace)
{
	char	sz[1024];

	//Buggered if I'm going to try and change __CEnumeratorTemplate<M>::PrivateAddGetNode.
	memcpy(sz, szName, iNameLen);
	sz[iNameLen] = 0;
	return __CEnumeratorTemplate<M>::PrivateAddGetNode(sz, pvData, sizeof(M), iKeySize, iNum, bReplace, NULL);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CEnumeratorTemplate<M>::WriteEnumeratorTemplate(CFileWriter* pcFileWriter)
{
	return WriteEnumeratorBlock(pcFileWriter);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>	
BOOL CEnumeratorTemplate<M>::ReadEnumeratorTemplate(CFileReader* pcFileReader)
{
	return ReadEnumeratorBlock(pcFileReader);
}


#endif //__TEMPLATE_ENUMERATOR_H__

