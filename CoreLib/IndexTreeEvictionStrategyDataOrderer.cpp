#include "BaseLib/Logger.h"
#include "IndexTreeEvicting.h"
#include "IndexTreeEvictionStrategyDataOrderer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeEvictionStrategyDataOrderer::Init(CIndexTreeDataOrderer* pcOrderer)
{
	CIndexTreeEvictionStrategy::Init();
	mpcOrderer = pcOrderer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeEvictionStrategyDataOrderer::Kill(void)
{
	mpcOrderer = NULL;
	mpcIndexTree = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeEvictionStrategyDataOrderer::SetIndexTree(CIndexTreeEvicting* pcIndexTree)
{
	CIndexTreeDataOrderer* pcTreeOrderer;

	CIndexTreeEvictionStrategy::SetIndexTree(pcIndexTree);

	pcTreeOrderer = pcIndexTree->GetDataOrderer();
	if (pcTreeOrderer == NULL)
	{
		gcLogger.Error2(__METHOD__, " IndexTree data orderer [NULL] must be set to this eviction strategy data orderer.", NULL);
	}
	else if (pcTreeOrderer != mpcOrderer)
	{
		gcLogger.Error2(__METHOD__, " IndexTree data orderer [Unknown] must be set to this eviction strategy data orderer.", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvictionStrategyDataOrderer::Run(CIndexTreeNodeFile* pcDontEvict)
{
	CIndexTreeNodeFile*		pcNode;

	pcNode = (CIndexTreeNodeFile*)mpcOrderer->GetLastTreeNode();
	if (pcNode == pcDontEvict || pcNode == NULL)
	{
		return FALSE;
	}

	return EvictNode(pcNode);
}

