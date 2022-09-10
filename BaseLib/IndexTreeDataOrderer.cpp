#include "IndexTreeDataOrderer.h"
#include "IndexTree.h"
#include "Logger.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeDataOrderer::Init(void)
{
	mcDataOrder.Init();
	mpcIndexTree = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeDataOrderer::Kill(void)
{
	mcDataOrder.Kill();
	mpcIndexTree = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeDataOrderer::SetIndexTree(CIndexTree* pcIndexTree)
{
	if (pcIndexTree != NULL)
	{
		if (mpcIndexTree != NULL)
		{
			gcLogger.Error2(__METHOD__, " Index Tree is already set on Data Reorderer.", NULL);
		}
	}
	mpcIndexTree = pcIndexTree;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeDataOrderer::New(CIndexTreeDataNode* psNode)
{
	mcDataOrder.InsertBeforeHead(psNode->GetListNode());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeDataOrderer::StartIteration(SDataOrderIterator* psIter, char* pvDestKey, int* piKeySize, int iDestKeySize, void* pvDestData, int* piDataSize, int iDestDataSize)
{
	SLLNode*			psNode;

	psNode = mcDataOrder.GetHead();
	return IterateNode(psNode, psIter, pvDestKey, piKeySize, iDestKeySize, pvDestData, piDataSize, iDestDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeDataOrderer::Iterate(SDataOrderIterator* psIter, char* pvDestKey, int* piKeySize, int iDestKeySize, void* pvDestData, int* piDataSize, int iDestDataSize)
{
	SLLNode*	psNode;

	if (psIter->pcNode)
	{
		
		psNode = mcDataOrder.GetNext(psIter->pcNode->GetListNode());
		return IterateNode(psNode, psIter, pvDestKey, piKeySize, iDestKeySize, pvDestData, piDataSize, iDestDataSize);
	}
	else
	{
		SafeAssign(piDataSize, 0);
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeDataOrderer::IterateNode(SLLNode* psNode, SDataOrderIterator* psIter, char* pvDestKey, int* piKeySize, int iDestKeySize, void* pvDestData, int* piDataSize, int iDestDataSize)
{
	CIndexTreeDataNode* pcDataNode;
	CIndexTreeNode*		pcNode;
	int					iKeySize;
	int					iDataSize;

	if (psNode)
	{
		pcDataNode = (CIndexTreeDataNode*)psNode;
		psIter->pcNode = pcDataNode;
		pcNode = mpcIndexTree->GetNodeForDataNode(pcDataNode);

		iKeySize = mpcIndexTree->GetNodeKey(pcNode, pvDestKey, iDestKeySize);
		SafeAssign(piKeySize, iKeySize);

		iDataSize = mpcIndexTree->GetNodeData(pcNode, pvDestData, iDestDataSize);
		SafeAssign(piDataSize, iDataSize);
		return true;
	}
	else
	{
		psIter->pcNode = NULL;
		SafeAssign(piDataSize, 0);
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNode* CIndexTreeDataOrderer::GetFirstTreeNode(void)
{
	SLLNode*			psNode;
	CIndexTreeDataNode* pcDataNode;
	CIndexTreeNode*		pcNode;

	psNode = mcDataOrder.GetHead();
	if (psNode != NULL)
	{
		pcDataNode = (CIndexTreeDataNode*)psNode;
		pcNode = mpcIndexTree->GetNodeForDataNode(pcDataNode);

		return pcNode;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNode* CIndexTreeDataOrderer::GetLastTreeNode(void)
{
	SLLNode* psNode;
	CIndexTreeDataNode* pcDataNode;
	CIndexTreeNode* pcNode;

	psNode = mcDataOrder.GetTail();
	if (psNode != NULL)
	{
		pcDataNode = (CIndexTreeDataNode*)psNode;
		pcNode = mpcIndexTree->GetNodeForDataNode(pcDataNode);

		return pcNode;
	}
	else
	{
		return NULL;
	}
}

