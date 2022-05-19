#ifndef __FILE_SYSTEM_GENERAL_H__
#define __FILE_SYSTEM_GENERAL_H__
#include "Filenames.h"
#include "FileNodeFileSystem.h"


typedef CFileNode<CFileNodeSystemFile>			CSystemFileNode;
typedef CFileNodeDirectory<CFileNodeSystemFile>	CSystemDirectoryNode;
typedef CFilenames<CFileNodeSystemFile>			CSystemFilenames;
typedef CArrayTemplate<CSystemFileNode*>		CArraySystemFilePtrs;


#endif // !__FILE_SYSTEM_GENERAL_H__

