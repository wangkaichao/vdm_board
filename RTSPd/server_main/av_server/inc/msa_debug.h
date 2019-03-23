#ifndef _MSA_DEBUG_H_
#define _MSA_DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DEBUG
#define DEBUG
#endif
//#undef DEBUG

#ifdef DEBUG
#define DPRINTK(fmt, args...)	printf("(%s,%d)%s: " fmt,__FILE__,__LINE__, __FUNCTION__ , ## args)
#else
#define DPRINTK(fmt, args...)
#endif


#ifdef __cplusplus
}
#endif

#endif 
