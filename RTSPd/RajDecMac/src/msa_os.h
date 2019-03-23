#ifndef _stream_buffer_os_h_
#define _stream_buffer_os_h_

#ifdef WIN32

#ifndef WINVER                          // 指定要求的最低平台是 Windows XP。
#define WINVER 0x0501          
#endif

#ifndef _WIN32_WINNT            // 指定要求的最低平台是 Windows Vista。
#define _WIN32_WINNT 0x0501    
#endif

#ifndef _WIN32_WINDOWS          // 指定要求的最低平台是 Windows 98。
#define _WIN32_WINDOWS 0x0410 
#endif

#ifndef _WIN32_IE                       // 指定要求的最低平台是 Internet Explorer 7.0。
#define _WIN32_IE 0x0600        // 将此值更改为相应的值，以适用于 IE 的其他版本。
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料

// Windows 头文件:
#include <windows.h>

#pragma warning (disable : 4127) //条件判断是常量
#pragma warning (disable : 4100) //未引用的形参

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#else // for linux

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#endif


#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32

typedef CRITICAL_SECTION Mutex_T;

#define MUTEX_INIT(mutex)   do {InitializeCriticalSection(&mutex);} while (0)
#define MUTEX_UNINIT(mutex) do {DeleteCriticalSection(&mutex);} while (0)

#define MUTEX_LOCK(mutex)   \
do {											\
	uint count = 0;								\
	while (!TryEnterCriticalSection(&mutex))	\
	{											\
		++count;								\
		if (count > 100)						\
		{										\
			count = 0;							\
			Sleep(0);							\
		}										\
	}											\
} while (0)

#define MUTEX_UNLOCK(mutex) do {LeaveCriticalSection(&mutex);} while (0)

#else //for linux

typedef pthread_mutex_t     Mutex_T;

#define MUTEX_INIT(mutex)   pthread_mutex_init(&mutex, NULL)
#define MUTEX_UNINIT(mutex) pthread_mutex_destroy(&mutex)
#define MUTEX_LOCK(mutex)   pthread_mutex_lock(&mutex)
#define MUTEX_UNLOCK(mutex) pthread_mutex_unlock(&mutex)

#endif


#ifdef __cplusplus
};
#endif

#endif // _stream_buffer_os_h_
