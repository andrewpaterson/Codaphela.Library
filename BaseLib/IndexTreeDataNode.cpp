#include "IndexTreeDataNode.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeDataNode::Init(size uiDataSize)
{
	muiDataSize = (uint16)uiDataSize;
	msListNode.psNext = NULL;
	msListNode.psPrev = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeDataNode::Kill(void)
{
	muiDataSize = 0;
	msListNode.psNext = NULL;
	msListNode.psPrev = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeDataNode::GetDataSize(void)
{
	return muiDataSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeDataNode::SetDataSize(size uiDataSize)
{
	muiDataSize = (uint16)uiDataSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SLLNode* CIndexTreeDataNode::GetListNode(void)
{
	return &msListNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeDataNode::RemapListNode(void)
{
	if (msListNode.psNext)
	{
		msListNode.psNext->psPrev = &msListNode;
	}
	if (msListNode.psPrev)
	{
		msListNode.psPrev->psNext = &msListNode;
	}
}

