#ifndef __INDEX_ACCESS_H__
#define __INDEX_ACCESS_H__
#include "Define.h"
#include "Chars.h"
#include "PrimitiveTypes.h"
#include "TemporaryMemory.h"
#include "IndexTreeIterator.h"


class CIndexAccess
{
public:
			void	Init(void);
	virtual bool	Kill(void);

	virtual bool	Flush(void)= 0;
	virtual int64	NumElements(void) =0;
	virtual bool	ValidateIndex(void) =0;
	virtual void	Dump(void) =0;

			bool	PutLongPtr(int64 lliKey, void* pvPointer);
			bool	PutLongInt(int64 lliKey, int iData);
			bool	PutLongChar(int64 lliKey, char cData);
			bool	PutLongString(int64 lliKey, char* pszData);
			bool	PutLongData(int64 lliKey, void* pvData, size_t iDataSize);
			bool	PutLongLong(int64 lliKey, int64 lliData);
			bool	PutIntPtr(int iKey, void* pvPointer);
			bool	PutIntInt(int iKey, int iData);
			bool	PutIntLong(int iKey, int64 lliData);
			bool	PutIntChar(int iKey, char cData);
			bool	PutIntString(int iKey, char* pszData);
			bool	PutIntData(int iKey, void* pvData, size_t iDataSize);
			bool	PutStringPtr(char* pszKey, void* pvPointer);
			bool	PutStringData(char* pszKey, void* pvData, size_t iDataSize);
			bool	PutStringChar(char* pszKey, char cData);
			bool	PutStringInt(char* pszKey, int iData);
			bool	PutStringLong(char* pszKey, int64 lliData);
			bool	PutStringString(char* pszKey, char* pszData);
			bool	PutKeyData(void* pvKey, int iKeySize, void* pvData, size_t iDataSize);
			bool	PutKeyInt(void* pvKey, int iKeySize, int iData);
			bool	PutKeyString(void* pvKey, int iKeySize, char* pszData);

			char*	GetLongString(int64 lliKey, char* pszDest, size_t uiMaxDataSize = 0);
			int		GetLongInt(int64 lliKey, int iNullValue = -1);
			int64	GetLongLong(int64 lliKey, int64 lliNullValue = -1);
			char	GetLongChar(int64 lliKey, char cNullValue = -1);
			bool	GetLongData(int64 lliKey, void* pvData, size_t* piDataSize = NULL, size_t uiMaxDataSize = 0);
			void*	GetLongPtr(int64 lliKey);
			char*	GetIntString(int iKey, char* pszDest, size_t uiMaxDataSize = 0);
			int		GetIntInt(int iKey, int iNullValue = -1);
			int64	GetIntLong(int iKey, int64 iNullValue = -1LL);
			char	GetIntChar(int iKey, char cNullValue = -1);
			void*	GetIntPtr(int iKey);
			bool	GetIntData(int iKey, void* pvData, size_t* piDataSize = NULL, size_t uiMaxDataSize = 0);
			bool	GetStringData(char* pszKey, void* pvData, size_t* piDataSize = NULL, size_t uiMaxDataSize = 0);
			char*	GetStringString(char* pszKey, char* pszDest, size_t uiMaxDataSize = 0);
			CChars	GetStringString(char* pszKey, size_t uiMaxDataSize = 0);
			char	GetStringChar(char* pszKey, char iNullValue = -1);
			int		GetStringInt(char* pszKey, int iNullValue = -1);
			int64	GetStringLong(char* pszKey, int64 iNullValue = -1LL);
			void*	GetStringPtr(char* pszKey);
			char*	GetKeyString(void* pvKey, int iKeySize, char* pszDest, size_t uiMaxDataSize = 0);
			bool	GetKeyData(void* pvKey, int iKeySize, void* pvData, size_t* piDataSize, size_t uiMaxDataSize = 0);
			int		GetKeyInt(void* pvKey, int iKeySize, int iNullValue = -1);

			bool	HasLong(int64 lliKey);
			bool	HasInt(int iKey);
			bool	HasString(char* pszKey);
			bool	HasKey(void* pvKey, int iKeySize);

			bool	DeleteLong(int64 lliKey);
			bool	DeleteString(char* pszKey);
			bool	DeleteKey(void* pvKey, int iKeySize);

			int		DataSizeLong(int64 lliKey);
			int		DataSizeString(char* pszKey);
			int		DataSizeKey(void* pvKey, int iKeySize);

			bool	FlushLong(int64 lliKey);
			bool	FlushString(char* pszKey);
			bool	FlushKey(void* pvKey, int iKeySize);

			bool	EvictLong(int64 lliKey);
			bool	EvictInt(int iKey);
			bool	EvictString(char* pszKey);
			bool	EvictKey(void* pvKey, int iKeySize);

public:
	virtual	CIndexTreeIterator* CreateIterator(void) =0;
	virtual	void				FreeIterator(CIndexTreeIterator* pcIter) =0;

protected:
			char*	GetString(void* pvKey, int iKeySize, char* pszDest, size_t uiMaxDataSize);

protected:
	virtual bool	Put(void* pvKey, int iKeySize, void* pvData, size_t iDataSize) =0;
	virtual bool	Get(void* pvKey, int iKeySize, void* pvData, size_t* piDataSize, size_t uiMaxDataSize) =0;
	virtual int		DataSize(void* pvKey, int iKeySize) =0;
	virtual bool	Remove(void* pvKey, int iKeySize) =0;
	virtual bool	Has(void* pvKey, int iKeySize) =0;
	virtual bool	Flush(void* pvKey, int iKeySize) =0;
	virtual bool	Evict(void* pvKey, int iKeySize) =0;
};


#endif // !__INDEX_ACCESS_H__

