#ifndef __CHUNK_FILE_HEADER_H__
#define __CHUNK_FILE_HEADER_H__
#include "AbstractFile.h"
#include "ChunkFileName.h"


#define CHUNK_HEADER_MAGIC 0xb4d85f9a


//The headers and the chunk index (SChunkIndex) are written as they are in memory, so their padding is explicit: they are
//laid out the same by every compiler (as 64 bit builds and Visual Studio always laid them out), on the W65C816 too.
struct SChunkFileHeader
{
	chunkName	miUserID;
	uint32		muiPadding;
	filePos		miChunkNamesPos;
	char		macMD5Hash[16];  //0's if not hashed.
	uint32		miMagic;
	uint32		muiEndPadding;

	void		WriteInit(chunkName iUserID);
};


class CChunkHeader
{
public:
	chunkName	miName;
	uint32		muiPadding;
	filePos		miChunkSize;
	filePos		miChunkIndexPos;  // -1 if no index.
	char		macMD5Hash[16];  //0's if not hashed.
	uint32		miMagic;
	uint32		muiEndPadding;

	void WriteInit(void);
};


#endif // __CHUNK_FILE_HEADER_H__
