#ifndef __INDEX_DESCRIPTORS_FILE_H__
#define __INDEX_DESCRIPTORS_FILE_H__
#include "IndexTreeTemplateFile.h"
#include "IndexedDataDescriptor.h"


class CIndexedDescriptorsFile
{
protected:
	CIndexTreeTemplateFile<CIndexedDataDescriptor>	mcIndexTree;

public:
	void	Init(CDurableFileController* pcDurableFileController, char* szIndexFileName, char* szIndexBackupName, BOOL bDirtyTesting);
	void	Kill(void);
	BOOL	Save(void);
	BOOL	RemoveFile(void);
	BOOL	Remove(OIndex oi);
	BOOL	Get(CIndexedDataDescriptor* pcDescriptor, OIndex oi);
	BOOL	Set(CIndexedDataDescriptor* pcDescriptor);
	int64	NumElements(void);
	int		NumCachedDatas(void);
};


#endif // __INDEX_DESCRIPTORS_FILE_H__

