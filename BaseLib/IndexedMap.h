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
	void	Kill(void);

	bool	CanKill(void);
	bool	DescriptorEvicted(OIndex oi, void* pvCache, lsize uiDataSize);
	bool	Flush(bool bClearCache);

	int64	NumIndices(void);
	size	NumIndicesCached(void);
	bool	IsDirty(OIndex oi);

	bool	EvictData(OIndex oi, CIndexedDataDescriptor* pcDescriptor);

	bool	TestGetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor);

protected:
	void	InitIndices(CDurableFileController* pcDurableFileControl, bool bDirtyTesting);
	void	NullCachedDescriptors(void);

	bool	GetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, bool bNoEviction = false);
	bool	SetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, bool bNoEviction = false);
	bool	UpdateDescriptorCache(OIndex oi, void* pvCache, lsize uiDataSize);
	bool	RemoveDescriptor(OIndex oi);
};


#endif // __INDEXED_FILES_EVICTED_DESCRIPTOR_LIST_H__

