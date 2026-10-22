#ifndef __EXCELLON_COMMAND_TOOL_DEFINE_H__
#define __EXCELLON_COMMAND_TOOL_DEFINE_H__
#include "BaseLib/Number.h"
#include "ExcellonCommand.h"


class CExcellonCommandToolDefine : public CExcellonCommand
{
protected:
	uint32		muiTool;
	CNumber		mcDiameter;

public:
	void		Init(uint32 uiTool, CNumber* pcDiameter);
	void		Kill(void);

	uint32		GetTool(void);
	CNumber*	GetDiameter(void);
};


#endif // __EXCELLON_COMMAND_TOOL_DEFINE_H__

