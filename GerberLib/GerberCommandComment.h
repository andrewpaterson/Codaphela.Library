#ifndef __GERBER_COMMAND_COMMENT_H__
#define __GERBER_COMMAND_COMMENT_H__
#include "BaseLib/Chars.h"
#include "GerberCommand.h"


class CGerberCommandComment : public CGerberCommand
{
protected:
	CChars	szComment;

public:
	void	Init(size iCommentLength);
	void	Kill(void);

	bool	IsComment(void);

	char*	Text(void);
};


#endif // __GERBER_COMMAND_H__


