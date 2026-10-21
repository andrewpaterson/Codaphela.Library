#ifndef __GERBER_VECTORISER_H__
#define __GERBER_VECTORISER_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ListVariable.h"
#include "BaseLib/Chars.h"
#include "GerberInterpreterListener.h"
#include "GerberVectorImage.h"


struct SGerberVectorBlock
{
	uint32					uiDCode;
	CGerberVectorObjects	cObjects;
};


struct SGerberVectorApertureShape
{
	CGerberAperture*		pcAperture;
	CGerberVectorObject		cShape;
};


typedef CArrayTemplate<CGerberVectorObjects*>			CArrayGerberVectorObjectsPtr;
typedef CArrayTemplate<SGerberVectorApertureShape>		CArrayGerberVectorApertureShape;


//Turns the operations of an interpreted Gerber file into the graphical objects of a vector image.
class CGerberVectoriser : public CGerberInterpreterListener
{
protected:
	CGerberVectorImage*					mpcImage;
	CGerberVectorObjects*				mpcTarget;			//Objects are added here; the image, a block or a step and repeat body.
	CArrayGerberVectorObjectsPtr		mapcTargets;		//The enclosing targets.
	CListVariable						macBlocks;			//SGerberVectorBlock; allocated individually so their objects stay put.
	CArrayGerberVectorApertureShape		masShapes;			//Each flashed aperture around its origin, made once.
	CGerberVectorObjects				mcStepAndRepeat;
	CGerberCommandStepAndRepeat*		mpcStepAndRepeat;
	CGerberVectorObject*				mpcRegion;
	bool								mbError;
	CChars								mszError;

public:
	void	Init(CGerberVectorImage* pcImage);
	void	Kill(void);

	bool	HasError(void);
	char*	GetError(void);

	void	Move(CGerberGraphicsState* pcState, SGerberPoint* psPoint);
	void	Draw(CGerberGraphicsState* pcState, SGerberPoint* psStart, SGerberPoint* psEnd);
	void	Arc(CGerberGraphicsState* pcState, SGerberPoint* psStart, SGerberPoint* psEnd, SGerberPoint* psCenter, bool bClockwise);
	void	Flash(CGerberGraphicsState* pcState, SGerberPoint* psPoint);
	void	BeginRegion(CGerberGraphicsState* pcState);
	void	EndRegion(CGerberGraphicsState* pcState);
	void	BeginBlock(CGerberGraphicsState* pcState, CGerberCommandApertureBlock* pcBlock);
	void	EndBlock(CGerberGraphicsState* pcState, CGerberAperture* pcAperture);
	void	BeginStepAndRepeat(CGerberGraphicsState* pcState, CGerberCommandStepAndRepeat* pcStepAndRepeat);
	void	EndStepAndRepeat(CGerberGraphicsState* pcState);

protected:
	float64					GetUnit(CGerberGraphicsState* pcState);
	void					ToMillimetres(CGerberGraphicsState* pcState, SGerberPoint* psPoint, SFloat64Vec2* psMillimetres);
	void					GetTransform(CGerberGraphicsState* pcState, SFloat64Vec2* psFlash, SGerberVectorTransform* psTransform);
	float64					GetStrokeRadius(CGerberGraphicsState* pcState);
	CGerberVectorObject*	GetApertureShape(CGerberGraphicsState* pcState, CGerberAperture* pcAperture);
	SGerberVectorBlock*		GetBlock(uint32 uiDCode);
	void					PushTarget(CGerberVectorObjects* pcTarget);
	void					PopTarget(void);
	void					SetError(const char* szError);
};


#endif // __GERBER_VECTORISER_H__

