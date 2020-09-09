// ****************************************************************************
//
//	rx_mem.c
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#ifdef linux
	#include <sys/sysinfo.h>
	#include <sys/mman.h>
#endif
#include "rx_error.h"
#include "rx_threads.h"
#include "rx_mem.h"
#include "errno.h"

static HANDLE _Mutex = NULL;

static int _Buffers=0;
static UINT64 _SizeAllocated=0;

//---------------------------------------------------------------------
//
//
//---------------------------------------------------------------------

typedef struct
{
	INT32   count;
	UINT64  size;
	HANDLE	sem_IsFree;
	BYTE	data[0];
} SBuffer;

//--- rx_mem_init ---------------------------------------------------
void  rx_mem_init(UINT64 size)
{
}

//--- rx_mem_alloc ------------------------------------------------------ 
BYTE* rx_mem_alloc(UINT64 size)
{
	int i=sizeof(SBuffer);
	int err=0;
	SBuffer *buf=NULL;
	
	if (_Mutex==NULL) _Mutex = rx_mutex_create();
	#ifdef linux
		if (size/(1024*1024)<=rx_mem_get_freeMB()) buf = malloc(size+sizeof(SBuffer));
		if (buf==NULL) 
		{
			rx_mem_clear_caches();
			Error(WARN, 0, "Linux cleared caches.");
			if (size/(1024*1024)>rx_mem_get_freeMB()) return NULL;
			buf = malloc(size+sizeof(SBuffer));
		}
	#else
		buf = malloc((size_t)(size+sizeof(SBuffer)));
	#endif
	if  (buf) 
	{
		#ifdef linux 
			// printf("mlock(%p, %u MB)\n", buf, (unsigned)(size/1024/1024));
			if (mlock(buf, size+sizeof(SBuffer)))
				err = errno;
		#endif
		_Buffers ++;
		_SizeAllocated += (size+sizeof(SBuffer));

		buf->count=0;
		buf->sem_IsFree = rx_sem_create();
		buf->size = size;
		return &buf->data[0];
	}
	return NULL;
}

//--- rx_use ---------------------------------------------------------
int rx_mem_use(BYTE *ptr)
{
	// increment counter
	int cnt=0;
	if (ptr)
	{
		SBuffer *buf = ((SBuffer*)ptr) - 1;
		
		rx_mutex_lock(_Mutex);
//		if (buf->count<0)
//			printf("rx_mem_use %p, cnt=%d ERROR\n", ptr, buf->count);
		cnt = (++buf->count);
		// printf("rx_mem_use %p, cnt=%d, _Buffers=%d\n", ptr, buf->count, _Buffers);
		rx_mutex_unlock(_Mutex);
	}
	return cnt;
}

//--- rx_mem_unuse -------------------------------------------------------
int  rx_mem_unuse(BYTE **ptr)
{
	int cnt=0;
	// decrement counter, free memory when zero
	if (*ptr)
	{
		SBuffer *buf = ((SBuffer*)*ptr) - 1;
		rx_mutex_lock(_Mutex);
		if (buf->count>0) buf->count--;
		cnt = buf->count;
		// printf("rx_mem_unuse %p, cnt=%d, _Buffers=%d\n", *ptr, buf->count, _Buffers);
		rx_mutex_unlock(_Mutex);
		if (cnt==0 && buf->sem_IsFree) rx_sem_post(buf->sem_IsFree);
	}
	return cnt;
}

//--- rx_mem_cnt --------------------------
int  rx_mem_cnt	(BYTE *ptr)
{
	int cnt=0;
	if (ptr)
	{
		SBuffer *buf = ((SBuffer*)ptr) - 1;
		rx_mutex_lock(_Mutex);
		cnt=buf->count;
	//	if (cnt<0)
	//		printf("rx_mem_cnt %p, cnt=%d\n", ptr, buf->count);
		rx_mutex_unlock(_Mutex);
	}
	return cnt;
}

//--- rx_mem_use_clear -----------------------------------
int  rx_mem_use_clear(BYTE *ptr)
{
	int cnt=0;
	if (ptr)
	{
		SBuffer *buf = ((SBuffer*)ptr) - 1;
		rx_mutex_lock(_Mutex);
		cnt=buf->count;
		buf->count=0;
		rx_mutex_unlock(_Mutex);
		if (buf->sem_IsFree) rx_sem_post(buf->sem_IsFree);
	}
	return cnt;
}


//--- rx_mem_free -------------------------------------------------------
void  rx_mem_free(BYTE **ptr)
{
	// decrement counter, free memory when zero
	if (ptr && *ptr)
	{
		SBuffer *buf = ((SBuffer*)*ptr) - 1;
		rx_mutex_lock(_Mutex);
		if (!buf->count)
		{
//			TrPrintfL(TRUE, "rx_mem_free 0x%08x", *ptr);
			#ifdef linux
				munlock(buf, buf->size+sizeof(SBuffer));
			#endif
			_Buffers--;
			_SizeAllocated -= (buf->size+sizeof(SBuffer));
			if (buf->sem_IsFree) rx_sem_destroy(&buf->sem_IsFree);
			free(buf);
			*ptr=NULL;
		} 
		rx_mutex_unlock(_Mutex);
	}
}

//--- rx_mem_free_force -------------------------------------------------------
void  rx_mem_free_force(BYTE **ptr)
{
	// decrement counter, free memory when zero
	if (ptr && *ptr)
	{
		SBuffer *buf = ((SBuffer*)*ptr) - 1;
		rx_mutex_lock(_Mutex);
		buf->count = 0;
		if (!buf->count)
		{
//			TrPrintfL(TRUE, "rx_mem_free 0x%08x", *ptr);
//			munlock(buf->data, buf->size);
			#ifdef linux
				munlock(buf, buf->size+sizeof(SBuffer));
			#endif
			_Buffers--;
			_SizeAllocated -= (buf->size+sizeof(SBuffer));
			if (buf->sem_IsFree) rx_sem_destroy(&buf->sem_IsFree);
			free(buf);
			*ptr=NULL;
		} 
		rx_mutex_unlock(_Mutex);
	}
}

//--- rx_mem_await_free -----------------------------------
int  rx_mem_await_free(BYTE *ptr, int timeout)
{
	if (ptr)
	{
		SBuffer *buf = ((SBuffer*)ptr) - 1;
		return rx_sem_wait(buf->sem_IsFree, timeout);
	}
	return REPLY_ERROR;
}

//--- rx_mem_await_abort ----------------------------
int  rx_mem_await_abort(BYTE *ptr)
{
	if (ptr)
	{
		SBuffer *buf = ((SBuffer*)ptr) - 1;
		return rx_sem_post(buf->sem_IsFree);
	}
	return REPLY_OK;
}



//--- rx_mem_physical --------------------------------
UINT64	rx_mem_physical(void)
{
	#ifdef linux
		return (UINT64)sysconf( _SC_PHYS_PAGES ) * (UINT64)sysconf( _SC_PAGESIZE );
	#else
		MEMORYSTATUS status;
		status.dwLength = sizeof(status);
		GlobalMemoryStatus( &status );
		return (UINT64)status.dwTotalPhys;
	#endif
}


//--- rx_mem_allocated ------------------------------
UINT64 rx_mem_allocated()
{
	return _SizeAllocated;
}

//--- rx_mem_get_totalMB ------------------------------
#ifdef linux
	UINT32 rx_mem_get_totalMB(void)
	{
		struct sysinfo info;
		sysinfo(&info);
		return info.totalram/1024/1024;
	}
#else
	UINT32 rx_mem_get_totalMB(void)
	{
		MEMORYSTATUSEX status;
		GlobalMemoryStatusEx(&status);
		return (int)(status.ullTotalPhys/1024/1024);
	}
#endif

//--- rx_mem_get_freeMB --------------------------------
#ifdef linux
	UINT32 rx_mem_get_freeMB(void)
	{
		struct sysinfo info;
		sysinfo(&info);
		return info.freeram/1024/1024;
	}
#else
	UINT32 rx_mem_get_freeMB(void)
	{
		MEMORYSTATUSEX status;
		status.dwLength = sizeof (status);
		GlobalMemoryStatusEx(&status);
		return (int)(status.ullAvailVirtual/1024/1024);
	}
#endif

//--- rx_mem_clear_caches ----------------------
void   rx_mem_clear_caches(void)
{
#ifdef linux
	int fd;
	char* data = "3";
	sync();
	fd = open("/proc/sys/vm/drop_caches", O_WRONLY);
	write(fd, data, sizeof(char));
	close(fd);
#endif 
}



