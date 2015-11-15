#ifndef __INDEXED_DATA_FILES_H__
#define __INDEXED_DATA_FILES_H__
#include "IndexedFiles.h"


class CIndexedDataFiles : public CIndexedFiles
{
public:
	void						Init(CDurableFileController* pcDurableFileControl, char* szDataExtension, char* szDescricptorName, char* szDescricptorRewrite);
	void						Kill(void);

	BOOL						WriteData(CIndexedDataDescriptor* pcDescriptor, void* pvData);
	BOOL						WriteNew(CIndexedDataDescriptor* pcDescriptor, void* pvData);
	BOOL						WriteExisting(CIndexedDataDescriptor* pcDescriptor, void* pvData);
	BOOL						Read(CIndexedDataDescriptor* pcDescriptor, void* pvData);
};


#endif // __INDEXED_DATA_FILES_H__

