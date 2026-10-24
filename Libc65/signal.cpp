//Signals for the 65C64, in C++ for the 65C64 generator.
#include <signal.h>
#include <stdlib.h>


#define SIGNAL_MAX		16


__sighandler_t	gafSignalHandlers[SIGNAL_MAX];


__sighandler_t signal(int iSignal, __sighandler_t fHandler)
{
	__sighandler_t	fPrevious;

	if ((iSignal <= 0) || (iSignal >= SIGNAL_MAX))
	{
		return SIG_ERR;
	}
	fPrevious = gafSignalHandlers[iSignal];
	gafSignalHandlers[iSignal] = fHandler;
	return fPrevious;
}


//The handler is reset to SIG_DFL before it is called, as C allows.
int raise(int iSignal)
{
	__sighandler_t	fHandler;

	if ((iSignal <= 0) || (iSignal >= SIGNAL_MAX))
	{
		return -1;
	}
	fHandler = gafSignalHandlers[iSignal];
	if (fHandler == SIG_IGN)
	{
		return 0;
	}
	if (fHandler == SIG_DFL)
	{
		_Exit(EXIT_FAILURE);
	}
	gafSignalHandlers[iSignal] = SIG_DFL;
	fHandler(iSignal);
	return 0;
}
