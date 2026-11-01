#ifndef __NO_IMAGE_REF_WINDOW_FACTORY_H__
#define __NO_IMAGE_REF_WINDOW_FACTORY_H__
#include "WinRefLib/WinRefWindowFactory.h"
#include "WinRefLib/WinRefCanvas.h"


//A reference canvas that does not share its image so canvas draws must use an image of their own and copy it on.
class CNoImageRefCanvas : public CWinRefCanvas
{
CONSTRUCTABLE(CNoImageRefCanvas);
public:
	Ptr<CImage>		GetImageOrNull(void) override;
};


class CNoImageRefWindowFactory : public CWinRefWindowFactory
{
CONSTRUCTABLE(CNoImageRefWindowFactory);
public:
	CNativeCanvas*	CreateNativeCanvas(CCanvas* pcCanvas) override;
};


#endif // __NO_IMAGE_REF_WINDOW_FACTORY_H__

