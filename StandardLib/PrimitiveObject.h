#ifndef __PRIMITIVE_OBJECT_H__
#define __PRIMITIVE_OBJECT_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/DataTypes.h"


#define LOG_PRIMITIVE_ACCESS_DEBUG()	LogPrimitiveAccessDebug((CPrimitiveObject*)this, __ENGINE_PRETTY_FUNCTION__)
#define LOG_PRIMITIVE_ASSIGN_DEBUG()	LogPrimitiveAssignDebug((CPrimitiveObject*)this, __ENGINE_PRETTY_FUNCTION__)


class CObject;
class CPrimitiveObject
{
private:
	uint16	muiClassType;  //EPrimitiveType
	uint16	muiNegativeEmbeddingOffset;

public:
					CPrimitiveObject(EPrimitiveType eType);

	EPrimitiveType	GetClassType(void);

	void			SetEmbedding(CObject* pcEmbedding);
	void			SetDirty(void);

	CObject*		Embedding(void);
};


void LogPrimitiveAccessDebug(CPrimitiveObject* pvThis, char* szMethod);
void LogPrimitiveAssignDebug(CPrimitiveObject* pvThis, char* szMethod);


#endif // !__PRIMITIVE_OBJECT_H__

