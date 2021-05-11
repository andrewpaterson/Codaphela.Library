#include "DataMacro.h"
#include "MapBlock.h"
#include "StringHelper.h"
#include "MapNode.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareMNode(const void* arg1, const void* arg2)
{
	SMNode*		pNode1;
	SMNode*		pNode2;
	void*		pvKey1;
	void*		pvKey2;
	int			iResult;

	pNode1 = *((SMNode**)arg1);
	pNode2 = *((SMNode**)arg2);

	pvKey1 = HeaderGetData<SMNode, void>(pNode1);
	pvKey2 = HeaderGetData<SMNode, void>(pNode2);

	iResult = pNode1->pcMapBlock->fKeyCompare(pvKey1, pvKey2);
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareMNodeKey(const void* arg1, const void* arg2)
{
	SMNode*		pNode1;
	SMNode*		pNode2;
	void*		pvKey1;
	void*		pvKey2;
	int			iResult;

	pNode1 = *((SMNode**)arg1);
	pNode2 = *((SMNode**)arg2);
	pvKey1 = HeaderGetData<SMNode, void>(pNode1);
	pvKey2 = HeaderGetData<SMNode, void>(pNode2);

	iResult = MemCmp(pvKey1, pNode1->iKeySize, pvKey2, pNode2->iKeySize);
	return iResult;
}

