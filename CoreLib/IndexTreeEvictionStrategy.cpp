#include "BaseLib/Logger.h"
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

