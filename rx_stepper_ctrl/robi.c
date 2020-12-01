#include "stdint.h"
#include "termios.h"
#include "comm.h"

#include "rx_common.h"
#include "rx_threads.h"
#include "rx_error.h"
#include "robi_def.h"
#include "robi.h"

#define ROBI_SOFTWARE_FILENAME      ("/opt/radex/bin/stepper/robi.bin")

#define ROBI_SERIAL_PORT            "/dev/ttyS0"

#define ROBI_MIN_MSG_ID             1

#define ROBI_FIFO_SIZE              32

#define ROBI_STATUS_UPDATE_INTERVAL 250
#define ROBI_CONNECTION_TIMEOUT     1000

#define ROBI_MAX_MSG_ID             UINT32_MAX

#define UPDATE_FIFO_THRESHOLD       4

// define inputs
#define SCREW_IN_REF                0
#define SCREW_IN_DOWN               1
#define SCREW_IN_UP                 2
#define Y_IN_FRONT                  3
#define Y_IN_REF                    4
#define X_IN_REF                    5

static int32_t send_command(uint32_t commandCode, uint8_t len, void *data);
static void *receive_thread(void *par);
static void *send_thread(void *par);
static int32_t set_serial_attributs(int fd, int speed, int parity);
static void *update_thread(void *par);
static void update_failed(int fd);

static int _usbPort;

static uint32_t _tStatus;
static int32_t _isInit;
static int32_t _isConnected;
static int32_t _isSync;
static uint32_t _msgId;
static uint32_t _msgSentCounter;
static uint32_t _msgReceivedCounter;
static int32_t _isUpdating;
static int32_t _updateFailed;
static uint32_t _lastMessageTimestamp;
static uint32_t _syncMessageId;
static uint32_t _MsgsSent;

static int _ScrewerStalled = FALSE;

volatile static SUsbTxMsg _txFifo[ROBI_FIFO_SIZE];
volatile static int32_t _txFifoInIndex;
volatile static int32_t _txFifoOutIndex;

static uint32_t _currentVersion;

static HANDLE _sendLock;

//--- robi_init ----------------------------------------------------------------
void robi_init(void)
{
    if (_isInit) return;

    _tStatus = 0;

    _isConnected = FALSE;
    _isSync = FALSE;

    _usbPort = open(ROBI_SERIAL_PORT, O_RDWR | O_NOCTTY | O_SYNC);
    set_serial_attributs(_usbPort, B115200, 0);

    _msgId = ROBI_MIN_MSG_ID;
    _msgSentCounter = _msgId;
    _msgReceivedCounter = _msgSentCounter;

    memset(_txFifo, 0, sizeof(_txFifo));
    _txFifoInIndex = 0;
    _txFifoOutIndex = 0;

    memset(&RX_RobiStatus, 0, sizeof(RX_RobiStatus));

    if (access(ROBI_SOFTWARE_FILENAME, R_OK) == 0)
    {
        int fd = open(ROBI_SOFTWARE_FILENAME, O_RDONLY);

        if (fd == -1)
        {
            _currentVersion = 0;
        }
        else
        {
            lseek(fd, -sizeof(_currentVersion), SEEK_END);
            read(fd, &_currentVersion, sizeof(_currentVersion));
            close(fd);
        }
    }
    else
    {
        _currentVersion = 0;
    }

    _isUpdating = FALSE;
    _updateFailed = FALSE;

    _sendLock = rx_mutex_create();

    _isInit = TRUE;

    rx_thread_start(send_thread, NULL, 0, "robi_send_thread");
    rx_thread_start(receive_thread, NULL, 0, "robi_receive_thread");
    return;
}

//--- robi_main -------------------------------------------------------------------------
void robi_main(int ticks, int menu)
{
    if (_isInit == FALSE) return;
    
    if (ticks > _tStatus)
	{
		send_command(STATUS_UPDATE, 0, NULL);
		_tStatus = ROBI_STATUS_UPDATE_INTERVAL*(1 + ticks / ROBI_STATUS_UPDATE_INTERVAL); 
	}

    if (ticks > (_lastMessageTimestamp + ROBI_CONNECTION_TIMEOUT))
        _isConnected = FALSE;
}

//--- robi_connected ------------------------------------------------------------------
int robi_connected(void)
{
    return _isConnected;
}

//--- robi_current_version ----------------------------------------------------------------
int robi_current_version(void)
{
    return _currentVersion;
}

//--- robi_is_updating -------------------------------------------------------------------------
int robi_is_updating(void)
{
    return _isUpdating;
}

//--- robi_is_init ------------------------------------------------------------------------
int robi_is_init(void)
{
    return _isInit;
}

int robi_in_ref(void)
{
    return RX_RobiStatus.gpio.inputs & (1UL << Y_IN_REF);
}

//--- robi_stop ------------------------------------------------------------------------
void robi_stop(void)
{
    send_command(MOTOR_ESTOP, 0, NULL);
}

//--- robi_reference ----------------------------------------------------------------------
void robi_reference(void)
{
    send_command(MOTORS_DO_REFERENCE, 0, NULL);
}

//--- robi_move_x_relative_steps ---------------------------------------------------------
void robi_move_x_relative_steps(INT32 steps)
{
    send_command(MOTOR_MOVE_X_RELATIVE, sizeof(steps), &steps);
}
//--- robi_move_Y_relative_steps ----------------------------------------------------
void robi_move_Y_relative_steps(INT32 steps)
{
    send_command(MOTOR_MOVE_Y_RELATIVE, sizeof(steps), &steps);
}

//--- robi_move_up -----------------------------------------------------------------------
void robi_move_up(void)
{
    send_command(MOTOR_MOVE_Z_UP, 0, NULL);
}

//--- robi_move_down ----------------------------------------------------
void robi_move_down(void)
{
    send_command(MOTOR_MOVE_Z_DOWN, 0, NULL);
}

//--- robi_turn_screw_relative ------------------------------------------------
void robi_turn_screw_relative(INT32 steps)
{
    send_command(MOTOR_TURN_SCREW_RELATIVE, sizeof(steps), &steps);
}

//--- robi_turn_screw_left ---------------------------------------------------------
void robi_turn_screw_left(INT32 ticks)
{
    send_command(MOTOR_TURN_SCREW_LEFT, sizeof(ticks), &ticks);
}

//--- robi_turn_screw_right ---------------------------------------------------------
void robi_turn_screw_right(INT32 ticks)
{
    send_command(MOTOR_TURN_SCREW_RIGHT, sizeof(ticks), &ticks);
}

//--- robi_set_screw_current --------------------------------------------------------
void robi_set_screw_current(uint8_t current)
{
    send_command(MOTOR_SET_SCREW_CURRENT, sizeof(current), &current);
}

//--- robi_set_output ---------------------------------------------------------------
void robi_set_output(int num, int val)
{
    uint8_t gpioData[] = {num, val};

    send_command(GPIO_SET_OUTPUT, sizeof(gpioData), &gpioData);
}

//--- robi_screwer_stalled ---------------------------------------------------------------
int robi_screwer_stalled(void)
{
    int tmp = _ScrewerStalled;
    _ScrewerStalled = FALSE;
    return tmp;
}

//--- robi_move_done ----------------------------------------------------------------------
int robi_move_done(void)
{
    if (RX_RobiStatus.commandRunning[COMMAND0] || RX_RobiStatus.commandRunning[COMMAND1])
        return FALSE;
    else
        return TRUE;
}

//--- send_command ------------------------------------------------------
static int32_t send_command(uint32_t commandCode, uint8_t len, void *data)
{
    SUsbTxMsg *pTxMessage;
    int i;

    rx_mutex_lock(_sendLock);

    int32_t nextFifoIndex = (_txFifoInIndex + 1) % ROBI_FIFO_SIZE;
    if (nextFifoIndex == _txFifoOutIndex)
    {
        rx_mutex_unlock(_sendLock);
        return REPLY_ERROR;
    }

    _isSync = FALSE;
    _syncMessageId = _msgId;

    pTxMessage = &_txFifo[_txFifoInIndex];
    pTxMessage->command = commandCode;
    pTxMessage->id = _msgId++;
    pTxMessage->length = len;
    if (data != NULL && len != 0) memcpy(pTxMessage->data, data, len);

    if (_msgId >= ROBI_MAX_MSG_ID) _msgId = ROBI_MIN_MSG_ID;

    _txFifoInIndex = nextFifoIndex;

    rx_mutex_unlock(_sendLock);

    return REPLY_OK;
}

//--- receive_thread ------------------------------------------------
static void* receive_thread(void *par)
{
	int32_t count;
	int32_t length;
	uint8_t buffer[2048];
	uint32_t bytesRead;
	SUsbRxMsg rxMessage;

    while (_isInit)
	{
		bytesRead = read(_usbPort, buffer, sizeof(buffer));
		
		for (count = 0; count < bytesRead; count++)
		{
			if (comm_received(0, buffer[count]))
			{
				if (comm_get_data(0, (BYTE *)&rxMessage, sizeof(rxMessage), &length))
				{
					if (length == sizeof(rxMessage) - sizeof(rxMessage.data) + rxMessage.length)
					{
						_msgReceivedCounter++;
                        
						if (_isConnected == FALSE)
							_isConnected = TRUE;
					
						_lastMessageTimestamp = rx_get_ticks();
					
						if (_syncMessageId == rxMessage.id)
							_isSync = TRUE;

                        memcpy(&RX_RobiStatus, &rxMessage.robi, sizeof(RX_RobiStatus));

                        if (_isUpdating == FALSE && _updateFailed == FALSE && RX_RobiStatus.version < _currentVersion)
                        {
                            _isUpdating = TRUE;
                            rx_thread_start(update_thread, NULL, 0, "robi_update_thread");
                        }

                        if (rxMessage.error)
                        {
                            if (rxMessage.length)
                            {
                                if (rxMessage.error == MOTOR_STALLED && RX_RobiStatus.motors[MOTOR_SCREW].isStalled)
                                        _ScrewerStalled = TRUE;
                                
                                Error(ERR_CONT, 0, "Robi Error. Flag: %x, Message: %s", rxMessage.error, rxMessage.data);
                                memset(rxMessage.data, 0x00, sizeof(rxMessage.data));
                            }
                            else
                            {
                                Error(ERR_CONT, 0, "Robi Error. Flag: %x", rxMessage.error);
                            }
                        }
                    }
                    else
                    {
                        Error(ERR_CONT, 0, "Received invalid message lenght from Robi-Board");
                    }
				}
			}
		}
	}
}

//--- send_thread -----------------------------------------------------
static void *send_thread(void *par)
{
    int32_t length;
    int32_t bufferLength;
    int32_t pendingBytes;
    uint8_t buffer[512];
    SUsbTxMsg *pTxMessage;

    while (_isInit)
    {
        while (_txFifoOutIndex != _txFifoInIndex)
        {
            pTxMessage = &_txFifo[_txFifoOutIndex];

            length = sizeof(SUsbTxMsg) - sizeof(pTxMessage->data) + pTxMessage->length;
            comm_encode(pTxMessage, length, buffer, sizeof(buffer), &bufferLength);

            write(_usbPort, buffer, bufferLength);
            if (tcdrain(_usbPort) != 0)
            {
                rx_sleep(200);
            }
            _msgSentCounter++;
            if (pTxMessage->command != STATUS_UPDATE) _MsgsSent++;

            _txFifoOutIndex = (_txFifoOutIndex + 1) % ROBI_FIFO_SIZE;
            rx_sleep(100);
        }
    }
}

//--- set_serial_attributs -------------------------------------------------
static int32_t set_serial_attributs(int fd, int speed, int parity)
{
    struct termios tty;
    if (tcgetattr(fd, &tty) != REPLY_OK) return REPLY_ERROR;

    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;

    tty.c_iflag &= ~IGNBRK; // disable break processing
    tty.c_lflag = 0;     // no signaling chars, no echo, no canonical processing
    tty.c_oflag = 0;     // no remapping, no delays
    tty.c_cc[VMIN] = 0;  // read doesn't block
    tty.c_cc[VTIME] = 0; // 0.5 seconds read timeout
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl
    tty.c_cflag |= (CLOCAL | CREAD);        // ignore modem controls,
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;
    if (tcsetattr(fd, TCSANOW, &tty) != REPLY_OK) return REPLY_ERROR;

    return REPLY_OK;
}

//--- get_output_fifo_capacity ---------------------------------------------------
static int32_t get_output_fifo_capacity(void)
{
    if (_txFifoOutIndex > _txFifoInIndex)
    {
        return ROBI_FIFO_SIZE - (ROBI_FIFO_SIZE - _txFifoOutIndex + _txFifoInIndex);
    }

    return ROBI_FIFO_SIZE - (_txFifoInIndex - _txFifoOutIndex);
}

//--- update_thread ------------------------------------------------------------
static void* update_thread(void *par)
{
    int fd = open(ROBI_SOFTWARE_FILENAME, O_RDONLY);

    if (fd != -1)
    {
        int32_t error = REPLY_OK;
        uint8_t *dataBuffer = 0;
        uint8_t *currentData = 0;
        uint32_t size = 0, size_read = 0, available_data = 0, total_data = 0;
        
        // Get File Size
        size = lseek(fd, 0L, SEEK_END);
        lseek(fd, 0L, SEEK_SET);

        // Allocate buffer
        dataBuffer = (uint8_t *)malloc(size);

        // Read file content (last 4 bytes are the version string, don't read that)
        size_read = read(fd, dataBuffer, size);

        send_command(BOOTLOADER_INIT_CMD, sizeof(size_read), &size_read);

        while (RX_RobiStatus.bootloaderStatus != WAITING_FOR_DATA)
        {
            rx_sleep(100);
        }

        currentData = dataBuffer;
        total_data = dataBuffer + size_read;
        while (currentData < total_data)
        {
            while (get_output_fifo_capacity() <= UPDATE_FIFO_THRESHOLD)
            {
                rx_sleep(25);
            }
            
            available_data = total_data - (uint32_t)currentData;

            if (available_data > 64)
            {
                if (REPLY_OK == send_command(BOOTLOADER_WRITE_DATA_CMD, 64, currentData))
                {
                    currentData += 64;
                }
            }
            else
            {
                if (REPLY_OK == send_command(BOOTLOADER_WRITE_DATA_CMD, available_data, currentData))
                {
                    currentData += available_data;
                }
            }

            if (RX_RobiStatus.bootloaderStatus != WAITING_FOR_DATA)
            {
                update_failed(fd);
            }
        }

        while (RX_RobiStatus.bootloaderStatus != WAITING_FOR_CONFIRM)
        {
            if (RX_RobiStatus.bootloaderStatus == UNINITIALIZED)
            {
                update_failed(fd);
            }
            rx_sleep(100);
            if (get_output_fifo_capacity() == 0)
            {
                rx_sleep(100);
            }
        }


        send_command(BOOTLOADER_CONFIRM_CMD, 0, 0);

        while (RX_RobiStatus.bootloaderStatus != UNINITIALIZED)
        {
            rx_sleep(100);
        }
        
        _isUpdating = FALSE;
         close(fd);
    }
}

//--- update_failed ------------------------------------------------------
static void update_failed(int fd)
{
    rx_sleep(100);
    send_command(BOOTLOADER_RESET_CMD, 0, 0);
    _currentVersion = 0;
    _isUpdating = FALSE;
    _updateFailed = TRUE;
    close(fd);
}

//--- robi_error -----------------------------------------------------------
int robi_error()
{
    int i;
    for (i = 0; i < MOTOR_COUNT; i++)
    {
        if (RX_RobiStatus.motors[i].isStalled || RX_RobiStatus.motors[i].timeout)
            return TRUE;
    }
    return FALSE;
}

