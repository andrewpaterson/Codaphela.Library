#ifndef __EXCELLON_VECTORISER_H__
#define __EXCELLON_VECTORISER_H__
#include "ExcellonInterpreterListener.h"
#include "GerberVectorImage.h"


//Turns the holes and slots of an interpreted Excellon file into the dark objects of a vector image, in millimetres.
class CExcellonVectoriser : public CExcellonInterpreterListener
{
protected:
	CGerberVectorImage*	mpcImage;
	float64				mfUnit;

public:
	void	Init(CGerberVectorImage* pcImage, bool bInches);
	void	Kill(void);

	void	Hole(CExcellonCommandToolDefine* pcTool, SGerberPoint* psPoint);
	void	Slot(CExcellonCommandToolDefine* pcTool, SGerberPoint* psStart, SGerberPoint* psEnd);
};


#endif // __EXCELLON_VECTORISER_H__

