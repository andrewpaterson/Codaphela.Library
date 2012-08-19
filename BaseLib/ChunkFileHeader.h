#ifndef __CHUNK_FILE_HEADER_H__
#define __CHUNK_FILE_HEADER_H__
#include "AbstractFile.h"


#define CHUNK_HEADER_MAGIC 0xb4d85f9a


class CChunkFileHeader
{
public:
	int			iUserID;
	filePos		iChunkNamesPos;
	char		acMD5Hash[16];  //0's if not hashed.
	int			iMagic;

	void		WriteInit(int iUserID);
};


class CChunkHeader
{
public:
	int		iName;
	filePos	iChunkSize;
	filePos	iChunkIndexPos;  // -1 if no index.
	char	acMD5Hash[16];  //0's if not hashed.
	int		iMagic;

	void WriteInit(void);
};


#endif // __CHUNK_FILE_HEADER_H__
