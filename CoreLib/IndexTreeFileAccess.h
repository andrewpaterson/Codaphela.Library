#ifndef __INDEX_TREE_FILE_ACCESS_H__
#define __INDEX_TREE_FILE_ACCESS_H__
#include "BaseLib/IndexTreeAccess.h"
#include "IndexTreeFile.h"


class CIndexTreeFileAccess : public CIndexTreeAccess
{
private:
	CIndexTreeFile*		mpcTree;

public:
	BOOL			Init(CIndexTreeFile* pcTree);
	BOOL			Kill(void);
	BOOL			Flush(void);

protected:
	BOOL			Put(void* pvKey, int iKeySize, void* pvObject, unsigned int uiDataSize);
	BOOL			Get(void* pvKey, int iKeySize, void* pvObject, unsigned int* puiDataSize);
	BOOL			Remove(void* pvKey, int iKeySize);
	unsigned int	DataSize(void* pvKey, int iKeySize);
};


#endif // __INDEX_TREE_FILE_ACCESS_H__


