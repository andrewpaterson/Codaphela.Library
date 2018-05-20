#ifndef __INDEX_DESCRIPTORS_FILE_H__
#define __INDEX_DESCRIPTORS_FILE_H__
#include "BaseLib/PrimitiveTypes.h"
#include "IndexTreeTemplateFile.h"
#include "IndexedDataDescriptor.h"


class CIndexedDescriptorsFile
{
protected:
	CIndexTreeTemplateFile<CIndexedDataDescriptor>	mcIndexTree;

public:
	void	Init(CDurableFileController* pcDurableFileController, BOOL bDirtyTesting);
	void	Kill(void);
	BOOL	Remove(OIndex oi);
	BOOL	Get(CIndexedDataDescriptor* pcDescriptor, OIndex oi);
	BOOL	Set(CIndexedDataDescriptor* pcDescriptor, OIndex oi);
	int64	NumElements(void);
	int		NumCachedDatas(void);
};


#endif // __INDEX_DESCRIPTORS_FILE_H__

