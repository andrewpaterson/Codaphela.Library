#ifndef __LEAF_DETECTOR_H__
#define __LEAF_DETECTOR_H__


#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC  
#endif // _MSC_VER

#include <stdlib.h>  

#ifdef _MSC_VER
#include <crtdbg.h>  
#endif // _MSC_VER

#ifdef __SANITIZE_ADDRESS__
#include <sanitizer/lsan_interface.h>
#endif // __SANITIZE_ADDRESS__


void BreakOnAllocation(long iAllocation);
void LeakDetection(void);


#endif // __LEAF_DETECTOR_H__

