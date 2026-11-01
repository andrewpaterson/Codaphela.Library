#ifndef __FILE_SYSTEM_GENERAL_H__
#define __FILE_SYSTEM_GENERAL_H__
#include "FileNames.h"
#include "FileNodeFileSystem.h"
#include "ArrayTemplatePtr.h"


typedef CFileNode<CFileNodeSystemFile>			CSystemFileNode;
typedef CFileNodeDirectory<CFileNodeSystemFile>	CSystemDirectoryNode;
typedef CFilenames<CFileNodeSystemFile>			CSystemFilenames;
typedef CArrayTemplatePtr<CSystemFileNode>		CArraySystemFilePtr;


#endif // __FILE_SYSTEM_GENERAL_H__

