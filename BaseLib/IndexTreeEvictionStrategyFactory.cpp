#include "ConstructorCall.h"
#include "LifeCycle.h"
#include "ModificationDataOrderer.h"
#include "AccessDataOrderer.h"
#include "CreationDataOrderer.h"
#include "IndexTreeEvictionStrategyFactory.h"
#include "IndexTreeEvictionStrategyFactory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLifeInit<CIndexTreeEvictionStrategy> CIndexTreeEvictionStrategyFactory::Create(EIndexTreeEvctionStrategySystemType eType)
{
	if (eType == ITESST_Random)
	{
		return LifeAlloc<CIndexTreeEvictionStrategyRandom, CIndexTreeEvictionStrategy>();
	}
	else if (eType == ITESST_Creation)
	{
		CLifeInit<CIndexTreeDataOrderer> cLifeDataOrderer = LifeAlloc<CCreationDataOrderer, CIndexTreeDataOrderer>();
		return LifeAlloc<CIndexTreeEvictionStrategyDataOrderer, CIndexTreeEvictionStrategy>(cLifeDataOrderer);
	}
	else if (eType == ITESST_Modification)
	{
		CLifeInit<CIndexTreeDataOrderer> cLifeDataOrderer = LifeAlloc<CCreationDataOrderer, CIndexTreeDataOrderer>();
		return LifeAlloc<CIndexTreeEvictionStrategyDataOrderer, CIndexTreeEvictionStrategy>(cLifeDataOrderer);
	}
	else if (eType == ITESST_Access)
	{
		CLifeInit<CIndexTreeDataOrderer> cLifeDataOrderer = LifeAlloc<CCreationDataOrderer, CIndexTreeDataOrderer>();
		return LifeAlloc<CIndexTreeEvictionStrategyDataOrderer, CIndexTreeEvictionStrategy>(cLifeDataOrderer);
	}
	else
	{
		CLifeInit<CIndexTreeEvictionStrategy> cLife;
		cLife.Null();
		return cLife;
	}
}


