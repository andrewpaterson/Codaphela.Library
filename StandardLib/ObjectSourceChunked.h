#ifndef __OBJECT_SOURCE_CHUNKED_H__
#define __OBJECT_SOURCE_CHUNKED_H__
#include "ChunkFileNames.h"
#include "ObjectSource.h"


class CObjectReader;
class CObjectReaderChunkFile;
class CObjectSourceChunked : public CObjectSource
{
CONSTRUCTABLE(CObjectSourceChunked);
protected:
	CArrayChars				mcNames;
	CChunkFileNames				mcChunkFile;
	CObjectReaderChunkFile*		mpcReader;

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

	CObjectReader*	GetReader(void);
};


#endif // __OBJECT_SOURCE_CHUNKED_H__

