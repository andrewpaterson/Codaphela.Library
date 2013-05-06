#ifndef __BASE_FILE_NODE_H__
#define __BASE_FILE_NODE_H__
#include "AbstractFile.h"
#include "Chars.h"


enum EFileNodeType
{
	FNT_Unknown,
	FNT_Directory,
	FNT_File,
};


class CBaseFileNode
{
protected:
	CChars			mszName;
	EFileNodeType	meType;
	CBaseFileNode*	mpcParent;

public:
	void					Init(char* szName, EFileNodeType eType, CBaseFileNode* pcParent);
	void					Kill(void);

	char*					GetName(void);
	char*					GetFullName(CChars* pszDest);
	BOOL					Is(char* szName);
	BOOL					IsDirectory(void);
	BOOL					IsFile(void);
	CBaseFileNode*			GetParent(void);
};


#endif // __BASE_FILE_NODE_H__
