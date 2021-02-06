#include "AccessDataOrderer.h"
#include "IndexTree.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAccessDataOrderer::Init(void)
{
	CIndexTreeDataOrderer::Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAccessDataOrderer::Kill(void)
{
	CIndexTreeDataOrderer::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAccessDataOrderer::Put(CIndexTreeDataNode* psNode)
{
	mcDataOrder.MoveToHead(psNode->GetListNode());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAccessDataOrderer::Get(CIndexTreeDataNode* psNode)
{
	mcDataOrder.MoveToHead(psNode->GetListNode());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAccessDataOrderer::Remove(CIndexTreeDataNode* psNode)
{
	mcDataOrder.Remove(psNode->GetListNode());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAccessDataOrderer::HasKey(CIndexTreeDataNode* psNode)
{
	mcDataOrder.MoveToHead(psNode->GetListNode());
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLifeInit<CIndexTreeDataOrderer> CAccessDataOrderer::Create(void)
{
	return LifeAlloc<CAccessDataOrderer, CIndexTreeDataOrderer>();
}

