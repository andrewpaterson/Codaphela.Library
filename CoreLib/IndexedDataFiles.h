#ifndef __INDEXED_DATA_FILES_H__
#define __INDEXED_DATA_FILES_H__
#include "IndexedFiles.h"


class CIndexedDataFiles : public CIndexedFiles
{
public:
	void						Init(CDurableFileController* pcDurableFileControl, char* szExtension);
	void						Kill(void);

	BOOL						WriteNew(CIndexedDataDescriptor* pcDescriptor, void* pvData);
	BOOL						WriteExisting(CIndexedDataDescriptor* pcDescriptor, void* pvData);
	BOOL						Read(CIndexedDataDescriptor* pcDescriptor, void* pvData);
};


#endif // __INDEXED_DATA_FILES_H__

