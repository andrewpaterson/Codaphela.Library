#ifndef __INDEX_TREE_CONFIG_H__
#define __INDEX_TREE_CONFIG_H__
#include "Mallocator.h"
#include "IndexKeyReverse.h"
#include "AccessDataOrderer.h"


class CIndexTreeConfig
{
protected:
	CMallocator*			mpcMalloc;
	EIndexKeyReverse		meKeyReverse;
	int						miMaxDataSize;
	int						miMaxKeySize;
	CIndexTreeDataOrderer*	mpcDataOrderer;

	void					Init(CMallocator* pcMalloc, EIndexKeyReverse eKeyReverse, int iMaxDataSize, int iMaxKeySize, CIndexTreeDataOrderer* pcDataOrderer);
	BOOL					Init(CFileReader* pcFileReader);

	BOOL					Write(CFileWriter* pcFileWrite);

protected:
	BOOL					Read(CFileReader* pcFileReader);

	EIndexKeyReverse		ReadKeyReverse(CFileReader* pcFileReader);
	BOOL					WriteKeyReverse(CFileWriter* pcFileWriter, EIndexKeyReverse	eKeyReverse);

	CIndexTreeDataOrderer*	ReadDataOrderer(CFileReader* pcFileReader);
	BOOL					WriteDataOrderer(CFileWriter* pcFileWriter, CIndexTreeDataOrderer*);
};


#endif // __INDEX_TREE_CONFIG_H__

