#ifndef __MAIN_PROCESS_DIRECTOR_H__
#define __MAIN_PROCESS_DIRECTOR_H__
#include "BaseLib/MapStringBlock.h"
#include "BaseLib/MapStringBlock.h"


typedef int(*main_func)(int argc, char* argv[]);


class CMainProcessDirector
{
private:
	CMapStringBlock mmszRedirects;
	bool			mbRanRedirect;
	int				miRedirectResult;

public:
	void	Init(void);
	void	Kill(void);

	void	AddRedirect(char* szName, main_func);

	bool	Run(int argc, char* argv[]);
	int		GetResult(void);
};


#endif // !__MAIN_PROCESS_DIRECTOR_H__


