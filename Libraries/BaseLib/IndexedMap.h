#ifndef __INDEXED_FILES_EVICTED_DESCRIPTOR_LIST_H__
#define __INDEXED_FILES_EVICTED_DESCRIPTOR_LIST_H__
#include "ListVariable.h"
#include "MapLongTemplate.h"
#include "IndexedFilesEvictionCallback.h"
#include "IndexedDataCommon.h"
#include "IndexedFilesEvicting.h"


class CIndexedMap : public CIndexedDataCommon, public CIndexedFilesEvictionCallback
{
protected:
	CMapLongTemplate<CIndexedDataDescriptor>	mcDescriptors;
	bool										mbDescriptorsWritten;
	CDurableFile								mcDescriptorsFile;

public:
	void	Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, const char* szDataExtension, const char* szDescriptorName, const char* szDescriptorRewrite, lsize iCacheSize, EIndexWriteThrough eWriteThrough);
	void	Init(CDurableFileController* pcDurableFileControl, const char* szSubDirectory, const char* szDataExtension, const char* szDescriptorName, const char* szDescriptorRewrite, lsize iCacheSize, EIndexWriteThrough eWriteThrough, CIndexedDataEvictionCallback* pcEvictionUserCallback);
	void	Kill(void) override;

	bool	CanKill(void);
	bool	DescriptorEvicted(OIndex oi, void* pvCache, lsize uiDataSize) override;
	bool	Flush(bool bClearCache) override;

	int64	NumIndices(void) override;
	lsize	NumIndicesCached(void) override;
	bool	IsDirty(OIndex oi) override;

	bool	EvictData(OIndex oi, CIndexedDataDescriptor* pcDescriptor) override;

	bool	TestGetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor);

protected:
	void	InitIndices(CDurableFileController* pcDurableFileControl, bool bDirtyTesting);
	void	NullCachedDescriptors(void);

	bool	GetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, bool bNoEviction = false) override;
	bool	SetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, bool bNoEviction = false) override;
	bool	UpdateDescriptorCache(OIndex oi, void* pvCache, lsize uiDataSize) override;
	bool	RemoveDescriptor(OIndex oi) override;
};


#endif // __INDEXED_FILES_EVICTED_DESCRIPTOR_LIST_H__

