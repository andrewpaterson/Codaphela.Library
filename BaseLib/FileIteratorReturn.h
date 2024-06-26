#ifndef __FILE_ITERATOR_RETURN_H__
#define __FILE_ITERATOR_RETURN_H__
#include "FileSystemIterator.h"
#include "PackFileIterator.h"


enum EFileIteratorReturnType
{
	FIRT_Unknown,
	FIRT_FileSystem,
	FIRT_PackFiles,
};


#define FILE_SYSTEM_RANK 8192


class CFileIteratorReturn
{
protected:
	EFileIteratorReturnType		meType;
	CChars						mszFullName;
	CBaseFileNode*				mpsNode;
	uint32						miFileRank;

public:
	void						Init(void);
	void						Init(EFileIteratorReturnType eType, CBaseFileNode* pcNode, uint32 iFileRank);
	void						Kill(void);

	void						InsertInName(size iIndex, char* szText);

	EFileIteratorReturnType		GetType(void);
	char*						GetFullName(void);
	CBaseFileNode*				GetNode(void);
	uint32						GetFileRank(void);
};


#endif // __FILE_ITERATOR_RETURN_H__

