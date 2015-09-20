#ifndef __PACK_FILES_GENERAL_H__
#define __PACK_FILES_GENERAL_H__
#include "FileNames.h"
#include "PackFileReadArrary.h"
#include "PackFileNode.h"


enum EPackFileMode
{
	PFM_Read,
	PFM_Write,
};


typedef CFileNode<CPackFileNode>				CFileNodePackFileNode;
typedef CFileNames<CPackFileNode>				CFileNamesPackFileNode;
typedef CArrayTemplate<CFileNodePackFileNode*>	CArrayPackFileNodePtrs;


#endif // __PACK_FILES_GENERAL_H__

