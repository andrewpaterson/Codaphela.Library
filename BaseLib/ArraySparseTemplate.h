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
#ifndef __ARRAY_SPARSE_TEMPLATE_H__
#define __ARRAY_SPARSE_TEMPLATE_H__
#include <stdlib.h>
#include "DataMacro.h"
#include "Define.h"
#include "FastMemset.h"
#include "FreeList.h"


struct SSparseNode
{
	int				iIndex;
	int				iLeafDepth;
	SSparseNode*	psParent;
	SSparseNode*	psLeft;
	SSparseNode*	psRight;

	//Element data follows node.

	SSparseNode** GetParentPointer(void);
};


template<class M>
class __CArraySparseTemplate
{
private:
	CFreeList		mcNodes;
	int				miElementSize;
	SSparseNode*	mpsRoot;

public:
	void 			Init(int iElementSize);
	void 			Kill(void);
	void			Set(int iElementPos, M* pvData);
	SSparseNode*	Search(int iElementPos);
	M* 				Set(int iElementPos);
	M*				Get(int iElementPos);
	void			Dump(void);
	void			RecurseDump(int iDepth, SSparseNode* psNode, CChars* psz, BOOL bLeft);
	SSparseNode*	TestGetRoot(void);

private:
	SSparseNode*	AllocateNode(int iElementPos, SSparseNode* psParent);
	SSparseNode*	RotateLeft(SSparseNode* psNode);
	SSparseNode*	RotateRight(SSparseNode* psNode);
	void			RecurseRebalance(SSparseNode* psNode);
	void			RecurseIncreaseDepths(SSparseNode* psNode, int iLeafDepth);
	void			RecurseUpdateDepth(SSparseNode* psNode);
};


template<class M>
class CArraySparseTemplate : public __CArraySparseTemplate<M>
{
public:
	void Init(void);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SSparseNode** SSparseNode::GetParentPointer(void)
{
	if (psParent)
	{
		if (psParent->psLeft == this)
		{
			return &psParent->psLeft;
		}
		if (psParent->psRight == this)
		{
			return &psParent->psRight;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArraySparseTemplate<M>::Init(int iElementSize)
{
	mcNodes.Init(sizeof(SSparseNode)+iElementSize);
	miElementSize = iElementSize;
	mpsRoot = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArraySparseTemplate<M>::Kill(void)
{
	mcNodes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArraySparseTemplate<M>::Set(int iElementPos, M* pvData)
{
	M*	psElement;

	psElement = Get(iElementPos);
	if (psElement)
	{
		memcpy(psElement, pvData, miElementSize);
	}
	else
	{
		InsertAt(pvData, iElementPos);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
SSparseNode* __CArraySparseTemplate<M>::AllocateNode(int iElementPos, SSparseNode* psParent)
{
	SSparseNode*	psNode;

	psNode = (SSparseNode*)mcNodes.Add();

	psNode->psLeft = NULL;
	psNode->psRight = NULL;
	psNode->psParent = psParent;
	psNode->iLeafDepth = 0;
	psNode->iIndex = iElementPos;
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArraySparseTemplate<M>::RecurseIncreaseDepths(SSparseNode* psNode, int iLeafDepth)
{
	if (psNode->iLeafDepth < iLeafDepth)
	{
		psNode->iLeafDepth = iLeafDepth;
		if (psNode->psParent)
		{
			RecurseIncreaseDepths(psNode->psParent, iLeafDepth+1);
		}
	}
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArraySparseTemplate<M>::RecurseUpdateDepth(SSparseNode* psNode)
{
	int	iLeftDepth;
	int	iRightDepth;
	int	iNewDepth;

	if (psNode)
	{
		if (psNode->psLeft)
			iLeftDepth = psNode->psLeft->iLeafDepth;
		else
			iLeftDepth = -1;

		if (psNode->psRight)
			iRightDepth = psNode->psRight->iLeafDepth;
		else
			iRightDepth = -1;

		if (iLeftDepth >= iRightDepth)
			iNewDepth = iLeftDepth+1;
		else
			iNewDepth = iRightDepth+1;

		if (iNewDepth != psNode->iLeafDepth)
		{
			psNode->iLeafDepth = iNewDepth;
			RecurseUpdateDepth(psNode->psParent);
		}	
	}
};

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
SSparseNode* __CArraySparseTemplate<M>::Search(int iElementPos)
{
	SSparseNode*	psNode;

	if (mpsRoot)
	{
		psNode = mpsRoot;
		for (;;)
		{
			if (psNode->iIndex == iElementPos)
			{
				return psNode;
			}
			if (iElementPos < psNode->iIndex)
			{
				if (psNode->psLeft)
				{
					psNode = psNode->psLeft;
				}
				else
				{
					return psNode;
				}
			}
			if (iElementPos > psNode->iIndex)
			{
				if (psNode->psRight)
				{
					psNode = psNode->psRight;
				}
				else
				{
					return psNode;
				}
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CArraySparseTemplate<M>::Set(int iElementPos)
{
	SSparseNode*	psNode;
	SSparseNode*	psNewNode;

	if (mpsRoot)
	{
		psNode = Search(iElementPos);
		if (iElementPos < psNode->iIndex)
		{
			psNewNode = AllocateNode(iElementPos, psNode);
			psNode->psLeft = psNewNode;
		}
		else if (iElementPos > psNode->iIndex)
		{
			psNewNode = AllocateNode(iElementPos, psNode);
			psNode->psRight = psNewNode;
		}
		else
		{
			return HeaderGetData<SSparseNode, M>(psNode);
		}
		RecurseIncreaseDepths(psNode, 1);
		RecurseRebalance(psNode);  //Not psNewNode?
		return HeaderGetData<SSparseNode, M>(psNewNode);
	}
	else
	{
		mpsRoot = AllocateNode(iElementPos, NULL);
		return HeaderGetData<SSparseNode, M>(mpsRoot);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CArraySparseTemplate<M>::Get(int iElementPos)
{
	SSparseNode*	psNode;

	psNode = Search(iElementPos);
	if (psNode->iIndex == iElementPos)
	{
		return HeaderGetData<SSparseNode, M>(psNode);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
SSparseNode* __CArraySparseTemplate<M>::RotateLeft(SSparseNode* psNode)
{
	SSparseNode*	psRight;
	SSparseNode*	psRightsLeft;
	SSparseNode**	ppsNodePointer;

	psRight = psNode->psRight;
	if (psRight)
	{
		ppsNodePointer = psNode->GetParentPointer();
		psRightsLeft = psRight->psLeft;

		//Set the parent node to point to right instead of node.
		if (ppsNodePointer)
		{
			*ppsNodePointer = psRight;
		}
		else
		{
			mpsRoot = psRight;
		}

		psRight->psParent = psNode->psParent;
		psNode->psParent = psRight;
		psRight->psLeft = psNode;
		psNode->psRight = psRightsLeft;
		if (psRightsLeft)
		{
			psRightsLeft->psParent = psNode;
		}

		RecurseUpdateDepth(psNode);
		return psRight;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
SSparseNode* __CArraySparseTemplate<M>::RotateRight(SSparseNode* psNode)
{
	SSparseNode*	psLeft;
	SSparseNode*	psLeftsRight;
	SSparseNode**	ppsNodePointer;

	psLeft = psNode->psLeft;
	if (psLeft)
	{
		ppsNodePointer = psNode->GetParentPointer();
		psLeftsRight = psLeft->psRight;

		//Set the parent node to point to right instead of node.
		if (ppsNodePointer)
		{
			*ppsNodePointer = psLeft;
		}
		else
		{
			mpsRoot = psLeft;
		}

		psLeft->psParent = psNode->psParent;
		psNode->psParent = psLeft;
		psLeft->psRight = psNode;
		psNode->psLeft = psLeftsRight;
		if (psLeftsRight)
		{
			psLeftsRight->psParent = psNode;
		}

		RecurseUpdateDepth(psNode);
		return psLeft;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArraySparseTemplate<M>::RecurseRebalance(SSparseNode* psNode)
{
	int		iLeftDepth;
	int		iRightDepth;
	int		iDiff;

	if (!psNode)
	{
		return;
	}

	if (psNode->psLeft)
	{
		iLeftDepth = psNode->psLeft->iLeafDepth;
	}
	else
	{
		iLeftDepth = -1;
	}

	if (psNode->psRight)
	{
		iRightDepth = psNode->psRight->iLeafDepth;
	}
	else
	{
		iRightDepth = -1;
	}

	iDiff = iLeftDepth - iRightDepth;
	if ((iDiff >= -1) && (iDiff <= 1))
	{
		return RecurseRebalance(psNode->psParent);
	}

	if (iDiff >= 2)
	{
		psNode = RotateRight(psNode);
		return RecurseRebalance(psNode->psParent);
	}
	else if (iDiff <= -2)
	{
		psNode = RotateLeft(psNode);
		return RecurseRebalance(psNode->psParent);
	}
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArraySparseTemplate<M>::Dump(void)
{
	CChars	sz;

	sz.Init();
	sz.Append(">>>>>>>>>\n");

	RecurseDump(0, mpsRoot, &sz, TRUE);

	sz.Append("<<<<<<<<<\n");
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArraySparseTemplate<M>::RecurseDump(int iDepth, SSparseNode* psNode, CChars* psz, BOOL bLeft)
{
	if (psNode)
	{
		if (bLeft)
		{
			psz->Append("L ");
		}
		else
		{
			psz->Append("R ");
		}
		psz->Append(' ', iDepth*2);
		psz->Append("(");
		psz->Append(psNode->iIndex);
		psz->Append("): ");
		psz->Append(psNode->iLeafDepth);
		psz->Append("\n");
		RecurseDump(iDepth+1, psNode->psLeft, psz, TRUE);
		RecurseDump(iDepth+1, psNode->psRight, psz, FALSE);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
SSparseNode* __CArraySparseTemplate<M>::TestGetRoot(void)
{
	return mpsRoot;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArraySparseTemplate<M>::Init()
{
	__CArraySparseTemplate<M>::Init(sizeof(M));
}


#endif // __ARRAY_SPARSE_TEMPLATE_H__

