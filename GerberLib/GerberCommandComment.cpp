#include "GerberCommandComment.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberCommandComment::Init(size uiCommentLength)
{
	CGerberCommand::Init(GC_G04);
	szComment.InitLength(uiCommentLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberCommandComment::Kill(void)
{
	szComment.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CGerberCommandComment::Text(void)
{
	return szComment.Text();
}

