#ifndef __PACK_FILES_GENERAL_H__
#define __PACK_FILES_GENERAL_H__
#include "FileNames.h"
#include "PackFileReadArrary.h"
#include "PackFileNode.h"
#include "ArrayTemplatePtr.h"


enum EPackFileMode
{
	PFM_Read,
	PFM_Write,
};


typedef CFileNode<CPackFileNode>				CFileNodePackFileNode;
typedef CFilenames<CPackFileNode>				CFilenamesPackFileNode;
typedef CArrayTemplatePtr<CFileNodePackFileNode>	CArrayPackFileNodePtrs;


#endif // __PACK_FILES_GENERAL_H__

