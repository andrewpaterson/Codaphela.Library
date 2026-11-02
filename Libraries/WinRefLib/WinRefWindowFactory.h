#ifndef __WIN_Ref_WINDOW_FACTORY_H__
#define __WIN_Ref_WINDOW_FACTORY_H__
#include "BaseLib/Define.h"
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/Chars.h"
#include "SupportLib/ColourFormat.h"
#include "WindowLib/NativeWindowFactory.h"


class CWinRefWindowFactory : public CNativeWindowFactory
{
CONSTRUCTABLE(CWinRefWindowFactory);
protected:
	CInt32Rect	mcWindowBounds;
	CChars		mszImagePath;
	CChars		mszClipboardText;  //The reference system's clipboard, shared by every window the factory makes.

public:
	void			Init(CMallocator* pcMallocator, int32 iWidth, int32 iHeight, char* szImagePath);
	void			Kill(void);

	CNativeWindow*	CreateNativeWindow(CWindow* pcWindow) override;
	CNativeCanvas*	CreateNativeCanvas(CCanvas* pcCanvas) override;
	CNativeClipboard*	CreateNativeClipboard(CClipboard* pcClipboard) override;

	int32			GetWidth(void);
	int32			GetHeight(void);
	CChars*			GetClipboardText(void);

protected:

};


#endif // __WIN_Ref_WINDOW_FACTORY_H__

