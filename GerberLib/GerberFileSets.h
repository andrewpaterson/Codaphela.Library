#ifndef __GERBER_FILE_SETS_H__
#define __GERBER_FILE_SETS_H__
#include "GerberFileSet.h"


//A collection of board file sets, eg: the boards to merge into one panel.
class CGerberFileSets
{
protected:
	CArrayGerberFileSetPtr	macSets;

public:
	void			Init(void);
	void			Kill(void);

	CGerberFileSet*	Add(char* szDirectory);
	bool			AddDirectories(char* szParentDirectory);

	size			NumSets(void);
	CGerberFileSet*	Get(size uiIndex);
	CGerberFileSet*	Get(char* szName);
};


#endif // __GERBER_FILE_SETS_H__
