#include "JavaComment.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaComment::Init(char* szComment, int iLength)
{
	CJavaToken::Init();
	mszComment = szComment;
	miLength = iLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaComment::Kill(void)
{
	CJavaToken::Kill();
}

