#include "BaseLib/Logger.h"
#include "BaseLib/StackMemory.h"
#include "IndexTreeEvictionStrategy.h"
#include "IndexTreeEvicting.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeEvictionStrategy::Init(void)
{
	mpcIndexTree = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeEvictionStrategy::SetIndexTree(CIndexTreeEvicting* pcIndexTree)
{
	if (pcIndexTree != NULL)
	{
		if (mpcIndexTree != NULL)
		{
			gcLogger.Error2(__METHOD__, " Index Tree is already set on Eviction Strategy.", NULL);
		}
	}
	mpcIndexTree = pcIndexTree;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvictionStrategy::EvictNode(CIndexTreeNodeFile* pcNode)
{
	CStackMemory<1 KB>		cStack;
	char*					pvKey;
	int						iKeySize;
	BOOL					bResult;

	if (mpcIndexTree->HasDiagnosticCallback())
	{
		iKeySize = mpcIndexTree->GetNodeKeySize(pcNode);
		pvKey = (char*)cStack.Init(iKeySize + 1);
		iKeySize = mpcIndexTree->GetNodeKey(pcNode, pvKey, iKeySize + 1);
	}
	else
	{
		iKeySize = 0;
		pvKey = NULL;
	}

	bResult = mpcIndexTree->EvictNode(pcNode, pvKey, iKeySize);

	if (mpcIndexTree->HasDiagnosticCallback())
	{
		cStack.Kill();
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeEvictionStrategy::GetRootNode(void)
{
	return mpcIndexTree->GetRoot();
}

