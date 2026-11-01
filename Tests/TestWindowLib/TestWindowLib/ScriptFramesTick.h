#ifndef __SCRIPT_FRAMES_TICK_H__
#define __SCRIPT_FRAMES_TICK_H__
#include "StandardLib/ClassDefines.h"
#include "WindowLib/Window.h"
#include "WindowLib/WindowTick.h"


//Called after frame iFrame has been painted to set up the next one.  Returns false to stop the window.
typedef bool (*ScriptFrameFunc)(Ptr<CWindow> pWindow, int32 iFrame, void* pvContext);


//Paints a frame, then lets the script change the window for the next one, until the script stops it.
class CScriptFramesTick : public CWindowTick
{
CONSTRUCTABLE(CScriptFramesTick);
DESTRUCTABLE(CScriptFramesTick);
protected:
	int32				miFrame;
	ScriptFrameFunc		mfScript;
	void*				mpvContext;

public:
	void Init(ScriptFrameFunc fScript, void* pvContext)
	{
		PreInit();
		CWindowTick::Init();
		miFrame = 0;
		mfScript = fScript;
		mpvContext = pvContext;
		PostInit();
	}

	void Class(void) override
	{
		CWindowTick::Class();
		U_Int32(miFrame);
		U_Data(ScriptFrameFunc, mfScript);
		U_Pointer(mpvContext);
	}

	void Tick(Ptr<CWindow> pWindow, int64 iUpdateTimeInMillieconds, int64 iTotalTimeInMillieconds) override
	{
		pWindow->Paint();
		if (!mfScript(pWindow, miFrame, mpvContext))
		{
			pWindow->Stop();
		}
		miFrame++;
	}
};


#endif // __SCRIPT_FRAMES_TICK_H__

