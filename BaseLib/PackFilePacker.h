#ifndef __PACK_FILE_PACKER_H__
#define __PACK_FILE_PACKER_H__
#include "Chars.h"


class CPackFiles;
class CPackFilePacker
{
public:
	BOOL Pack(char* szDestPakFile, char* szSourceDirectory);
	BOOL Unpack(char* szSourcePakFile, char* szDestDirectory);
	BOOL List(char* szSourcePakFile, CChars* pszDest);
	void List(CPackFiles* pcPackFiles, CChars* pszDest);
};


#endif // __PACK_FILE_PACKER_H__

