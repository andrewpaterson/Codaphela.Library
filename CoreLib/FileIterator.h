#ifndef __FILE_ITERATOR_H__
#define __FILE_ITERATOR_H__
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

	EFileIteratorReturnType		GetType(void);
	char*						GetFullName(void);
	CBaseFileNode*				GetNode(void);
	int							GetFileRank(void);
};


//This iterator gives back every file, including duplicates.
class CFileIterator
{
public:
	int						miPackFileIndex;
	CPackFileIterator		mcPackFileIterator;
	CFileSystemIterator		mcFileSystemIterator;
	BOOL					mbMoveOn;
	BOOL					mbFileSystem;
	
	CFileIteratorReturn		mcCurrent;
	BOOL					mbHasCurrent;

	void					Init(void);
	void					Kill(void);

	CFileIteratorReturn*	GetCurrent(void);
	CFileIteratorReturn*	SetCurrent(EFileIteratorReturnType eType, CBaseFileNode* pcNode, int iFileRank);
	CFileIteratorReturn*	SetCurrent(void);
};


#endif // __FILE_ITERATOR_H__

