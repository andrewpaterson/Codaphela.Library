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
	CListVariable								mcEvicted;

	CMapLongTemplate<CIndexedDataDescriptor>	mcDescriptors;
	BOOL										mbDescriptorsWritten;
	CDurableFile								mcDescriptorsFile;

public:
	void	Init(CDurableFileController* pcDurableFileControl, char* szDataExtension, char* szDescricptorName, char* szDescricptorRewrite, size_t iCacheSize, BOOL bWriteThrough);
	BOOL	Kill(void);

	int		NumEvicted(void);
	void*	GetEvicted(int iIndex);
	void	ClearEvicted(void);

	BOOL	DescriptorsEvicted(CArrayVoidPtr* papsEvictedIndexedCacheDescriptors);
	BOOL	Flush(BOOL bClearCache);

	int64	NumElements(void);
	BOOL	IsDirty(OIndex oi);

	BOOL	KeyEvicted(OIndex oi, CIndexedDataDescriptor* pcDescriptor);

	BOOL	TestGetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor);

protected:
	void	InitIndices(CDurableFileController* pcDurableFileControl, BOOL bDirtyTesting);
	void	NullCachedDescriptors(void);

	BOOL	GetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor);
	BOOL	SetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor);
	BOOL	UpdateDescriptorCache(OIndex oi, void* pvCache);
	BOOL	RemoveDescriptor(OIndex oi);
};


#endif // __INDEXED_FILES_EVICTED_DESCRIPTOR_LIST_H__

