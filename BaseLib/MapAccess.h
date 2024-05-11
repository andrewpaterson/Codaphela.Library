#ifndef __MAP_ACCESS_H__
#define __MAP_ACCESS_H__
#include "Define.h"
#include "Chars.h"
#include "Killable.h"
#include "PrimitiveTypes.h"
#include "TemporaryMemory.h"
#include "MapIterator.h"


class CMapAccess : public CKillable
{
public:
			void	Init(void);

	virtual int64	NumElements(void) =0;
	virtual void	Dump(void) =0;

			void**	PutLongPtr(int64 lliKey, void* pvPointer);
			int16*	PutLongShort(int64 lliKey, int16 iData);
			int32*	PutLongInt(int64 lliKey, int32 iData);
			int64*	PutLongLong(int64 lliKey, int64 lliData);
			char*	PutLongChar(int64 lliKey, char cData);
			char*	PutLongString(int64 lliKey, char* pszData);
			void*	PutLongData(int64 lliKey, void* pvData, size iDataSize);
			void**	PutShortPtr(int16 iKey, void* pvPointer);
			int16*	PutShortShort(int16 iKey, int16 iData);
			int32*	PutShortInt(int16 iKey, int32 iData);
			int64*	PutShortLong(int16 iKey, int64 lliData);
			char*	PutShortChar(int16 iKey, char cData);
			char*	PutShortString(int16 iKey, char* pszData);
			void*	PutShortData(int16 iKey, void* pvData, size iDataSize);
			void**	PutIntPtr(int32 iKey, void* pvPointer);
			int16*	PutIntShort(int32 iKey, int16 iData);
			int32*	PutIntInt(int32 iKey, int32 iData);
			int64*	PutIntLong(int32 iKey, int64 lliData);
			char*	PutIntChar(int32 iKey, char cData);
			char*	PutIntString(int32 iKey, char* pszData);
			void*	PutIntData(int32 iKey, void* pvData, size iDataSize);
			void**	PutStringPtr(char* pszKey, void* pvPointer);
			void*	PutStringData(char* pszKey, void* pvData, size iDataSize);
			char*	PutStringChar(char* pszKey, char cData);
			int16*	PutStringShort(char* pszKey, int16 iData);
			int32*	PutStringInt(char* pszKey, int32 iData);
			int64*	PutStringLong(char* pszKey, int64 lliData);
			char*	PutStringString(char* pszKey, char* pszData);
			void*	PutKeyData(void* pvKey, size iKeySize, void* pvData, size iDataSize);
			int32*	PutKeyInt(void* pvKey, size iKeySize, int32 iData);
			char*	PutKeyString(void* pvKey, size iKeySize, char* pszData);

			char*	GetLongString(int64 lliKey);
			int16	GetLongShort(int64 lliKey, int16 iNullValue = -1);
			int32	GetLongInt(int64 lliKey, int32 iNullValue = -1);
			int64	GetLongLong(int64 lliKey, int64 lliNullValue = -1);
			char	GetLongChar(int64 lliKey, char cNullValue = -1);
			void*	GetLongData(int64 lliKey, size* piDataSize = NULL);
			void*	GetLongPtr(int64 lliKey);
			char*	GetShortString(int16 iKey);
			int16	GetShortShort(int16 iKey, int16 iNullValue = -1);
			int32	GetShortInt(int16 iKey, int32 iNullValue = -1);
			int64	GetShortLong(int16 iKey, int64 iNullValue = -1LL);
			char	GetShortChar(int16 iKey, char cNullValue = -1);
			void*	GetShortData(int16 iKey, size* piDataSize = NULL);
			void*	GetShortPtr(int16 iKey);
			char*	GetIntString(int32 iKey);
			int16	GetIntShort(int32 iKey, int16 iNullValue = -1);
			int32	GetIntInt(int32 iKey, int32 iNullValue = -1);
			int64	GetIntLong(int32 iKey, int64 iNullValue = -1LL);
			char	GetIntChar(int32 iKey, char cNullValue = -1);
			void*	GetIntData(int32 iKey, size* piDataSize = NULL);
			void*	GetIntPtr(int32 iKey);
			void*	GetStringData(char* pszKey, size* piDataSize = NULL);
			char*	GetStringString(char* pszKey);
			char	GetStringChar(char* pszKey, char iNullValue = -1);
			int16	GetStringShort(char* pszKey, int16 iNullValue = -1);
			int32	GetStringInt(char* pszKey, int32 iNullValue = -1);
			int64	GetStringLong(char* pszKey, int64 iNullValue = -1LL);
			void*	GetStringPtr(char* pszKey);
			char*	GetKeyString(void* pvKey, size iKeySize);
			void*	GetKeyData(void* pvKey, size iKeySize, size* piDataSize = NULL);
			int16	GetKeyShort(void* pvKey, size iKeySize, int16 iNullValue = -1);
			int32	GetKeyInt(void* pvKey, size iKeySize, int32 iNullValue = -1);
			int64	GetKeyLong(void* pvKey, size iKeySize, int64 lliNullValue = -1);
			
			bool	HasShort(int16 iKey);
			bool	HasInt(int32 iKey);
			bool	HasLong(int64 lliKey);
			bool	HasString(char* pszKey);
			bool	HasKey(void* pvKey, size iKeySize);

			bool	DeleteShort(int16 iKey);
			bool	DeleteInt(int32 iKey);
			bool	DeleteLong(int64 lliKey);
			bool	DeleteString(char* pszKey);
			bool	DeleteKey(void* pvKey, size iKeySize);

			size	DataSizeInt(int16 lliKey);
			size	DataSizeShort(int32 lliKey);
			size	DataSizeLong(int64 lliKey);
			size	DataSizeString(char* pszKey);
			size	DataSizeKey(void* pvKey, size iKeySize);

public:
	virtual CMapIterator*	CreateIterator(void) =0;
	virtual void			FreeIterator(CMapIterator* pcIter) =0;

protected:
			char*	GetString(void* pvKey, size iKeySize);

protected:
	virtual void*	Put(void* pvKey, size iKeySize, void* pvData, size iDataSize) =0;
	virtual void*	Get(void* pvKey, size iKeySize, size* piDataSize) =0;
	virtual size	DataSize(void* pvKey, size iKeySize) =0;
	virtual bool	Remove(void* pvKey, size iKeySize) =0;
	virtual bool	Has(void* pvKey, size iKeySize) =0;
};


#endif // __MAP_ACCESS_H__

