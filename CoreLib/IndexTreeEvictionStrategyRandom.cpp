#include "IndexTreeEvicting.h"
#include "IndexTreeEvictionStrategyRandom.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeEvictionStrategyRandom::Init(void)
{
	mcRandom.Init(67);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeEvictionStrategyRandom::Kill(void)
{
	mcRandom.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvictionStrategyRandom::Run(CIndexTreeNodeFile* pcDontEvict)
{
	int i;

	for (i = 0; i < 5; i++)
	{
		if (EvictRandomNode(pcDontEvict))
		{
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvictionStrategyRandom::EvictRandomNode(CIndexTreeNodeFile* pcDontEvict)
{
	CIndexTreeNodeFile*		pcNode;
	int						iNumIndexes;
	int						iIndex;
	int						iKeyDepth;

	pcNode = mpcIndexTree->GetRoot();
	iKeyDepth = 0;
	for (;;)
	{
		if (pcNode == NULL)
		{
			//Nothing left to evict.
			return FALSE;
		}

		iNumIndexes = pcNode->NumValidIndexes();
		if (iNumIndexes > 1)
		{
			iIndex = mcRandom.Next(0, iNumIndexes - 1);

			pcNode = pcNode->GetValidMemoryNode(iIndex);
		}
		else if (iNumIndexes == 1)
		{
			pcNode = pcNode->GetValidMemoryNode(0);
		}
		else
		{
			if (pcNode != pcDontEvict)
			{
				return mpcIndexTree->Evict(pcNode);
			}
			else
			{
				return FALSE;
			}
		}
	}
}

