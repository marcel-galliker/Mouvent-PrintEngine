/*******************************************************************************
* Copyright (C) 2013 Spansion LLC. All Rights Reserved. 
*
* This software is owned and published by: 
* Spansion LLC, 915 DeGuigne Dr. Sunnyvale, CA  94088-3453 ("Spansion").
*
* BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND 
* BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
*
* This software contains source code for use with Spansion 
* components. This software is licensed by Spansion to be adapted only 
* for use in systems utilizing Spansion components. Spansion shall not be 
* responsible for misuse or illegal use of this software for devices not 
* supported herein.  Spansion is providing this software "AS IS" and will 
* not be responsible for issues arising from incorrect user implementation 
* of the software.  
*
* SPANSION MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE,
* REGARDING THE SOFTWARE (INCLUDING ANY ACOOMPANYING WRITTEN MATERIALS), 
* ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED USE, INCLUDING, 
* WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, THE IMPLIED 
* WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE OR USE, AND THE IMPLIED 
* WARRANTY OF NONINFRINGEMENT.  
* SPANSION SHALL HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT, 
* NEGLIGENCE OR OTHERWISE) FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT 
* LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, 
* LOSS OF BUSINESS INFORMATION, OR OTHER PECUNIARY LOSS) ARISING FROM USE OR 
* INABILITY TO USE THE SOFTWARE, INCLUDING, WITHOUT LIMITATION, ANY DIRECT, 
* INDIRECT, INCIDENTAL, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA, 
* SAVINGS OR PROFITS, 
* EVEN IF SPANSION HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES. 
* YOU ASSUME ALL RESPONSIBILITIES FOR SELECTION OF THE SOFTWARE TO ACHIEVE YOUR
* INTENDED RESULTS, AND FOR THE INSTALLATION OF, USE OF, AND RESULTS OBTAINED 
* FROM, THE SOFTWARE.  
*
* This software may be replicated in part or whole for the licensed use, 
* with the restriction that this Disclaimer and Copyright notice must be 
* included with each copy of this software, whether used in part or whole, 
* at all times.  
*/
/******************************************************************************/
/** \file usbhost.h
 **
 ** A detailed description is available at 
 ** @link UsbHostGroup USB Host Module description @endlink
 **
 ** History:
 **   - 2012-08-28  2.0  MSc  First version (starting at version 2.0).
 **   - 2012-11-13  2.1  MSc  Some problems while switching from Host to Device fixed
 **   - 2013-01-28  2.2  MSc  Better Device Detection
 **   - 2013-06-04  2.3  MSc  FM4 support added
 **   - 2014-02-28  2.4  MSc  Speedup in package handling, fixes in fifo initialization
 **                           ISO transfers added
 **   - 2014-05-06  2.5  MSc  Fixing some variable initializations in disconnect routine
 **                           Added USBHOST_CONNECTION_DELAY to specify a delay in interations 
 **                           for bus-reset / connection delay
 **
 ******************************************************************************/

#ifndef __USBHOST_H__
#define __USBHOST_H__

/**
 ******************************************************************************
 ** \defgroup UsbHostGroup USB Host Mode Functionality
 **
 ** Provided functions of USB module:
 ** 
 ** - UsbHost_AddHostEndpoint()
 ** - UsbHost_EndpointFromNumber()
 ** - UsbHost_AddressDevice()
 ** - UsbHost_ReceiveSetupData()
 ** - UsbHost_SetupRequestWithData()
 ** - UsbHost_SetupRequest()
 ** - UsbHost_ConnectionCallback()
 ** - UsbHost_ClearAllEndpoints()
 ** - UsbHost_DisconnectionCallback()
 ** - UsbHost_CompletionCallback()
 ** - UsbHost_SofCallback()
 ** - UsbHost_BusresetCallback()
 ** - UsbHost_InitCallback()
 ** - UsbHost_TransferDataToFifo()
 ** - UsbHost_TransferFifoToBuffer()
 ** - UsbHost_TransferData()
 ** - UsbHost_Init()
 ** - UsbHost_GetDeviceStatus()
 ** - UsbHost_SetConfigurationDescriptor()
 ** - UsbHost_GetUsbEndpointDescriptor()
 **
 ** Used to transfer data with EP0..EP15, to do enumeration process and support different 
 ** USB class drivers via UsbClassSupervisor.
 **
 ******************************************************************************/
//@{

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/

#include "usb.h"

#if (FM_PERIPHERAL_USB_HOST_ENABLED == ON)

#define USBHOST_CONNECTION_DELAY 200000
#define MAX_HOST_ENDPOINTS 6
// Token mask in the HTOKEN register
#define HTOKEN_SETUP 						0x10
#define HTOKEN_IN    						0x20
#define HTOKEN_OUT   						0x30
#define HTOKEN_SOF   						0x40
#define HTOKEN_ISO_IN    					0x50
#define HTOKEN_ISO_OUT   					0x60
  
#define CREATE_TOKEN(toggle, type, endpoint)  ((toggle) << 7) | type | endpoint

#define HERR_SOF_MASK  						0x80 
#define HERR_RCV_MASK  						0x40
#define HERR_TOUT_MASK 						0x20
#define HERR_CRC_MASK  						0x10
#define HERR_TOGGLE_MASK					0x8
#define HERR_BITSTUFF_MASK				        0x4

#define USBHOST_SCHEDULER_COMPLETION  0x01
#define USBHOST_SCHEDULER_SOF  0x02
#define USBHOST_SCHEDULER_IDLE 0x03
#define USBHOST_SCHEDULER_BUSY 0x04

#define USBHOST_SUCCESS 0
#define USBHOST_ERROR   1

#define USBHOST_ENDPOINTSTATUS_INITIATE    0x01
#define USBHOST_ENDPOINTSTATUS_ACTIVE      0x02
#define USBHOST_ENDPOINTSTATUS_INUSE       0x04
#define USBHOST_ENDPOINTSTATUS_DATA        0x08
#define USBHOST_ENDPOINTSTATUS_STALL       0x10

#define USBHOST_EPADDR0_OUT    0x00
#define USBHOST_EPADDR0_IN     0x80


#define HERR_ACK          				      0
#define HERR_NAK          				    0x01
#define HERR_STALL        				    0x02
#define HERR_NULL         				    0x03

#define BM_REQUESTTYPE_DIROUT    0x00
#define BM_REQUESTTYPE_DIRIN     0x80

#define BM_REQUESTTYPE_DIR       0x80
#define BM_REQUESTTYPE_TYPE      0x60
#define BM_REQUESTTYPE_RECEIVER  0x1F


#define USBHOST_STAGE_IDLE      1
#define USBHOST_STAGE_SETUP     2
#define USBHOST_STAGE_DATA      3
#define USBHOST_STAGE_STATUS    4
#define USBHOST_STAGE_HANDSHAKE 5

#define USBHOST_DEVICE_IDLE 1
#define USBHOST_DEVICE_ADDRESSING 2
#define USBHOST_DEVICE_ADDRESSED 4
#define USBHOST_DEVICE_ENUMERATED 8
#define USBHOST_DEVICE_CONFIGURATING 16
#define USBHOST_DEVICE_CONFIGURED 32

#ifndef IS_SET
    #define IS_SET(x,y)  ((x & y) > 0) 
#endif

#ifndef IS_CLEARED
    #define IS_CLEARED(x,y)  ((x & y) == 0) 
#endif
    
#ifndef SET_MASK
    #define SET_MASK(x,y)  x |= y 
#endif

#ifndef CLEAR_MASK
    #define CLEAR_MASK(x,y)  x &= ~y
#endif

#ifndef TOGGLE_MASK
    #define TOGGLE_MASK(x,y) SET_MASK(x,y); CLEAR_MASK(x,y) 
#endif
#define ISOUT(x)    (!ISIN(x))
#define ISIN(x)     ((Handler->u8Address) & 0x80)
     
#define USBHOST_RESETENDPOINTBUFFER(endpoint,buffer,datasize)  (endpoint->u32DataSize) = datasize;\
                                                               (endpoint->pu8Buffer) = buffer;\
                                                               (endpoint->pu8BufferPosNextPackage) = buffer;\
                                                               (endpoint->pu8BufferPos) = buffer;\
                                                               if (endpoint->u32BufferSize < datasize) (endpoint->u32BufferSize) = datasize

#define USB_DESCRIPTOR_LENGTH                    0x00
#define USB_DESCRIPTOR_DESCRIPTORTYPE            0x01
#define USB_INTERFACE_DESCRIPTOR_INTERFACENUMBER 0x03

#define USBH_STATUS_SUCCESS 0x0000
#define USBH_STATUS_ERROR   0x0001
#define USBH_STATUS_STALL   0x0008
#define USBH_STATUS_LENGTH  0x000D

#define  UsbHost_GetUsbInterfaceDescriptor(x,y,z)    UsbHost_GetUsbEndpointDescriptor(x,y,z, 0)

/*****************************************************************************/
/* Global type definitions ('typedef')                                       */
/*****************************************************************************/


/**
 ******************************************************************************
 ** \brief Host endpoint
 ** 
 ** Used to setup virtual endpoints
 **
 ******************************************************************************/
typedef struct stc_host_endpoint
{
    uint8_t*  pu8Buffer;                                       ///< Buffer for sending / receiving data
    uint8_t*  pu8BufferPos;                                    ///< Current woring position 
    uint8_t*  pu8BufferPosNextPackage;                         ///< Next working position
    uint32_t  u32DataSize;                                     ///< Size of data in buffer
    uint32_t  u32BufferSize;                                   ///< Size of buffer
    uint16_t  u16FifoSize;                                     ///< FIFO size of virtual endpoint
    void      (* CompletionHandler)(stc_usbn_t* pstcUsb);     ///< Data transferred callback handle
    boolean_t bToggle;                                         ///< Data toggle 
    uint8_t   u8LastToken;                                     ///< Last token via this endpoint
    volatile uint8_t   u8Status;                               ///< endpoint status
    volatile uint8_t   u8Stage;                                ///< endpoint stage
    uint8_t   u8Address;                                       ///< Endpoint address
    uint8_t   u8Interval;                                      ///< Endpoint interrupt value (for interrupt transfers)
    uint8_t   u8IntervalCount;                                 ///< Counter used for priority calulations
    uint8_t   u8Retries;                                       ///< Number of retries before abort the transfer
    boolean_t bAbortTransfer;                                  ///< Force abort transfer
        boolean_t bIsochronous;                                ///< TRUE, if type is isochronous
} stc_host_endpoint_t;

/**
 ******************************************************************************
 ** \brief Host internal data
 ** 
 ** used to store different values
 **
 ******************************************************************************/
typedef struct stc_usbhost_intern_data
{
    stc_usbn_endpoint_data_t* pstcUsbHalEndpointOUT;                     ///< Endpoint used for data OUT transfers
    stc_usbn_endpoint_data_t* pstcUsbHalEndpointIN;                      ///< Endpoint used for data IN transfers
    stc_host_endpoint_t* Ep0IN;                                          ///< Control endpoint IN
    stc_host_endpoint_t* Ep0OUT;                                         ///< Control endpoint OUT
    stc_host_endpoint_t* ActiveEndpoint;                                 ///< Current active endpoint
    uint8_t u8DeviceStatus;                                              ///< Status of connected device
    uint8_t u8DeviceAddress;                                             ///< Address of connected device
    uint8_t u8EnumerationCounter;                                        ///< Statemachine counter for initial enumeration process
    stc_usb_request_t stcLastSetup;                                      ///< Last sent setup request
    stc_usb_request_t stcLastSetupBeforeStall;                           ///< Last sent setup request before stall
    uint8_t* pu8SetupData;                                               ///< Additional data of setup request
    volatile uint32_t u32Timeout;                                        ///< Timout variable
    volatile uint32_t u32ConTimeout;                                     ///< Connection timout variable
    volatile uint32_t u32BusResetTimeout;                                ///< Busreset timout variable
    volatile uint32_t u32ConnectionDelay;                                ///< Connection delay variable
    uint8_t u8CurrentEndpointPosition;                                   ///< Current position in endpoint list
    uint8_t u8EndpointListEnd;                                           ///< Last element in endpoint list
    volatile boolean_t bSchedulerLock;                                   ///< Lock Scheduler
    volatile boolean_t bBusResetSend;                                    ///< Bus reset was executed (used to determine second connect after bus reset)
    stc_host_endpoint_t astcHostEndpoints[MAX_HOST_ENDPOINTS];           ///< Endpoint list
    uint8_t pu8Setup[8];                                                 ///< Setup buffer
    uint8_t u8SyncToken;                                                 ///< Sync Token
    uint8_t u8SetupDelay;                                                ///< Send delayed setup
    uint8_t u8LastToken;                                                 ///< Last token which was sent
    uint8_t u8SchedulerState;                                            ///< Status of scheduler
    stc_host_endpoint_t* pstcClearEndpoint;                              ///< Endpoint handle which was cleared
    volatile boolean_t bFullSpeed;                                       ///< Fullspeed mode?
    volatile uint32_t u32WaitForCompletionTimeout;                       ///< Wait for completion timeout
    volatile boolean_t bSofToken;                                        ///< Send token with SOF
    uint8_t   pu8GlobalBuffer[2000];                                      ///< Global buffer
    void (* StallCompletionHandler)(stc_usbn_t* pstcUsb);               ///< Stall callback handler
    void (* SetupCompletionHandler)(stc_usbn_t* pstcUsb, uint8_t* pu8Buffer, uint32_t u32DataSize); ///< Setup package finished callback handler
} stc_usbhost_intern_data_t;

/**
 ******************************************************************************
 ** \brief Host internal instance data
 ** 
 ** used to sync USB HAL with internal variables
 **
 ******************************************************************************/
typedef struct stc_usbhostn_instance_data
{
    volatile stc_usbn_t*  pstcInstance;  ///< pointer to registers of an instance
    stc_usbhost_intern_data_t stcInternData; ///< module internal data of instance
} stc_usbhostn_instance_data_t;








uint8_t UsbHost_AddHostEndpoint(stc_usbn_t* pstcUsb, stc_host_endpoint_t** pstcHandler, uint8_t u8EndpointAddress, uint16_t u16FifoSize, uint8_t u8Interval, void(* CompletionHandler)(stc_usbn_t* pstcUsb));

stc_host_endpoint_t* UsbHost_EndpointFromNumber(stc_usbn_t* pstcUsb, uint8_t u8EndpointNumber);

void UsbHost_AddressDevice(stc_usbn_t* pstcUsb, uint8_t u8NewAddress);

uint32_t UsbHost_ReceiveSetupData(stc_usbn_t* pstcUsb, uint8_t** pu8Buffer);

void UsbHost_SetupRequestWithData(stc_usbn_t* pstcUsb, stc_usb_request_t* pstcSetup, uint8_t* pu8Data, void(*SetupCompletion)(stc_usbn_t* pstcUsb, uint8_t* pu8Buffer, uint32_t u32DataSize));

void UsbHost_SetupRequest(stc_usbn_t* pstcUsb, stc_usb_request_t* pstcSetup, void(*SetupCompletion)(stc_usbn_t* pstcUsb, uint8_t* pu8Buffer, uint32_t u32DataSize));

void UsbHost_ConnectionCallback(stc_usbn_t* pstcUsb);

void UsbHost_ClearAllEndpoints(stc_usbn_t* pstcUsb);

void UsbHost_DisconnectionCallback(stc_usbn_t* pstcUsb);

void UsbHost_CompletionCallback(stc_usbn_t* pstcUsb);

void UsbHost_SofCallback(stc_usbn_t* pstcUsb);

void UsbHost_BusresetCallback(stc_usbn_t* pstcUsb);

void UsbHost_InitCallback(stc_usbn_t* pstcUsb,struct stc_usb_config* pstcConfig);

void UsbHost_TransferDataToFifo(stc_usbn_t* pstcUsb, uint8_t* pu8Buffer, uint16_t u16Size, uint16_t u16FifoSize);

uint16_t UsbHost_TransferFifoToBuffer(stc_usbn_t* pstcUsb, uint8_t* pu8Buffer, uint32_t u32MaxBufferSize);

void UsbHost_TransferData(stc_usbn_t* pstcUsb, stc_host_endpoint_t* Handler,uint8_t* pu8Buffer, uint32_t u32BufferSize, void (* CompletionHandler)(stc_usbn_t* pstcUsb));

void UsbHost_Init(stc_usbn_t* pstcUsb, stc_usb_config_t* pstcUsbConfig);

uint8_t UsbHost_GetDeviceStatus(stc_usbn_t* pstcUsb);

void UsbHost_ClearEndpoint(stc_usbn_t* pstcUsb, stc_host_endpoint_t* pstcEndpoint);

void UsbHost_AbortTransfer(stc_host_endpoint_t* pstcEndpoint);

void UsbHost_ResetToggle(stc_host_endpoint_t* pstcEndpoint);

void UsbHost_ClearEndpointCompletion(stc_usbn_t* pstcUsb, uint8_t* pu8Buffer, uint32_t u32DataSize);

void UsbHost_ClearEndpointPoll(stc_usbn_t* pstcUsb, stc_host_endpoint_t* pstcEndpoint);

void UsbHost_SetConfigurationDescriptor(stc_usbn_t* pstcUsb, uint8_t u8Configuration);

uint8_t* UsbHost_GetUsbEndpointDescriptor(uint8_t* pu8Buffer, uint16_t u16Size, uint8_t u8InterfaceNumber, uint8_t u8EndpointNumber);

void UsbHost_SchedulerInitiateEndpoint(stc_usbn_t* pstcUsb, stc_usbhost_intern_data_t* pstcUsbHostIntern, stc_host_endpoint_t* pstcEndpoint);

void UsbHost_Tick(void);

#ifdef __cplusplus
}
#endif


#endif /* (FM_PERIPHERAL_USB_HOST_ENABLED == ON) */
//@} // UsbHostGroup

#endif
