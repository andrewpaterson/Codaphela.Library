#ifndef __INDEXED_DATA_FILES_H__
#define __INDEXED_DATA_FILES_H__
#include "IndexedFiles.h"


class CIndexedDataFiles : public CIndexedFiles
{
public:
	void						Init(CDurableFileController* pcDurableFileControl, char* szExtension);
	void						Kill(void);

	BOOL						WriteNew(CIndexDescriptor* pcDescriptor, void* pvData);
	BOOL						WriteExisting(CIndexDescriptor* pcDescriptor, void* pvData);
	BOOL						Read(CIndexDescriptor* pcDescriptor, void* pvData);
};


#endif // __INDEXED_DATA_FILES_H__

