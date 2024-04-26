#ifndef __PRIMITIVE_CLASSES_H__
#define __PRIMITIVE_CLASSES_H__
#include "BaseLib/GlobalMemory.h"
#include "Class.h"


class CClasses;
class CPrimitiveClasses
{
private:
	CClasses* mpcClasses;

protected:
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
	CClass* mpcBool;
	CClass* mpcVoid;
	CClass* mpcChar8;
	CClass* mpcChar16;

public:
	void		Init(CClasses* pcClasses);
	void		Kill(void);

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
	CClass*		GetBool(void);
	CClass*		GetVoid(void);
	CClass*		GetChar(void);
	CClass*		GetWidechar(void);

protected:
	template<class Class>
	CClass*		AddClass(EPrimitiveType eFieldType);
	template<class Class>
	CClass*		AddClassWithConstructorAndIO(EPrimitiveType eFieldType);
	CClass*		AddClass(const char* szClassName, size_t uiClassSize, EPrimitiveType eFieldType, uint32 eFieldOffest);
	CClass*		AddVoid(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class Class>
CClass* CPrimitiveClasses::AddClass(EPrimitiveType eFieldType)
{
	Class	cInstance;
	size_t	uiOffset;
	CClass* pcClass;

	uiOffset = (char*)(&cInstance.mVal) - (char*)(&cInstance);
	pcClass = AddClass(cInstance.ClassName(), cInstance.ClassSize(), eFieldType, uiOffset);

	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class Class>
CClass* CPrimitiveClasses::AddClassWithConstructorAndIO(EPrimitiveType eFieldType)
{
	Class			c;
	const char*		szClassName;
	CClass*			pcClass;

	szClassName = c.ClassName();
	pcClass = AddClass<Class>(eFieldType);

	mpcClasses->AddConstructorAndIO<Class>();

	return pcClass;
}


#endif // __PRIMITIVE_CLASSES_H__

