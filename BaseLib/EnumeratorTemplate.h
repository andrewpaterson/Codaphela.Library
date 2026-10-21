/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
	CArrayTemplatePrimitive<ptr>	mcIDArray;  //SENode pointers indexed by ID.  (ptr)-1 if there is no node for the ID.
	CLinkListSENode		mcList;
	bool				mbCaseSensitive;

public:
	void		Init(void);
	void		Kill(void);
	int			Get(const char* szName, M** pvData);
	int			Get(const char* szName, size iNameLen, M** pvData);
	int			GetNumMatchingWithKey(char* szName, M* pvKey, size uiKeySize);
	int			GetWithKey(const char* szName, M* pvKey, size uiKeySize, M** pvData, int iMatchNum = 0, char** ppszName = NULL);
	int			GetWithKey(const char* szName, size iNameLen, M* pvKey, size uiKeySize, M** pvData, int iMatchNum = 0, char** ppszName = NULL);
	bool		GetWithID(size iID, M** pvData, char** pcName);
	size		NumElements(void);
	void		StartIteration(SEnumeratorIterator* psIterator, char** ppszName, size* piID, M** ppvData);
	void		Iterate(SEnumeratorIterator* psIterator, char** ppszName, size* piID, M** ppvData);
	void		Remove(size iID);
	void		Remove(const char* szName);
	void		Swap(SENode* psNode1, SENode* psNode2);
	void		SwapUp(char* szName);
	void		SwapDown(char* szName);
	void		BubbleSort(void);
	void		RecreateIDArray(void);
	void		AllocateNodeData(SENode* psNode, size iLen);

protected:
	int			PrivateAddGetNode(const char* szName, M* pvData, size uiDataSize, size uiKeySize, int uiNum, bool bReplace, SENode** pcThisNode);
	SENode*		PrivateGetWithKey(const char* szName, M* pvKey, size uiKeySize, SENode* psStartNode);
	SENode*		PrivateGetWithKey(const char* szName, size iNameLen, M* pvKey, size uiKeySize, SENode* psStartNode);
	int			PrivateGetNextID(int uiNum);
	void		PrivateInsertID(SENode* psNode);
	void		PrivateRemoveID(SENode* psNode);
	void		PrivateRemove(SENode* psNode);
	bool		PrivateCompare(const char* szName, M* pvKey, size uiKeySize, SENode* psNode);
	bool		PrivateCompare(const char* szName, size iNameLen, M* pvKey, size uiKeySize, SENode* psNode);
};


template<class M>
class CEnumeratorTemplate : public __CEnumeratorTemplate<M>
{
public:
	using __CEnumeratorTemplate<M>::mcIDArray;
	using __CEnumeratorTemplate<M>::mcList;
	using __CEnumeratorTemplate<M>::mbCaseSensitive;
	using __CEnumeratorTemplate<M>::AllocateNodeData;

	int			Add(const char* szName, M* pvData, int uiNum);
	int			Add(const char* szName, M* pvData, size uiKeySize, int uiNum, bool bReplace = true);
	int			Add(char* szName, size iNameLen, M* pvData, size uiKeySize, int uiNum, bool bReplace = true);

	bool		WriteEnumeratorBlock(CFileWriter* pcFileWriter);
	bool		ReadEnumeratorBlock(CFileReader* pcFileReader);
	bool		WriteEnumeratorTemplate(CFileWriter* pcFileWriter);
	bool		ReadEnumeratorTemplate(CFileReader* pcFileReader);

protected:
	bool		WriteIDArray(CFileWriter* pcFileWriter);
	bool		ReadIDArray(CFileReader* pcFileReader);
};


int AlphabeticalComparisionCallbackCaseSensitive(const void* pvNode1, const void* pvNode2);
int AlphabeticalComparisionCallbackCaseInsensitive(const void* pvNode1, const  void* pvNode2);


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::Init(void)
{
	mbCaseSensitive = true;
	mcIDArray.Init();
	mcList.Init(&gcSystemAllocator);
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
				mcList.Free(psNode->pvData);
				psNode->pvData = NULL;
			}
		}
		if (psNode->szName)
		{
			mcList.Free(psNode->szName);
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
void __CEnumeratorTemplate<M>::AllocateNodeData(SENode* psNode, size iLen)
{
	psNode->szName = (char*)mcList.Malloc(iLen);
	if (psNode->uiDataSize)
	{
		psNode->pvData = mcList.Malloc(psNode->uiDataSize);
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
int __CEnumeratorTemplate<M>::PrivateAddGetNode(const char* szName, M* pvData, size uiDataSize, size uiKeySize, int uiNum, bool bReplace, SENode** ppsThisNode)
{
	SENode*		psNode;
	size		iLen;

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
					mcList.Free(psNode->pvData);
					psNode->pvData = NULL;
				}
			}

			//If no data exists then allocate it.
			if (uiDataSize)
			{
				if (psNode->pvData == NULL)
				{
					psNode->pvData = mcList.Malloc(uiDataSize);
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
					mcList.Free(psNode->pvData);
				}
				psNode->pvData = NULL;
			}
		}
		psNode->iFlags = 0;
		psNode->uiDataSize = 0;

		//Return the node just allocated.
		SafeAssign(ppsThisNode, psNode);
		return psNode->uiNum;
	}

	//Handle the case where the node exists but is not being replaced.
	if (psNode)
	{
		//Return the node just allocated.
		SafeAssign(ppsThisNode, psNode);
		return psNode->uiNum;
	}

	//Handle the case where the node does not exist.
	if (psNode == NULL)
	{
		iLen = (size)strlen(szName) + 1;
		psNode = mcList.InsertAfterTail();

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
		SafeAssign(ppsThisNode, psNode);
		return psNode->uiNum;
	}

	//Something probably went wrong.
	SafeAssign(ppsThisNode, NULL);
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::PrivateInsertID(SENode* psNode)
{
	size	iID;

	iID = psNode->uiNum;
	mcIDArray.AddRemap(iID, (ptr)psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::PrivateRemoveID(SENode* psNode)
{
	size	iID;

	iID = psNode->uiNum;
	mcIDArray.RemoveRemap(iID);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
SENode* __CEnumeratorTemplate<M>::PrivateGetWithKey(const char* szName, M* pvKey, size uiKeySize, SENode* psStartNode)
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
		if (PrivateCompare(szName, pvKey, uiKeySize, psNode))
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
SENode* __CEnumeratorTemplate<M>::PrivateGetWithKey(const char* szName, size iNameLen, M* pvKey, size uiKeySize, SENode* psStartNode)
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
		if (PrivateCompare(szName, iNameLen, pvKey, uiKeySize, psNode))
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
int __CEnumeratorTemplate<M>::PrivateGetNextID(int uiNum)
{
	if (uiNum == -1)
	{
		return (int)mcIDArray.NumElements();
	}
	return uiNum;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CEnumeratorTemplate<M>::Get(const char* szName, M** ppvData)
{
	return GetWithKey(szName, NULL, 0, ppvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CEnumeratorTemplate<M>::Get(const char* szName, size iNameLen, M** ppvData)
{
	return GetWithKey(szName, iNameLen, NULL, 0, ppvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CEnumeratorTemplate<M>::GetNumMatchingWithKey(char* szName, M* pvKey, size uiKeySize)
{
	SENode*		psNode;
	int			uiNum;
	SENode*		psStart;

	uiNum = 0;
	psStart = mcList.GetHead();
	for (;;)
	{
		psNode = PrivateGetWithKey(szName, pvKey, uiKeySize, psStart);
		if (psNode)
		{
			uiNum++;
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
	return uiNum;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CEnumeratorTemplate<M>::GetWithKey(const char* szName, M* pvKey, size uiKeySize, M** ppvData, int iMatchNum, char** ppszName)
{
	SENode*		psNode;
	int		uiNum;
	SENode*		psStart;

	uiNum = 0;
	psStart = mcList.GetHead();
	for (;;)
	{
		psNode = PrivateGetWithKey(szName, pvKey, uiKeySize, psStart);
		if (psNode)
		{
			if (uiNum == iMatchNum)
			{
				SafeAssign(ppvData, (M*)psNode->pvData);
				SafeAssign(ppszName, psNode->szName);
				return psNode->uiNum;
			}
			uiNum++;
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
int __CEnumeratorTemplate<M>::GetWithKey(const char* szName, size iNameLen, M* pvKey, size uiKeySize, M** ppvData, int iMatchNum, char** ppszName)
{
	SENode*		psNode;
	size		uiNum;
	SENode*		psStart;

	uiNum = 0;
	psStart = mcList.GetHead();
	for (;;)
	{
		psNode = PrivateGetWithKey(szName, iNameLen, pvKey, uiKeySize, psStart);
		if (psNode)
		{
			if ((int)uiNum == iMatchNum)
			{
				SafeAssign(ppvData, (M*)psNode->pvData);
				SafeAssign(ppszName, psNode->szName);
				return psNode->uiNum;
			}
			uiNum++;
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
bool __CEnumeratorTemplate<M>::GetWithID(size iID, M** ppvData, char** ppszName)
{
	SENode*		psNode;

	if (iID == SIZE_MAX)
	{
		psNode = NULL;
	}
	else
	{
		psNode = (SENode*)mcIDArray.SafeGetValue(iID, (ptr)-1);
		if (psNode == (void*)-1)
		{
			psNode = NULL;
		}
	}

	if (psNode)
	{
		SafeAssign(ppvData, (M*)psNode->pvData);
		SafeAssign(ppszName, psNode->szName);
		return true;
	}
	else
	{
		SafeAssign(ppvData, NULL);
		SafeAssign(ppszName, NULL);
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool __CEnumeratorTemplate<M>::PrivateCompare(const char* szName, M* pvKey, size uiKeySize, SENode* psNode)
{
	size		iVal;

	if (mbCaseSensitive)
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
bool __CEnumeratorTemplate<M>::PrivateCompare(const char* szName, size iNameLen, M* pvKey, size uiKeySize, SENode* psNode)
{
	size		iVal;

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
	size		iCount;

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
void __CEnumeratorTemplate<M>::StartIteration(SEnumeratorIterator* psIterator, char** ppszName, size* piID, M** ppvData)
{
	psIterator->psNode = mcList.GetHead();
	if (psIterator->psNode)
	{
		psIterator->bValid = true;
		SafeAssign(ppszName, psIterator->psNode->szName);
		SafeAssign(ppvData, (M*)psIterator->psNode->pvData);
		SafeAssign(piID, psIterator->psNode->uiNum);
	}
	else
	{
		SafeAssign(ppszName, NULL);
		psIterator->bValid = false;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::Iterate(SEnumeratorIterator* psIterator, char** ppszName, size* piID, M** ppvData)
{
	psIterator->psNode = mcList.GetNext(psIterator->psNode);
	if (psIterator->psNode)
	{
		psIterator->bValid = true;
		SafeAssign(ppszName, psIterator->psNode->szName);
		SafeAssign(ppvData, (M*)psIterator->psNode->pvData);
		SafeAssign(piID, psIterator->psNode->uiNum);
	}
	else
	{
		SafeAssign(ppszName, NULL);
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
				mcList.Free(psNode->pvData);
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
void __CEnumeratorTemplate<M>::Remove(size iID)
{
	SENode*		psNode;

	if ((iID >= 0) && (iID < mcIDArray.NumElements()))
	{
		psNode = (SENode*)mcIDArray.GetValue(iID);
		PrivateRemove(psNode);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::Remove(const char* szName)
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
	SENode*		psNode1;
	SENode*		psNode2;

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
	SENode*		psNode1;
	SENode*		psNode2;

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


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CEnumeratorTemplate<M>::RecreateIDArray(void)
{
	SENode*		psNode;

	mcIDArray.SetArrayValues((ptr)-1);

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
int CEnumeratorTemplate<M>::Add(const char* szName, M* pvData, int uiNum)
{
	return CEnumeratorTemplate<M>::Add(szName, pvData, 0, uiNum, true);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CEnumeratorTemplate<M>::Add(const char* szName, M* pvData, size uiKeySize, int uiNum, bool bReplace)
{
	return __CEnumeratorTemplate<M>::PrivateAddGetNode(szName, pvData, sizeof(M), uiKeySize, uiNum, bReplace, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CEnumeratorTemplate<M>::Add(char* szName, size iNameLen, M* pvData, size uiKeySize, int uiNum, bool bReplace)
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

	if (!pcFileWriter->WriteData(&mbCaseSensitive, sizeof(bool))) 
	{ 
		return false; 
	}

	if (!WriteIDArray(pcFileWriter))
	{
		return false;
	}

	if (!mcList.Write(pcFileWriter))
	{

		return false;
	}

	psNode = mcList.GetHead();
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
		psNode = mcList.GetNext(psNode);
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
	size		iReadSize;

	if (!pcFileReader->ReadData(&mbCaseSensitive, sizeof(bool))) 
	{ 
		return false; 
	}

	if (!ReadIDArray(pcFileReader))
	{
		return false;
	}

	if (!mcList.Read(pcFileReader))
	{
		return false;
	}

	psNode = mcList.GetHead();
	while (psNode)
	{
		//Actually this is copied from ReadString because I need the string length first.
		if (!pcFileReader->ReadStringLength(&iReadSize)) 
		{ 
			return false; 
		}

		AllocateNodeData(psNode, iReadSize);
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
		psNode = mcList.GetNext(psNode);
	}

	this->RecreateIDArray();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
//The ID array holds node pointers.  It is written as the array of ints it used to be so the file is the same in 32 and
//64 bit builds.  The pointers mean nothing once read so ReadEnumeratorBlock rebuilds the array from the nodes.
template<class M>
bool CEnumeratorTemplate<M>::WriteIDArray(CFileWriter* pcFileWriter)
{
	CArrayInt	aiIDs;
	size		i;
	bool		bResult;

	aiIDs.Init();
	for (i = 0; i < mcIDArray.NumElements(); i++)
	{
		aiIDs.Add((int)mcIDArray.GetValue(i));
	}
	bResult = aiIDs.Write(pcFileWriter);
	aiIDs.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CEnumeratorTemplate<M>::ReadIDArray(CFileReader* pcFileReader)
{
	CArrayInt	aiIDs;

	if (!aiIDs.Read(pcFileReader))
	{
		return false;
	}
	aiIDs.Kill();

	mcIDArray.Init();
	return true;
}


#endif // __TEMPLATE_ENUMERATOR_H__

