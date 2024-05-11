#ifndef __INDEX_TREE_CONFIG_H__
#define __INDEX_TREE_CONFIG_H__
#include "Mallocator.h"
#include "IndexKeyReverse.h"
#include "AccessDataOrderer.h"


class CIndexTreeConfig
{
protected:
	CLifeInit<CMallocator>				mcMalloc;
	CLifeInit<CIndexTreeDataOrderer>	mcDataOrderer;
	size								miMaxDataSize;
	size								miMaxKeySize;
	EIndexKeyReverse					meKeyReverse;

public:
			void								Init(CLifeInit<CMallocator> cMalloc, EIndexKeyReverse eKeyReverse, size iMaxDataSize, size iMaxKeySize, CLifeInit<CIndexTreeDataOrderer> cDataOrderer);
			bool								Init(CFileReader* pcFileReader);
	virtual void								Kill(void) =0;

	virtual bool								Write(CFileWriter* pcFileWrite);

			CLifeInit<CMallocator>				GetMalloc(void);
			EIndexKeyReverse					GetKeyReverse(void);
			int									GetMaxDataSize(void);
			int									GetMaxKeySize(void);
			CLifeInit<CIndexTreeDataOrderer>	GetDataOrderer(void);

protected:
	virtual bool					Read(CFileReader* pcFileReader);

			EIndexKeyReverse		ReadKeyReverse(CFileReader* pcFileReader);
			bool					WriteKeyReverse(CFileWriter* pcFileWriter, EIndexKeyReverse	eKeyReverse);

			CIndexTreeDataOrderer*	ReadDataOrderer(CFileReader* pcFileReader);
			bool					WriteDataOrderer(CFileWriter* pcFileWriter, CIndexTreeDataOrderer*);
};


#endif // __INDEX_TREE_CONFIG_H__

