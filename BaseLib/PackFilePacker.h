#ifndef __PACK_FILE_PACKER_H__
#define __PACK_FILE_PACKER_H__
#include "Chars.h"


class CPackFiles;
class CPackFilePacker
{
public:
	bool Pack(char* szDestPakFile, char* szSourceDirectory, char* szPackDirectory = NULL);
	bool Unpack(char* szSourcePakFile, char* szDestDirectory, bool bRemoveDestDir = false);
	bool List(char* szSourcePakFile, CChars* pszDest);
	void List(CPackFiles* pcPackFiles, CChars* pszDest);
};


bool PackDirectory(char* szDestPakFile, char* szSourceDirectory, char* szPackDirectory);


#endif // !__PACK_FILE_PACKER_H__

