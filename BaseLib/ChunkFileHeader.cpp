#include "ChunkFileHeader.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CChunkFileHeader::WriteInit(int iUserID)
{
	memset(this, 0xEE, sizeof(CChunkFileHeader));
	miUserID = iUserID;
	miChunkNamesPos = -1;
	memset(macMD5Hash, 0, 16);
	miMagic = CHUNK_HEADER_MAGIC;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CChunkHeader::WriteInit(void)
{
	memset(this, 0xEE, sizeof(CChunkFileHeader));
	miChunkIndexPos = -1;
	miChunkSize = 0;
	miName = -1;
	memset(macMD5Hash, 0, 16);
	miMagic = CHUNK_HEADER_MAGIC;
}
