#ifndef __FILE_ITERATOR_RETURN_H__
#define __FILE_ITERATOR_RETURN_H__
#include "BaseLib/FileSystemIterator.h"
#include "BaseLib/PackFileIterator.h"


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
	int							miFileRank;

public:
	void						Init(void);
	void						Init(EFileIteratorReturnType eType, CBaseFileNode* pcNode, int iFileRank);
	void						Kill(void);

	void						InsertInName(int iIndex, char* szText);

	EFileIteratorReturnType		GetType(void);
	char*						GetFullName(void);
	CBaseFileNode*				GetNode(void);
	int							GetFileRank(void);
};


#endif // !__FILE_ITERATOR_RETURN_H__

