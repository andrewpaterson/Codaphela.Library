#ifndef __WIN_GDI_HELPER_H__
#define __WIN_GDI_HELPER_H__
#include <windows.h>
#include "SupportLib/Int32Rect.h"


void CopyRectangleToGDIRect(RECT* psRect, CInt32Rect* pcRectangle);


#endif // __WIN_GDI_HELPER_H__

