#ifndef __EXCELLON_COMMAND_COORDINATE_H__
#define __EXCELLON_COMMAND_COORDINATE_H__
#include "BaseLib/Number.h"
#include "ExcellonCommand.h"


//A hit (XY), a route move (G00) or a linear route (G01).  An omitted coordinate is taken from the previous one.
class CExcellonCommandCoordinate : public CExcellonCommand
{
protected:
	CNumber		mcX;
	CNumber		mcY;
	bool		mbX;
	bool		mbY;

public:
	void		Init(EExcellonCommandEnum eType);
	void		Kill(void);

	void		SetX(CNumber* pcX);
	void		SetY(CNumber* pcY);

	bool		HasX(void);
	bool		HasY(void);
	CNumber*	GetX(void);
	CNumber*	GetY(void);
};


#endif // __EXCELLON_COMMAND_COORDINATE_H__

