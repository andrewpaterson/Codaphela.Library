#include "DataMacro.h"
#include "SetBlock.h"
#include "StringHelper.h"
#include "SetNode.h"
#include "Chars.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int SSNode::Compare(const void* arg1, const void* arg2)
{
	return fCompare(arg1, arg2);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareSNode(const void* arg1, const void* arg2)
{
	SSNode*		pNode1;
	SSNode*		pNode2;
	void*		pvData1;
	void*		pvData2;
	int			iResult;

	pNode1 = *((SSNode**)arg1);
	pNode2 = *((SSNode**)arg2);
	pvData1 = HeaderGetData<SSNode, void>(pNode1);
	pvData2 = HeaderGetData<SSNode, void>(pNode2);

	iResult = pNode1->Compare(pvData1, pvData2);
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareSNodeData(const void* arg1, const void* arg2)
{
	SSNode*		pNode1;
	SSNode*		pNode2;
	void*		pvData1;
	void*		pvData2;
	int			iResult;

	pNode1 = *((SSNode**)arg1);
	pNode2 = *((SSNode**)arg2);
	pvData1 = HeaderGetData<SSNode, void>(pNode1);
	pvData2 = HeaderGetData<SSNode, void>(pNode2);

	iResult = MemCmp(pvData1, pNode1->iDataSize, pvData2, pNode2->iDataSize);
	return iResult;
}

