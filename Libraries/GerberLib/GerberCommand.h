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

	virtual bool	IsApertureMacro(void) const;
	virtual bool	IsComment(void) const;
	virtual bool	IsFileAttribute(void) const;
	virtual bool	IsApertureAttribute(void) const;
	virtual bool	IsObjectAttribute(void) const;
	virtual bool	IsDeleteAttribute(void) const;
	virtual bool	IsApertureDefine(void) const;
	virtual bool	IsSetAperture(void) const;
	virtual bool	IsOperation(void) const;
	virtual bool	IsFormatSpecifier(void) const;
	virtual bool	IsLoadPolarity(void) const;
	virtual bool	IsLoadMirroring(void) const;
	virtual bool	IsLoadRotation(void) const;
	virtual bool	IsLoadScaling(void) const;
	virtual bool	IsMeasurementMode(void) const;
	virtual bool	IsPlotMode(void) const;
	virtual bool	IsQuadrantMode(void) const;
	virtual bool	IsCompound(void) const;
	virtual bool	IsRegion(void) const;
	virtual bool	IsApertureBlock(void) const;
	virtual bool	IsStepAndRepeat(void) const;
	virtual bool	IsEndOfFile(void) const;
};


#endif // __GERBER_COMMAND_H__

