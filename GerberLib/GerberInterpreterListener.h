#ifndef __GERBER_INTERPRETER_LISTENER_H__
#define __GERBER_INTERPRETER_LISTENER_H__
#include "GerberGraphicsState.h"
#include "GerberCommandApertureBlock.h"
#include "GerberCommandStepAndRepeat.h"


//Receives the operations of a Gerber file with their coordinates resolved.  The graphics state holds the
//current aperture, polarity, mirroring, rotation and scaling at the time of the operation.
//Inside a region (BeginRegion to EndRegion) Draw and Arc are contour segments; they are not stroked.
class CGerberInterpreterListener
{
public:
	virtual void	Move(CGerberGraphicsState* pcState, SGerberPoint* psPoint);
	virtual void	Draw(CGerberGraphicsState* pcState, SGerberPoint* psStart, SGerberPoint* psEnd);
	virtual void	Arc(CGerberGraphicsState* pcState, SGerberPoint* psStart, SGerberPoint* psEnd, SGerberPoint* psCenter, bool bClockwise);
	virtual void	Flash(CGerberGraphicsState* pcState, SGerberPoint* psPoint);

	virtual void	BeginRegion(CGerberGraphicsState* pcState);
	virtual void	EndRegion(CGerberGraphicsState* pcState);

	//The block's objects are reported between BeginBlock and EndBlock.  They are placed by flashing the block aperture.
	virtual void	BeginBlock(CGerberGraphicsState* pcState, CGerberCommandApertureBlock* pcBlock);
	virtual void	EndBlock(CGerberGraphicsState* pcState, CGerberAperture* pcAperture);

	//The objects between BeginStepAndRepeat and EndStepAndRepeat are reported once; the listener replicates them.
	virtual void	BeginStepAndRepeat(CGerberGraphicsState* pcState, CGerberCommandStepAndRepeat* pcStepAndRepeat);
	virtual void	EndStepAndRepeat(CGerberGraphicsState* pcState);
};


#endif // __GERBER_INTERPRETER_LISTENER_H__

