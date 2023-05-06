#include "IndexTreeEvicting.h"
#include "IndexTreeEvictionStrategyRandom.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeEvictionStrategyRandom::Init(void)
{
	Init(67);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeEvictionStrategyRandom::Init(int iSeed)
{
	CIndexTreeEvictionStrategy::Init();
	mcRandom.Init(iSeed);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeEvictionStrategyRandom::Kill(void)
{
	mcRandom.Kill();
	mpcIndexTree = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeEvictionStrategyRandom::Run(CIndexTreeNodeFile* pcDontEvict)
{
	int i;

	for (i = 0; i < 15; i++)
	{
		if (EvictRandomNode(pcDontEvict))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeEvictionStrategyRandom::EvictRandomNode(CIndexTreeNodeFile* pcDontEvict)
{
	CIndexTreeNodeFile*		pcNode;
	int						iNumIndexes;
	int						iIndex;
	int						iKeyDepth;

	pcNode = GetRootNode();
	iKeyDepth = 0;
	for (;;)
	{
		if (pcNode == NULL)
		{
			//Nothing left to evict.
			return false;
		}

		iNumIndexes = pcNode->NumValidMemoryIndexes();
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
				return EvictNode(pcNode);
			}
			else
			{
				return false;
			}
		}
		iKeyDepth++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CRandom* CIndexTreeEvictionStrategyRandom::GetRandom(void)
{
	return &mcRandom;
}

