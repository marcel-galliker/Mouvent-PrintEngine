/*
        *** THIS IS A FILE FOR DOXYGEN AND NOT A C HEADER FILE ***
*/

/**
 *******************************************************************************
 ** \mainpage PDL for FM4 - Documentation
 ** <hr>
 *******************************************************************************
 ** \image html Spansion_logo.png
 **
 ** Copyright (C) 2014 Spansion LLC. All Rights Reserved. 
 **                                                                                               
 ** This software is owned and published by: 
 ** Spansion LLC, 915 DeGuigne Dr. Sunnyvale, CA  94088-3453 ("Spansion").
 **                                                                                               
 ** BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND 
 ** BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
 **                                                                                               
 ** This software contains source code for use with Spansion 
 ** components. This software is licensed by Spansion to be adapted only 
 ** for use in systems utilizing Spansion components. Spansion shall not be 
 ** responsible for misuse or illegal use of this software for devices not 
 ** supported herein.  Spansion is providing this software "AS IS" and will 
 ** not be responsible for issues arising from incorrect user implementation 
 ** of the software.  
 **                                                                                               
 ** SPANSION MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE,
 ** REGARDING THE SOFTWARE (INCLUDING ANY ACOOMPANYING WRITTEN MATERIALS), 
 ** ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED USE, INCLUDING, 
 ** WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, THE IMPLIED 
 ** WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE OR USE, AND THE IMPLIED 
 ** WARRANTY OF NONINFRINGEMENT.  
 ** SPANSION SHALL HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT, 
 ** NEGLIGENCE OR OTHERWISE) FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT 
 ** LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, 
 ** LOSS OF BUSINESS INFORMATION, OR OTHER PECUNIARY LOSS) ARISING FROM USE OR 
 ** INABILITY TO USE THE SOFTWARE, INCLUDING, WITHOUT LIMITATION, ANY DIRECT, 
 ** INDIRECT, INCIDENTAL, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA, 
 ** SAVINGS OR PROFITS, 
 ** EVEN IF SPANSION HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES. 
 ** YOU ASSUME ALL RESPONSIBILITIES FOR SELECTION OF THE SOFTWARE TO ACHIEVE YOUR
 ** INTENDED RESULTS, AND FOR THE INSTALLATION OF, USE OF, AND RESULTS OBTAINED 
 ** FROM, THE SOFTWARE.  
 **                                                                                               
 ** This software may be replicated in part or whole for the licensed use, 
 ** with the restriction that this Disclaimer and Copyright notice must be 
 ** included with each copy of this software, whether used in part or whole, 
 ** at all times.   
 **
 ** <br>
 ** <hr>
 ** <b>History</b>
 **  - 2013-07-01
 **    - Version 0.0.1
 **    - Author: FSL, FLSA, FSEU
 **    - Comment: First alpha release version
 **
 **  - 2013-09-01
 **    - Version 0.0.2
 **    - Author: FSL, FLSA, FSEU
 **    - Comment: Beta-one release version
 **
 **  - 2013-10-31
 **    - Version 1.0
 **    - Author: Spansion Inc., Japan, China, Europe
 **    - Comment: Version 1.0 release
 **
 **  - 2014-01-27
 **    - Version 1.1
 **    - Author: Spansion Inc., Japan, China, Europe
 **    - Comment: Version 1.1 release
 **    - New modules:
 **      - sd.c, sd.h, sd_cfg.h, sdcard.c, sdcard.h, sdcmd.c, sdcmd.h
 **    - Enhancements:
 **      - rtc.c:                    bResetNoInit parameter for Rtc_Init() 
 **                                  introduced; Rtc_GetRunStatus() indruduced
 **      - mfs.c/.h:                 Mfs_ErrorClear() introduced for clearing 
 **                                  reception errors and FIFOs; Mfs_GetFifoBytes() 
 **                                  introduced for retrieving FIFO fill level
 **      - mcu.h:                    includes mb9abxxx.h
 **      - system_mb9abxxx.h:        Enable of Flash accelerator added
 **      - gpio.h                    New macros for parameterized port usage
 **      - gpio_parameterized_ports  New GPIO example
 **    - Bug fixes:
 **      - extif.c:           Timing constraints check introduced for TIM.WACC, TIM.WADC,
 **                           TIM.WWEC, TIM.RADC, and TIM.RACC
 **                           pstcConfig->enWriteAccessCycle configuration corrected
 **      - mft_icu.c:         MFT1 only usage corrected
 **      - adc.h:             Bug in priority conversion w/o interrupts corrected;
 **                           Usage of ch16-31 fixed; Fifio typo corrected to Fifo in 
 **                           Adc_ScanFifoStatus(), Adc_ScanFifoClear(),
 **                           Adc_PrioFifoStatus(), Adc_PrioFifoClear()
 **      - rtc.c/.h:          RTC_INTERRUPT_ENABLE_MASK corrected to 0xFF;
 **                           RTC interrupt enable/disable is used instead of global
 **                           interrupt enable/disable when using Rtc_SetDateTime()
 **      - can.c/.h:          Missing PDL definitions corrected
 **      - pdl_user.h:        PDL_INTERRUPT_ENABLE_CSV was defined two times
 **      - interrupts.c:      MFS1_TX_IRQHandler() now uses MfsInstanceIndexMfs1
 **                           instead of MfsInstanceIndexMfs0
 **
 **  - 2014-05-27
 **    - Version 1.2
 **    - Author: Spansion Inc., Japan, China, Europe
 **    - Comment: Version 1.2 release
 **    - Enhancements:
 **      - system_mb9abxxx.c/.h:  The Low-speed CR prescaler definition can be added.
 **      - mfs_hl.c/.h:           Return error when reception errors occurs in Mfs_Hl_Read();
 **                               Mfs_Hl_GetBufferPointer() added.
 **    - Bug fixes:
 **      - extint.c:              Channel enable bug in Exint_Init() fixed.
 **      - system_mb9abxxx.h:     CHECK_RSVD() macro fixed for HS-CR PLL usage.
 **      - USB general:           IRQ naming, mcu headerfile typedef fixes; IRQ double
 **                               definition with use of PDL > V1.0 solved
 **      - USB host:              Speedup in package handling; fixes in fifo initialization
 **                               ISO transfers added; Fixes in some variable initializations
 **                               in disconnect routine; PUSBHOST_CONNECTION_DELAY to specify
 **                               a delay in interations added for bus-reset/connection delay
 **
 ** <hr>
 ** 
 ** <b>Implementations in PDL:</b>
 ** <table border="0">
 ** <tr><td> ADC   </td><td>&nbsp;Analog Digital Cionverter                              </td></tr>
 ** <tr><td> BT    </td><td>&nbsp;Base Time                                              </td></tr>
 ** <tr><td> CAN   </td><td>&nbsp;Controller Area Network                                </td></tr>
 ** <tr><td> CLK   </td><td>&nbsp;Clock Module                                           </td></tr>
 ** <tr><td> CRC   </td><td>&nbsp;Cyclic Redundancy Check                                </td></tr>
 ** <tr><td> CR    </td><td>&nbsp;High-CR trimming                                       </td></tr>
 ** <tr><td> CSV   </td><td>&nbsp;Clock supervisor                                       </td></tr>
 ** <tr><td> DAC   </td><td>&nbsp;Digital Analog Converter                               </td></tr>
 ** <tr><td> DMA   </td><td>&nbsp;Direc Memory Access                                    </td></tr>
 ** <tr><td> DT    </td><td>&nbsp;Dual Timer                                             </td></tr>
 ** <tr><td> DTSC  </td><td>&nbsp;Descriptor System data Transfer Controller             </td></tr>
 ** <tr><td> EXINT </td><td>&nbsp;External Interrupt                                     </td></tr>
 ** <tr><td> EXTIF </td><td>&nbsp;External Bus Inerface                                  </td></tr>
 ** <tr><td> Flash </td><td>&nbsp;Main/Work Flash operation                              </td></tr>
 ** <tr><td> GPIO  </td><td>&nbsp;General Purpose Inoput/Output Ports (only header file) </td></tr>
 ** <tr><td> LPM   </td><td>&nbsp;Low Power Modes                                        </td></tr>
 ** <tr><td> LVD   </td><td>&nbsp;Low Voltage Detection                                  </td></tr>
 ** <tr><td> MFS   </td><td>&nbsp;Multi Function Serial Interface                        </td></tr>
 ** <tr><td> MFT   </td><td>&nbsp;Multi Function Timer                                   </td></tr>
 ** <tr><td> PPG   </td><td>&nbsp;Programmable Pulse Generator                           </td></tr>
 ** <tr><td> QPRC  </td><td>&nbsp;Quad Decoder and Position/Revolution Counter           </td></tr>
 ** <tr><td> RESET </td><td>&nbsp;Reset Cause Register                                   </td></tr>
 ** <tr><td> RTC   </td><td>&nbsp;Real Time Clock                                        </td></tr>
 ** <tr><td> SDIF  </td><td>&nbsp;SD Card Interface                                      </td></tr>
 ** <tr><td> UID   </td><td>&nbsp;Unique ID                                              </td></tr>
 ** <tr><td> USB   </td><td>&nbsp;Universal Serial Bus                                   </td></tr>
 ** <tr><td> WC    </td><td>&nbsp;Watch Counter                                          </td></tr>
 ** <tr><td> WDG   </td><td>&nbsp;Watch Dog Timer (Hardware/Software)                    </td></tr>
 ** </table>
 **
 ** <b>USB Middleware:</b>
 ** <table border="0">
 ** <tr><td> USB Device </td><td>&nbsp;&nbsp;CDC Com - Virtual Com Port  </td></tr>
 ** <tr><td> USB Device </td><td>&nbsp;&nbsp;HID Com - Data Communicaton </td></tr>
 ** <tr><td> USB Device </td><td>&nbsp;&nbsp;HID Joystick                </td></tr>
 ** <tr><td> USB Device </td><td>&nbsp;&nbsp;HID Keyboard                </td></tr>
 ** <tr><td> USB Device </td><td>&nbsp;&nbsp;HID Mouse                   </td></tr>
 ** <tr><td> USB Device </td><td>&nbsp;&nbsp;LibUSB                      </td></tr>
 ** <tr><td> USB Device </td><td>&nbsp;&nbsp;Printer Class               </td></tr>
 ** <tr><td> USB Host   </td><td>&nbsp;&nbsp;HID Com - Data Communicaton </td></tr>
 ** <tr><td> USB Host   </td><td>&nbsp;&nbsp;HID Keyboard                </td></tr>
 ** <tr><td> USB Host   </td><td>&nbsp;&nbsp;HID Mouse                   </td></tr>
 ** <tr><td> USB Host   </td><td>&nbsp;&nbsp;Mass Storage                </td></tr>
 ** </table>
 **
 ** <hr>
 **
 ** <b>Examples:</b>
 ** 
 ** - ADC
 **   - <pre>adc_scan_irq_bt       ADC scan sample with interrupt mode triggered by base timer</pre>
 **   - <pre>adc_scan_dma          Usage of ADC together with DMA in demand transfer mode</pre>
 **   - <pre>adc_scan_irq_sw       ADC scan conversion with usage of interrupt callback triggered by software</pre>
 **   - <pre>adc_scan_irq_mft      ADC scan conversion with usage of interrupt callback triggered by MFT</pre>
 **   - <pre>adc_scan_polling_sw   ADC scan conversion with polling for finished conversion triggered by software</pre>
 **   - <pre>adc_prio_irq_sw       ADC priority conversion with usage of interrupt callback triggered by software</pre>
 **   - <pre>adc_prio_polling_sw   ADC priority conversion with polling for finished conversion triggered by software</pre>
 **                       
 ** - BT
 **   - <pre>bt_iomode             Base Timer IO modes demonstration</pre>
 **   - <pre>bt_ppg                Base Timer configured as Programmable Pulse Generator</pre>
 **   - <pre>bt_pwc                Base Timer configured as Pulse Width Counter</pre>
 **   - <pre>bt_pwm                Base Timer configured as Pulse With Modulator</pre>
 **   - <pre>bt_rt                 Base Timer configured as Reload Timer</pre>
 **           
 ** - CAN 
 **   - <pre>can_simple       CAN communication with CANoe</pre>       
 **                      
 ** - CLK  
 **   - <pre>clk_gating       Clock gating setting</pre>
 **   - <pre>clk_init         User clock initialization</pre>
 ** 
 ** - CR
 **   - <pre>cr_trimming      CR trimming procedure demonstration</pre>
 **                      
 ** - CRC                
 **   - <pre>crc_16_32        Calculation of CRC-16 and CRC-32</pre>
 ** 
 ** - CSV
 **   - <pre>csv_reset        CSV reset demonstration when main/sub clock frequency is abnormal</pre>
 **   - <pre>fcs_int          FCS interrupt demonstration when main clock frequency is abnormal</pre>
 **   - <pre>fcs_reset        CSV reset demonstration when main clock frequency is abnormal</pre>
 **                
 ** - DAC                
 **   - <pre>dac_sine_wave    DAC channels generate sine wave tones</pre>
 **                      
 ** - DMA                
 **   - <pre>dma_software     The DMA perform a sotware block transfer (For demand transfer see ADC)</pre>
 ** 
 ** - DSTC
 **   - <pre>dstc_hw_transfer DSTC triggered by peripheral (BT0)</pre>
 **   - <pre>dstc_sw_transfer DSTC software transfer</pre>
 **                      
 ** - DT                 
 **   - <pre>dt_unuse_int     Dual Timer configured as periodic mode for ch.0 and one-shot mode for ch.1 with usage interruption callback</pre>
 **   - <pre>dt_use_int       Dual Timer configured as periodic mode for ch.0 and one-shot mode for ch.1 with polling for interruption</pre>
 ** 
 ** - DSTC
 **   - <pre>sw_transfer      Perform DSTC transfer by software trigger</pre>
 **   - <pre>hw_transfer      Perform DSTC transfer by hardware trigger</pre>
 ** 
 ** - EXINT
 **   - <pre>exint_simple     Simple external interrupt occurence demonstration</pre>
 ** 
 ** - EXTIF &nbsp;&nbsp;(no example)
 ** 
 ** - Flash
 **   - <pre>main_flash_test  Main Flash operation test</pre>
 **   - <pre>work_flash_test  Work Flash operation test</pre>
 **                      
 ** - GPIO
 **   - <pre>gpio_parameterized_ports  New macros with aibility of port parameterization</pre>           
 **   - <pre>gpio_ports                Simple Port setting and Port read-out</pre>
 ** 
 ** - LPM
 **   - <pre>lpm_bu_reg       Reading and writing to Backup Registers</pre>
 **   - <pre>lpm_sleep_stop   MCU is set to sleep and stop mode, EXINT0 wakes up MCU</pre>
 **                     
 ** - LVD                
 **   - <pre>lvd_unuse_int_27  LVD interrupts when voltage is vicinity of 2.7 volts. Interruption detection uses polling</pre>
 **   - <pre>lvd_unuse_int_28  LVD interrupts when voltage is vicinity of 2.8 volts. Interruption detection uses polling</pre>
 **   - <pre>lvd_unuse_int_30  LVD interrupts when voltage is vicinity of 3.0 volts. Interruption detection uses polling</pre>
 **   - <pre>lvd_use_int_26    LVD interrupts when voltage is vicinity of 2.6 volts. Interruption detection uses callback</pre>
 **   - <pre>lvd_use_int_28    LVD interrupts when voltage is vicinity of 2.8 volts. Interruption detection uses callback</pre>
 **   - <pre>lvd_use_int_32    LVD interrupts when voltage is vicinity of 3.2 volts. Interruption detection uses callback</pre>
 **                      
 ** - MFS                
 **   - High Level       &nbsp;&nbsp;The samples used High Level MFS driver.
 **     - CSIO
 **       - <pre>mfs_csio_normal_master_unuse_int           MFS performs as CSIO master mode by unusing interruption</pre>
 **       - <pre>mfs_csio_normal_master_unuse_int_with_tmr  MFS performs as CSIO master mode by unusing interruption and using timer mode</pre>
 **       - <pre>mfs_csio_normal_master_use_int             MFS performs as CSIO master mode by using interruption</pre>
 **       - <pre>mfs_csio_normal_master_use_int_with_tmr    MFS performs as CSIO master mode by using interruption and using timer mode</pre>
 **       - <pre>mfs_csio_normal_slave_use_int              MFS performs as CSIO slave mode by using interruption</pre>
 **       - <pre>mfs_csio_spi_master_unuse_int              MFS performs as SPI master mode by unusing interruption and controling CS by GPIO</pre>
 **       - <pre>mfs_csio_spi_master_unuse_int_with_cs      MFS performs as SPI master mode by unusing interruption and controling CS by peripheral</pre>
 **       - <pre>mfs_csio_spi_master_use_int                MFS performs as SPI master mode by using interruption and controling CS by GPIO</pre>
 **       - <pre>mfs_csio_spi_master_use_int_with_cs        MFS performs as SPI master mode by using interruption and controling CS by peripheral</pre>
 **       - <pre>mfs_csio_spi_slave_use_int                 MFS performs as SPI slave mode by using interruption and unusing CS peripheral</pre>
 **       - <pre>mfs_csio_spi_slave_use_int_with_cs         MFS performs as SPI slave mode by using interruption and using CS peripheral</pre>
 **     - LIN
 **       - <pre>mfs_lin                                    MFS performs as LIN master and slave mode communicating on same MCU</pre>
 **     - UART
 **       - <pre>mfs_uart_unuse_int                         MFS performs as UART normal mode by unusing interruption</pre>
 **       - <pre>mfs_uart_use_int                           MFS performs as UART normal mode by using interruption</pre>
 **   - Low Level        &nbsp;&nbsp;The samples used Low Level MFS driver.
 **     - CSIO
 **       - <pre>mfs_csio_normal_master_unuse_int           MFS performs as CSIO master mode by unusing interruption</pre>
 **       - <pre>mfs_csio_normal_master_unuse_int_with_tmr  MFS performs as CSIO master mode by unusing interruption and using timer mode</pre>
 **       - <pre>mfs_csio_normal_master_use_int             MFS performs as CSIO master mode by using interruption</pre>
 **       - <pre>mfs_csio_normal_master_use_int_with_tmr    MFS performs as CSIO master mode by using interruption and using timer mode</pre>
 **       - <pre>mfs_csio_normal_slave_use_int              MFS performs as CSIO slave mode by using interruption</pre>
 **       - <pre>mfs_csio_spi_master_unuse_int              MFS performs as SPI master mode by unusing interruption and controling CS by GPIO</pre>
 **       - <pre>mfs_csio_spi_master_unuse_int_with_cs      MFS performs as SPI master mode by unusing interruption and controling CS by peripheral</pre>
 **       - <pre>mfs_csio_spi_master_use_int                MFS performs as SPI master mode by using interruption and controling CS by GPIO</pre>
 **       - <pre>mfs_csio_spi_master_use_int_with_cs        MFS performs as SPI master mode by using interruption and controling CS by peripheral</pre>
 **       - <pre>mfs_csio_spi_slave_use_int                 MFS performs as SPI slave mode by using interruption and unusing CS peripheral</pre>
 **       - <pre>mfs_csio_spi_slave_use_int_with_cs         MFS performs as SPI slave mode by using interruption and using CS peripheral</pre>
 **     - LIN
 **       - <pre>mfs_lin                                    MFS performs as LIN master and slave mode communicating on same MCU</pre>
 **     - UART
 **       - <pre>mfs_uart_unuse_int                         MFS performs as UART normal mode by unusing interruption</pre>
 **       - <pre>mfs_uart_use_int                           MFS performs as UART normal mode by using interruption</pre>
 **                      
 ** - MFT                
 **   - FRT              
 **     - <pre>frt_int                  Free-run Timer function with usage of zero/peak match interrupt callback</pre>
 **     - <pre>frt_polling              Free-run Timer function with polling for zero/peak match</pre>
 **   - ICU                             
 **     - <pre>icu_int                  Input Captuer Unit function with usage of interrupt callback</pre>
 **     - <pre>icu_polling              Input Captuer Unit function with polling for capture completion</pre>
 **   - OCU                             
 **     - <pre>ocu_ch_linked            Perform 2 change linked mode of Output Compare Unit</pre>
 **     - <pre>ocu_ch_single            Perform 1-change/low-active/high-active mode of Output Compare Unit</pre>
 **   - WFG
 **     - <pre>wfg_nzcl_dtif_dttix_trig Perform DTIF interrupt triggered DTTIX</pre>
 **     - <pre>wfg_nzcl_dtif_sw_trig    Perform DTIF interrupt triggered software</pre>
 **     - <pre>wfg_ppg_dead_timer_mode  Perform PPG-dead timer mode of Waveform Generator Unit</pre>
 **     - <pre>wfg_rt_dead_timer_mode   Perform RT-dead timer mode of Waveform Generator Unit</pre>
 **     - <pre>wfg_rt_ppg_mode          Perform RT-PPG mode of Waveform Generator Unit</pre>
 **     - <pre>wfg_through_mode         Perform through mode of Waveform Generator Unit</pre>
 **     - <pre>wfg_timer_ppg_mode       Perform Timer-PPG mode of Waveform Generator Unit</pre>
 **                                     
 ** - QPRC                              
 **   - <pre>qprc_pc_directional        Perform Count mode with direction (mode 3) of Position Counter</pre>
 **   - <pre>qprc_pc_updown             Perform Up/down count mode (mode 1) of Position Counter</pre>
 **   - <pre>qprc_pc_phase_diff         Perform Phase different count mode (mode 2) of Position Counter</pre>
 **   - <pre>qprc_rc_pc_ofuf_trig       Perform Revolution Counter mode 2 (triggered by Position Counter)</pre>
 **   - <pre>qprc_rc_zin_pcofuf_trig    Perform Revolution Counter mode 3 (triggered by ZIN or Position Counter)</pre>
 **   - <pre>qprc_rc_zin_trig           Perform Revolution Counter mode 1 (triggered by ZIN)</pre>
 **          
 ** - RTC
 **   - <pre>rtc_time_count             Usage of time counting and alarm interruption for RTC</pre>
 **   - <pre>rtc_timer_interval         Usage of time counting and timer interval mode for RTC</pre>
 **   - <pre>rtc_timer_oneshot          Usage of time counting and timer one shot mode for RTC</pre>
 ** 
 ** - SD
 **   - <pre>sd_sector_wr      Write/read to/from a sector of SD card via SD I/F</pre>
 ** 
 ** - UID
 **   - <pre>uid_read          Several methods to read the Unique ID</pre>
 ** 
 ** - USB
 **   - <pre>sbdevice_cdc_virtual com port           MCU acting as virtual COM port using Communications Device Class</pre>
 **   - <pre>sbdevice_hid_data_cdc_virtual_com_port  MCU acting as composition device with HID and CDC</pre>
 **   - <pre>sbdevice_hid_data_communication         Data communication using HID class</pre>
 **   - <pre>sbdevice_hid_joystick                   MCU acting as a joystick</pre>
 **   - <pre>sbdevice_hid_keyboard                   MCU acting as a keyboard (writing a text)</pre>
 **   - <pre>sbdevice_hid_mouse                      MCU acting as mouse</pre>
 **   - <pre>sbdevice_hid_keyboard_mouse             MCU acting as composition</pre>
 **   - <pre>sbdevice_libusb                         Usage of LibUSB</pre>
 **   - <pre>sbdevice_lineprinter                    Data communication via USB Printer and CDC Class</pre>
 **   - <pre>sbhost_keyboard_mouse                   MCU is recognizing a USB keyoard and mouse</pre>
 ** 
 ** - WC
 **   - <pre>wc_int            Watch Counter function with usage of interrupt callback</pre>
 **   - <pre>wc_polling        Watch Counter function with polling for interrupt occurrence</pre>
 ** 
 ** - WDG
 **   - <pre>hwwdg             Usage of hardware watchdog</pre>
 **   - <pre>swwdg</pre>
 **     - <pre>swwdg_normal    Usage of normal mode for software watchdog</pre>
 **     - <pre>swwdg_normal    Usage of window mode for software watchdog</pre>
 ** 
 ** 
 ** <b>Usage of example code:</b>
 ** 
 ** Each example code consists of a 'main.c' and a 'pdl_user.h' module.
 ** The main module contains the example code, where 'pdl_user.h'
 ** contains all necessary settings and definitions for the example
 ** itself.
 ** To use and compile an example, just copy these two modules
 ** to the 'template/source' folder, overwriting the modules located
 ** there. The overwritten modules have backups in the 'template/source/backup' folder for recovery.
 ** All adjustments for IAR Workbench and ARM/KEIL uVision IDE are
 ** done in the template project and will fit to the example code.
 ** Follow the explanations and advices in the main.c module's
 ** description in the comment lines at the top.
 ** 
 ** 
 ** <br><hr><br>
 ** 
 ******************************************************************************/