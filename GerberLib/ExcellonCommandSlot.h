#ifndef __EXCELLON_COMMAND_SLOT_H__
#define __EXCELLON_COMMAND_SLOT_H__
#include "BaseLib/Number.h"
#include "GerberNumbers.h"
#include "ExcellonCommand.h"


//XxYyG85XxYy  A canned slot.  An omitted start coordinate is taken from the previous point and an omitted end coordinate from the start.
class CExcellonCommandSlot : public CExcellonCommand
{
protected:
	CNumber		mcStartX;
	CNumber		mcStartY;
	CNumber		mcEndX;
	CNumber		mcEndY;
	bool		mbStartX;
	bool		mbStartY;
	bool		mbEndX;
	bool		mbEndY;

public:
	void		Init(void);
	void		Kill(void);

	void		SetStartX(CNumber* pcX);
	void		SetStartY(CNumber* pcY);
	void		SetEndX(CNumber* pcX);
	void		SetEndY(CNumber* pcY);

	bool		HasStartX(void);
	bool		HasStartY(void);
	bool		HasEndX(void);
	bool		HasEndY(void);
	CNumber*	GetStartX(void);
	CNumber*	GetStartY(void);
	CNumber*	GetEndX(void);
	CNumber*	GetEndY(void);
};


#endif // __EXCELLON_COMMAND_SLOT_H__

