#include "BaseLib/StackMemory.h"
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
BOOL CIndexTreeEvictionStrategyRandom::Run(CIndexTreeNodeFile* pcDontEvict)
{
	int i;

	for (i = 0; i < 15; i++)
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
	CStackMemory<1 KB>		cStack;
	char*					pvKey;
	int						iKeySize;
	BOOL					bResult;

	pcNode = mpcIndexTree->GetRoot();
	iKeyDepth = 0;
	for (;;)
	{
		if (pcNode == NULL)
		{
			//Nothing left to evict.
			return FALSE;
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
				if (mpcIndexTree->HasDiagnosticCallback())
				{
					pvKey = (char*)cStack.Init(iKeyDepth+1);
					iKeySize = mpcIndexTree->GetNodeKey(pcNode, pvKey, iKeyDepth+1);
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
			else
			{
				return FALSE;
			}
		}
		iKeyDepth++;
	}
}

