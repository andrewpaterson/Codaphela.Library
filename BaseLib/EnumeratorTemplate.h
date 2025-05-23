/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "PointerFunctions.h"
#include "DataTypes.h"
#include "ArrayInt.h"
#include "LinkedListTemplate.h"
#include "StringHelper.h"
#include "EnumeratorNode.h"


#define ENUMERATOR_MANAGED	1


struct SEnumeratorIterator
{
	bool		bValid;
	SENode*		psNode;
};


typedef CLinkedListTemplate<SENode>  CLinkListSENode;

//Do not confuse the enumerator with a map.  The enumerator is designed for fast access based on an integer (which is not an index)
template<class M>
class __CEnumeratorTemplate
{
public:
	CArrayInt			mcIDArray;
	CLinkListSENode		mcList;
	bool				mbCaseSensitive;

public:
	void		Init(void);
	void		Kill(void);
	size		Get(char* szName, M** pvData);
	size		Get(char* szName, size iNameLen, M** pvData);
	size		GetNumMatchingWithKey(char* szName, M* pvKey, size uiKeySize);
	size		GetWithKey(char* szName, M* pvKey, size uiKeySize, M** pvData, size iMatchNum = 0);
	size		GetWithKey(char* szName, size iNameLen, M* pvKey, size uiKeySize, M** pvData, size iMatchNum = 0);
	bool		GetWithID(size iID, M** pvData, char** pcName);
	size		NumElements(void);
	void		StartIteration(SEnumeratorIterator* psIterator, char** szName, size* piID, M** pvData);
	void		Iterate(SEnumeratorIterator* psIterator, char** szName, size* piID, M** pvData);
	void		Remove(size iID);
	void		Remove(char* szName);
	void		Swap(SENode* psNode1, SENode* psNode2);
	void		SwapUp(char* szName);
	void		SwapDown(char* szName);
	void		QuickSort(void);
	void		BubbleSort(void);
	void		RecreateIDArray(void);
	void		AllocateNodeData(SENode* psNode, size iLen);

protected:
	void*		Malloc(size uiSize);
	void*		Realloc(void* pv, size iMemSize);
	void		Free(void* pv);

	size		PrivateAddGetNode(char* szName, M* pvData, size uiDataSize, size uiKeySize, size uiNum, bool bReplace, SENode** pcThisNode);
	SENode*		PrivateGetWithKey(char* szName, M* pvKey, size uiKeySize, SENode* psStartNode);
	SENode*		PrivateGetWithKey(char* szName, size iNameLen, M* pvKey, size uiKeySize, SENode* psStartNode);
	size		PrivateGetNextID(size uiNum);
	void		PrivateInsertID(SENode* psNode);
	void		PrivateRemoveID(SENode* psNode);
	void		PrivateRemove(SENode* psNode);
	bool		PrivateCompare(char* szName, M* pvKey, size uiKeySize, SENode* psNode);
	bool		PrivateCompare(char* szName, size iNameLen, M* pvKey, size uiKeySize, SENode* psNode);
};


template<class M>
class CEnumeratorTemplate : public __CEnumeratorTemplate<M>
{
public:
	size		Add(char* szName, M* pvData, size uiNum);
	size		Add(char* szName, M* pvData, size uiKeySize, size uiNum, bool bReplace = true);
	size		Add(char* szName, size iNameLen, M* pvData, size uiKeySize, size uiNum, bool bReplace = true);

	bool		WriteEnumeratorBlock(CFileWriter* pcFileWriter);
	bool		ReadEnumeratorBlock(CFileReader* pcFileReader);
	bool		WriteEnumeratorTemplate(CFileWriter* pcFileWriter);
	bool		ReadEnumeratorTemplate(CFileReader* pcFileReader);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void* __CEnumeratorTemplate<M>::Malloc(size uiSize)
{
	return malloc(uiSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::Free(void* pv)
{
	SafeFree(pv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void* __CEnumeratorTemplate<M>::Realloc(void* pv, size uiSize)
{
	pv = realloc(pv, uiSize);
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::Init(void)
{
	this->mbCaseSensitive = true;
	this->mcIDArray.Init();
	this->mcList.Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::Kill(void)
{
	SENode*		psNode;

	psNode = this->mcList.GetHead();
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
		psNode = this->mcList.GetNext(psNode);
	}

	this->mcIDArray.Kill();
	this->mcList.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::AllocateNodeData(SENode* psNode, size iLen)
{
	psNode->szName = (char*)Malloc(iLen);
	if (psNode->uiDataSize)
	{
		psNode->pvData = Malloc(psNode->uiDataSize);
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
size __CEnumeratorTemplate<M>::PrivateAddGetNode(char* szName, M* pvData, size uiDataSize, size uiKeySize, size uiNum, bool bReplace, SENode** pcThisNode)
{
	SENode*		psNode;
	size			iLen;

	psNode = PrivateGetWithKey(szName, pvData, uiKeySize, NULL);

	//Handle the case where the node exists and must be replaced.
	if (psNode && bReplace)
	{
		//Make sure the replaced nodes ID does not have and effect on the new ID.
		PrivateRemoveID(psNode);
		psNode->uiNum = -1;
		if (uiNum == -1)
		{
			psNode->uiNum = PrivateGetNextID(uiNum);
		}
		else
		{
			psNode->uiNum = uiNum;
		}
		PrivateInsertID(psNode);

		//If the user wants to add data to this node.
		if (pvData)
		{
			//If the data sizes are different then free the current node.
			if (psNode->uiDataSize != uiDataSize)
			{
				if (psNode->iFlags & ENUMERATOR_MANAGED)
				{
					Free(psNode->pvData);
					psNode->pvData = NULL;
				}
			}

			//If no data exists then allocate it.
			if (uiDataSize)
			{
				if (psNode->pvData == NULL)
				{
					psNode->pvData = Malloc(uiDataSize);
				}
				psNode->iFlags = ENUMERATOR_MANAGED;

				//Copy the data into the nodes data.
				memcpy(psNode->pvData, pvData, uiDataSize);
			}
			else
			{
				psNode->iFlags = 0;
				psNode->pvData = pvData;
			}

			psNode->uiDataSize = uiDataSize;
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
		psNode->uiDataSize = 0;

		//Return the node just allocated.
		if (pcThisNode)
		{
			*pcThisNode = psNode;
		}
		return psNode->uiNum;
	}

	//Handle the case where the node exists but is not being replaced.
	if (psNode)
	{
		//Return the node just allocated.
		if (pcThisNode)
		{
			*pcThisNode = psNode;
		}
		return psNode->uiNum;
	}

	//Handle the case where the node does not exist.
	if (psNode == NULL)
	{
		iLen = (size)strlen(szName) + 1;
		psNode = this->mcList.InsertAfterTail();

		psNode->iFlags = 0;
		psNode->uiNum = PrivateGetNextID(uiNum);
		psNode->uiDataSize = uiDataSize;

		AllocateNodeData(psNode, iLen);

		if (uiDataSize)
		{
			psNode->iFlags = ENUMERATOR_MANAGED;
			if (pvData)
			{
				memcpy(psNode->pvData, pvData, uiDataSize);
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
		return psNode->uiNum;
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
	size		iID;

	iID = psNode->uiNum;
	this->mcIDArray.AddRemap(iID, (size)((size) psNode));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::PrivateRemoveID(SENode* psNode)
{
	size iID;

	iID = psNode->uiNum;
	this->mcIDArray.RemoveRemap(iID);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
SENode* __CEnumeratorTemplate<M>::PrivateGetWithKey(char* szName, M* pvKey, size uiKeySize, SENode* psStartNode)
{
	SENode*		psNode;

	if (psStartNode)
	{
		psNode = psStartNode;
	}
	else
	{
		psNode = this->mcList.GetHead();
	}
	while (psNode)
	{
		if (PrivateCompare(szName, pvKey, uiKeySize, psNode))
		{
			return psNode;
		}
		psNode = this->mcList.GetNext(psNode);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
SENode* __CEnumeratorTemplate<M>::PrivateGetWithKey(char* szName, size iNameLen, M* pvKey, size uiKeySize, SENode* psStartNode)
{
	SENode*		psNode;

	if (psStartNode)
	{
		psNode = psStartNode;
	}
	else
	{
		psNode = this->mcList.GetHead();
	}
	while (psNode)
	{
		if (PrivateCompare(szName, iNameLen, pvKey, uiKeySize, psNode))
		{
			return psNode;
		}
		psNode = this->mcList.GetNext(psNode);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size __CEnumeratorTemplate<M>::PrivateGetNextID(size uiNum)
{
	if (uiNum == -1)
	{
		return this->mcIDArray.NumElements();
	}
	return uiNum;

	//Longer version which might be safer.
	//size			iNextID;
	//SENode*		psNode;
	//psNode = this->mcList.GetHead();
	//iNextID = 0;
	//while(psNode)
	//{
	//	if (psNode->uiNum >= iNextID)
	//	{
	//		iNextID = psNode->uiNum + 1;
	//	}
	//	psNode = this->mcList.GetNext(psNode);
	//}
	//return iNextID;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size __CEnumeratorTemplate<M>::Get(char* szName, M** pvData)
{
	return GetWithKey(szName, NULL, 0, pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size __CEnumeratorTemplate<M>::Get(char* szName, size iNameLen, M** pvData)
{
	return GetWithKey(szName, iNameLen, NULL, 0, pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size __CEnumeratorTemplate<M>::GetNumMatchingWithKey(char* szName, M* pvKey, size uiKeySize)
{
	SENode*		psNode;
	size			uiNum;
	SENode*		psStart;

	uiNum = 0;
	psStart = this->mcList.GetHead();
	for (;;)
	{
		psNode = PrivateGetWithKey(szName, pvKey, uiKeySize, psStart);
		if (psNode)
		{
			uiNum++;
			psStart = this->mcList.GetNext(psNode);
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
	return uiNum;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size __CEnumeratorTemplate<M>::GetWithKey(char* szName, M* pvKey, size uiKeySize, M** pvData, size iMatchNum)
{
	SENode*		psNode;
	size			uiNum;
	SENode*		psStart;

	uiNum = 0;
	psStart = this->mcList.GetHead();
	for (;;)
	{
		psNode = PrivateGetWithKey(szName, pvKey, uiKeySize, psStart);
		if (psNode)
		{
			if (uiNum == iMatchNum)
			{
				if (pvData)
				{
					(*pvData) = (M*)psNode->pvData;
				}
				return psNode->uiNum;
			}
			uiNum++;
			psStart = this->mcList.GetNext(psNode);
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
size __CEnumeratorTemplate<M>::GetWithKey(char* szName, size iNameLen, M* pvKey, size uiKeySize, M** pvData, size iMatchNum)
{
	SENode*		psNode;
	size			uiNum;
	SENode*		psStart;

	uiNum = 0;
	psStart = this->mcList.GetHead();
	for (;;)
	{
		psNode = PrivateGetWithKey(szName, iNameLen, pvKey, uiKeySize, psStart);
		if (psNode)
		{
			if (uiNum == iMatchNum)
			{
				if (pvData)
				{
					(*pvData) = (M*)psNode->pvData;
				}
				return psNode->uiNum;
			}
			uiNum++;
			psStart = this->mcList.GetNext(psNode);
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
bool __CEnumeratorTemplate<M>::GetWithID(size iID, M** pvData, char** pcName)
{
	SENode*		psNode;

	if (iID == -1)
	{
		psNode = NULL;
	}
	else
	{
		psNode = (SENode*)(size) this->mcIDArray.SafeGetValue(iID);
		if (psNode == (void*)-1)
		{
			psNode = NULL;
		}
	}

	if (psNode)
	{
		SafeAssign(pvData, (M*)psNode->pvData);
		SafeAssign(pcName, psNode->szName);
		return true;
	}
	else
	{
		SafeAssign(pvData, NULL);
		SafeAssign(pcName, NULL);
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool __CEnumeratorTemplate<M>::PrivateCompare(char* szName, M* pvKey, size uiKeySize, SENode* psNode)
{
	size		iVal;

	if (this->mbCaseSensitive)
	{
		iVal = StringCompare(szName, psNode->szName);
	}
	else
	{
		iVal = StringInsensitiveCompare(szName, psNode->szName);
	}
	if (iVal == 0)
	{
		if (uiKeySize)
		{
			//Find where the keys are (always the first members of the user data struct).
			iVal = memcmp(pvKey, psNode->pvData, uiKeySize);
		}
		if (iVal == 0)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool __CEnumeratorTemplate<M>::PrivateCompare(char* szName, size iNameLen, M* pvKey, size uiKeySize, SENode* psNode)
{
	size		iVal;

	if (this->mbCaseSensitive)
	{
		iVal = memcmp(szName, psNode->szName, iNameLen);
	}
	else
	{
		iVal = MemICmp(szName, psNode->szName, iNameLen);
	}
	if (iVal == 0)
	{
		if (uiKeySize)
		{
			//Find where the keys are (always the first members of the user data struct).
			iVal = memcmp(pvKey, psNode->pvData, uiKeySize);
		}
		if (iVal == 0)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size __CEnumeratorTemplate<M>::NumElements(void)
{
	SENode*		psNode;
	size			iCount;

	iCount = 0;
	psNode = this->mcList.GetHead();
	while (psNode)
	{
		iCount++;
		psNode = this->mcList.GetNext(psNode);
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::StartIteration(SEnumeratorIterator* psIterator, char** szName, size* piID, M** pvData)
{
	psIterator->psNode = this->mcList.GetHead();
	if (psIterator->psNode)
	{
		psIterator->bValid = true;
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
			*piID = psIterator->psNode->uiNum;
		}
	}
	else
	{
		psIterator->bValid = false;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::Iterate(SEnumeratorIterator* psIterator, char** szName, size* piID, M**pvData)
{
	psIterator->psNode = this->mcList.GetNext(psIterator->psNode);
	if (psIterator->psNode)
	{
		psIterator->bValid = true;
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
			*piID = psIterator->psNode->uiNum;
		}
	}
	else
	{
		if (szName)
		{
			*szName = NULL;
		}
		psIterator->bValid = false;
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
		this->mcList.Remove(psNode);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::Remove(size iID)
{
	SENode*		psNode;

	if ((iID >= 0) && (iID < this->mcIDArray.NumElements()))
	{
		psNode = (SENode*)(size) this->mcIDArray.GetValue(iID);
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
	this->mcList.Swap(psNode1, psNode2);
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
		psNode2 = this->mcList.GetPrev(psNode1);
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
		psNode2 = this->mcList.GetNext(psNode1);
		if (psNode2)
		{
			Swap(psNode1, psNode2);
		}
	}
}


size AlphabeticalComparisionCallbackCaseSensitive(const void* pvNode1, const void* pvNode2);
size AlphabeticalComparisionCallbackCaseInsensitive(const void* pvNode1, const  void* pvNode2);


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::RecreateIDArray(void)
{
	SENode*		psNode;

	this->mcIDArray.SetArrayValues(-1);

	psNode = this->mcList.GetHead();
	while (psNode)
	{
		PrivateInsertID(psNode);
		psNode = this->mcList.GetNext(psNode);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::QuickSort(void)
{

	if (this->mbCaseSensitive)
	{
		this->mcList.QuickSort(AlphabeticalComparisionCallbackCaseSensitive);
	}
	else
	{
		this->mcList.QuickSort(AlphabeticalComparisionCallbackCaseInsensitive);
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
	if (this->mbCaseSensitive)
	{
		this->mcList.BubbleSort(AlphabeticalComparisionCallbackCaseSensitive);
	}
	else
	{
		this->mcList.BubbleSort(AlphabeticalComparisionCallbackCaseInsensitive);
	}
	RecreateIDArray();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CEnumeratorTemplate<M>::Add(char* szName, M* pvData, size uiNum)
{
	return CEnumeratorTemplate<M>::Add(szName, pvData, 0, uiNum, true);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CEnumeratorTemplate<M>::Add(char* szName, M* pvData, size uiKeySize, size uiNum, bool bReplace)
{
	return __CEnumeratorTemplate<M>::PrivateAddGetNode(szName, pvData, sizeof(M), uiKeySize, uiNum, bReplace, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CEnumeratorTemplate<M>::Add(char* szName, size iNameLen, M* pvData, size uiKeySize, size uiNum, bool bReplace)
{
	char	sz[1024];

	//Buggered if I'm going to try and change __CEnumeratorTemplate<M>::PrivateAddGetNode.
	memcpy(sz, szName, iNameLen);
	sz[iNameLen] = 0;
	return __CEnumeratorTemplate<M>::PrivateAddGetNode(sz, pvData, sizeof(M), uiKeySize, uiNum, bReplace, NULL);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CEnumeratorTemplate<M>::WriteEnumeratorTemplate(CFileWriter* pcFileWriter)
{
	return WriteEnumeratorBlock(pcFileWriter);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>	
bool CEnumeratorTemplate<M>::ReadEnumeratorTemplate(CFileReader* pcFileReader)
{
	return ReadEnumeratorBlock(pcFileReader);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>	
bool CEnumeratorTemplate<M>::WriteEnumeratorBlock(CFileWriter* pcFileWriter)
{
	SENode*		psNode;

	if (!pcFileWriter->WriteData(&this->mbCaseSensitive, sizeof(bool))) 
	{ 
		return false; 
	}

	if (!this->mcIDArray.Write(pcFileWriter))
	{
		return false;
	}

	if (!this->mcList.Write(pcFileWriter))
	{

		return false;
	}

	psNode = this->mcList.GetHead();
	while (psNode)
	{
		if (!pcFileWriter->WriteString(psNode->szName))
		{
			return false;
		}

		if (psNode->uiDataSize)
		{
			if (!pcFileWriter->WriteData(psNode->pvData, psNode->uiDataSize))
			{
				return false;
			}
		}
		psNode = this->mcList.GetNext(psNode);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>	
bool CEnumeratorTemplate<M>::ReadEnumeratorBlock(CFileReader* pcFileReader)
{
	SENode*		psNode;
	size			iReadSize;

	if (!pcFileReader->ReadData(&this->mbCaseSensitive, sizeof(bool))) 
	{ 
		return false; 
	}

	if (!this->mcIDArray.Read(pcFileReader))
	{
		return false;
	}

	if (!this->mcList.Read(pcFileReader))
	{
		return false;
	}

	psNode = this->mcList.GetHead();
	while (psNode)
	{
		//Actually this is copied from ReadString because I need the string length first.
		if (!pcFileReader->ReadData(&iReadSize, sizeof(size))) 
		{ 
			return false; 
		}

		this->AllocateNodeData(psNode, iReadSize);
		if (!pcFileReader->ReadData(psNode->szName, iReadSize)) 
		{ 
			return false; 
		}

		if (psNode->uiDataSize)
		{
			if (!pcFileReader->ReadData(psNode->pvData, psNode->uiDataSize))
			{ 
				return false; 
			}
		}
		psNode = this->mcList.GetNext(psNode);
	}
	return true;
}


#endif // __TEMPLATE_ENUMERATOR_H__

