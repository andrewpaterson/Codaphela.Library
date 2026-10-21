#ifndef __EXCELLON_COMMAND_CODE_H__
#define __EXCELLON_COMMAND_CODE_H__
#include "ExcellonCommand.h"


//A command without parameters; M48, %, G90, G05 and M30.
class CExcellonCommandCode : public CExcellonCommand
{
public:
	void	Init(EExcellonCommandEnum eType);
	void	Kill(void);
};


#endif // __EXCELLON_COMMAND_CODE_H__

