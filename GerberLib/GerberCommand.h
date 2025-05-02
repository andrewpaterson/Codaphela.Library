#ifndef __GERBER_COMMAND_H__
#define __GERBER_COMMAND_H__
#include "BaseLib/Chars.h"
#include "GerberCommandEnum.h"

class CGerberCommand
{
protected:
	EGerberCommandEnum	meType;

public:
			void	Init(EGerberCommandEnum eType);
	virtual void	Kill(void) =0;
			bool	IsType(EGerberCommandEnum eType);

	virtual bool	IsApertureMacro(void);
	virtual bool	IsComment(void);
	virtual bool	IsFileAttribute(void);
	virtual bool	IsFormatSpecifier(void);
	virtual bool	IsLoadPolarity(void);
	virtual bool	IsLoadMirroring(void);
	virtual bool	IsLoadRotation(void);
	virtual bool	IsLoadScaling(void);
	virtual bool	IsMeasurementMode(void);
	virtual bool	IsPlotMode(void);
	virtual bool	IsEndOfFile(void);
};


#endif // __GERBER_COMMAND_H__

