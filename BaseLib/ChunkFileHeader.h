#ifndef __CHUNK_FILE_HEADER_H__
#define __CHUNK_FILE_HEADER_H__
#include "AbstractFile.h"


#define CHUNK_HEADER_MAGIC 0xb4d85f9a


struct SChunkFileHeader
{
	int			miUserID;
	filePos		miChunkNamesPos;
	char		macMD5Hash[16];  //0's if not hashed.
	int			miMagic;

	void		WriteInit(int iUserID);
};


class CChunkHeader
{
public:
	int		miName;
	filePos	miChunkSize;
	filePos	miChunkIndexPos;  // -1 if no index.
	char	macMD5Hash[16];  //0's if not hashed.
	int		miMagic;

	void WriteInit(void);
};


#endif // !__CHUNK_FILE_HEADER_H__
