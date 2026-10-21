#ifndef __EXCELLON_COMMAND_TOOL_SELECT_H__
#define __EXCELLON_COMMAND_TOOL_SELECT_H__
#include "ExcellonCommand.h"


class CExcellonCommandToolSelect : public CExcellonCommand
{
protected:
	uint32	muiTool;

public:
	void	Init(uint32 uiTool);
	void	Kill(void);

	uint32	GetTool(void);
};


#endif // __EXCELLON_COMMAND_TOOL_SELECT_H__

