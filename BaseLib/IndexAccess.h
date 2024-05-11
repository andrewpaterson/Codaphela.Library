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
			bool	PutLongShort(int64 lliKey, int32 iData);
			bool	PutLongInt(int64 lliKey, int32 iData);
			bool	PutLongChar(int64 lliKey, char cData);
			bool	PutLongString(int64 lliKey, char* pszData);
			bool	PutLongData(int64 lliKey, void* pvData, size iDataSize);
			bool	PutLongLong(int64 lliKey, int64 lliData);
			bool	PutShortPtr(int16 iKey, void* pvPointer);
			bool	PutShortShort(int16 iKey, int16 iData);
			bool	PutShortInt(int16 iKey, int32 iData);
			bool	PutShortLong(int16 iKey, int64 lliData);
			bool	PutShortChar(int16 iKey, char cData);
			bool	PutShortString(int16 iKey, char* pszData);
			bool	PutShortData(int16 iKey, void* pvData, size iDataSize);
			bool	PutIntPtr(int32 iKey, void* pvPointer);
			bool	PutIntShort(int32 iKey, int16 iData);
			bool	PutIntInt(int32 iKey, int32 iData);
			bool	PutIntLong(int32 iKey, int64 lliData);
			bool	PutIntChar(int32 iKey, char cData);
			bool	PutIntString(int32 iKey, char* pszData);
			bool	PutIntData(int32 iKey, void* pvData, size iDataSize);
			bool	PutStringPtr(char* pszKey, void* pvPointer);
			bool	PutStringData(char* pszKey, void* pvData, size iDataSize);
			bool	PutStringChar(char* pszKey, char cData);
			bool	PutStringShort(char* pszKey, int16 iData);
			bool	PutStringInt(char* pszKey, int32 iData);
			bool	PutStringLong(char* pszKey, int64 lliData);
			bool	PutStringString(char* pszKey, char* pszData);
			bool	PutKeyData(void* pvKey, size iKeySize, void* pvData, size iDataSize);
			bool	PutKeyShort(void* pvKey, size iKeySize, int16 iData);
			bool	PutKeyInt(void* pvKey, size iKeySize, int32 iData);
			bool	PutKeyLong(void* pvKey, size iKeySize, int64 iData);
			bool	PutKeyString(void* pvKey, size iKeySize, char* pszData);

			char*	GetLongString(int64 lliKey, char* pszDest, size uiMaxDataSize = 0);
			int16	GetLongShort(int64 lliKey, int16 iNullValue = -1);
			int32	GetLongInt(int64 lliKey, int32 iNullValue = -1);
			int64	GetLongLong(int64 lliKey, int64 lliNullValue = -1);
			char	GetLongChar(int64 lliKey, char cNullValue = -1);
			bool	GetLongData(int64 lliKey, void* pvData, size* piDataSize = NULL, size uiMaxDataSize = 0);
			void*	GetLongPtr(int64 lliKey);
			char*	GetShortString(int16 iKey, char* pszDest, size uiMaxDataSize = 0);
			int16	GetShortShort(int16 iKey, int16 iNullValue = -1);
			int32	GetShortInt(int16 iKey, int32 iNullValue = -1);
			int64	GetShortLong(int16 iKey, int64 iNullValue = -1LL);
			char	GetShortChar(int16 iKey, char cNullValue = -1);
			void*	GetShortPtr(int16 iKey);
			bool	GetShortData(int16 iKey, void* pvData, size* piDataSize = NULL, size uiMaxDataSize = 0);
			char*	GetIntString(int32 iKey, char* pszDest, size uiMaxDataSize = 0);
			int16	GetIntShort(int32 iKey, int16 iNullValue = -1);
			int32	GetIntInt(int32 iKey, int32 iNullValue = -1);
			int64	GetIntLong(int32 iKey, int64 iNullValue = -1LL);
			char	GetIntChar(int32 iKey, char cNullValue = -1);
			void*	GetIntPtr(int32 iKey);
			bool	GetIntData(int32 iKey, void* pvData, size* piDataSize = NULL, size uiMaxDataSize = 0);
			bool	GetStringData(char* pszKey, void* pvData, size* piDataSize = NULL, size uiMaxDataSize = 0);
			char*	GetStringString(char* pszKey, char* pszDest, size uiMaxDataSize = 0);
			CChars	GetStringString(char* pszKey, size uiMaxDataSize = 0);
			char	GetStringChar(char* pszKey, char iNullValue = -1);
			int16	GetStringShort(char* pszKey, int16 iNullValue = -1);
			int32	GetStringInt(char* pszKey, int32 iNullValue = -1);
			int64	GetStringLong(char* pszKey, int64 iNullValue = -1LL);
			void*	GetStringPtr(char* pszKey);
			char*	GetKeyString(void* pvKey, size iKeySize, char* pszDest, size uiMaxDataSize = 0);
			bool	GetKeyData(void* pvKey, size iKeySize, void* pvData, size* piDataSize, size uiMaxDataSize = 0);
			int16	GetKeyShort(void* pvKey, size iKeySize, int16 iNullValue = -1);
			int32	GetKeyInt(void* pvKey, size iKeySize, int32 iNullValue = -1);
			int64	GetKeyLong(void* pvKey, size iKeySize, int64 iNullValue = -1);

			bool	HasShort(int16 iKey);
			bool	HasInt(int32 iKey);
			bool	HasLong(int64 lliKey);
			bool	HasString(char* pszKey);
			bool	HasKey(void* pvKey, size iKeySize);

			bool	DeleteShort(int16 lliKey);
			bool	DeleteInt(int32 lliKey);
			bool	DeleteLong(int64 lliKey);
			bool	DeleteString(char* pszKey);
			bool	DeleteKey(void* pvKey, size iKeySize);

			size	DataSizeLong(int16 lliKey);
			size	DataSizeLong(int32 lliKey);
			size	DataSizeLong(int64 lliKey);
			size	DataSizeString(char* pszKey);
			size	DataSizeKey(void* pvKey, size iKeySize);

			bool	FlushLong(int16 lliKey);
			bool	FlushLong(int32 lliKey);
			bool	FlushLong(int64 lliKey);
			bool	FlushString(char* pszKey);
			bool	FlushKey(void* pvKey, size iKeySize);

			bool	EvictShort(int16 iKey);
			bool	EvictInt(int32 iKey);
			bool	EvictLong(int64 lliKey);
			bool	EvictString(char* pszKey);
			bool	EvictKey(void* pvKey, size iKeySize);

public:
	virtual	CIndexTreeIterator* CreateIterator(void) =0;
	virtual	void				FreeIterator(CIndexTreeIterator* pcIter) =0;

protected:
			char*	GetString(void* pvKey, size iKeySize, char* pszDest, size uiMaxDataSize);

protected:
	virtual bool	Put(void* pvKey, size iKeySize, void* pvData, size iDataSize) =0;
	virtual bool	Get(void* pvKey, size iKeySize, void* pvData, size* piDataSize, size uiMaxDataSize) =0;
	virtual size	DataSize(void* pvKey, size iKeySize) =0;
	virtual bool	Remove(void* pvKey, size iKeySize) =0;
	virtual bool	Has(void* pvKey, size iKeySize) =0;
	virtual bool	Flush(void* pvKey, size iKeySize) =0;
	virtual bool	Evict(void* pvKey, size iKeySize) =0;
};


#endif // __INDEX_ACCESS_H__

