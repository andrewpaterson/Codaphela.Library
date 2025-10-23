#ifndef __NATIVE_WINDOW_FACTORY_H__
#define __NATIVE_WINDOW_FACTORY_H__
#include "BaseLib/Constructable.h"
#include "NativeWindow.h"
#include "NativeCanvas.h"


class CWindow;
class CCanvas;
class CNativeWindowFactory : public CConstructable
{
CONSTRUCTABLE(CNativeWindowFactory);
friend class CWindowFactory;
protected:
	CMallocator*	mpcMallocator;

public:
			void			Init(CMallocator* pcMallocator);
			void			Kill(void);

	virtual CNativeWindow*	CreateNativeWindow(CWindow* pcWindow) =0;
			bool			DestroyNativeWindow(CNativeWindow* pcWindow);

	virtual CNativeCanvas*	CreateNativeCanvas(CCanvas* pcCanvas) =0;
			bool			DestroyNativeCanvas(CNativeCanvas* pcCanvas);

protected:
	template<class Class>
	Class* NativeMalloc(void)
	{
		Class* pcInstance;

		pcInstance = (Class*)mpcMallocator->Malloc(sizeof(Class));
		New<Class>(pcInstance);

		return pcInstance;
	}
};



#endif // __NATIVE_WINDOW_FACTORY_H__

