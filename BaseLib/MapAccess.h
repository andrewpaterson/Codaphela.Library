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
			int*	PutLongInt(int64 lliKey, int iData);
			int64*	PutLongLong(int64 lliKey, int64 lliData);
			char*	PutLongChar(int64 lliKey, char cData);
			char*	PutLongString(int64 lliKey, char* pszData);
			void*	PutLongData(int64 lliKey, void* pvData, size_t iDataSize);
			void**	PutIntPtr(int iKey, void* pvPointer);
			int*	PutIntInt(int iKey, int iData);
			int64*	PutIntLong(int iKey, int64 lliData);
			char*	PutIntChar(int iKey, char cData);
			char*	PutIntString(int iKey, char* pszData);
			void*	PutIntData(int iKey, void* pvData, size_t iDataSize);
			void**	PutStringPtr(char* pszKey, void* pvPointer);
			void*	PutStringData(char* pszKey, void* pvData, size_t iDataSize);
			char*	PutStringChar(char* pszKey, char cData);
			int*	PutStringInt(char* pszKey, int iData);
			int64*	PutStringLong(char* pszKey, int64 lliData);
			char*	PutStringString(char* pszKey, char* pszData);
			void*	PutKeyData(void* pvKey, int iKeySize, void* pvData, size_t iDataSize);
			int*	PutKeyInt(void* pvKey, int iKeySize, int iData);
			char*	PutKeyString(void* pvKey, int iKeySize, char* pszData);

			char*	GetLongString(int64 lliKey);
			int		GetLongInt(int64 lliKey, int iNullValue = -1);
			int64	GetLongLong(int64 lliKey, int64 lliNullValue = -1);
			char	GetLongChar(int64 lliKey, char cNullValue = -1);
			void*	GetLongData(int64 lliKey, size_t* piDataSize = NULL);
			void*	GetLongPtr(int64 lliKey);
			char*	GetIntString(int iKey);
			int		GetIntInt(int iKey, int iNullValue = -1);
			int64	GetIntLong(int iKey, int64 iNullValue = -1LL);
			char	GetIntChar(int iKey, char cNullValue = -1);
			void*	GetIntData(int iKey, size_t* piDataSize = NULL);
			void*	GetIntPtr(int iKey);
			void*	GetStringData(char* pszKey, size_t* piDataSize = NULL);
			char*	GetStringString(char* pszKey);
			char	GetStringChar(char* pszKey, char iNullValue = -1);
			int		GetStringInt(char* pszKey, int iNullValue = -1);
			int64	GetStringLong(char* pszKey, int64 iNullValue = -1LL);
			void*	GetStringPtr(char* pszKey);
			char*	GetKeyString(void* pvKey, int iKeySize);
			void*	GetKeyData(void* pvKey, int iKeySize, size_t* piDataSize = NULL);
			int		GetKeyInt(void* pvKey, int iKeySize, int iNullValue = -1);
			int64	GetKeyLong(void* pvKey, int iKeySize, int64 lliNullValue = -1);
			
			BOOL	HasLong(int64 lliKey);
			BOOL	HasInt(int iKey);
			BOOL	HasString(char* pszKey);
			BOOL	HasKey(void* pvKey, int iKeySize);

			BOOL	DeleteInt(int iKey);
			BOOL	DeleteLong(int64 lliKey);
			BOOL	DeleteString(char* pszKey);
			BOOL	DeleteKey(void* pvKey, int iKeySize);

			int		DataSizeLong(int64 lliKey);
			int		DataSizeString(char* pszKey);
			int		DataSizeKey(void* pvKey, int iKeySize);

public:
	virtual CMapIterator*	CreateIterator(void) =0;
	virtual void			FreeIterator(CMapIterator* pcIter) =0;

protected:
			char*	GetString(void* pvKey, int iKeySize);

protected:
	virtual void*	Put(void* pvKey, int iKeySize, void* pvData, size_t iDataSize) =0;
	virtual void*	Get(void* pvKey, int iKeySize, size_t* piDataSize) = 0;
	virtual size_t	DataSize(void* pvKey, int iKeySize) =0;
	virtual BOOL	Remove(void* pvKey, int iKeySize) =0;
	virtual BOOL	Has(void* pvKey, int iKeySize) =0;
};


#endif // !__MAP_ACCESS_H__

