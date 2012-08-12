#ifndef __STRING_H__
#define __STRING_H__
#include "BaseLib/Chars.h"
#include "Object.h"

class CString : public CObject
{
BASE_FUNCTIONS(CString);
protected:
	CChars	msz;

public:
	void	Init(void);
	void	Kill(void);

	BOOL	Save(CFileWriter* pcFile);
	BOOL	Load(CFileReader* pcFile);

};


#endif // __STRING_H__

