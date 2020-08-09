#ifndef __INDEX_TREE_CONFIG_H__
#define __INDEX_TREE_CONFIG_H__
#include "Mallocator.h"
#include "IndexKeyReverse.h"
#include "AccessDataOrderer.h"


class CIndexTreeConfig
{
public:
protected:
	CMallocator*			mpcMalloc;
	EIndexKeyReverse		meKeyReverse;
	int						miMaxDataSize;
	int						miMaxKeySize;
	CIndexTreeDataOrderer*	mpcDataOrderer;

	void				Init(CMallocator* pcMalloc, EIndexKeyReverse eKeyReverse, int iMaxDataSize, int iMaxKeySize, CIndexTreeDataOrderer* pcDataOrderer);
	void				Init(CFileReader* pcFileReader);

protected:
	EIndexKeyReverse	ReadKeyReverse(CFileReader* pcFileReader);
	BOOL				WriteKeyReverse(CFileWriter* pcFileWriter, EIndexKeyReverse	eKeyReverse);

	CAccessDataOrderer*	ReadDataOrderer(CFileReader* pcFileReader);
	BOOL				WriteDataOrderer(CFileWriter* pcFileWriter, CAccessDataOrderer*);
};


#endif // __INDEX_TREE_CONFIG_H__

