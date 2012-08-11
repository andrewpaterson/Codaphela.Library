#ifndef __FILE_SYSTEM_GENERAL_H__
#define __FILE_SYSTEM_GENERAL_H__
#include "FileNames.h"
#include "FileNodeFileSystem.h"


typedef CFileNode<CFileNodeSystemFile>			CSystemFileNode;
typedef CFileNodeDirectory<CFileNodeSystemFile>	CSystemDirectoryNode;
typedef CFileNames<CFileNodeSystemFile>			CSystemFileNames;
typedef CArrayTemplate<CSystemFileNode*>		CArraySystemFilePtrs;


#endif // __FILE_SYSTEM_GENERAL_H__

