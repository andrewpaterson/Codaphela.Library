#ifndef __OBJECT_SOURCE_CHUNKED_H__
#define __OBJECT_SOURCE_CHUNKED_H__
#include "ChunkFileNames.h"
#include "ObjectMultipleSource.h"


class CObjectReader;
class CObjectReaderChunkFile;
class CObjectSourceChunked : public CObjectMultipleSource
{
BASE_FUNCTIONS(CObjectSourceChunked);
protected:
	CChunkFileNames				mcChunkFile;
	CObjectReaderChunkFile*		mpcReader;

public:
			BOOL				Init(CObjectConverter* pcConverter, CAbstractFile* pcFile, char* szFileName);
			void				Kill(void);

			BOOL				ReadNames(void);
			BOOL				Contains(char* szFullName);
	virtual CPointerObject		Convert(char* szFullName);
			BOOL				IsChunked(void);
			BOOL				IsNative(void);

			CObjectReader*		GetReader(void);
};


#endif // __OBJECT_SOURCE_CHUNKED_H__

