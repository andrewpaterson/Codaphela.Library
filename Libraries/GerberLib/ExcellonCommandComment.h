#ifndef __EXCELLON_COMMAND_COMMENT_H__
#define __EXCELLON_COMMAND_COMMENT_H__
#include "BaseLib/Chars.h"
#include "ExcellonCommand.h"


class CExcellonCommandComment : public CExcellonCommand
{
protected:
	CChars	mszComment;

public:
	void	Init(const char* szComment, size uiLength);
	void	Kill(void);

	char*	Text(void);
};


#endif // __EXCELLON_COMMAND_COMMENT_H__

