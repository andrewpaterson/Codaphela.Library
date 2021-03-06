#include "ModificationDataOrderer.h"
#include "IndexTree.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CModificationDataOrderer::Init(void)
{
	CIndexTreeDataOrderer::Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CModificationDataOrderer::Kill(void)
{
	CIndexTreeDataOrderer::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CModificationDataOrderer::Put(CIndexTreeDataNode* psNode)
{
	mcDataOrder.MoveToHead(psNode->GetListNode());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CModificationDataOrderer::Get(CIndexTreeDataNode* psNode)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CModificationDataOrderer::Remove(CIndexTreeDataNode* psNode)
{
	mcDataOrder.Remove(psNode->GetListNode());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CModificationDataOrderer::HasKey(CIndexTreeDataNode* psNode)
{
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLifeInit<CIndexTreeDataOrderer> CModificationDataOrderer::Create(void)
{

	return LifeAlloc<CModificationDataOrderer, CIndexTreeDataOrderer>();
}

