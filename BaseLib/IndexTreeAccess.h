#ifndef __INDEX_TREE_ACCESS_H__
#define __INDEX_TREE_ACCESS_H__
#include "Define.h"
#include "PrimitiveTypes.h"
#include "TemporaryMemory.h"

class CIndexTreeAccess
{
private:
	CTemporaryMemory		mcTemp;

public:
			void			Init(void);
	virtual BOOL			Kill(void);

	virtual BOOL			Flush(void)= 0;

			BOOL			PutLongPtr(int64 lliKey, void* pvPointer);
			BOOL			PutLongString(int64 lliKey, char* pszData);
			BOOL			PutLongData(int64 lliKey, void* pvObject, unsigned int uiDataSize);
			BOOL			PutStringPtr(char* pszKey, void* pvPointer);
			BOOL			PutStringData(char* pszKey, void* pvObject, unsigned int uiDataSize);
			BOOL			PutStringInt(char* pszKey, int iIndex);
			BOOL			PutStringLong(char* pszKey, int64 lliIndex);
			BOOL			PutStringString(char* pszKey, char* pszData);
			BOOL			PutKeyData(void* pvKey, int iKeySize, void* pvObject, unsigned int uiDataSize);
			BOOL			PutKeyString(void* pvKey, int iKeySize, char* pszData);

			char*			GetLongString(int64 lliKey, char* pszDest);
			BOOL			GetStringData(char* pszKey, void* pvObject, unsigned int* puiDataSize = NULL);
			char*			GetStringString(char* pszKey, char* pszDest);
			int				GetStringInt(char* pszKey);
			int64			GetStringLong(char* pszKey);
			void*			GetStringPtr(char* pszKey);
			BOOL			GetKeyData(void* pvKey, int iKeySize, void* pvObject, unsigned int* puiDataSize);
			char*			GetKeyString(void* pvKey, int iKeySize, char* pszDest);

			BOOL			HasString(char* pszKey);

			BOOL			DeleteLong(int64 lliKey);
			BOOL			DeleteString(char* pszKey);

protected:
	virtual BOOL			Put(void* pvKey, int iKeySize, void* pvObject, unsigned int uiDataSize) =0;
	virtual BOOL			Get(void* pvKey, int iKeySize, void* pvObject, unsigned int* puiDataSize) =0;
	virtual unsigned int	DataSize(void* pvKey, int iKeySize) =0;
	virtual BOOL			Remove(void* pvKey, int iKeySize) =0;
};


#endif // __INDEX_TREE_ACCESS_H__


