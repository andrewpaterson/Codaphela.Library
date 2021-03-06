#include "CreationDataOrderer.h"
#include "IndexTree.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCreationDataOrderer::Init(void)
{
	CIndexTreeDataOrderer::Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCreationDataOrderer::Kill(void)
{
	CIndexTreeDataOrderer::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCreationDataOrderer::Put(CIndexTreeDataNode* psNode)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCreationDataOrderer::Get(CIndexTreeDataNode* psNode)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCreationDataOrderer::Remove(CIndexTreeDataNode* psNode)
{
	mcDataOrder.Remove(psNode->GetListNode());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCreationDataOrderer::HasKey(CIndexTreeDataNode* psNode)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLifeInit<CIndexTreeDataOrderer> CCreationDataOrderer::Create(void)
{
	return LifeAlloc<CCreationDataOrderer, CIndexTreeDataOrderer>();
}

