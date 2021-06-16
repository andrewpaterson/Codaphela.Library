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
	BOOL			Init(CObjectConverter* pcConverter, CAbstractFile* pcFile, char* szFileName);
	void			Kill(void);

	BOOL			ReadNames(void);
	BOOL			Contains(char* szFullName);
	CBaseObject*	Convert(char* szFullName);
	BOOL			IsChunked(void);
	BOOL			IsNative(void);
	BOOL			IsMultiSource(void);

	char*			GetName(int iIndex);
	int				NumNames(void);

	CExternalObjectReader*	GetReader(void);
};


#endif // __OBJECT_SOURCE_CHUNKED_H__

