#ifndef __WIN_GDI_HELPER_H__
#define __WIN_GDI_HELPER_H__
#include <windows.h>
#include "SupportLib/Rectangle.h"


void CopyRectangleToGDIRect(RECT* psRect, CRectangle* pcRectangle);


#endif // __WIN_GDI_HELPER_H__

