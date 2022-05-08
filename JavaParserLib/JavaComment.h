#ifndef __JAVA_COMMENT_H__
#define __JAVA_COMMENT_H__
#include "BaseLib/Chars.h"
#include "JavaToken.h"


class CJavaComment : public CJavaToken
{
CONSTRUCTABLE(CJavaComment);
protected:
	char*	mszComment;
	int		miLength;

public:
	void 	Init(char* szComment, int iLength);
	void 	Kill(void);

	void	Print(CChars* pszDest);
	char*	GetType(void);
	BOOL	Is(char* szComment);
	void	GetComment(CChars* psz);

	BOOL	IsComment(void);
};


#endif // !__JAVA_COMMENT_H__

