#ifndef __JAVA_COMMENT_H__
#define __JAVA_COMMENT_H__
#include "BaseLib/Chars.h"
#include "JavaToken.h"


class CJavaTokenComment : public CJavaToken
{
CONSTRUCTABLE(CJavaTokenComment);
protected:
	char*	mszComment;
	int		miLength;

public:
	void 	Init(STextPosition* psPosition, char* szComment, int iLength);
	void 	Kill(void);

	void	Print(CChars* pszDest);
	char*	GetType(void);
	bool	Is(char* szComment);
	void	GetComment(CChars* psz);

	bool	IsComment(void);
};


#endif // !__JAVA_COMMENT_H__

