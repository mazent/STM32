; Ricavato da startup_stm32h735xx.s
; Il vettore delle interruzioni contiene solo il reset, che iar
; forza in flash


        MODULE  ?cstartup

        ;; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .intvec:CODE:NOROOT(2)

        EXTERN  __iar_program_start
        EXTERN  SystemInit
        PUBLIC  __vector_table

        DATA
__vector_table
        DCD     sfe(CSTACK)
        DCD     Reset_Handler                     ; Reset Handler
                                                  
        DCD     0     ; NMI Handler
        DCD     0     ; Hard Fault Handler
        DCD     0     ; MPU Fault Handler
        DCD     0     ; Bus Fault Handler
        DCD     0     ; Usage Fault Handler
        DCD     0     ; Reserved
        DCD     0     ; Reserved
        DCD     0     ; Reserved
        DCD     0     ; Reserved
        DCD     0     ; SVCall Handler
        DCD     0     ; Debug Monitor Handler
        DCD     0     ; Reserved
        DCD     0     ; PendSV Handler
        DCD     0     ; SysTick Handler

        ; External Interrupts
        DCD     0     ; Window WatchDog Interrupt ( wwdg1_it)                                                 
        DCD     0     ; PVD/AVD through EXTI Line detection                                  
        DCD     0     ; Tamper and TimeStamps through the EXTI line                      
        DCD     0     ; RTC Wakeup through the EXTI line                                 
        DCD     0     ; FLASH                                                
        DCD     0     ; RCC                                                  
        DCD     0     ; EXTI Line0                                                       
        DCD     0     ; EXTI Line1                                                       
        DCD     0     ; EXTI Line2                                                       
        DCD     0     ; EXTI Line3                                                       
        DCD     0     ; EXTI Line4             
        DCD     0     ; DMA1 Stream 0     
        DCD     0     ; DMA1 Stream 1                                        
        DCD     0     ; DMA1 Stream 2                                        
        DCD     0     ; DMA1 Stream 3                                        
        DCD     0     ; DMA1 Stream 4                                        
        DCD     0     ; DMA1 Stream 5                                        
        DCD     0     ; DMA1 Stream 6       
        DCD     0     ; ADC1, ADC2                                  
        DCD     0     ; FDCAN1 interrupt line 0                                    
        DCD     0     ; FDCAN2 interrupt line 0                                           
        DCD     0     ; FDCAN1 interrupt line 1                                    
        DCD     0     ; FDCAN2 interrupt line 1                                           
        DCD     0     ; External Line[9:5]s                                              
        DCD     0     ; TIM1 Break interrupt        
        DCD     0     ; TIM1 Update 
        DCD     0     ; TIM1 Trigger and Commutation Interrupt 
        DCD     0     ; TIM1 Capture Compare                                             
        DCD     0     ; TIM2                                                 
        DCD     0     ; TIM3                                                 
        DCD     0     ; TIM4                                                 
        DCD     0     ; I2C1 Event                                                       
        DCD     0     ; I2C1 Error                                                       
        DCD     0     ; I2C2 Event                                                       
        DCD     0     ; I2C2 Error                                                         
        DCD     0     ; SPI1                                                 
        DCD     0     ; SPI2                                                 
        DCD     0     ; USART1                                               
        DCD     0     ; USART2                                               
        DCD     0     ; USART3                                               
        DCD     0     ; External Line[15:10]              
        DCD     0     ; RTC Alarm (A and B) through EXTI Line	            
        DCD     0     ; Reserved                        
        DCD     0     ; TIM8 Break Interrupt and TIM12 global interrupt      
        DCD     0     ; TIM8 Update Interrupt and TIM13 global interrupt
        DCD     0     ; TIM8 Trigger and Commutation Interrupt and TIM14 glob
        DCD     0     ; TIM8 Capture Compare Interrupt            
        DCD     0     ; DMA1 Stream7                                                     
        DCD     0     ; FMC                                 
        DCD     0     ; SDMMC1                                   
        DCD     0     ; TIM5                                 
        DCD     0     ; SPI3                                 
        DCD     0     ; UART4                                
        DCD     0     ; UART5                                
        DCD     0     ; TIM6 and DAC1&2 underrun errors                  
        DCD     0     ; TIM7           
        DCD     0     ; DMA2 Stream 0                        
        DCD     0     ; DMA2 Stream 1                        
        DCD     0     ; DMA2 Stream 2                        
        DCD     0     ; DMA2 Stream 3                        
        DCD     0     ; DMA2 Stream 4                        
        DCD     0     ; Ethernet                             
        DCD     0     ; Ethernet Wakeup through EXTI line                          
        DCD     0     ; FDCAN calibration unit interrupt                                 
        DCD     0     ; Reserved                                          
        DCD     0     ; Reserved             
        DCD     0     ; Reserved             
        DCD     0     ; Reserved                           
        DCD     0     ; DMA2 Stream 5                        
        DCD     0     ; DMA2 Stream 6                        
        DCD     0     ; DMA2 Stream 7                        
        DCD     0     ; USART6                                 
        DCD     0     ; I2C3 event                                         
        DCD     0     ; I2C3 error                                         
        DCD     0     ; USB OTG HS End Point 1 Out                      
        DCD     0     ; USB OTG HS End Point 1 In                       
        DCD     0     ; USB OTG HS Wakeup through EXTI                         
        DCD     0     ; USB OTG HS                         
        DCD     0     ; DCMI, PSSI
        DCD     0     ; CRYP crypto                      
        DCD     0     ; Hash and Rng
        DCD     0     ; FPU
        DCD     0     ; UART7
        DCD     0     ; UART8
        DCD     0     ; SPI4
        DCD     0     ; SPI5
        DCD     0     ; SPI6
        DCD     0     ; SAI1
        DCD     0     ; LTDC
        DCD     0     ; LTDC error
        DCD     0     ; DMA2D
        DCD     0     ; Reserved
        DCD     0     ; OCTOSPI1
        DCD     0     ; LPTIM1
        DCD     0     ; HDMI_CEC
        DCD     0     ; I2C4 Event                                         
        DCD     0     ; I2C4 Error 
        DCD     0     ; SPDIF_RX
        DCD     0     ; Reserved
        DCD     0     ; Reserved
        DCD     0     ; Reserved
        DCD     0     ; Reserved                 
        DCD     0     ; DMAMUX1 Overrun interrupt  
        DCD     0     ; Reserved
        DCD     0     ; Reserved
        DCD     0     ; Reserved
        DCD     0     ; Reserved
        DCD     0     ; Reserved
        DCD     0     ; Reserved
        DCD     0     ; Reserved
        DCD     0     ; DFSDM Filter0 Interrupt   
        DCD     0     ; DFSDM Filter1 Interrupt                              
        DCD     0     ; DFSDM Filter2 Interrupt                              
        DCD     0     ; DFSDM Filter3 Interrupt                              
        DCD     0     ; Reserved                               
        DCD     0     ; Serial Wire Interface 1 global interrupt        
        DCD     0     ; TIM15 global Interrupt                          
        DCD     0     ; TIM16 global Interrupt                          
        DCD     0     ; TIM17 global Interrupt                          
        DCD     0     ; MDIOS Wakeup  Interrupt                         
        DCD     0     ; MDIOS global Interrupt                          
        DCD     0     ; Reserved                           
        DCD     0     ; MDMA global Interrupt                           
        DCD     0     ; Reserved                            
        DCD     0     ; SDMMC2 global Interrupt                         
        DCD     0     ; HSEM1 global Interrupt                          
        DCD     0     ; Reserved                          
        DCD     0     ; ADC3 global Interrupt                           
        DCD     0     ; DMAMUX Overrun interrupt                         
        DCD     0     ; BDMA Channel 0 global Interrupt                  
        DCD     0     ; BDMA Channel 1 global Interrupt                  
        DCD     0     ; BDMA Channel 2 global Interrupt                  
        DCD     0     ; BDMA Channel 3 global Interrupt                  
        DCD     0     ; BDMA Channel 4 global Interrupt                  
        DCD     0     ; BDMA Channel 5 global Interrupt                  
        DCD     0     ; BDMA Channel 6 global Interrupt                  
        DCD     0     ; BDMA Channel 7 global Interrupt                  
        DCD     0     ; COMP1 global Interrupt                          
        DCD     0     ; LP TIM2 global interrupt                        
        DCD     0     ; LP TIM3 global interrupt                        
        DCD     0     ; LP TIM4 global interrupt                        
        DCD     0     ; LP TIM5 global interrupt                        
        DCD     0     ; LP UART1 interrupt                              
        DCD     0     ; Reserved              
        DCD     0     ; Clock Recovery Global Interrupt                 
        DCD     0     ; ECC diagnostic Global Interrupt
        DCD     0     ; SAI4 global interrupt                           
        DCD     0     ; Digital Temperature Sensor              
        DCD     0     ; Reserved                             
        DCD     0     ; Interrupt for all 6 wake-up pins
        DCD     0     ; OCTOSPI2 Interrupt
        DCD     0     ; OTFDEC1 Interrupt
        DCD     0     ; OTFDEC2 Interrupt
        DCD     0     ; FMAC Interrupt
        DCD     0     ; CORDIC Interrupt
        DCD     0     ; UART9 Interrupt
        DCD     0     ; USART10 Interrupt
        DCD     0     ; I2C5 Event Interrupt
        DCD     0     ; I2C5 Error Interrupt
        DCD     0     ; FDCAN3 interrupt line 0
        DCD     0     ; FDCAN3 interrupt line 1
        DCD     0     ; TIM23 global interrup
        DCD     0     ; TIM24 global interrup
		
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;
        THUMB
        PUBWEAK Reset_Handler
        SECTION .text:CODE:NOROOT:REORDER(2)
Reset_Handler

        LDR     R0, =SystemInit
        BLX     R0
        LDR     R0, =__iar_program_start
        BX      R0

		
        END
