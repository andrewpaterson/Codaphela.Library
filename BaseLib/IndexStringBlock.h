#ifndef __INDEX_STRING_BLOCK_H__
#define __INDEX_STRING_BLOCK_H__
#include "IndexBlock.h"


class CIndexStringBlock : public CIndexBlock
{
public:
	void*	Get(const char* szKey, const char* szLastCharInclusive = NULL);
	bool	Get(const char* szKey, void** ppvData, size* piDataSize, const char* szLastCharInclusive = NULL);
	void*	GetLongestPartial(const char* szKey, const char* szLastCharInclusive = NULL);
	bool	GetLongestPartial(char* szKey, void** ppvData, size* piDataSize, char* szLastCharInclusive = NULL);

	void*	Put(const char* szKey, size iDataSize, const char* szLastCharInclusive = NULL);
	bool	Put(const char* szKey, void* pvData, size iDataSize, const char* szLastCharInclusive = NULL);

	bool	HasKey(const char* szKey, const char* szLastCharInclusive = NULL);

	bool	Remove(const char* szKey, const char* szLastCharInclusive = NULL);
};


#endif // __INDEX_STRING_BLOCK_H__

