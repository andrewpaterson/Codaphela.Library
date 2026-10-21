#ifndef __EXCELLON_INTERPRETER_LISTENER_H__
#define __EXCELLON_INTERPRETER_LISTENER_H__
#include "GerberNumbers.h"
#include "ExcellonCommandToolDefine.h"


class CExcellonInterpreterListener
{
public:
	//A round hole drilled with pcTool at psPoint, in the unit of the file.
	virtual void	Hole(CExcellonCommandToolDefine* pcTool, SGerberPoint* psPoint);

	//A straight cut by pcTool from psStart to psEnd; a routed segment with the tool down or a canned slot (G85).
	virtual void	Slot(CExcellonCommandToolDefine* pcTool, SGerberPoint* psStart, SGerberPoint* psEnd);
};


#endif // __EXCELLON_INTERPRETER_LISTENER_H__

