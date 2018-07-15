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
void CIndexTreeEvictionStrategyRandom::Run(CIndexTreeNodeFile* pcDontEvict)
{
	int i;

	for (i = 0; i < 5; i++)
	{
		if (EvictRandomNode(pcDontEvict))
		{
			break;
		}
	}
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
	BOOL					bEvict;
	int						iKeyDepth;

	pcNode = mpcIndexTree->GetRoot();
	iKeyDepth = 0;
	for (;;)
	{
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
				if (pcNode->HasObject())
				{
					bEvict = mpcIndexTree->EvictNodeCallback(pcNode);
				}
				else
				{
					bEvict = TRUE;
				}

				if (bEvict)
				{
					mpcIndexTree->Evict(pcNode);
				}
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
	}
}

