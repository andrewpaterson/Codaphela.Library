#ifndef __INDEX_TREE_ACCESS_H__
#define __INDEX_TREE_ACCESS_H__
#include "Define.h"
#include "Chars.h"
#include "PrimitiveTypes.h"
#include "TemporaryMemory.h"


class CIndexTreeAccess
{
private:
	CTemporaryMemory	mcTemp;

public:
			void	Init(void);
	virtual BOOL	Kill(void);

	virtual BOOL	Flush(void)= 0;
	virtual int64	NumElements(void) =0;

			BOOL	PutLongPtr(int64 lliKey, void* pvPointer);
			BOOL	PutLongInt(int64 lliKey, int iData);
			BOOL	PutLongString(int64 lliKey, char* pszData);
			BOOL	PutLongData(int64 lliKey, void* pvData, int iDataSize);
			BOOL	PutStringPtr(char* pszKey, void* pvPointer);
			BOOL	PutStringData(char* pszKey, void* pvData, int iDataSize);
			BOOL	PutStringInt(char* pszKey, int iData);
			BOOL	PutStringLong(char* pszKey, int64 lliData);
			BOOL	PutStringString(char* pszKey, char* pszData);
			BOOL	PutKeyData(void* pvKey, int iKeySize, void* pvData, int iDataSize);
			BOOL	PutKeyString(void* pvKey, int iKeySize, char* pszData);

			char*	GetLongString(int64 lliKey, char* pszDest);
			int		GetLongInt(int64 lliKey, int iNUllValue = -1);
			BOOL	GetLongData(int64 lliKey, void* pvData, int* piDataSize = NULL);
			BOOL	GetStringData(char* pszKey, void* pvData, int* piDataSize = NULL);
			char*	GetStringString(char* pszKey, char* pszDest);
			CChars	GetStringString(char* pszKey);
			int		GetStringInt(char* pszKey, int iNullValue = -1);
			int64	GetStringLong(char* pszKey, int64 iNullValue = -1LL);
			void*	GetStringPtr(char* pszKey);
			char*	GetKeyString(void* pvKey, int iKeySize, char* pszDest);
			BOOL	GetKeyData(void* pvKey, int iKeySize, void* pvData, int* piDataSize);

			BOOL	HasLong(int64 lliKey);
			BOOL	HasString(char* pszKey);
			BOOL	HasKey(void* pvKey, int iKeySize);

			BOOL	DeleteLong(int64 lliKey);
			BOOL	DeleteString(char* pszKey);
			BOOL	DeleteKey(void* pvKey, int iKeySize);

			int		DataSizeLong(int64 lliKey);
			int		DataSizeString(char* pszKey);
			int		DataSizeKey(void* pvKey, int iKeySize);

			BOOL	FlushLong(int64 lliKey);
			BOOL	FlushString(char* pszKey);
			BOOL	FlushKey(void* pvKey, int iKeySize);

			BOOL	EvictLong(int64 lliKey);
			BOOL	EvictString(char* pszKey);
			BOOL	EvictKey(void* pvKey, int iKeySize);

protected:
	virtual BOOL	Put(void* pvKey, int iKeySize, void* pvData, int iDataSize) =0;
	virtual BOOL	Get(void* pvKey, int iKeySize, void* pvData, int* piDataSize) =0;
	virtual int		DataSize(void* pvKey, int iKeySize) =0;
	virtual BOOL	Remove(void* pvKey, int iKeySize) =0;
	virtual BOOL	Has(void* pvKey, int iKeySize) =0;
	virtual BOOL	Flush(void* pvKey, int iKeySize) =0;
	virtual BOOL	Evict(void* pvKey, int iKeySize) = 0;
};


#endif // __INDEX_TREE_ACCESS_H__

