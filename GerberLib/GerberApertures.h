#ifndef __GERBER_APERTURES_H__
#define __GERBER_APERTURES_H__
#include "BaseLib/ListVariable.h"
#include "GerberAperture.h"


//The aperture dictionary.  Apertures are allocated individually so pointers to them stay valid.
class CGerberApertures
{
protected:
	CListVariable	macApertures;

public:
	void				Init(void);
	void				Kill(void);

	CGerberAperture*	Add(uint32 uiDCode, EGerberApertureType eType);
	CGerberAperture*	Get(uint32 uiDCode);

	size				NumApertures(void);
	CGerberAperture*	GetAperture(size uiIndex);
};


#endif // __GERBER_APERTURES_H__

