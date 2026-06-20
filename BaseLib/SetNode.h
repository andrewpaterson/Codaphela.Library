#ifndef __SET_NODE_H__
#define __SET_NODE_H__
#include "PrimitiveTypes.h"
#include "DataCompare.h"


class CSetBlock;
struct SSNode
{
	size			iDataSize;
	DataCompare		fCompare;

	int Compare(const void* arg1, const void* arg2);
};


int CompareSNode(const void* arg1, const void* arg2);
int CompareSNodeData(const void* arg1, const void* arg2);


#endif // __SET_NODE_H__

