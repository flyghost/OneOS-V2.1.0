
; ////////////////////////////////////////////////////////////////////////////////
; /// @file     startup_mm32_keil.s
; /// @author   AE TEAM
; /// @brief    THIS FILE PROVIDES ALL THE Device Startup File of MM32 Cortex-M
; ///           Core Device for ARM KEIL toolchain.
; ////////////////////////////////////////////////////////////////////////////////
; /// @attention
; ///
; /// THE EXISTING FIRMWARE IS ONLY FOR REFERENCE, WHICH IS DESIGNED TO PROVIDE
; /// CUSTOMERS WITH CODING INFORMATION ABOUT THEIR PRODUCTS SO THEY CAN SAVE
; /// TIME. THEREFORE, MINDMOTION SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT OR
; /// CONSEQUENTIAL DAMAGES ABOUT ANY CLAIMS ARISING OUT OF THE CONTENT OF SUCH
; /// HARDWARE AND/OR THE USE OF THE CODING INFORMATION CONTAINED HEREIN IN
; /// CONNECTION WITH PRODUCTS MADE BY CUSTOMERS.
; ///
; /// <H2><CENTER>&COPY; COPYRIGHT MINDMOTION </CENTER></H2>
; //////////////////////////////////////////////////////////////////////////////
;
; Amount of memory (in bytes) allocated for Stack
; Tailor this value to your application needs
; <h> Stack Configuration
;   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Stack_Size      EQU     0x00000400

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp


; <h> Heap Configuration
;   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Heap_Size       EQU     0x00000200

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit

                PRESERVE8
                THUMB


; Vector Table Mapped to Address 0 at Reset
                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__Vectors       DCD     __initial_sp                                            ;       Top of Stack
                DCD     Reset_Handler                                           ;       Reset Handler
                DCD     NMI_Handler                                             ; -14   NMI Handler
                DCD     HardFault_Handler                                       ; -13   Hard Fault Handler
                DCD     0                                                       ; -12   Reserved
                DCD     0                                                       ; -11   Reserved
                DCD     0                                                       ; -10   Reserved
__vector_table_0x1c
                DCD     0                                                       ;  -9   Reserved
                DCD     0                                                       ;  -8   Reserved
                DCD     0                                                       ;  -7   Reserved
                DCD     0                                                       ;  -6   Reserved
                DCD     SVC_Handler                                             ;  -5   SVCall Handler
                DCD     0                                                       ;  -4   Reserved
                DCD     0                                                       ;  -3   Reserved
                DCD     PendSV_Handler                                          ;  -2   PendSV Handler
                DCD     SysTick_Handler                                         ;  -1   SysTick Handler

                ; External Interrupts
                DCD     WWDG_IRQHandler                                         ;   0   Window Watchdog
                DCD     PVD_IRQHandler                                          ;   1   PVD through EXTI Line detect
                DCD     RTC_BKP_IRQHandler                                      ;   2   RTC through EXTI Line & Tamper
                DCD     FLASH_IRQHandler                                        ;   3   FLASH
                DCD     RCC_CRS_IRQHandler                                      ;   4   RCC & CRS
                DCD     EXTI0_1_IRQHandler                                      ;   5   EXTI Line 0 and 1
                DCD     EXTI2_3_IRQHandler                                      ;   6   EXTI Line 2 and 3
                DCD     EXTI4_15_IRQHandler                                     ;   7   EXTI Line 4 to 15
                DCD     HWDIV_IRQHandler                                        ;   8   HWDIV
                DCD     DMA1_Channel1_IRQHandler                                ;   9   DMA1 Channel 1
                DCD     DMA1_Channel2_3_IRQHandler                              ;  10   DMA1 Channel 2 and Channel 3
                DCD     DMA1_Channel4_5_6_7_IRQHandler                          ;  11   DMA1 Channel 4, Channel 5, Channel 6 and Channel 7
                DCD     ADC1_COMP_IRQHandler                                    ;  12   ADC1 & COMP
                DCD     TIM1_BRK_UP_TRG_COM_IRQHandler                          ;  13   TIM1 Break, Update, Trigger and Commutation
                DCD     TIM1_CC_IRQHandler                                      ;  14   TIM1 Capture Compare
                DCD     TIM2_IRQHandler                                         ;  15   TIM2
                DCD     TIM3_IRQHandler                                         ;  16   TIM3
                DCD     LPUART1_IRQHandler                                      ;  17   LPUART
                DCD     LPTIMER1_IRQHandler                                     ;  18   LPTIMER
                DCD     TIM14_IRQHandler                                        ;  19   TIM14
                DCD     TIM15_IRQHandler                                        ;  20   TIM15
                DCD     TIM16_IRQHandler                                        ;  21   TIM16
                DCD     TIM17_IRQHandler                                        ;  22   TIM17
                DCD     I2C1_IRQHandler                                         ;  23   I2C1
                DCD     I2C2_IRQHandler                                         ;  24   I2C2
                DCD     SPI1_IRQHandler                                         ;  25   SPI1
                DCD     SPI2_IRQHandler                                         ;  26   SPI2
                DCD     UART1_IRQHandler                                        ;  27   UART1
                DCD     UART2_IRQHandler                                        ;  28   UART2
                DCD     UART3_4_IRQHandler                                      ;  29   UART3 and UART4
                DCD     CAN_IRQHandler                                          ;  30   CAN
                DCD     USB_IRQHandler                                          ;  31   USB

                                                                                ; Total Cortex-M0 32 Interrupts are setting
__Vectors_End
__Vectors_Size  EQU     __Vectors_End - __Vectors

                AREA    |.text|, CODE, READONLY

; Reset handler routine
Reset_Handler   PROC
                EXPORT  Reset_Handler                  [WEAK]
                IMPORT  __main
                IMPORT  SystemInit


                LDR     R0, =__initial_sp                                       ; set stack pointer
                MSR     MSP, R0

ApplicationStart
                LDR     R0, =SystemInit
                BLX     R0
                LDR     R0, =__main
                BX      R0
                ENDP

; Dummy Exception Handlers (infinite loops which can be modified)

NMI_Handler     PROC
                EXPORT  NMI_Handler                    [WEAK]
                B       .
                ENDP
HardFault_Handler\
                PROC
                EXPORT  HardFault_Handler              [WEAK]
                B       .
                ENDP
SVC_Handler     PROC
                EXPORT  SVC_Handler                    [WEAK]
                B       .
                ENDP
PendSV_Handler  PROC
                EXPORT  PendSV_Handler                 [WEAK]
                B       .
                ENDP
SysTick_Handler PROC
                EXPORT  SysTick_Handler                [WEAK]
                B       .
                ENDP

Default_Handler PROC

                EXPORT  WWDG_IRQHandler                [WEAK]
                EXPORT  PVD_IRQHandler                 [WEAK]
                EXPORT  RTC_BKP_IRQHandler             [WEAK]
                EXPORT  FLASH_IRQHandler               [WEAK]
                EXPORT  RCC_CRS_IRQHandler             [WEAK]
                EXPORT  EXTI0_1_IRQHandler             [WEAK]
                EXPORT  EXTI2_3_IRQHandler             [WEAK]
                EXPORT  EXTI4_15_IRQHandler            [WEAK]
                EXPORT  HWDIV_IRQHandler               [WEAK]
                EXPORT  DMA1_Channel1_IRQHandler       [WEAK]
                EXPORT  DMA1_Channel2_3_IRQHandler     [WEAK]
                EXPORT  DMA1_Channel4_5_6_7_IRQHandler [WEAK]
                EXPORT  ADC1_COMP_IRQHandler           [WEAK]
                EXPORT  TIM1_BRK_UP_TRG_COM_IRQHandler [WEAK]
                EXPORT  TIM1_CC_IRQHandler             [WEAK]
                EXPORT  TIM2_IRQHandler                [WEAK]
                EXPORT  TIM3_IRQHandler                [WEAK]
                EXPORT  LPUART1_IRQHandler              [WEAK]
                EXPORT  LPTIMER1_IRQHandler             [WEAK]
                EXPORT  TIM14_IRQHandler               [WEAK]
                EXPORT  TIM15_IRQHandler               [WEAK]
                EXPORT  TIM16_IRQHandler               [WEAK]
                EXPORT  TIM17_IRQHandler               [WEAK]
                EXPORT  I2C1_IRQHandler                [WEAK]
                EXPORT  I2C2_IRQHandler                [WEAK]
                EXPORT  SPI1_IRQHandler                [WEAK]
                EXPORT  SPI2_IRQHandler                [WEAK]
                EXPORT  UART1_IRQHandler               [WEAK]
                EXPORT  UART2_IRQHandler               [WEAK]
                EXPORT  UART3_4_IRQHandler             [WEAK]
                EXPORT  CAN_IRQHandler                 [WEAK]
                EXPORT  USB_IRQHandler                 [WEAK]


WWDG_IRQHandler
PVD_IRQHandler
RTC_BKP_IRQHandler
FLASH_IRQHandler
RCC_CRS_IRQHandler
EXTI0_1_IRQHandler
EXTI2_3_IRQHandler
EXTI4_15_IRQHandler
HWDIV_IRQHandler
DMA1_Channel1_IRQHandler
DMA1_Channel2_3_IRQHandler
DMA1_Channel4_5_6_7_IRQHandler
ADC1_COMP_IRQHandler
TIM1_BRK_UP_TRG_COM_IRQHandler
TIM1_CC_IRQHandler
TIM2_IRQHandler
TIM3_IRQHandler
LPUART1_IRQHandler
LPTIMER1_IRQHandler
TIM14_IRQHandler
TIM15_IRQHandler
TIM16_IRQHandler
TIM17_IRQHandler
I2C1_IRQHandler
I2C2_IRQHandler
SPI1_IRQHandler
SPI2_IRQHandler
UART1_IRQHandler
UART2_IRQHandler
UART3_4_IRQHandler
CAN_IRQHandler
USB_IRQHandler

                B       .

                ENDP

                ALIGN

;*******************************************************************************
; User Stack and Heap initialization
;*******************************************************************************
                IF      :DEF:__MICROLIB

                EXPORT  __initial_sp
                EXPORT  __heap_base
                EXPORT  __heap_limit

                ELSE

                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap

__user_initial_stackheap

                LDR     R0, = Heap_Mem
                LDR     R1, = (Stack_Mem + Stack_Size)
                LDR     R2, = (Heap_Mem +  Heap_Size)
                LDR     R3, = Stack_Mem
                BX      LR

                ALIGN

                ENDIF
                END

