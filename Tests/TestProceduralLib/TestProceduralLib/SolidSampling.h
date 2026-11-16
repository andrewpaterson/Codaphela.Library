#ifndef __SOLID_SAMPLING_H__
#define __SOLID_SAMPLING_H__
#include "StandardLib/Pointer.h"
#include "SupportLib/SolidTexture.h"


//A solid texture at a point, for its tests: at no width, on a surface facing up (z).


void	TestSolidSampleAt(Ptr<CSolidTexture> pTexture, float32 x, float32 y, float32 z, SSolidSample* psSample);
float32	TestSolidValueAt(Ptr<CSolidTexture> pTexture, float32 x, float32 y, float32 z);


#endif // __SOLID_SAMPLING_H__

