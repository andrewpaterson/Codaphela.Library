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
BOOL CIndexTreeDataOrderer::StartIteration(SDataOrderIterator* psIter, void** ppvData, int* piDataSize)
{
	SLLNode*			psNode;

	psNode = mcDataOrder.GetHead();
	return IterateNode(psNode, psIter, ppvData, piDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeDataOrderer::Iterate(SDataOrderIterator* psIter, void** ppvData, int* piDataSize)
{
	SLLNode*	psNode;

	if (psIter->pcNode)
	{
		
		psNode = mcDataOrder.GetNext(psIter->pcNode->GetListNode());
		return IterateNode(psNode, psIter, ppvData, piDataSize);
	}
	else
	{
		SafeAssign(ppvData, NULL);
		SafeAssign(piDataSize, 0);
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeDataOrderer::IterateNode(SLLNode* psNode, SDataOrderIterator* psIter, void** ppvData, int* piDataSize)
{
	CIndexTreeDataNode* pcDataNode;
	void* pvData;

	if (psNode)
	{
		pcDataNode = (CIndexTreeDataNode*)psNode;
		psIter->pcNode = pcDataNode;
		pvData = mpcIndexTree->GetDataForDataNode(pcDataNode);
		SafeAssign(ppvData, pvData);
		SafeAssign(piDataSize, pcDataNode->GetDataSize());
		return TRUE;
	}
	else
	{
		psIter->pcNode = NULL;
		SafeAssign(ppvData, NULL);
		SafeAssign(piDataSize, 0);
		return FALSE;
	}
}


