#include "IndexTreeDataOrdererConstructors.h"
#include "AccessDataOrderer.h"
#include "CreationDataOrderer.h"
#include "ModificationDataOrderer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void IndexTreeDataOrdererInit(CConstructors* pcConstructors)
{
	pcConstructors->Add<CAccessDataOrderer>();
	pcConstructors->Add<CCreationDataOrderer>();
	pcConstructors->Add<CModificationDataOrderer>();
}

