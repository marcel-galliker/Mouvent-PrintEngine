// ****************************************************************************
//
//	rx_fifo.c
//
// ****************************************************************************
//
//	Copyright 2018 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Hubert Zimmermann
//
// ****************************************************************************

//--- Includes ----------------------------------------------------------------

#include "rx_common.h"
#include "rx_fifo.h"

#ifdef linux
#include <unistd.h>
#include <pthread.h>
#endif

//--- Structures --------------------------------------------------------------

typedef struct {
#ifdef WIN32	
	CRITICAL_SECTION		mutex;					/* Mutex for cond and structure access */
	HANDLE					cond;					/* Conditional variable */
#endif	
#ifdef linux
	pthread_mutex_t			mutex;					/* Mutex for cond and structure access */
	pthread_cond_t			cond;					/* Conditional variable */
#endif	
	int						size;					/* Size of array */
	int 					count;					/* Number of elements actually stored */
	int						top;					/* Top element (next free) */
	int						bottom;					/* Bottom element */
	void					**elems;				/* Array of elements */
	int						*len;					/* Array of lengths */
} SHFifo;

//--- Prototypes --------------------------------------------------------------

#ifdef linux
static void i_cleanup_push(void *para);
#endif


//***************************************************************************** 

//--- rx_fifo_create ----------------------------------------------------------
int rx_fifo_create (HANDLE *handle, int size)
{
	SHFifo *fifo;
	
	if ( (!handle) || (size<=1) )
		return (REPLY_ERROR);

	*handle = NULL;

	fifo = (SHFifo*) malloc(sizeof (SHFifo));
	fifo->size		= size;
	fifo->count		= 0;
	fifo->elems		= (void**) malloc (sizeof (void*)*size);
	fifo->len		= (int*) malloc (sizeof (int)*size);
	fifo->top		= 0;
	fifo->bottom	= 0;

#ifdef WIN32
	InitializeCriticalSection (&fifo->mutex);
	fifo->cond = CreateEvent (0, 1, 0, 0);
#endif
#ifdef linux
	pthread_mutex_init(&fifo->mutex, NULL);
	pthread_cond_init(&fifo->cond, NULL);
#endif

	*handle = fifo;
	return (REPLY_OK);
}

//--- rx_fifo_close -----------------------------------------------------------
int rx_fifo_close (HANDLE handle)
{
	SHFifo *fifo = (SHFifo*) handle;

	if (!handle)
		return (REPLY_ERROR);
	
#ifdef WIN32
	EnterCriticalSection (&fifo->mutex);
	CloseHandle (fifo->cond);
	DeleteCriticalSection (&fifo->mutex);
#endif	
#ifdef linux
	if (pthread_mutex_lock(&fifo->mutex))
		return (REPLY_ERROR);
	pthread_mutex_unlock(&fifo->mutex);
	pthread_mutex_destroy(&fifo->mutex);
	pthread_cond_destroy(&fifo->cond);
#endif

	free (fifo->elems);
	free (fifo->len);
	free (fifo);
	
	return (REPLY_OK);
}

//--- rx_fifo_put -------------------------------------------------------------
int rx_fifo_put(HANDLE handle, void* buffer, int len)
{
	SHFifo *fifo = (SHFifo*)handle;

	if ( (!handle) || (len<0) )
		return (REPLY_ERROR);
	if ( (len>0) && (!buffer) )
		return (REPLY_ERROR);
	
#ifdef WIN32
	EnterCriticalSection(&fifo->mutex);
	while (fifo->count >= fifo->size) {
		LeaveCriticalSection(&fifo->mutex);
		WaitForSingleObjectEx(fifo->cond, INFINITE, 0);
		EnterCriticalSection(&fifo->mutex);
	}
#endif	
#ifdef linux
	if (pthread_mutex_lock(&fifo->mutex))
		return (REPLY_ERROR);
	pthread_cleanup_push(&i_cleanup_push, fifo);
	while (fifo->count >= fifo->size)
		pthread_cond_wait(&fifo->cond, &fifo->mutex);
#endif

	fifo->count++;
	fifo->elems [fifo->top] = buffer;
	fifo->len [fifo->top] = len;
	fifo->top++;
	if (fifo->top >= fifo->size)
		fifo->top = 0;

#ifdef WIN32
	SetEvent (fifo->cond);
	LeaveCriticalSection (&fifo->mutex);
#endif
#ifdef linux
	pthread_cleanup_pop (1);
	pthread_cond_signal (&fifo->cond);
	if (pthread_mutex_unlock (&fifo->mutex))
		return (REPLY_ERROR);
#endif
	return (REPLY_OK);
}

//--- rx_fifo_get -------------------------------------------------------------
int rx_fifo_get (HANDLE handle, void** buffer, int* len, int timeout)
{
	int ret;
	SHFifo *fifo = (SHFifo*)handle;

	if ((!handle) || (!buffer) || (!len) || (timeout < 0) )
		return (REPLY_ERROR);
	*buffer = NULL;
	*len = 0;

#ifdef WIN32
	EnterCriticalSection(&fifo->mutex);
	while (fifo->count == 0) {
		LeaveCriticalSection(&fifo->mutex);
		ret = WaitForSingleObjectEx(fifo->cond, timeout == 0 ? INFINITE : timeout, 0);
		if (ret == -1)
			return (REPLY_ERROR);
		EnterCriticalSection(&fifo->mutex);
		if (ret == WAIT_TIMEOUT)
			break;
		if (fifo->count == 0)
			ResetEvent(fifo->cond);
	}
#endif
#ifdef linux
	if (pthread_mutex_lock(&fifo->mutex))
		return (REPLY_ERROR);
	pthread_cleanup_push(&i_cleanup_push, fifo);
	while (fifo->count == 0) {
		if (timeout == 0) {
			pthread_cond_wait(&fifo->cond, &fifo->mutex);
		}
		else {
			struct timespec ts;
			clock_gettime(CLOCK_REALTIME, &ts);
			ts.tv_sec += timeout / 1000;
			ts.tv_nsec += (timeout % 1000) * 1000 * 1000;
			ret = pthread_cond_timedwait(&fifo->cond, &fifo->mutex, &ts);
			if (ret == ETIMEDOUT)
				break;
		}
	}
#endif

	if (fifo->count > 0) {
		fifo->count--;
		*buffer = fifo->elems [fifo->bottom];
		*len = fifo->len [fifo->bottom];
		fifo->bottom++;
		if (fifo->bottom >= fifo->size)
			fifo->bottom = 0;
	}

#ifdef WIN32
	if (fifo->count == 0)		
		ResetEvent (fifo->cond);
	else
		SetEvent (fifo->cond);
	LeaveCriticalSection (&fifo->mutex);
#endif
#ifdef linux
	pthread_cleanup_pop(1);
	pthread_cond_signal(&fifo->cond);
	if (pthread_mutex_unlock(&fifo->mutex))
		return (REPLY_ERROR);
#endif
	return (REPLY_OK);
}

//--- rx_fifo_count -----------------------------------------------------------
int rx_fifo_count (HANDLE handle)
{
	int count;
	SHFifo *fifo = (SHFifo*)handle;

	if (!handle)
		return -1;
#ifdef WIN32
	EnterCriticalSection (&fifo->mutex);
	count = fifo->count;
	LeaveCriticalSection (&fifo->mutex);
#endif
#ifdef linux
	if (pthread_mutex_lock(&fifo->mutex))
		return -1;
	pthread_cleanup_push(&i_cleanup_push, fifo);
	count = fifo->count;
	pthread_cleanup_pop(1);
	if (pthread_mutex_unlock(&fifo->mutex))
		return -1;
#endif
	return count;
}

//--- rx_fifo_remain ----------------------------------------------------------
int rx_fifo_remain (HANDLE handle)
{
	int remain;
	SHFifo *fifo = (SHFifo*)handle;

	if (!handle)
		return -1;
#ifdef WIN32
	EnterCriticalSection (&fifo->mutex);
	remain = fifo->size - fifo->count;
	LeaveCriticalSection (&fifo->mutex);
#endif
#ifdef linux
	if (pthread_mutex_lock(&fifo->mutex))
		return -1;
	pthread_cleanup_push(&i_cleanup_push, fifo);
	remain = fifo->size - fifo->count;
	pthread_cleanup_pop(1);
	if (pthread_mutex_unlock(&fifo->mutex))
		return -1;
#endif
	return remain;
}

//*****************************************************************************
//*							P R I V A T E
//*****************************************************************************
#ifdef linux
//--- i_cleanup_push ----------------------------------------------------------
void i_cleanup_push (void *para)
{
	SHFifo *fifo = (SHFifo*)para;
	pthread_mutex_unlock (&fifo->mutex);	
}
#endif
