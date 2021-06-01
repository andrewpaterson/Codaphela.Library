#include "ChunkFileHeader.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SChunkFileHeader::WriteInit(int iUserID)
{
	memset(this, 0xEE, sizeof(SChunkFileHeader));
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
	memset(this, 0xEE, sizeof(SChunkFileHeader));
	miChunkIndexPos = -1;
	miChunkSize = 0;
	miName = -1;
	memset(macMD5Hash, 0, 16);
	miMagic = CHUNK_HEADER_MAGIC;
}
