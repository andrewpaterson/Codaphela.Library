#ifndef __MAP_NODE_H__
#define __MAP_NODE_H__


class CMapBlock;
struct SMNode
{
	int			iKeySize;
	int			iDataSize;
	CMapBlock*	pcMapBlock;
};


typedef int(*CompareFunc)(const void*, const void*);


int CompareMNode(const void* arg1, const void* arg2);
int CompareMNodeKey(const void* arg1, const void* arg2);


#endif // __MAP_NODE_H__


