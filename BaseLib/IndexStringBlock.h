#ifndef __INDEX_STRING_BLOCK_H__
#define __INDEX_STRING_BLOCK_H__
#include "IndexBlock.h"


class CIndexStringBlock : public CIndexBlock
{
public:
	void*	Get(char* szKey, char* szLastCharInclusive = NULL);
	void*	Get(const char* szKey, const char* szLastCharInclusive = NULL);
	bool	Get(char* szKey, void** ppvData, int* piDataSize, char* szLastCharInclusive = NULL);
	bool	Get(const char* szKey, void** ppvData, int* piDataSize, const char* szLastCharInclusive = NULL);
	void*	GetLongestPartial(char* szKey, char* szLastCharInclusive = NULL);
	bool	GetLongestPartial(char* szKey, void** ppvData, int* piDataSize, char* szLastCharInclusive = NULL);

	void*	Put(char* szKey, int iDataSize, char* szLastCharInclusive = NULL);
	bool	Put(char* szKey, void* pvData, int iDataSize, char* szLastCharInclusive = NULL);
	void*	Put(const char* szKey, int iDataSize, const char* szLastCharInclusive = NULL);
	bool	Put(const char* szKey, void* pvData, int iDataSize, const char* szLastCharInclusive = NULL);

	bool	HasKey(char* szKey, char* szLastCharInclusive = NULL);
	bool	HasKey(const char* szKey, const char* szLastCharInclusive = NULL);

	bool	Remove(char* szKey, char* szLastCharInclusive = NULL);
	bool	Remove(const char* szKey, const char* szLastCharInclusive = NULL);
};


#endif // !__INDEX_STRING_BLOCK_H__

