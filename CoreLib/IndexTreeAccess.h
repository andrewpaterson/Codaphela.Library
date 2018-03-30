#ifndef __INDEX_TREE_ACCESS_H__
#define __INDEX_TREE_ACCESS_H__
#include "BaseLib/Define.h"
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/TemporaryMemory.h"

class CIndexTreeAccess
{
private:
	CTemporaryMemory		mcTemp;

public:
			void	Init(void);
	virtual void	Kill(void);
			BOOL	PutStringPtr(char* pszKey, void* pvPointer);
			BOOL	PutStringLong(char* pszKey, int64 lliIndex);
			BOOL	PutStringData(char* pszKey, void* pvObject, int iDataSize);
			BOOL	PutLongPtr(int64 lliKey, void* pvPointer);
			BOOL	PutLongString(int64 lliKey, char* pszData);
			BOOL	PutLongData(int64 lliKey, void* pvObject, int iDataSize);

			BOOL	GetLongString(int64 lliKey, char* pszDest);

			BOOL	RemoveLong(int64 lliKey);

protected:
	virtual BOOL	Put(void* pvKey, int iKeySize, void* pvObject, unsigned char uiDataSize) =0;
			BOOL	Put(void* pvKey, int iKeySize, unsigned char uiDataSize);

	virtual void*	Get(void* pvKey, int iKeySize) =0;

	virtual BOOL	Remove(void* pvKey, int iKeySize) =0;
};


#endif // __INDEX_TREE_ACCESS_H__


