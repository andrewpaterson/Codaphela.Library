#ifndef __GERBER_VECTOR_IMAGE_H__
#define __GERBER_VECTOR_IMAGE_H__
#include "BaseLib/ArrayTemplate.h"
#include "GerberVectorObject.h"


typedef CArrayTemplate<CGerberVectorObject>	CArrayGerberVectorObject;


class CGerberVectorObjects
{
protected:
	CArrayGerberVectorObject	macObjects;

public:
	void					Init(void);
	void					Kill(void);

	CGerberVectorObject*	Add(EGerberPolarity ePolarity);
	CGerberVectorObject*	Add(CGerberVectorObject* pcSource);

	size					NumObjects(void);
	CGerberVectorObject*	GetObject(size uiIndex);
};


//The objects of a layer in the order they were created, in millimetres.
class CGerberVectorImage : public CGerberVectorObjects
{
public:
	bool	GetBounds(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum);
};


#endif // __GERBER_VECTOR_IMAGE_H__

