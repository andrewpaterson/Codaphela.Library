#ifndef __STRING_H__
#define __STRING_H__
#include "BaseLib/Chars.h"
#include "Object.h"


class CObjectSerialiser;
class CObjectDeserialiser;
class CString : public CObject, public CChars
{
CONSTRUCTABLE(CString);
public:
	Ptr<CString>	Init(char* sz);
	void			Kill(void);
	void			Class(void);
	void			Free(void);

	BOOL			Save(CObjectSerialiser* pcFile);
	BOOL			Load(CObjectDeserialiser* pcFile);
};


#endif // __STRING_H__

