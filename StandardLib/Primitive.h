#ifndef __PRIMITIVE_H__
#define __PRIMITIVE_H__
#include "BaseLib/FileReader.h"
#include "BaseLib/FileWriter.h"
#include "PrimitiveObject.h"


class UInt32;

template<class PRIMITIVE_TYPE>
class CPrimitive : public CPrimitiveObject
{
friend class CPrimitiveClasses;
protected:
	PRIMITIVE_TYPE mVal;

public:
	CPrimitive(EPrimitiveType eType) : CPrimitiveObject(eType) {}

	void Assign(PRIMITIVE_TYPE val)
	{
		LOG_PRIMITIVE_ASSIGN_DEBUG();

		//We need to only set the containing object dirty if the new value differs from the old one to make primitive assignment consistent with pointers.
		if (mVal != val)
		{
			mVal = val;
			SetDirty();
		}
	}

	operator PRIMITIVE_TYPE() const
	{
		LOG_PRIMITIVE_ACCESS_DEBUG();

		return mVal;
	}

	PRIMITIVE_TYPE* GetPrimitivePointer(void)
	{
		return &mVal;
	}
};


#define PRIMITIVE_TYPE_DEFINITION(CLASS_TYPE, C_PRIMITIVE, DATA_TYPE, WRITE_FUNC, READ_FUNC)  \
class CLASS_TYPE : public CPrimitive<C_PRIMITIVE> \
{ \
CONSTRUCTABLE(CLASS_TYPE); \
public: \
	CLASS_TYPE() : CPrimitive(DATA_TYPE) {}\
 \
	CLASS_TYPE& operator= (C_PRIMITIVE val) \
	{ \
		Assign(val); \
		return *this; \
	} \
\
	bool Save(CFileWriter* pcFile) \
	{ \
		return pcFile-> WRITE_FUNC (mVal); \
	} \
\
	bool Load(CFileReader* pcFile) \
	{ \
		return pcFile-> READ_FUNC (&mVal); \
	} \
};


#endif // __PRIMITIVE_H__

