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
};


#endif // !__JAVA_COMMENT_H__

