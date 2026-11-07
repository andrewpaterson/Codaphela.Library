#ifndef __OBJECT_SOURCE_CHUNKED_H__
#define __OBJECT_SOURCE_CHUNKED_H__
#include "BaseLib/ChunkFileFileSystem.h"
#include "ObjectSource.h"


class CExternalObjectReader;
class CChunkFileObjectReader;
class CObjectSourceChunked : public CObjectSource
{
CONSTRUCTABLE(CObjectSourceChunked)
protected:
	CArrayChars					mcNames;
	CChunkFile					mcChunkFile;
	CChunkFileFileSystem		mcChunkFileFileSystem;
	CChunkFileObjectReader*		mpcReader;

public:
	bool			Init(CObjectConverter* pcConverter, CAbstractFile* pcFile, const char* szFilename);
	void			Kill(void) override;

	bool			ReadNames(void);
	bool			Contains(const char* szFullName) override;
	CBaseObject*	Convert(const char* szFullName) override;
	bool			IsNative(void) override;
	bool			IsMultiSource(void) override;

	char*			GetName(size iIndex);
	size			NumNames(void);

	CExternalObjectReader*	GetReader(void) override;
};


#endif // __OBJECT_SOURCE_CHUNKED_H__

