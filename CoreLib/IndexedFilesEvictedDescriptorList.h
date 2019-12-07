#ifndef __INDEXED_FILES_EVICTED_DESCRIPTOR_LIST_H__
#define __INDEXED_FILES_EVICTED_DESCRIPTOR_LIST_H__
#include "BaseLib/ListVariable.h"
#include "BaseLib/MapLongTemplate.h"
#include "IndexedFilesEvictionCallback.h"
#include "IndexedDataCommon.h"
#include "IndexedFilesEvicting.h"


class CIndexedFilesEvictedDescriptorList : public CIndexedDataCommon, public CIndexedFilesEvictionCallback
{
protected:
	CMapLongTemplate<CIndexedDataDescriptor>	mcDescriptors;
	BOOL										mbDescriptorsWritten;
	CDurableFile								mcDescriptorsFile;

public:
	void	Init(CDurableFileController* pcDurableFileControl, char* szDataExtension, char* szDescricptorName, char* szDescricptorRewrite, size_t iCacheSize, BOOL bWriteThrough);
	void	Init(CDurableFileController* pcDurableFileControl, char* szDataExtension, char* szDescricptorName, char* szDescricptorRewrite, size_t iCacheSize, BOOL bWriteThrough, CEvictionCallback* pcEvictionUserCallback);
	BOOL	Kill(void);

	BOOL	DescriptorsEvicted(CArrayVoidPtr* papsEvictedIndexedCacheDescriptors);
	BOOL	Flush(BOOL bClearCache);

	int64	NumElements(void);
	BOOL	IsDirty(OIndex oi);

	BOOL	EvictData(OIndex oi, CIndexedDataDescriptor* pcDescriptor);

	BOOL	TestGetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor);

protected:
	void	InitIndices(CDurableFileController* pcDurableFileControl, BOOL bDirtyTesting);
	void	NullCachedDescriptors(void);

	BOOL	GetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, BOOL bNoEviction = FALSE);
	BOOL	SetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, BOOL bNoEviction = FALSE);
	BOOL	UpdateDescriptorCache(OIndex oi, void* pvCache, unsigned int uiDataSize);
	BOOL	RemoveDescriptor(OIndex oi);
};


#endif // __INDEXED_FILES_EVICTED_DESCRIPTOR_LIST_H__

