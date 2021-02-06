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
	void			Init(void);
	void			Init(char* sz);
	void			Init(const char* sz);
	void			Class(void);
	void			Free(void);

	BOOL			Save(CObjectSerialiser* pcFile);
	BOOL			Load(CObjectDeserialiser* pcFile);
};


#endif // __STRING_H__

