#include "IndexTreeEvictionStrategy.h"
#include "IndexTreeEvicting.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeEvictionStrategy::SetIndexTree(CIndexTreeEvicting* pcIndexTree)
{
	mpcIndexTree = pcIndexTree;
}
