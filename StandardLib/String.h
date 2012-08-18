#ifndef __STRING_H__
#define __STRING_H__
#include "BaseLib/Chars.h"
#include "Object.h"

class CObjectWriter;
class CObjectReader;
class CString : public CObject, public CChars
{
BASE_FUNCTIONS(CString);
public:
	void	Kill(void);

	BOOL	Save(CObjectWriter* pcFile);
	BOOL	Load(CObjectReader* pcFile);

};


#endif // __STRING_H__

