#ifndef __INDEX_TREE_ACCESS_H__
#define __INDEX_TREE_ACCESS_H__
#include "Define.h"
#include "Chars.h"
#include "PrimitiveTypes.h"
#include "TemporaryMemory.h"
#include "IndexTreeIterator.h"


class CIndexTreeAccess
{
public:
			void	Init(void);
	virtual BOOL	Kill(void);

	virtual BOOL	Flush(void)= 0;
	virtual int64	NumElements(void) =0;
	virtual BOOL	ValidateIndex(void) =0;
	virtual void	Dump(void) =0;

			BOOL	PutLongPtr(int64 lliKey, void* pvPointer);
			BOOL	PutLongInt(int64 lliKey, int iData);
			BOOL	PutLongChar(int64 lliKey, char cData);
			BOOL	PutLongString(int64 lliKey, char* pszData);
			BOOL	PutLongData(int64 lliKey, void* pvData, size_t iDataSize);
			BOOL	PutIntPtr(int iKey, void* pvPointer);
			BOOL	PutIntInt(int iKey, int iData);
			BOOL	PutIntLong(int iKey, int64 lliData);
			BOOL	PutIntChar(int iKey, char cData);
			BOOL	PutIntString(int iKey, char* pszData);
			BOOL	PutIntData(int iKey, void* pvData, size_t iDataSize);
			BOOL	PutStringPtr(char* pszKey, void* pvPointer);
			BOOL	PutStringData(char* pszKey, void* pvData, size_t iDataSize);
			BOOL	PutStringChar(char* pszKey, char cData);
			BOOL	PutStringInt(char* pszKey, int iData);
			BOOL	PutStringLong(char* pszKey, int64 lliData);
			BOOL	PutStringString(char* pszKey, char* pszData);
			BOOL	PutKeyData(void* pvKey, int iKeySize, void* pvData, size_t iDataSize);
			BOOL	PutKeyInt(void* pvKey, int iKeySize, int iData);
			BOOL	PutKeyString(void* pvKey, int iKeySize, char* pszData);

			char*	GetLongString(int64 lliKey, char* pszDest, size_t uiMaxDataSize = 0);
			int		GetLongInt(int64 lliKey, int iNullValue = -1);
			int64	GetLongLong(int64 lliKey, int64 lliNullValue = -1);
			char	GetLongChar(int64 lliKey, char cNullValue = -1);
			BOOL	GetLongData(int64 lliKey, void* pvData, size_t* piDataSize = NULL, size_t uiMaxDataSize = 0);
			void*	GetLongPtr(int64 lliKey);
			char*	GetIntString(int iKey, char* pszDest, size_t uiMaxDataSize = 0);
			int		GetIntInt(int iKey, int iNullValue = -1);
			int64	GetIntLong(int iKey, int64 iNullValue = -1LL);
			char	GetIntChar(int iKey, char cNullValue = -1);
			BOOL	GetIntData(int iKey, void* pvData, size_t* piDataSize = NULL, size_t uiMaxDataSize = 0);
			BOOL	GetStringData(char* pszKey, void* pvData, size_t* piDataSize = NULL, size_t uiMaxDataSize = 0);
			char*	GetStringString(char* pszKey, char* pszDest, size_t uiMaxDataSize = 0);
			CChars	GetStringString(char* pszKey, size_t uiMaxDataSize = 0);
			char	GetStringChar(char* pszKey, char iNullValue = -1);
			int		GetStringInt(char* pszKey, int iNullValue = -1);
			int64	GetStringLong(char* pszKey, int64 iNullValue = -1LL);
			void*	GetStringPtr(char* pszKey);
			char*	GetKeyString(void* pvKey, int iKeySize, char* pszDest, size_t uiMaxDataSize = 0);
			BOOL	GetKeyData(void* pvKey, int iKeySize, void* pvData, size_t* piDataSize, size_t uiMaxDataSize = 0);

			BOOL	HasLong(int64 lliKey);
			BOOL	HasInt(int iKey);
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
			BOOL	EvictInt(int iKey);
			BOOL	EvictString(char* pszKey);
			BOOL	EvictKey(void* pvKey, int iKeySize);

public:
	virtual	CIndexTreeIterator* CreateIterator(void) =0;
	virtual	void				FreeIterator(CIndexTreeIterator* pcIter) =0;

protected:
	virtual BOOL	Put(void* pvKey, int iKeySize, void* pvData, size_t iDataSize) =0;
	virtual BOOL	Get(void* pvKey, int iKeySize, void* pvData, size_t* piDataSize, size_t uiMaxDataSize) =0;
	virtual int		DataSize(void* pvKey, int iKeySize) =0;
	virtual BOOL	Remove(void* pvKey, int iKeySize) =0;
	virtual BOOL	Has(void* pvKey, int iKeySize) =0;
	virtual BOOL	Flush(void* pvKey, int iKeySize) =0;
	virtual BOOL	Evict(void* pvKey, int iKeySize) =0;
};


#endif // __INDEX_TREE_ACCESS_H__

