#ifndef __GERBER_NUMBERS_H__
#define __GERBER_NUMBERS_H__
#include "BaseLib/Number.h"
#include "BaseLib/ArrayTemplate.h"


typedef CArrayTemplate<CNumber>	CArrayGerberNumber;


struct SGerberPoint
{
	CNumber		cX;
	CNumber		cY;
};


#endif // __GERBER_NUMBERS_H__

