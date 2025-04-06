#ifndef __GERBER_COMMAND_END_H__
#define __GERBER_COMMAND_END_H__
#include "BaseLib/Chars.h"
#include "GerberCommand.h"


class CGerberCommandEndOfFile : public CGerberCommand
{
public:
	void	Init(void);
	void	Kill(void);

	bool	IsEndOfFile(void);
};


#endif // __GERBER_COMMAND_END_H__


