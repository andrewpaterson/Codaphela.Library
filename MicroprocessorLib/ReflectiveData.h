#ifndef __REFLECTIVE_DATA__
#define __REFLECTIVE_DATA__
#include "SaveData.h"


class CReflectiveData : public CSaveData
{
public:
    void Init(void)
    {
    }

    void* getObject(void)
    {
        return this;
    }
};


#endif // __REFLECTIVE_DATA__

