#ifndef __GERBER_COMMAND_OPERATION_H__
#define __GERBER_COMMAND_OPERATION_H__
#include "BaseLib/Number.h"
#include "GerberCommand.h"


//D01 (plot), D02 (move) and D03 (flash).  Coordinates are decimal values in the
//unit set by MO, already scaled by the FS format.  An omitted coordinate means
//the current point's coordinate is used.
class CGerberCommandOperation : public CGerberCommand
{
protected:
	CNumber		mcX;
	CNumber		mcY;
	CNumber		mcI;
	CNumber		mcJ;
	bool		mbX;
	bool		mbY;
	bool		mbI;
	bool		mbJ;

public:
	void		Init(EGerberCommandEnum eType);
	void		Kill(void);

	bool		IsOperation(void);
	bool		IsPlot(void);
	bool		IsMove(void);
	bool		IsFlash(void);

	void		SetX(CNumber* pcX);
	void		SetY(CNumber* pcY);
	void		SetI(CNumber* pcI);
	void		SetJ(CNumber* pcJ);

	bool		HasX(void);
	bool		HasY(void);
	bool		HasI(void);
	bool		HasJ(void);

	CNumber*	GetX(void);
	CNumber*	GetY(void);
	CNumber*	GetI(void);
	CNumber*	GetJ(void);
};


#endif // __GERBER_COMMAND_OPERATION_H__

