#ifndef __OBJECT_SOURCE_CHUNKED_H__
#define __OBJECT_SOURCE_CHUNKED_H__
#include "BaseLib/ChunkFileFileSystem.h"
#include "ObjectSource.h"


class CExternalObjectReader;
class CChunkFileObjectReader;
class CObjectSourceChunked : public CObjectSource
{
CONSTRUCTABLE(CObjectSourceChunked);
protected:
	CArrayChars					mcNames;
	CChunkFile					mcChunkFile;
	CChunkFileFileSystem				mcChunkFileFileSystem;
	CChunkFileObjectReader*		mpcReader;

public:
	bool			Init(CObjectConverter* pcConverter, CAbstractFile* pcFile, char* szFilename);
	void			Kill(void);

	bool			ReadNames(void);
	bool			Contains(char* szFullName);
	CBaseObject*	Convert(char* szFullName);
	bool			IsChunked(void);
	bool			IsNative(void);
	bool			IsMultiSource(void);

	char*			GetName(int iIndex);
	int				NumNames(void);

	CExternalObjectReader*	GetReader(void);
};


#endif // __OBJECT_SOURCE_CHUNKED_H__

