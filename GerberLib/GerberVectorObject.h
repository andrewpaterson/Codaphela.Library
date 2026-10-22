#ifndef __GERBER_VECTOR_OBJECT_H__
#define __GERBER_VECTOR_OBJECT_H__
#include "SupportLib/VectorPath.h"
#include "GerberCommandLoadPolarity.h"


//A graphical object of a layer in millimetres; a vector path with a polarity.  The polarity says whether the object
//darkens or clears the layer beneath it.
class CGerberVectorObject : public CVectorPath
{
protected:
	EGerberPolarity		mePolarity;

public:
	void				Init(EGerberPolarity ePolarity);
	void				Init(CGerberVectorObject* pcSource);

	void				SetPolarity(EGerberPolarity ePolarity);
	EGerberPolarity		GetPolarity(void);
	bool				IsDark(void);
};


#endif // __GERBER_VECTOR_OBJECT_H__

