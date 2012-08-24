#ifndef __OBJECT_READER_CHUNKED_H__
#define __OBJECT_READER_CHUNKED_H__
#include "ChunkFileNames.h"
#include "ObjectReader.h"


class CObjectReaderChunked : public CObjectReader
{
BASE_FUNCTIONS(CObjectReaderChunked);
protected:
	CChunkFileNames		mcChunkFile;
	CChars				mszFileName;

public:
	void				Init(char* szDirectory, char* szChunkFileName);
	void				Kill(void);

	BOOL				Begin(void);
	CSerialisedObject*  Read(char* szChunkName);
	BOOL				End(void);
};


#endif // __OBJECT_READER_CHUNKED_H__

