#ifndef __UNMANAGED_CLASSES_H__
#define __UNMANAGED_CLASSES_H__
#include "Class.h"


class CClasses;
class CUnmanagedClasses
{
private:
	CClasses*	mpcClasses;
	size		uiNumUnmanagedClasses;

protected:
	CClass* mpcUndefined;
	CClass* mpcVoidPointer;

	CClass* mpcInt8;
	CClass* mpcInt16;
	CClass* mpcInt32;
	CClass* mpcInt64;
	CClass* mpcUInt8;
	CClass* mpcUInt16;
	CClass* mpcUInt32;
	CClass* mpcUInt64;	
	CClass* mpcFloat32;
	CClass* mpcFloat64;
	CClass* mpcM2float32;
	CClass* mpcM3float32;
	CClass* mpcM4float32;
	CClass* mpcM2float64;
	CClass* mpcM3float64;
	CClass* mpcM4float64;
	CClass* mpcM2int32;
	CClass* mpcM3int32;	
	CClass* mpcM4int32;	
	CClass* mpcBool; 	
	CClass* mpcVoid;	
	CClass* mpcChar8;
	CClass* mpcChar16;

	CClass* mpcString;
	CClass* mpcStringImmutable;
	CClass* mpcNumber;
	CClass* mpcDate;
	CClass* mpcDateTime;
	CClass* mpcEnum;

	CClass* mpcBit;
	CClass* mpcCrumb;
	CClass* mpcTribble;
	CClass* mpcNybble;	
	CClass* mpcNickle;	
	CClass* mpcSixbits;

public:
	void		Init(CClasses* pcClasses);
	void		Kill(void);

	CClass*		GetUndefined(void);
	CClass*		GetVoidPointer(void);
	CClass*		GetByte(void);
	CClass*		GetShort(void);
	CClass*		GetInt(void);
	CClass*		GetLong(void);
	CClass*		GetUByte(void);
	CClass*		GetUShort(void);
	CClass*		GetUInt(void);
	CClass*		GetULong(void);
	CClass*		GetFloat(void);
	CClass*		GetDouble(void);
	CClass*		GetFloat2(void);
	CClass*		GetFloat3(void);
	CClass*		GetFloat4(void);
	CClass*		GetDouble2(void);
	CClass*		GetDouble3(void);
	CClass*		GetDouble4(void);
	CClass*		GetInt2(void);
	CClass*		GetInt3(void);
	CClass*		GetInt4(void);
	CClass*		GetBool(void);
	CClass*		GetVoid(void);
	CClass*		GetChar(void);
	CClass*		GetWidechar(void);
	CClass*		GetString(void);
	CClass*		GetStringImmutable(void);
	CClass*		GetNumber(void);
	CClass*		GetDate(void);
	CClass*		GetDateTime(void);
	CClass*		GetBit(void);
	CClass*		GetCrumb(void);
	CClass*		GetTribble(void);
	CClass*		GetNybble(void);
	CClass*		GetNickle(void);
	CClass*		GetSixbits(void);
	CClass*		GetEnum(void);

protected:
	CClass*		AddClassByPrettyName(EPrimitiveType eType);
	CClass*		AddClassByCPPName(EPrimitiveType eType);
	CClass*		AddClass(EPrimitiveType eType, const char* szName, int iSize);
};


#endif // __UNMANAGED_CLASSES_H__

