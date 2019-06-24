/**
  ******************************************************************************
  * @file    motor.h
  * @author  IPC Rennes
  * @version V1.3.0
  * @date    November 12, 2014
  * @brief   This file contains all the functions prototypes for motor drivers.   
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MOTOR_H
#define __MOTOR_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
   
/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */ 

/** @defgroup Motor
  * @{
  */
    
/** @defgroup Motor_Exported_Constants
  * @{
  */   
   
/// boolean for false condition 
#ifndef FALSE
#define FALSE (0)
#endif
/// boolean for true condition 
#ifndef TRUE
#define TRUE  (1)
#endif

   /**
  * @}
  */
   
/** @defgroup Motor_Exported_Types
  * @{
  */

   /** @defgroup Motor_Boolean_Type
  * @{
  */
///bool Type
typedef uint8_t  bool;
/**
  * @}
  */   
   
/** @defgroup Device_Direction_Options
  * @{
  */
/// Direction options
typedef enum {
  BACKWARD = 0,
  FORWARD = 1
} motorDir_t;
/**
  * @}
  */
  
/** @defgroup Device_Action_Options
  * @{
  */
/// Action options
typedef enum {
  ACTION_RESET = ((uint8_t)0x00),
  ACTION_COPY  = ((uint8_t)0x08)
} motorAction_t;
/**
  * @}
  */  

/** @defgroup Device_States
  * @{
  */
/// Device states
typedef enum {
  ACCELERATING = 0, 
  DECELERATING = 1, 
  STEADY = 2,
  INACTIVE= 3
} motorState_t;
/**
  * @}
  */   

/** @defgroup Device_Step_mode
  * @{
  */
 /// Stepping options 
typedef enum {
  STEP_MODE_FULL   = ((uint8_t)0x00), 
  STEP_MODE_HALF   = ((uint8_t)0x01),
  STEP_MODE_1_4    = ((uint8_t)0x02),
  STEP_MODE_1_8    = ((uint8_t)0x03),
  STEP_MODE_1_16   = ((uint8_t)0x04),
  STEP_MODE_1_32   = ((uint8_t)0x05),
  STEP_MODE_1_64   = ((uint8_t)0x06),
  STEP_MODE_1_128  = ((uint8_t)0x07)
} motorStepMode_t;
/**
  * @}
  */   


/** @defgroup Motor_Driver_Structure
  * @{
  */
/// Motor driver structure definition  
typedef struct
{
  /// Function pointer to Init
  void (*Init)(uint8_t);
  /// Function pointer to ReadID
  uint16_t (*ReadID)(void);
  /// Function pointer to AttachErrorHandler
  void(*AttachErrorHandler)(void (*callback)(uint16_t));
  /// Function pointer to AttachFlagInterrupt
  void (*AttachFlagInterrupt)(void (*callback)(void));
  /// Function pointer to AttachBusyInterrupt
  void (*AttachBusyInterrupt)(void (*callback)(void));
  /// Function pointer to FlagInterruptHandler
  void (*FlagInterruptHandler)(void);
  /// Function pointer to GetAcceleration
  uint16_t (*GetAcceleration)(uint8_t);
  /// Function pointer to GetCurrentSpeed
  uint16_t (*GetCurrentSpeed)(uint8_t);
  /// Function pointer to GetDeceleration
  uint16_t (*GetDeceleration)(uint8_t);
  /// Function pointer to GetDeviceState
  motorState_t(*GetDeviceState)(uint8_t);
  /// Function pointer to GetFwVersion
  uint8_t (*GetFwVersion)(void);
  /// Function pointer to GetMark
  int32_t (*GetMark)(uint8_t);
  /// Function pointer to GetMaxSpeed
  uint16_t (*GetMaxSpeed)(uint8_t);
  /// Function pointer to GetMinSpeed
  uint16_t (*GetMinSpeed)(uint8_t);
  /// Function pointer to GetPosition
  int32_t (*GetPosition)(uint8_t);
  /// Function pointer to GoHome
  void (*GoHome)(uint8_t);
  /// Function pointer to GoMark
  void (*GoMark)(uint8_t);
  /// Function pointer to GoTo
  void (*GoTo)(uint8_t, int32_t);
  /// Function pointer to HardStop
  void (*HardStop)(uint8_t);
  /// Function pointer to Move
  void (*Move)(uint8_t, motorDir_t, uint32_t );
  /// Function pointer to ResetAllDevices
  void (*ResetAllDevices)(void);
  /// Function pointer to Run
  void (*Run)(uint8_t, motorDir_t);
  /// Function pointer to SetAcceleration
  bool(*SetAcceleration)(uint8_t ,uint16_t );
  /// Function pointer to SetDeceleration
  bool(*SetDeceleration)(uint8_t , uint16_t );
  /// Function pointer to SetHome
  void (*SetHome)(uint8_t);
  /// Function pointer to SetMark
  void (*SetMark)(uint8_t);
  /// Function pointer to SetMaxSpeed
  bool (*SetMaxSpeed)(uint8_t, uint16_t );
  /// Function pointer to SetMinSpeed
  bool (*SetMinSpeed)(uint8_t, uint16_t );
  /// Function pointer to SoftStop
  bool (*SoftStop)(uint8_t);
  /// Function pointer to StepClockHandler
  void (*StepClockHandler)(uint8_t deviceId);
  /// Function pointer to WaitWhileActive
  void (*WaitWhileActive)(uint8_t);
  /// Function pointer to CmdDisable
  void (*CmdDisable)(uint8_t);
  /// Function pointer to CmdEnable
  void (*CmdEnable)(uint8_t);
  /// Function pointer to CmdGetParam
  uint32_t (*CmdGetParam)(uint8_t, uint32_t);
  /// Function pointer to CmdGetStatus
  uint16_t (*CmdGetStatus)(uint8_t);
  /// Function pointer to CmdNop
  void (*CmdNop)(uint8_t);
  /// Function pointer to CmdSetParam  
  void (*CmdSetParam)(uint8_t, uint32_t, uint32_t);
  /// Function pointer to ReadStatusRegister
  uint16_t (*ReadStatusRegister)(uint8_t);
  /// Function pointer to ReleaseReset
  void (*ReleaseReset)(void);
  /// Function pointer to Reset
  void (*Reset)(void);
  /// Function pointer to SelectStepMode
  void (*SelectStepMode)(uint8_t deviceId, motorStepMode_t);
  /// Function pointer to SetDirection
  void (*SetDirection)(uint8_t, motorDir_t);
  /// Function pointer to CmdGoToDir
  void (*CmdGoToDir)(uint8_t, motorDir_t, int32_t);
  /// Function pointer to CheckBusyHw
  uint8_t (*CheckBusyHw)(void);
  /// Function pointer to CheckStatusHw
  uint8_t (*CheckStatusHw)(void);
  /// Function pointer to CmdGoUntil
  void (*CmdGoUntil)(uint8_t, motorAction_t, motorDir_t, uint32_t);
  /// Function pointer to CmdHardHiZ
  void (*CmdHardHiZ)(uint8_t);
  /// Function pointer to CmdReleaseSw
  void (*CmdReleaseSw)(uint8_t, motorAction_t, motorDir_t);
  /// Function pointer to CmdResetDevice
  void (*CmdResetDevice)(uint8_t);
  /// Function pointer to CmdResetPos
  void (*CmdResetPos)(uint8_t);
  /// Function pointer to CmdRun
  void (*CmdRun)(uint8_t, motorDir_t, uint32_t);
  /// Function pointer to CmdSoftHiZ
  void (*CmdSoftHiZ)(uint8_t);
  /// Function pointer to CmdStepClock
  void (*CmdStepClock)(uint8_t, motorDir_t);
  /// Function pointer to FetchAndClearAllStatus
  void (*FetchAndClearAllStatus)(void);
  /// Function pointer to GetFetchedStatus
  uint16_t (*GetFetchedStatus)(uint8_t);
  /// Function pointer to GetNbDevices
  uint8_t (*GetNbDevices)(void);
  /// Function pointer to IsDeviceBusy
  bool (*IsDeviceBusy)(uint8_t);
  /// Function pointer to SendQueuedCommands
  void (*SendQueuedCommands)(void);
  /// Function pointer to QueueCommands
  void (*QueueCommands)(uint8_t, uint8_t, int32_t);
  /// Function pointer to WaitForAllDevicesNotBusy
  void (*WaitForAllDevicesNotBusy)(void);
  /// Function pointer to ErrorHandler
  void (*ErrorHandler)(uint16_t);
  /// Function pointer to BusyInterruptHandler
  void (*BusyInterruptHandler)(void);
  /// Function pointer to CmdSoftStop
  void (*CmdSoftStop)(uint8_t);
  /// Function pointer to StartStepClock
  void (*StartStepClock)(uint16_t);
  /// Function pointer to StopStepClock
  void (*StopStepClock)(void);
}motorDrv_t;

/**
  
* @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */
  
/**
  * @}
  */ 

/**
  * @}
  */ 

#ifdef __cplusplus
}
#endif

#endif /* __MOTOR_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
