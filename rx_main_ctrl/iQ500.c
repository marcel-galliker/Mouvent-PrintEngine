#include "iQ500.h"
#include "rx_common.h"
#include "rx_sok.h"
#include "network.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "rx_error.h"
#include "rx_threads.h"
#ifdef linux
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include "coreremote.h"
#else
// Some mocking for Windows to allow a minimum of simulations
#define IQREMOTEHANDLE INT32
#define IqError INT32
typedef unsigned int uint32_t;
#define IQ_OK 0
typedef enum TStatesEnum
{
    STATE_OFF,
    STATE_INIT,
    STATE_IDLE,
    STATE_RM,
    STATE_PROD_LEARNING,
    STATE_PROD
} StatesEnum;
static IqError IqConnect(IQREMOTEHANDLE *handle, const char *ipAddrStr, INT32 timeout);
static IqError IqDisconnect(IQREMOTEHANDLE *handle);
static IqError IqGetState(IQREMOTEHANDLE handle, StatesEnum *state);
static IqError IqStartRM(IQREMOTEHANDLE handle);
static IqError IqStartProdLearning(IQREMOTEHANDLE handle, INT32 nbOfRef);
static IqError IqGetNbOfProdLearning(IQREMOTEHANDLE handle, uint32_t *formatSuccess, uint32_t *formatErrors);
static IqError IqStartProd(IQREMOTEHANDLE handle);
static void usleep(INT32 t) { Sleep(t/1000); }
#endif // linux

#define IQ500_UNINITIALIZED 0
#define IQ500_INITIALIZED 1
#define IQ500_CAMERA_UNUSED 2

#define IQ500_TRACE TRUE

typedef void (*TAsyncCallback)();
#define IQ500_MAX_ASYNC_CALLS 8

typedef struct
{
    TAsyncCallback callbacks[IQ500_MAX_ASYNC_CALLS];
    UINT32 firstIndex;
    UINT32 lastIndex;
    HANDLE sem;
} _SAsyncFifo;

typedef struct
{
    char ipAddrStr[32];
    IQREMOTEHANDLE handle;
    uint32_t timeout;
    int stopPrinting;
    int initialization_status;
    int enabled;
    INT32 distance_from_job_start; // in mm
    INT32 distance_from_last_start; // in mm
    INT32 start_distance_reached;
    INT32 stop_distance_reached;
    _SAsyncFifo async_fifo;
} _SiQ500Camera;

static _SiQ500Camera _IQ500Camera;

// Set the number of copies to do during the Production Learning.
// Currently, we have been told it is set to 6 but 3 could be
// sufficient.
static uint32_t nbOfRef = 6U;

static int is_initialized();
static int check_iq_error(IqError status, const char *fct_name, const char *caller_name);
static void start_learning_callback();
static void stop_inspection_callback();
static void *iq500_thread(void *lpParameter);
static BOOL async_call(TAsyncCallback callback);

//--- iq500_init ---------------------------------------------------------
void iq500_init()
{
    memset(&_IQ500Camera, 0, sizeof(_IQ500Camera));
    _IQ500Camera.initialization_status = IQ500_UNINITIALIZED;

    _IQ500Camera.async_fifo.sem = rx_sem_create();

    if (RX_Config.iQ500Cfg.hasInspectionCamera)
    {
        // rx_thread_start(iq500_thread, &_IQ500Camera, 0, "_iq500_thread");
    }
    else
    {
        Error(LOG, 0, "iq500 camera unused according to configuration.");
        _IQ500Camera.initialization_status = IQ500_CAMERA_UNUSED;
    }
}

//--- iq500_end ---------------------------------------------------------
void iq500_end()
{
    if (!is_initialized()) return;

    IqError status = IqDisconnect(&_IQ500Camera.handle);
    if (!check_iq_error(status, "IqDisconnect", __FUNCTION__)) return;

    _IQ500Camera.initialization_status = IQ500_UNINITIALIZED;
}

//--- iq500_start_printing ----------------------------------------------
void iq500_start_printing()
{
    if (!is_initialized()) return;
    
    _IQ500Camera.distance_from_job_start = 0;
    _IQ500Camera.distance_from_last_start = 0;
    _IQ500Camera.start_distance_reached = FALSE;
    _IQ500Camera.stop_distance_reached = FALSE;
    _IQ500Camera.stopPrinting = FALSE;

    // Get current IQ 500 state
    StatesEnum state;
    IqError status = IqGetState(_IQ500Camera.handle, &state);
    if (!check_iq_error(status, "IqGetState", __FUNCTION__)) return;

    // The operator must enable inspection by entering the PROD state before starting
    _IQ500Camera.enabled = (state == STATE_PROD);
}

void iq500_stop_printing()
{
    if (!is_initialized()) return;

    // Stop inspection, same as clicking "Stop inspection" on HMI
    _IQ500Camera.stopPrinting = TRUE;
    IqError status = IqStartRM(_IQ500Camera.handle);
    check_iq_error(status, "IqStartRM", __FUNCTION__);
}

//--- iq500_new_job ---------------------------------------------------
// Not useful for us. This fucntion is call when starting to print but not between jobs
void iq500_new_job()
{
}

//--- iq500_print_done ---------------------------------------------------------
// to be called from_rx_print_ctrl after a copy is printed
void iq500_print_done(SPrintQueueItem *item)
{
    if (!is_initialized()) return;

    if (!_IQ500Camera.enabled) return;
    
    // srcHeight is in um
    _IQ500Camera.distance_from_job_start += (INT32) (item->srcHeight / 1000.0);
    _IQ500Camera.distance_from_last_start += (INT32) (item->srcHeight / 1000.0);

    // Stop previous inspection when needed
    if (!_IQ500Camera.stop_distance_reached &&
        _IQ500Camera.distance_from_last_start >= (INT32) RX_Config.iQ500Cfg.distanceToCamera)
    {
        _IQ500Camera.stop_distance_reached = TRUE;        
        async_call(stop_inspection_callback);
    }

    // Start learning when needed
    if (!_IQ500Camera.start_distance_reached)
    {
        // Skip short jobs
        if (_IQ500Camera.distance_from_job_start >= (INT32) RX_Config.iQ500Cfg.distanceToCamera)
        {
            _IQ500Camera.start_distance_reached = TRUE;
            if (item->copies >= (INT32) RX_Config.iQ500Cfg.minNumberOfCopies && !_IQ500Camera.stopPrinting)
            {
                async_call(&start_learning_callback);
            }
        }
    }
}

//--- iq500_job_done ---------------------------------------------------------
void iq500_job_done()
{
    if (!is_initialized()) return;

    // Update all counters
    _IQ500Camera.distance_from_job_start = 0;
    if (_IQ500Camera.stop_distance_reached)
    {
        // Special cases to handle short jobs... Short jobs are skipped but
        // we need to stop inspection of the previous one at the right position.
        _IQ500Camera.distance_from_last_start = 0;
    }
    _IQ500Camera.start_distance_reached = FALSE;
    _IQ500Camera.stop_distance_reached = FALSE;
}

//--- iq500_abort ---------------------------------------------------------
// to be called when printing is aborted
void iq500_abort()
{
    iq500_stop_printing();
}

//--- start_learning_callback ---------------------------------------------
static void start_learning_callback()
{
    // Make sure to be in the ready state
    StatesEnum state;
    IqError status = IqGetState(_IQ500Camera.handle, &state);
    if (!check_iq_error(status, "IqGetState", __FUNCTION__)) return;

    if (state == STATE_PROD_LEARNING || state == STATE_PROD)
    {
        status = IqStartRM(_IQ500Camera.handle);
        if (!check_iq_error(status, "IqStartRM", __FUNCTION__)) return;
    }

    // Start Production Learning, same as clicking "Start Production Learning" on HMI
    TrPrintfL(IQ500_TRACE, "iQ500 Info(%s): Calling IqStartProdLearning.", __FUNCTION__);
    status = IqStartProdLearning(_IQ500Camera.handle, nbOfRef);
    // Here we do not return in case of error since we sometime receive an error 
    // when learning has actually started correctly (to be investigated)
    check_iq_error(status, "IqStartProdLearning", __FUNCTION__);

    uint32_t formatSuccess = 0U;
    uint32_t formatErrors = 0U;
    while (formatSuccess != nbOfRef)
    {
        status = IqGetNbOfProdLearning(_IQ500Camera.handle, &formatSuccess, &formatErrors);
        if (!check_iq_error(status, "IqGetNbOfProdLearning", __FUNCTION__)) return;
        if (formatErrors >= nbOfRef)
        {
            // Too much format in errors, most of the time due to invalid
            // alignment models.then iQ 500 cannot inspect.
            TrPrintfL(IQ500_TRACE, "iQ500 Error(%s): Too many format errors.", __FUNCTION__);
            return;
        }
        // Sleep for 100 milliseconds to avoid too much useless CPU usage
        usleep(100000);
    }

    // Start Production, same as clicking "Start Prod" on HMI. This call is
    // required to exit from "Production Learning" state
    TrPrintfL(IQ500_TRACE, "iQ500 Info(%s): Calling IqStartProd.", __FUNCTION__);
    status = IqStartProd(_IQ500Camera.handle);
    if (!check_iq_error(status, "IqStartProd", __FUNCTION__)) return;
}

//--- stop_inspection_callback ------------------------------------------------
static void stop_inspection_callback()
{
    TrPrintfL(IQ500_TRACE, "iQ500 Info(%s): Calling IqStartRM.", __FUNCTION__);
    IqError status = IqStartRM(_IQ500Camera.handle);
    if (!check_iq_error(status, "IqStartRM", __FUNCTION__)) return;
}

//--- is_initialized ---------------------------------------------
static int is_initialized()
{
    if (_IQ500Camera.initialization_status == IQ500_CAMERA_UNUSED)
    {
        return FALSE;
    }
    if (_IQ500Camera.initialization_status == IQ500_UNINITIALIZED)
    {
        return FALSE;
    }
    return TRUE;
}

//--- check_iq_error ---------------------------------------------
static int check_iq_error(IqError status, const char *fct_name, const char *caller_name)
{
    if (status != IQ_OK)
    {
        TrPrintfL(IQ500_TRACE, "iQ500 Error(%s): %s failed.", caller_name, fct_name);
        return FALSE;
    }
    return TRUE;
}

//--- iq500_thread ---------------------------------------------
static void *iq500_thread(void *lpParameter)
{
    IqError status;

    net_device_to_ipaddr(dev_iq500, 0, _IQ500Camera.ipAddrStr, SIZEOF(_IQ500Camera.ipAddrStr));
    _IQ500Camera.timeout = 60000U;

    do
    {
        status = IqConnect(&_IQ500Camera.handle, _IQ500Camera.ipAddrStr, _IQ500Camera.timeout);
        if (status != IQ_OK)
        {
            // If IQ_ERROR_RPC is received, the communication is broken or
            // server is not running
            Error(WARN, 0, "Failed to connect to iq500 camera.");
            rx_sleep(1000);
        }
    } 
    while (status != IQ_OK);

    _IQ500Camera.initialization_status = IQ500_INITIALIZED;
    Error(LOG, 0, "Successfully connected to iq500 camera.");

    // Manage queue of async calls
    while (TRUE) 
    {
        // Check if a call is waiting
        if (_IQ500Camera.async_fifo.firstIndex != _IQ500Camera.async_fifo.lastIndex) 
        {
            _IQ500Camera.async_fifo.callbacks[_IQ500Camera.async_fifo.firstIndex]();
            _IQ500Camera.async_fifo.firstIndex = (_IQ500Camera.async_fifo.firstIndex+1) % IQ500_MAX_ASYNC_CALLS;
        }
        else 
        {
            rx_sem_wait(_IQ500Camera.async_fifo.sem, 500);
        }
    }
}

static BOOL async_call(TAsyncCallback callback)
{
    // Check if fifo is full
    UINT32 newLastIndex = (_IQ500Camera.async_fifo.lastIndex+1) % IQ500_MAX_ASYNC_CALLS;
    if (_IQ500Camera.async_fifo.firstIndex == newLastIndex) return FALSE;

    // Queue call
    _IQ500Camera.async_fifo.callbacks[_IQ500Camera.async_fifo.lastIndex] = callback;
    _IQ500Camera.async_fifo.lastIndex = newLastIndex;
    rx_sem_post(_IQ500Camera.async_fifo.sem);
    return TRUE;
}

//----------------------------------------------------------------------
// Some mocking for Windows to allow a minimum of simulations
#ifndef linux
static IqError IqConnect(IQREMOTEHANDLE *handle, const char *ipAddrStr, INT32 timeout)
{
    return IQ_OK;
}

static IqError IqDisconnect(IQREMOTEHANDLE *handle)
{
    return IQ_OK;
}

static IqError IqGetState(IQREMOTEHANDLE handle, StatesEnum *state)
{
    *state = STATE_PROD;
    return IQ_OK;
}

static IqError IqStartRM(IQREMOTEHANDLE handle)
{
    return IQ_OK;
}

static IqError IqStartProdLearning(IQREMOTEHANDLE handle, INT32 nbOfRef)
{
    return IQ_OK;
}

static IqError IqGetNbOfProdLearning(IQREMOTEHANDLE handle, uint32_t *formatSuccess, uint32_t *formatErrors)
{
    *formatSuccess = nbOfRef;
    *formatErrors = 0;
    return IQ_OK;
}

static IqError IqStartProd(IQREMOTEHANDLE handle)
{
    return IQ_OK;
}

static void IqAsync(IQREMOTEHANDLE handle, void (*callback)(IQREMOTEHANDLE handle, void *param), INT32 *returnCode)
{
     callback(handle, returnCode);
}
#endif
