#ifndef __CHUNK_FILE_HEADER_H__
#define __CHUNK_FILE_HEADER_H__
#include "AbstractFile.h"
#include "ChunkFileName.h"


#define CHUNK_HEADER_MAGIC 0xb4d85f9a


struct SChunkFileHeader
{
	chunkName	miUserID;
	filePos		miChunkNamesPos;
	char		macMD5Hash[16];  //0's if not hashed.
	uint32		miMagic;

	void		WriteInit(uint32 iUserID);
};


class CChunkHeader
{
public:
	chunkName	miName;
	filePos		miChunkSize;
	filePos		miChunkIndexPos;  // -1 if no index.
	char		macMD5Hash[16];  //0's if not hashed.
	uint32		miMagic;

	void WriteInit(void);
};


#endif // __CHUNK_FILE_HEADER_H__
