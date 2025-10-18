#ifndef __COMPONENT_INPUT_LISTENER_H__
#define __COMPONENT_INPUT_LISTENER_H__
#include "StandardLib/Listener.h"
#include "StandardLib/Unknown.h"


class CComponentInput;
class CComponentInputListener : public CListener
{
CONSTRUCTABLE(CComponentInputListener);
protected:
	CComponentInput*	mpcComponentInput;

public:
			void Init(CComponentInput* pcComponentInput);

	virtual void ButtonPressed(CUnknown* pcSource, void* pvContext);
	virtual void ButtonReleased(CUnknown* pcSource, void* pvContext);
	virtual void NumpadButtonPressed(CUnknown* pcSource, void* pvContext);

	virtual void ShiftPressed(CUnknown* pcSource, void* pvContext);
	virtual void ShiftReleased(CUnknown* pcSource, void* pvContext);
	virtual void ControlPressed(CUnknown* pcSource, void* pvContext);
	virtual void ControlReleased(CUnknown* pcSource, void* pvContext);
	virtual void AltPressed(CUnknown* pcSource, void* pvContext);
	virtual void AltReleased(CUnknown* pcSource, void* pvContext);
	virtual void CapsLockPressed(CUnknown* pcSource, void* pvContext);
	virtual void NumLockPressed(CUnknown* pcSource, void* pvContext);
	virtual void InsertPressed(CUnknown* pcSource, void* pvContext);
	virtual void PointerMotion(CUnknown* pcSource, void* pvContext);
};


#endif // __COMPONENT_INPUT_LISTENER_H__

