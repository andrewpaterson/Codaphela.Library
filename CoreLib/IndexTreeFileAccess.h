#ifndef __INDEX_TREE_FILE_ACCESS_H__
#define __INDEX_TREE_FILE_ACCESS_H__
#include "IndexTreeFile.h"
#include "IndexTreeAccess.h"


class CIndexTreeFileAccess : public CIndexTreeAccess
{
private:
	CIndexTreeFile*	mpcTree;

public:
	void	Init(CIndexTreeFile* pcTree);
	void	Kill(void);

protected:
	BOOL	Put(void* pvKey, int iKeySize, void* pvObject, unsigned char uiDataSize);
	void*	Get(void* pvKey, int iKeySize);
	BOOL	Remove(void* pvKey, int iKeySize);
};


#endif // __INDEX_TREE_FILE_ACCESS_H__


