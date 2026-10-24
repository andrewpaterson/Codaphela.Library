//Signals for the 65C64.  Nothing raises them but raise; a signal without a handler (SIG_DFL) stops the program
//unsuccessfully, as abort does.
#ifndef __LIBC65_SIGNAL_H__
#define __LIBC65_SIGNAL_H__


typedef int		sig_atomic_t;
typedef void	(*__sighandler_t)(int);


#define SIGINT		2
#define SIGILL		4
#define SIGABRT		6
#define SIGFPE		8
#define SIGSEGV		11
#define SIGTERM		15

#define SIG_DFL		((__sighandler_t)0)
#define SIG_IGN		((__sighandler_t)1)
#define SIG_ERR		((__sighandler_t)-1)


__sighandler_t	signal(int iSignal, __sighandler_t fHandler);
int				raise(int iSignal);


#endif // __LIBC65_SIGNAL_H__
