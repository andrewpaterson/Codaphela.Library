#ifndef __INDEX_STRING_BLOCK_H__
#define __INDEX_STRING_BLOCK_H__
#include "IndexBlock.h"


class CIndexStringBlock : public CIndexBlock
{
public:
	void*	Get(char* szKey, char* szLastCharInclusive = NULL);
	void*	Get(const char* szKey, const char* szLastCharInclusive = NULL);
	BOOL	Get(char* szKey, void** ppvData, int* piDataSize, char* szLastCharInclusive = NULL);
	BOOL	Get(const char* szKey, void** ppvData, int* piDataSize, const char* szLastCharInclusive = NULL);
	void*	GetLongestPartial(char* szKey, char* szLastCharInclusive = NULL);
	BOOL	GetLongestPartial(char* szKey, void** ppvData, int* piDataSize, char* szLastCharInclusive = NULL);

	void*	Put(char* szKey, int iDataSize, char* szLastCharInclusive = NULL);
	BOOL	Put(char* szKey, void* pvData, int iDataSize, char* szLastCharInclusive = NULL);
	void*	Put(const char* szKey, int iDataSize, const char* szLastCharInclusive = NULL);
	BOOL	Put(const char* szKey, void* pvData, int iDataSize, const char* szLastCharInclusive = NULL);

	BOOL	HasKey(char* szKey, char* szLastCharInclusive = NULL);
	BOOL	HasKey(const char* szKey, const char* szLastCharInclusive = NULL);

	BOOL	Remove(char* szKey, char* szLastCharInclusive = NULL);
	BOOL	Remove(const char* szKey, const char* szLastCharInclusive = NULL);
};


#endif // !__INDEX_STRING_BLOCK_H__

