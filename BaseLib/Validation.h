#ifndef __VALIDATION_H__
#define __VALIDATION_H__


#define Break()	__asm int 3;
#define Validate(test)	if (!test) Break();


#endif // __VALIDATION_H__

