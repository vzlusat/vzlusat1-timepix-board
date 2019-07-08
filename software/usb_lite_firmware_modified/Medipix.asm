;
; Authors       : Jan Jakubek, Zdenek Vykydal
;
; Date          : November 2004, June 2005, May 2006, September 2007
;
; File          : Main1.asm
;
; Hardware      : ADuC841
;
; Description   : Main program for Medipix2 USB 1.X interface
;
;*******************************************************************************

; CERN cable:
;==============
; Coditional compilations for different CERN cable lengths are driven by
; definition of CERN_CABLE symbol just one of following four lines has to
; be uncomented:

CERN_CABLE		SET	0	; No cable
;CERN_CABLE		SET	1 	; Short cable           (<20m)
;CERN_CABLE		SET	2 	; Long cable       	(<40m)
;CERN_CABLE		SET	3 	; Very long cable 	(<60m)

; Version control:
;===================
VERSION_MAJOR 		SET	1
VERSION_MINOR 		SET	2
VERSION_SUBMINOR	SET	0



; Communication protocol:
;==========================

; Command received has a following format:
;   X[parameters]
; where:
;   X is single byte command character.
;   parameters is optional block of parameters (length depends on command)

; As a responste to each command the command character is echoed
; followed by optional block of data and "OK" suffix:
;   X[data]"OK"CR
; where:
;   X is single byte command character (the same as it was received)
;   data is optional block of output data (length depends on command)
;   "OK" is suffix informing about success (without quotes)
;   CR is cariage return character (0Dh) terminating packet

; If command finishes with an error then output has this format:
;   X"!Error string"CR
; where
;   X is single byte command character (the same as it was received)
;   "!Error string" is suffix (without quotes) informing about failure. The '!' sign is mandatory!
;   CR is cariage return character (0Dh) terminating packet

; All other errors reported by the device (not in response to any command)
; have format:
;   "!Error string"CR
; where
;   "!Error string" is suffix (without quotes) informing about failure. The '!' sign is mandatory!
;   CR is cariage return character (0Dh) terminating packet


; Main program loop
;==========================

; is reading commands received by USB interface (FTDI 245BM)
; When command is recognized then the command is echoed on the USB
; and appropriate service routine is called
; After return from the command service the result is examined and appropriate
; message is transmited (i.e. in case of no error it is "OK", and error message
; starting withn '!' character elsewhere).
; List of known commands and adresses of their services are in "Com_Table"
; Received command is passed to command service in B register!

; Main loop also cheks flags from background processes running as interrupt services
; If some of them would like to transmit its output it have to set the flag.
; Main loop will call Service appropriate to this flag.
; Main loop doesn't check the result of a background service call.


; Command services
;-------------------
; Command service don't have to preserve any register value.
; The command character is passed to the service in B register.
; Command may not to echo the command character.

; If service finishes with no error it has to set CY=0.
; Main loop will transmit the "OK" message.

; In case of an error it has to set CY=1 and DPTR to point to error message.
; Main loop will transmit this message. Service can transmit error message itself
; in that case DPTR has to be set to zero.


; Background processes:
;==========================

; Interval Timer
;-----------------
; As resolution of built in interval timer is not satisfactory the more
; precise timer is implemnted using timer/counter TO.
; Timer T0 is configured as 16 bit timer (Mode 1) generating interrupt
; The whole amount of clocks which have to be counted is stored in 6 bytes of memory INTERVAL_TIME
; allowing to measure interval of 162 days with precission of 50ns.
; When interval timer overflows actions given by TODO register (INTERVAL_TODO) are performed.

; Period timer:
;---------------
; Is implemented using timer T1. Resolution is the same as in case of Interval Timer.
; When period timer overflows actions given by TODO register (PERIOD_TODO) are performed
; and timer is restarted.

; Frame counter:
;----------------
; When a frame is transmited as a result of some of background processes the frma counter is
; decremented and specific TODO actions (FRAME_TODO) are performed. If Frame counter underflows all
; background processes are stopped.

; Scope
;-----------------
; ADC is configured to perform DMA measurement, on max 8 preselected channels
; with preselected timing. Whole amount of samples which it can take per single trigger is
; given by scope memory which is 1kB.

; Trigger
;-----------------
; INT0 is configured to generate interrupt.
; The interrupt service can call actions given by its TODO register (TRIGGER_TODO).
; Coincidence modes can be used with trigger (configured by TRIGGER_COINC):
; - If TRIGGER_COINC is set and trigger occures then if TRIGGER_TODO contain flag
;   TODO_CloseShutter the shutter will be closed and value of P3.0 (RxD) will be checked
; - If P3.0 (RxD) is the same as value configured in bit 2 of TRIGGER_COINC register
;   the coincidence is signalised and data are transfered from Medipix chip to USB.
; - If P3.0 is different then coincidence is rejected and Medipix data are reseted.
;   The shutter is opened and trigger started to continue the measurement.
; - The coincidence can be notified on P3.1 (TxD) line. Mode of notification can be
;   configured by bits 3,4,5 of TRIGGER_COINC register.

; Counter
;-----------------
; T1 is configured as counter of external pulses. Max number of pulses is given by 6 bytes integer.
; It generates interrupt and calls service routione in dependence on predefined TODO Action (COUNTER_TODO).


; TODO actions:
;==========================
; Each of background processes can on its action perform its specific Actions.
; Actions are defined for each of them by specific TODO register. Each bit
; of this register can start certain action:
; - Close shutter and stop interval measurement
; - Open the shutter
; - Start the Interval
; - Trigger for scope
; - Start waiting for external trigger (interrupt)
; - Start of waiting for counter event (counter overflow)
; - Makes predefined command number 1
; - Makes predefined command number 2


;###############################################################################
;#                                                                             #
;#                      I M P L E M E N T A T I O N                            #
;#                                                                             #
;###############################################################################

;REV.  1.0   30 September 2003
;ADuC841   Apps, Analog Devices Inc.
I2CADD1  DATA  091H  ;I2C ADDRESS REGISTER1
I2CADD2  DATA  092H  ;I2C ADDRESS REGISTER2
I2CADD3  DATA  093H  ;I2C ADDRESS REGISTER3
I2CDAT   DATA  09AH  ;I2C DATA REGISTER
I2CADD   DATA  09BH  ;I2C ADDRESS REGISTER
T3CON    DATA  09EH  ;TIMER 3 CONTROL
T3FD     DATA  09DH  ;TIMER 3 FRACTIONAL DIVIDER
TIMECON  DATA  0A1H  ;TIC CONTROL
HTHSEC   DATA  0A2H  ;TIC - HTHSEC DATA
SEC      DATA  0A3H  ;TIC - SEC DATA
MIN      DATA  0A4H  ;TIC - MIN DATA
HOUR     DATA  0A5H  ;TIC - HOUR DATA
INTVAL   DATA  0A6H  ;TIC INTERVAL REGISTER
DPCON    DATA  0A7H  ;DUAL DATA POINTER CONTROL REGISTER
IEIP2    DATA  0A9H  ;INTERRUPT ENABLE 2
PWMCON   DATA  0AEH  ;PWM CONTROL REGISTER
CFG841   DATA  0AFH  ;GENERAL FLASH/PWM CONTROL REGISTER
PWM0L    DATA  0B1H  ;PWM DATA REGISTER
PWM0H    DATA  0B2H  ;PWM DATA REGISTER
PWM1L    DATA  0B3H  ;PWM DATA REGISTER
PWM1H    DATA  0B4H  ;PWM DATA REGISTER
SPH      DATA  0B7H  ;EXTENDED STACK POINTER REGISTER
ECON     DATA  0B9H  ;FLASH EEPROM CONTROL
EDATA1   DATA  0BCH  ;FLASH EEPROM DATA1 
EDATA2   DATA  0BDH  ;FLASH EEPROM DATA2 
EDATA3   DATA  0BEH  ;FLASH EEPROM DATA3 
EDATA4   DATA  0BFH  ;FLASH EEPROM DATA4 
WDCON    DATA  0C0H  ;WATCHDOG TIMER CONTROL
CHIPID   DATA  0C2H  ;CHIPID REGISTER
EADRL    DATA  0C6H  ;FLASH EEPROM PAGE ADDRESS - LOW BYTE
EADRH    DATA  0C7H  ;FLASH EEPROM PAGE ADDRESS - LOW BYTE
T2CON    DATA  0C8H  ;TIMER 2 CONTROL
RCAP2L   DATA  0CAH  ;TIMER 2 CAPTURE REGISTER - LOW BYTE
RCAP2H   DATA  0CBH  ;TIMER 2 CAPTURE REGISTER - HIGH BYTE
TL2      DATA  0CCH  ;TIMER 2 - LOW BYTE
TH2      DATA  0CDH  ;TIMER 2 - HIGH BYTE
DMAL     DATA  0D2H  ;DMA ADDRESS LOW BYTE
DMAH     DATA  0D3H  ;DMA ADDRESS HIGH BYTE
DMAP     DATA  0D4H  ;DMA ADDRESS PAGE BYTE
PLLCON   DATA  0D7H  ;PLL CONTROL
ADCCON2  DATA  0D8H  ;ADC CONTROL
ADCDATAL DATA  0D9H  ;ADC DATA LOW BYTE
ADCDATAH DATA  0DAH  ;ADC DATA HIGH BYTE
PSMCON   DATA  0DFH  ;POWER SUPPLY MONITOR
DCON     DATA  0E8H  ;D1 AND D0 CONTROL
I2CCON   DATA  0E8H  ;I2C CONTROL
ADCCON1  DATA  0EFH  ;ADC CONTROL

ADCOFSL  DATA  0F1H  ;ADC OFFSET LOW BYTE
ADCOFSH  DATA  0F2H  ;ADC OFFSET HIGH BYTE
ADCGAINL DATA  0F3H  ;ADC GAIN LOW BYTE
ADCGAINH DATA  0F4H  ;ADC GAIN HIGH BYTE
ADCCON3  DATA  0F5H  ;ADC CONTROL
SPIDAT   DATA  0F7H  ;SPI DATA REGISTER
SPICON   DATA  0F8H  ;SPI CONTROL REGISTER
DAC0L    DATA  0F9H  ;DAC0 LOW BYTE
DAC0H    DATA  0FAH  ;DAC0 HIGH BYTE
DAC1L    DATA  0FBH  ;DAC1 LOW BYTE
DAC1H    DATA  0FCH  ;DAC1 HIGH BYTE
DACCON   DATA  0FDH  ;DAC CONTROL REGISTER
DPP      DATA  084H  ;DATA POINTER - PAGE BYTE




T2       BIT   090H  ;P1.0 - TIMER 2 TRIGGER INPUT
T2EX     BIT   091H  ;P1.1 - TIMER 2 COUNT INPUT


EADC     BIT   0AEH  ;IE.6 - ENABLE ADC INTURRUPT
PT2      BIT   0BDH  ;IP.5 - TIMER 2 PRIORITY
PADC     BIT   0BEH  ;IP.6 - ADC PRIORITY
PSI      BIT   0BFH  ;IP.7 - SPI OR 2-WIRE SERIAL INTERFACE PRIORITY
WDWR     BIT   0C0H  ;WDCON.0 - WATCHDOG WRITE ENABLE
WDE      BIT   0C1H  ;WDCON.1 - WATCHDOG ENABLE
WDS      BIT   0C2H  ;WDCON.2 - WATCHDOG STATUS BIT
WDIR     BIT   0C3H  ;WDCON.3 - WATCHDOG INTERRUPT RESPONSE ENABLE BIT
PRE0     BIT   0C4H  ;WDCON.4 - WATCHDOG TIMEOUT SELECTION BIT0
PRE1     BIT   0C5H  ;WDCON.5 - WATCHDOG TIMEOUT SELECTION BIT1
PRE2     BIT   0C6H  ;WDCON.6 - WATCHDOG TIMEOUT SELECTION BIT2
PRE3     BIT   0C7H  ;WDCON.7 - WATCHDOG TIMEOUT SELECTION BIT3
CAP2     BIT   0C8H  ;T2CON.0 - CAPTURE OR RELOAD SELECT
CNT2     BIT   0C9H  ;T2CON.1 - TIMER OR COUNTER SELECT
TR2      BIT   0CAH  ;T2CON.2 - TIMER 2 ON/OFF CONTROL
EXEN2    BIT   0CBH  ;T2CON.3 - TIMER 2 EXTERNAL ENABLE FLAG
TCLK     BIT   0CCH  ;T2CON.4 - TRANSMIT CLOCK SELECT
RCLK     BIT   0CDH  ;T2CON.5 - RECEIVE CLOCK SELECTT
EXF2     BIT   0CEH  ;T2CON.6 - EXTERNAL TRANSITION FLAG
TF2      BIT   0CFH  ;T2CON.7 - TIMER 2 OVERFLOW FLAG
F1       BIT   0D1H  ;PSW.1 - FLAG 0
CS0      BIT   0D8H  ;ADCCON2.0 - ADC INPUT CHANNEL SELECT BIT0
CS1      BIT   0D9H  ;ADCCON2.1 - ADC INPUT CHANNEL SELECT BIT1
CS2      BIT   0DAH  ;ADCCON2.2 - ADC INPUT CHANNEL SELECT BIT2
CS3      BIT   0DBH  ;ADCCON2.3 - ADC INPUT CHANNEL SELECT BIT3
SCONV    BIT   0DCH  ;ADCCON2.4 - SINGLE CONVERSION ENABLE
CCONV    BIT   0DDH  ;ADCCON2.5 - CONTINUOUS CONVERSION ENABLE
DMA      BIT   0DEH  ;ADCCON2.6 - DMA MODE ENABLE
ADCI     BIT   0DFH  ;ADCCON2.7 - ADC INTURRUPT FLAG
I2CI     BIT   0E8H  ;I2CCON.0 - I2C INTURRUPT FLAG
I2CTX    BIT   0E9H  ;I2CCON.1 - I2C TRANSMIT SELECT
I2CRS    BIT   0EAH  ;I2CCON.2 - I2C RESET
I2CM     BIT   0EBH  ;I2CCON.3 - I2C MASTER MODE SELECT
MDI      BIT   0ECH  ;I2CCON.4 - I2C MASTER MODE SDATA INPUT
I2CID0   BIT   0ECH  ;I2CCON.4 - I2C SLAVE MODE INTERRUPT DECODE
MCO      BIT   0EDH  ;I2CCON.5 - I2C MASTER MODE SCLOCK OUTPUT
I2CID1   BIT   0EDH  ;I2CCON.5 - I2C SLAVE MODE INTERRUPT DECODE
MDE      BIT   0EEH  ;I2CCON.6 - I2C MASTER MODE SDATA ENABLE
I2CGC    BIT   0EEH  ;I2CCON.6 - I2C SLAVE MODE GENERAL CALL STATUS BIT
MDO      BIT   0EFH  ;I2CCON.7 - I2C MASTER MODE SDATA OUTPUT
I2CSI    BIT   0EFH  ;I2CCON.7 - I2C SLAVE  MODE STOP INTERRUPT ENABLE 
SPR0     BIT   0F8H  ;SPICON.0 - SPI BITRATE SELECT BIT0
SPR1     BIT   0F9H  ;SPICON.1 - SPI BITRATE SELECT BIT1
CPHA     BIT   0FAH  ;SPICON.2 - SPI CLOCK PHASE SELECT
CPOL     BIT   0FBH  ;SPICON.3 - SPI CLOCK POLARITY SELECT
SPIM     BIT   0FCH  ;SPICON.4 - SPI MASTER/SLAVE MODE SELECT
SPE      BIT   0FDH  ;SPICON.5 - SPI INTERFACE ENABLE
WCOL     BIT   0FEH  ;SPICON.6 - SPI WRITE COLLISION ERROR FLAG
ISPI     BIT   0FFH  ;SPICON.7 - SPI END OF TRANSFER FLAG
ET2      BIT   0ADH  ;IE.5 - TIMER 2 INTERRUPT ENABLE

; $MOD841                         ; Use 8052&ADuC841 predefined symbols

;===============================================================================
; DEFINE VARIABLES IN INTERNAL RAM

DSEG
ORG 0030h

; Chip counter
;----------------
MPX_CHIPS:	  	DS   	1	; Counter of Medipix chips
MPX_ISMXR:	  	DS   	1       ; If MXR is connected then this is nonzero, if timepix is connected then 0x80 mask is added (MSB is set)
;2

; Interval timer
;----------------
INTERVAL_TIME:    	DS   	6 	; Preset interval duration LSB byte is first
INTERVAL_CURR:	  	DS   	4     	; Upper four bytes of interval counter LSB first
INTERVAL_TODO:	  	DS   	1	; Mask of actions which should be done in interrupt
INTERVAL_MODE:	  	DS   	1       ; Reflects if timer has been started loaded by zeroes (to measure interval) or loaded by INTERVAL_TIME (to generate interrupt)
;12+2=14

; Period timer
;----------------
PERIOD_TIME:      	DS   	6 	; Preset period duration LSB byte is first
PERIOD_CURR:	  	DS   	4     	; Upper four bytes of period counter LSB first
PERIOD_TODO:	  	DS   	1	; Mask of actions which should be done in interrupt
;11+14=25

; External Counter:
;--------------------
COUNTER_COUNT: 	  	DS   	6	; Preset count
COUNTER_CURR:	  	DS   	4	; Current count
COUNTER_TODO:	  	DS   	1	; Mask of actions which should be done in interrupt
;11+25=36

; External trigger:
;--------------------
TRIGGER_COUNT:	  	DS   	4	; Counter of external triggers
TRIGGER_TODO:	  	DS   	1	; Mask of actions which should be done in interrupt
TRIGGER_COINC:		DS	1	; Flags for coincident measurement
TRIGGER_COINCDELAY:	DS	1	; Wait time for coincidence (for TriggerIn input)
;7+36=43

; Delay loop:
;--------------------
DELAY_TODOWHEN:		DS	1	; Flags (same like TODO) teling when delay should be applied
DELAY_HOWMUCH:		DS	2	; How long delay should be used
;3+43=46

; Frame counter:
;--------------------
FRAME_COUNT:	  	DS   	4	; How many frames have to be taken
FRAME_CURR: 	  	DS   	4	; Current number of frames
FRAME_TODO: 	  	DS   	1	; Mask of actions which should be done after matrix readout
;9+46=55

; TODO Commands:
;----------------
TODO_Command1:	  	DS   	1   	; Command which should be done if TODO mask contains bit TODO_MakeCommand1
TODO_Command2:	  	DS   	1	; Command which should be done if TODO mask contains bit TODO_MakeCommand2
;2+55=57

; To do mask bits:
;------------------
TODO_CloseShutter	EQU	ACC.0   ; Closes shutter and stops interval measurement
TODO_OpenShutter	EQU	ACC.1   ; Opens shutter
TODO_StartInterval	EQU	ACC.2   ; Starts Interval
TODO_StartScope		EQU	ACC.3   ; Trigger for scope
TODO_StartTrigger	EQU	ACC.4	; Start of waiting for external trigger
TODO_StartCounter	EQU	ACC.5	; Start of waiting for counter event
TODO_MakeCommand1	EQU	ACC.6	; Makes command number 1
TODO_MakeCommand2	EQU	ACC.7	; Makes command number 2

; Triggered coincidence modes:
;------------------------------
COINC_Test		EQU	ACC.0	; Coincidence mode is activated
; COINC_TestLevel		EQU	ACC.1	; This level of P3.0 (RxD) means coincidence (data have to be transmitted), oposite level means noncoincidence (data are scratched)
COINC_Notify		EQU	ACC.2	; The notification on P3.1 (TxD) should be generated
COINC_NotifyInitLevel	EQU	ACC.3   ; This is default level of P3.1 (TxD)
COINC_NotifyValidCoinc	EQU	ACC.4   ; If TRUE just coincidence will be notified, if FALSE each trigger generates notification
COINC_NotifyByPulse	EQU	ACC.5	; If TRUE then two transitions (forming pulse) are generated on P3.1. If FALSE then just one
COINC_CoincEvent	EQU	ACC.6   ; If TRUE then Back_ReadMatrix service generates notifcation after data transfer.
COINC_InValidCoinc	EQU	ACC.7   ; Flag for Back_ReadMatrix service. If set then matrix should be reseted instead of Read.

; Scope using DMA ADC mode:
;--------------------------
SCOPE_MAXCOUNT 		EQU  	450	; Maximal Number of ADC readings to be taken
SCOPE_DATALEN		EQU 	SCOPE_MAXCOUNT*2
SCOPE_BUFFLEN 		EQU 	SCOPE_DATALEN+4
SCOPE_DEFCHANMSK 	EQU 	02h	; Default is ADC 1

SCOPE_SAMPLES:		DS 	2	; Number of samples to take (All channels summed)
SCOPE_TIMING:		DS 	1	; Timing of ADC conversions: 0 0 0 0 CK1 CK0 AQ1 AQ0 (see ADC manual)
SCOPE_CHANNELS:		DS 	1	; Which channels to sample ? Each bit means appropriate channel
SCOPE_CHANBUFF: 	DS 	8	; Buffer for channel numbers (to configure memory for DMA)
SCOPE_CHANCNT:		DS 	1   	; Number of channels
;13+57=70


; Flash DATA commands:
;-------------------------
FLASH_READ		EQU	1
FLASH_WRITE		EQU	2
FLASH_VERIFY		EQU	4
FLASH_ERASE		EQU	5
FLASH_READBYTE 		EQU     81h
FLASH_WRITEBYTE         EQU     82h


;SPI configurations:
;-------------------------
SPI_CONFIG_MPX_1	SET	34h	; SPI is enabled in Master Mode (I2C is enabled), CPOL=0, CPHA=1, frequency Fosc/2, fastest (no cable)
SPI_CONFIG_MPX_2	SET	35h	; SPI is enabled in Master Mode (I2C is enabled), CPOL=0, CPHA=1, frequency Fosc/4
SPI_CONFIG_MPX_3	SET	36h	; SPI is enabled in Master Mode (I2C is enabled), CPOL=0, CPHA=1, frequency Fosc/8
SPI_CONFIG_MPX_4	SET	37h	; SPI is enabled in Master Mode (I2C is enabled), CPOL=0, CPHA=1, frequency Fosc/16, slowest (long cable)

; SPI configuration for TimePix clock generation
SPI_CONFIG_CLOCK	SET	34h	; SPI is enabled in Master Mode (I2C is enabled), CPOL=0, CPHA=1, frequency Fosc/2
; SPI configuration for BIAS voltage settings:
SPI_CONFIG_BIAS		SET	33h	; SPI is enabled in Master Mode (I2C is enabled), CPOL=0, CPHA=0, frequency Fosc/16


; The slowest communication speed requires certain wait time:
SPI_WAIT_MAX	MACRO
	NOP
	NOP
	NOP
	NOP
	NOP
	ENDM

; Actual SPI speed for MPX communication:
;========================================
; is given here according to CERN_CABLE
SPI_CONFIG_MPX		SET	(SPI_CONFIG_MPX_1 + CERN_CABLE)

; Actual wait time is given here (according to value of SPI_CONFIG_MPX):
SPI_WAIT MACRO
     IF (SPI_CONFIG_MPX = SPI_CONFIG_MPX_4)
       SPI_WAIT_MAX
     ENDIF
  ENDM

;===============================================================================
; External memory reservation

XSEG           	; Set the external memory space segment
ORG 000000h


CHIPBOARD_ID:   DS 8*4			; Buffer for chip ID (max 8 chips per 4 bytes)
SCOPE_BUFF:  	DS SCOPE_BUFFLEN	; Buffer for scope - DMA acquisition will be targeted here
MONITOR_BUFF: 	DS 14*2+4 		; Buffer for monitor - DMA acquisition will be targeted here

;===============================================================================
; Code segment

CSEG                            ; Set the program memory space segment

;===============================================================================
; Flags:
;===============================================================================

; Background register holds flags from background processes
; Main loop checks this register periodicaly
; If some of these flags is set its appropriate service is called

Background      DATA    020H
Back_Interval   BIT     000H	; Flag is set from interval interrupt service (T0 timer)
Back_Period	BIT	001H	; Flag is set from period interrupt service  (T2 timer)
Back_Trigger    BIT     002H    ; Flag is set form interrupt caused by external pin INT0
Back_Counter    BIT     003H	; Flag is set from interrupt caused by T1 counter
Back_Scope	BIT	004H 	; Flag is set from ADC interrupt (after scope action)
Back_ReadMatrix	BIT	005H	; Flag is set from either interrupt - Data readout has to be done
Back_ToDoCom1	BIT	006H 	; Flag is set from either interrupt - TODO_Command1 has to be performed
Back_ToDoCom2	BIT	007H 	; Flag is set from either interrupt - TODO_Command2 has to be performed

; Medipix CMOS inputs:
;-----------------------
M_CmosIn        DATA    021H    ; Adress of byte with Medipix CMOS inputs settings
M_Reset         BIT     008H
M_PulseAdress   BIT     009H
M_EnablePulse   BIT     00AH
M_Polarity      BIT     00BH
M_SpareFSR      BIT     00CH
M_M1            BIT     00DH
M_M0            BIT     00EH
M_Shutter       BIT     00FH	; 1 = Shutter is closed

; ADC is used (for):
;--------------------
ADCused		DATA    022H
ADCused_Scope	BIT	010H
ADCused_Monitor	BIT	011H

; Other flags:
;--------------------
OFlags		DATA    023H
IgnoreNextTrg	BIT     018H    ; If IgnoreNextTrg=1 then next trigger will be received but ignored
ScopeTgrDisabled BIT    019H    ; Scope can't be started - ADC is busy or old scope data are not fully transmited


;===============================================================================
; Macros:
;===============================================================================

; Pushes all registers used by firmware on the stack
PUSHALL MACRO
	PUSH	02
	PUSH	01
	PUSH	00
	PUSH	B
	PUSH	DPP
	PUSH	DPH
	PUSH	DPL
	PUSH	ACC
	PUSH	PSW
	ENDM

; Pops all registers pushed by PUSHALL
POPALL MACRO
	POP	PSW
	POP	ACC
	POP	DPL
	POP	DPH
	POP	DPP
	POP	B
	POP	00
	POP	01
	POP	02
	ENDM

; Start watchdog timer
StartWatchdog MACRO
	SETB    WDWR
	MOV     WDCON,#72h
	ENDM

; Refresh Watchdog - use just when interrupts are disabled
RefreshWatchdogBase MACRO
	SETB    WDWR
	SETB    WDE
	ENDM

; Refresh watchdog timer
RefreshWatchdog MACRO
        CLR     EA
        RefreshWatchdogBase
        SETB    EA
	ENDM

; Macro starting interval timer: Starts the T0 timer, Enables its interrupt
IntervalStart	MACRO
	SETB	ET0             ; Enable interrupt from T0
	SETB	TR0 		; Start T0 counting
	ENDM

;  Macro stoping interval timer: Stops the T0 timer, disables its interrrupt
IntervalStop	MACRO
	CLR	TR0		; Stop T0
	CLR	ET0		; Disable T0 interrupt
	ENDM

; Macro starting interval timer: Starts the T1 counter, Enables its interrupt
CounterStart	MACRO
	SETB	ET1             ; Enable interrupt from T1
	SETB	TR1 		; Start T1 counting
	ENDM

;  Macro stoping interval timer: Stops the T1 counter, disables its interrrupt
CounterStop	MACRO
	CLR	TR1		; Stop T1
	CLR	ET1		; Disable T1 interrupt
	ENDM

; Macro starting period timer: Starts the T2 timer, Enables its interrupt
PeriodStart	MACRO
	SETB	ET2             ; Enable interrupt from T2
	SETB	TR2 		; Start T2 counting
	ENDM

;  Macro stoping period timer: Stops the T2 timer, disables its interrrupt
PeriodStop	MACRO
	CLR	TR2		; Stop T2
	CLR	ET2		; Disable T2 interrupt
	ENDM

; Macro enabling external trigger
TriggerStart	MACRO
	SETB	IgnoreNextTrg   ; Following interrupt will be ignored if it hapens imediatelly
	SETB	EX0             ; Enable interrupt from INT0
	NOP 			; Wait a litle bit
	NOP 			; Wait a litle bit
	CLR	IgnoreNextTrg   ; Now an interrupt can hapen
	ENDM

; Macro disabling external trigger
TriggerStop	MACRO
	CLR	EX0		; Disable interrupt from INT0
	ENDM

; Macro starting scope measurement
ScopeStart	MACRO
        MOV     ADCCON2,#040h   	; enable DMA mode
	SETB    CCONV           	; start continuous ADC conversions using DMA
	SETB	EADC			; Enable ADC interrupt
	SETB	ADCused_Scope		; Capture ADC
	SETB	ScopeTgrDisabled	; Scope operation is not finished
	ENDM

;===============================================================================
; Medipix support:
;===============================================================================

; Symbols:

LatchEnable     EQU     P2.4            ; 1->transparent, 0->previous
M_EnableOut     EQU     P2.5
M_EnableIn      EQU     P2.6
DCShutdown      EQU     P2.7
M_port          EQU     P0
TriggerOut	EQU	P3.1		; Txd pin (output)
TriggerIn	EQU	P3.0		; RxD pin (input)

SCL_I2C		EQU	P3.6
SDA_I2C		EQU	P3.4

Tmpx_FREQ0	EQU	P3.6		; FS0
Tmpx_FREQ1	EQU	P3.4            ; FS1


; Macros:
;=========

; Macro for setting levels stored in M_CmosIn register to CMOS inputs of Medipix
SET_CmosIn	MACRO
        MOV     M_port,M_CmosIn ; Send Medipix CMOS input configuration to latch
        SETB    LatchEnable     ; Enable Medipix CMOS input latch
        CLR     LatchEnable     ; Disable Medipix CMOS input latch
	ENDM

; Macro openning Medipix shutter
OpenShutter	MACRO
        SETB	M_M0
        SETB	M_M1
        SET_CmosIn              ; Set correct M0 and M1 (needed for timepix, can't be combined with M_Shutter - it must follow)
        CLR     M_Shutter       ; M_Shutter = 0 ... Shutter is open
        SET_CmosIn              ; Shutter is open now
	ENDM

; Macro closing Medipix shutter (TimePix version):
CloseShutterTPX	MACRO
        SETB    M_Shutter       ; M_Shutter = 1 ... Shutter is closed
        SET_CmosIn              ; Close shutter now (clock generator is stopped now in case of Timepix => some extra clock are needed)
   	MOV     SPIDAT,A        ; Write any data from ACC to SPI => This initiates clock generation (8 clocks) - needed for Timepix
        CLR	M_M0
        CLR	M_M1
        SET_CmosIn              ; Set correct M0 and M1 (needed for Timepix)
 	JNB     ISPI,$          ; Wait until whole byte is transmited by SPI => clock is quiet now
	ENDM

; Macro closing Medipix shutter (Medipix version):
CloseShutterMPX	MACRO
        SETB    M_Shutter
        CLR	M_M0
        CLR	M_M1
        SET_CmosIn              ; Close shutter
	ENDM

; Macro openning Medipix shutter and starting Interval measurement from the zero
OpenShutterInt	MACRO
	CALL	IntervalMeasure	; Starts Interval timer to measure exposition time
	OpenShutter
	ENDM

;===============================================================================
; Begining of the code
;===============================================================================


ORG 0000H                       ; Load Code at adress 0
        JMP     MAIN            ; Jump to MAIN program

;===============================================================================
; Interrupt jump table

ORG 0003H
        JMP     Int_IE0         ; External interrupt 0
ORG 000BH
        JMP     Int_TF0         ; Timer/Counter 0 interrupt
ORG 0013H
        JMP     Int_IE1         ; External interrupt 1
ORG 001BH
        JMP     Int_TF1         ; Timer/Counter 1 interrupt
ORG 0023H
        JMP     Int_RI_TI       ; Serial interrupt
ORG 002BH
        JMP     Int_TF2_EXF2    ; Timer/Counter 2 interrupt
ORG 0033H
        JMP     Int_ADCI        ; ADC interupt
ORG 003BH
        JMP     Int_ISPI_I2CI   ; SPI interrupt/I2C interrupt
ORG 0043H
        JMP     Int_PSMI        ; Power supply monitor interrupt
ORG 0053H
        JMP     Int_TII         ; Timer interval counter interrupt
ORG 005BH
        JMP     Int_WDS         ; Watchdog timer interrupt

;===============================================================================
; Interrupt services (sorted by priority level)
;===============================================================================

Int_PSMI:               	; Power supply monitor interrupt - priority 1 (Highest)
        RETI

Int_WDS:                	; Watchdog timer interrupt - priority 2
        RETI

;-------------------------------------------------------------------------------
; Trigger in:

Int_IE0:                	; External interrupt 0 - priority 3

	JB	IgnoreNextTrg,TriggerVeryReturn ; This Trigger shoulfd be ignored

	PUSH	ACC
	PUSH	PSW

	TriggerStop				; Trigger is stopped and can be restarted by TODO action

  	MOV	A,TRIGGER_TODO
	CALL	ProcessTODO			; Make tasks given by TRIGGER_TODO

	PUSH	B

  ;Coincidence mode?
  ;-----------------
	JNB 	TODO_CloseShutter, NonCoincidenceMode		; Coincidence mode is available just if TODO_CloseShutter is in TRIGGER_TODO actions
	MOV	A,TRIGGER_COINC
	JNB 	COINC_Test, NonCoincidenceMode

        ; We are working in coincidence mode: If notification is enabled raise it
        SETB	COINC_CoincEvent				; This is a coincidence event (valid or invalid)
	JNB 	COINC_Notify, CoincidenceMode 			; If no notification is desired then continue
	JB	COINC_NotifyValidCoinc, CoincidenceMode         ; If just positive coincidences should be notified then skip notification now (generate it later)

	; Generate notification:
	MOV 	C,COINC_NotifyInitLevel                         ; Trigger out initial level is restored (no change should be seen on output)
	CPL	C
	MOV	TriggerOut,C					; Trigger out is set

    ; Wait for coincidence flag (to be signaled on TriggerIn input = RxD):
    CoincidenceMode:
  	; MOV	B,TRIGGER_COINCDELAY				; Time of maximum allowed delay of TriggerIn signal
        ; JNB  	COINC_TestLevel, CoincTestLow
      CoincTestHigh:
	JB	TriggerIn, CoincApproved			; TriggerIn is signaling the coincidence        2 cycles
	DJNZ	B,CoincTestHigh                                 ; Wait 256 times                                3 cycles => 5 cycles = 250ns => max 64us
	JMP	CoincRejected
      CoincTestLow:
	JNB	TriggerIn, CoincApproved			; TriggerIn is signaling the coincidence        2 cycles
	DJNZ	B,CoincTestLow 					; Wait 256 times                                3 cycles => 5 cycles = 250ns => max 64us
	JMP	CoincRejected

      ; Coincidence is not recognized
      CoincRejected:
        SETB	COINC_InValidCoinc				; Invalid coincidence => matrix to be reseted
	JNB 	COINC_Notify, CoincidenceModeEnd 		; If no notification is desired then goto End
	JB	COINC_NotifyValidCoinc, CoincidenceModeEnd      ; If just valid coincidence should be notified then skip notification
	JMP	CoincNotifPulse

      ; Coincidence is positively identified
      CoincApproved:
	CLR     COINC_InValidCoinc     				; Valid coincidence
	JNB 	COINC_Notify, CoincidenceModeEnd 		; If no notification is desired then goto End
	JNB	COINC_NotifyValidCoinc, CoincNotifPulse		; Just valid coincidences should be notified => this is valid => notify it
	MOV 	C,COINC_NotifyInitLevel                         ; Trigger out initial level is restored (no change should be seen on output)
	CPL	C
	MOV	TriggerOut,C					; Trigger out is set

      ; If pulse mode notification is desired:
      CoincNotifPulse:
	JNB	COINC_NotifyByPulse, CoincidenceModeEnd		; If non-pulse mode is selected for notification then goto end
	MOV 	C,COINC_NotifyInitLevel                         ; Trigger out initial level is restored (no change should be seen on output)
	MOV	TriggerOut,C					; Trigger out is reset

    CoincidenceModeEnd:
    	MOV 	TRIGGER_COINC,A					; Set coincidence flags back to register

  ;Standard non coincidence mode:
  ;------------------------------
  NonCoincidenceMode:
	PUSH	00

	MOV	R0,#TRIGGER_COUNT    		; Address of current counter value is in R0
	MOV	B,#4            		; 4 bytes of counter
  TriggerIncrement:
	INC	@R0             		; Increment one byte of counter
	MOV	A,@R0
	JNZ	TriggerEvent  			; If value of the byte is not zero after increment => Overflow doesn't occured
        INC	R0              		; Next counter byte has to be inctremented as well
        DJNZ	B,TriggerIncrement     		; Do it 4 times at maximum

  TriggerEvent:                     		; overflow occured => service it
	SETB	Back_Trigger	                ; Set flag for Main loop

	POP	00
	POP	B
	POP	PSW
	POP	ACC

  TriggerVeryReturn:
        RETI

;-------------------------------------------------------------------------------

Int_ADCI:                       ; ADC interupt - priority 4
        CLR     CCONV           		; stop conversions
        JB	ADCused_Monitor, JustMonitor
        SETB    Back_Scope     			; Set flag to foreground process (scope data have to be transmited)
  JustMonitor:
        CALL	ScopeSetupADC			; Prepare ADC for next batch
        MOV	ADCused,#0			; ADC is free now
	RETI

;-------------------------------------------------------------------------------
; Interval timer:

Int_TF0:                	; Timer/Counter 0 interrupt - priority 5
	PUSH	ACC
	PUSH	PSW
	PUSH	00
	PUSH	B

	MOV	R0,#INTERVAL_CURR    		; Address of curent counter value is in R0
	MOV	B,#4            		; 4 bytes of counter
  IntervalIncrement:
	INC	@R0             		; Increment one byte of counter
	MOV	A,@R0
	JNZ	IntervalReturn  		; If value of the byte is not zero after increment => Overflow doesn't occured
        INC	R0              		; Next counter byte has to be inctremented as well
        DJNZ	B,IntervalIncrement     	; Do it 4 times at maximum

  IntervalOverflow:                     	; Timer overflow occured => call service if needed
        MOV	A, INTERVAL_MODE
        JZ	IntervalReturn                  ; If MODE is 0 then no action have to be done

      	SETB	Back_Interval	                ; Set flag for Main loop in the service of this flag the data are transfered if needed

  	IntervalStop				; Interval is stoped, can be restarted by TODO action
  	MOV	A,INTERVAL_TODO
	CALL	ProcessTODO			; Make tasks given by TODO

  IntervalReturn:
	POP	B
	POP	00
	POP	PSW
	POP	ACC
        RETI

;-------------------------------------------------------------------------------

Int_IE1:                	; External interrupt 1 - priority 6
        RETI

;-------------------------------------------------------------------------------
; Counter

Int_TF1:                	; Timer/Counter 1 interrupt - priority 7
	PUSH	ACC
	PUSH	PSW
	PUSH	00
	PUSH	B

	MOV	R0,#COUNTER_CURR    		; Address of curent counter value is in R0
	MOV	B,#4            		; 4 bytes of counter
  CounterIncrement:
	INC	@R0             		; Increment one byte of counter
	MOV	A,@R0
	JNZ	CounterReturn  			; If value of the byte is not zero after increment => Overflow doesn't occured
        INC	R0              		; Next counter byte has to be inctremented as well
        DJNZ	B,CounterIncrement     		; Do it 4 times at maximum

  CounterOverflow:                     		; Timer overflow occured => call service if some exists
  	CLR	ET1             		; Disable interrupt (can be reenabled in TODO action)
	SETB	Back_Counter	                ; Set flag for Main loop

  	MOV	A,COUNTER_TODO
	CALL	ProcessTODO			; Make tasks given by TODO

  CounterReturn:
	POP	B
	POP	00
	POP	PSW
	POP	ACC
        RETI

;-------------------------------------------------------------------------------
Int_ISPI_I2CI:          	; SPI interrupt/I2C interrupt - priority 8
        RETI

Int_RI_TI:              	; Serial interrupt - priority 9
        RETI

;-------------------------------------------------------------------------------
; Period timer:

Int_TF2_EXF2:           	; Timer/Counter 2 interrupt - priority 10

	CLR	TF2		; Clear the interrupt flag

	PUSH	ACC
	PUSH	PSW
	PUSH	00
	PUSH	B

	MOV	R0,#PERIOD_CURR    		; Address of curent counter value is in R0
	MOV	B,#4            		; 4 bytes of counter
  PeriodIncrement:
	INC	@R0             		; Increment one byte of counter
	MOV	A,@R0
	JNZ	PeriodReturn  			; If value of the byte is not zero after increment => Overflow doesn't occured
        INC	R0              		; Next counter byte has to be inctremented as well
        DJNZ	B,PeriodIncrement     		; Do it 4 times at maximum

  PeriodOverflow:                     		; Timer overflow occured => call service if some exists
  	CALL	PeriodRestart			; PERIOD_CURR is refilled

	SETB	Back_Period	                ; Set flag for Main loop

  	MOV	A,PERIOD_TODO
	CALL	ProcessTODO			; Make tasks given by TODO

  PeriodReturn:
	POP	B
	POP	00
	POP	PSW
	POP	ACC
        RETI

;-------------------------------------------------------------------------------

Int_TII:                	; Timer interval counter interrupt - priority 11 (Lowest)
	RETI


;===============================================================================
; Utilities:

;Closing Medipix shutter	- In past this was realized by MACRO but due to support of
				; TimePix it has to be changed
CloseShutter:
	PUSH 	ACC
	MOV	A,MPX_ISMXR
	JB	ACC.7,CloseShutterTimepix       ; It is Timepix => Optimized version of WaitForCommand

	CloseShutterMPX
	POP	ACC
	RET

    CloseShutterTimepix:
        CloseShutterTPX
        POP	ACC
        RET

; Closing Medipix shutter and stoping Interval measurement
CloseShutterInt:
	PUSH 	ACC
	MOV	A,MPX_ISMXR
	JB	ACC.7,CloseShutterIntTimepix    ; It is Timepix => Optimized version of WaitForCommand

	IntervalStop		; Stops Interval timer - now it contains time of exposition
	CloseShutterMPX
	POP	ACC
	RET

    CloseShutterIntTimepix:
	IntervalStop		; Stops Interval timer - now it contains time of exposition
        CloseShutterTPX
        POP	ACC
        RET

;###############################################################################
;###############################################################################
;###############                                                ################
;###############            Main program: Initialization        ################
;###############                                                ################
;###############################################################################
;###############################################################################

MAIN:
        ;JMP	$				;

; Global settings
        CLR     EA              		; Disable all interrupts
        MOV     SP,#127         		; Set the top of the stack (stack begin at adress 128)
        MOV     PLLCON,#00h     		; Core clock divider = 0, Set the highest clock frequency 20MHz
        ORL     CFG841,#01h     		; Map internal X-RAM into lower 2kB of external adress space

; Configure SPI
        MOV     SPICON,#SPI_CONFIG_MPX		; SPI is enabled in Master Mode (I2C is enabled), CPOL=0, CPHA=1, frequency Fosc/N

; Configure Timer T0 (used for Interval):
	CLR	TR0             		; Timer 0 is turned OFF
	;MOV	TMOD,#00100010h			; Both timers (0 and 1) are configured in Mode 1 (16bit timer), gating is disabled
;	CALL    IntervalDefault   		; Clears variables of Interval timer

; Configure UART
	MOV SCON,#50h
	ORL TMOD,#20h
	MOV TL1, #0FDh
	MOV TH1,#0FDh
	
	;MOV   SCON,#01010010B
	
	;SET_SMOD equ 80h ; or mask 
	;CLR_SMOD equ 7Fh ; and mask 
	;orl PCON,#SET_SMOD ; set SMOD to '1' 

	SETB	TR1
	SETB	EA
	SETB	ES
	;MOV   SCON,#01000000B
	;MOV   SCON,#00010010B

; Configure Timer T1 (used for Counter):f
            		; Timer 1 is turned OFF	
	;ORL	TMOD,#00100000b
	;ORL	TMOD,#40h			; T1 is counter now
	;CALL    CounterDefault 			; Clears variables of the counter

; Configure Timer T2 (used for Period):
	MOV	T2CON,#00h			; T2 is timer now (turned off)
	MOV	RCAP2L,#00h
	MOV	RCAP2H,#00h			; Autoreload by zero => no autoreload
	CALL    PeriodDefault  			; Clears variables of the period

; External Interrupt INT0 (used for Trigger):
	CLR	EX0				; Disable interrupt from INT0
	SETB	IT0				; INT0 is edge sensitive 1->0 edge will cause the interrupt
	SETB	PX0				; Interupt priority: high
	CALL	TriggerDefault

; DACs:
	MOV	DACCON,#01Fh			; 12bit, Range 0-2.5V, No Clear, No Sync, Power ON

; Initialize registers and memories:
	MOV 	Background,#0   		; None of background processes wants to output data
	MOV	ADCused,#0			; ADC is not used
	MOV	OFlags,#0          		; No other flag
	MOV	TODO_Command1,#'e' 		; Default TODO command1 is Erase matrix
	MOV	TODO_Command2,#'=' 		; Default TODO command2 is Periodic erase and wait for trigger
	CALL 	FrameCounterDefault		; Initialize frame counter to its default state
	CALL	MonitorSetup			; Initialize Monitor XRAM buffer
	CALL	ScopeSetDefaults		; Initialize Scope XRAM buffer

;-------------------------------------------------------------------------------
; Medipix default configuration and watchdog test:

	CALL	FT_ClearFIFO			; EMpty FTDI FIFO to discard some old messages

	JNB     WDS,NormalReset                 ; If not watchdog alert then continue to NormalReset

    ResetByWatchdog:
	MOV	DPTR,#Watchdog_Alert    	; Send Watchdog Alert message
	CALL    FT_SendString
	JMP	WatchdogConfig                  ; Skip normal Medipix cinfiguration in case of reset by Watchdog

    NormalReset:
	MOV     DPTR,#Str_Online        	; Send OnLine Message
	CALL    FT_SendString

	;CALL	ComServ_Info

    InitMedipix:
	; Timepix defaults:
	CLR	Tmpx_FREQ0			; P3.6
	SETB	Tmpx_FREQ1			; P3.4  => Lowest frequency = 10MHz (options are: 10, 20, 40, 80)

    	CALL	ComServ_ResetBias		; Reset bias
	CALL    ComServ_DCswitchON              ; Switch on medipix power supply
	CALL    WaitQuarter                     ; Wait quarter of second to power stabilisation
	CALL    ComServ_ResetToDefault          ; Medipix reset and default settings of its CMOS inputs

	CALL	GetChipboardTypeAndIDSilent	; Determine type of the chipboard connected (According to chipID - not very reliable)
	;CALL	NotifyChipboardType		; Send information concerning chip type and count
	;CALL	NotifyNumberOfChips	        ; Sends notification with number of chips

	CALL    ComServ_DCswitchOFF             ; Switch off medipix power supply
	CALL    WaitQuarter                     ; Wait quarter of second to power stabilisation
	CALL    ComServ_DCswitchON              ; Switch on medipix power supply
	CALL    WaitQuarter                     ; Wait quarter of second to power stabilisation

	CALL    ComServ_ResetToDefault          ; Medipix reset and default settings of its CMOS inputs
	CALL    ComServ_SetMatrixDefault  	; Sets default mask
	CALL	NotifyChipboardType		; Send information concerning chip type and count
	CALL	NotifyNumberOfChips	        ; Sends notification with number of chips

	MOV	DPTR,#Str_ReadySpc
	CALL	FT_SendString   		; Sends READY message

;-------------------------------------------------------------------------------
; Configure Watchdog timer
			; Enable Watchdog timer to cause
			; - 2.0 second timeout period
			; - enable WDIR bit to generate
			;   a reset and not an interrupt
WatchdogConfig:

	StartWatchdog   ; Done by macro
	MOV	B,#0

	; If Timepix is connected then Optimized version of WaitForCommand loop is used:
	MOV	A,MPX_ISMXR
	JB	ACC.7, WaitForCommand      ; It is Timepix => Optimized version of WaitForCommand


;===============================================================================
; Main Endless loop - waits for command character received via USB
;===============================================================================

WaitForCommand:         ; WaitForCommand is Main program loop
			; It is periodically checking flags from background processes
			; as well as command received from USB
			; Refreshes watchdog timer.

  	; Refresh watchdog timer
  	DJNZ	B,DontRefreshNow
  	RefreshWatchdog

     DontRefreshNow:
  	; Check Background processes:
  	PUSH	B
  	CALL	CheckBackground				; Checks whether some of background processes wants to output data

  	; Check commands received form host PC:
	CALL    FT_CheckChar    			; Checks whether the character has been received by FTDI, sets CY if no
  	POP	B
	JC	WaitForCommand  			; No command has been received

     CommandReceived:
	CALL	ProcessCommand  			; Call process command procedure
	MOV	B,#0
	JMP 	WaitForCommand				; Wait for next command


; Optimized version - for timepix clock generation
; Clock signal is generated by SPI interface just if shutter is opened
; With fastest setting it generates 8 periods with 10MHz in 16 cycles

WaitForCommandOptim:                            ; Cycles/Sum
        MOV     SPICON,#SPI_CONFIG_CLOCK

  WaitForCommandOptimLoop:
        ; If shutter is open then generate 8 clocks by SPI interface:
	JB 	M_Shutter,WFCO_ShutterClosed1   ; 4   7	; 0 = Shutter is opened, 1 = Shutter is closed
        MOV     SPIDAT,A                	; 1  12	; Write any data from ACC to SPI => This initiates the transmission and clock generation
        WFCO_ShutterClosed1:			        ; There is time for next 16 cycles but next initialization will take 5 cycles =>
        						; => time for 11 cycles
  CheckBackgroundProc:
	MOV	A,Background    		; 1   1	; Register of flags from background processes
	JZ	CheckFTDICommand		; 3   4	; Checks whether some of background processes wants to output data, if not => jump
	CALL	CheckBackground
	MOV     SPICON,#SPI_CONFIG_CLOCK	; 1   1 ; SPI is enabled in Master Mode (I2C is enabled), CPOL=0, CPHA=1, frequency Fosc/2

  CheckFTDICommand:
        ;JB 	FT_RxF,MakeWathchdogRefresh	; 4   8	; Check if FTDI is ready to be read (if not ready then jump)
	JB	RI,MakeWathchdogRefresh
	CALL	FT_TakeChar				; Takes a command to A
	POP	B
	JC	WaitForCommandOptim
	CALL	ProcessCommand 				; Process command in A
	MOV     SPICON,#SPI_CONFIG_CLOCK	; 1   1 ; SPI is enabled in Master Mode (I2C is enabled), CPOL=0, CPHA=1, frequency Fosc/2

  MakeWathchdogRefresh:
        ; If shutter is open then generate 8 clocks by SPI interface:
	JB 	M_Shutter,MakeRefreshWatchdog   ; 4  12 ; 0 = Shutter is opened, 1 = Shutter is closed
        MOV     SPIDAT,A                	; 1  17	; Write any data from ACC to SPI => This initiates the transmission and clock generation

        RefreshWatchdog 			; 8   8 ; Watchdog reset done by macro

        ; The shutter is open => generate 8 clocks by SPI interface:
        MOV     SPIDAT,A                	; 1  17	; Write any data from ACC to SPI => This initiates the transmission and clock generation
        JMP 	WaitForCommandOptimLoop		; 3   3	; Go to begining of the loop

  MakeRefreshWatchdog:
        RefreshWatchdog 			; 8   8 ; Watchdog reset done by macro
        JMP 	WaitForCommandOptimLoop		; 3   3	; Go to begining of the loop

  JustRefreshWatchdog:
  	RefreshWatchdog
	RET

;-------------------------------------------------------------------------------

ProcessCommand: 	; ProcessCommand will search the Com_Table for command A
			; if it is found it will be called else error message will be transmitted.
			; Service call is preceeded with command character transmission and
			; followed by OK/Error message transmission.

	; Refresh watchdog timer before command processing:
	RefreshWatchdog

        MOV     B,A             			; Store received command in B
        MOV	DPP,#0
        MOV     DPTR,#Com_Table 			; DPTR points to Command table

  TestItem:
    	; Compare current table item with zero (end of the table) or with received command
        CLR     A
        MOVC    A,@A+DPTR       			; Read a command from the table
        INC     DPTR            			; Points to command service routine adress
        JZ      UnknownCom      			; End of table => go to UnknownCom
        CJNE    A,B,NextItem    			; This is not just received command

        ; This is the command => after command code there is address of the command service
        CLR     A
        MOVC    A,@A+DPTR       			; Read lower half of command service address
        MOV     R1,A
        INC     DPTR
        CLR     A
        MOVC    A,@A+DPTR                               ; Read upper half of command service address
        MOV     R0,A            			; R1R0 contains address of the command service

        ; Send an echo:
	MOV	A,B
	CALL	FT_SendChar				; Echoes received command

	; Store return address on the stack and call service:
        MOV     DPTR,#EndOfCommand
        PUSH    DPL
        PUSH    DPH             			; Address of the EndOfCommand is stored in the stack (return address from command service routine)
        PUSH    00
        PUSH    01              			; Address of the command service routine is in the stack
        CLR	C					; Clear the CY flag (CY=1 after service call indicates an error)
        RET                     			; Jump to service, service has to end with RET instruction to jump to EndOfCommand

  EndOfCommand:
        ; Command service finished - check its output:
        ; if CY=0 then OK message has to be transmited,
        ; if CY=1 then message pointed by dptr (if not zero) has to be transmited

	MOV	DPP,#0          ; Message is in built in EEROM
        JC	ErrorOccured
        MOV     DPTR,#Str_OK 	; Send OK message
    SendReport:
        CALL    FT_SendString
        JMP	EndOfProcessCommand			; Command processing finished => end

    ErrorOccured:
      	MOV	A,DPH
      	ORL	A,DPL
      	JNZ	SendReport      			; Pointer to error message is in dptr => just send it
        JMP	EndOfProcessCommand                     ; Command service reported an Error itself => no other message form here, finished

  NextItem:
        INC     DPTR            			; Skip the first part of the command service routine address
        INC     DPTR            			; Skip the second part of the command service routine address
        JMP     TestItem        			; Check next table item

  UnknownCom:
        MOV     DPTR,#STR_UnknownComBeg
        CALL    FT_SendString
        MOV	A,B					; Received unknown command is in A
        CALL    FT_SendChar
        MOV     DPTR,#STR_UnknownComEnd
        CALL    FT_SendString

  EndOfProcessCommand:
	; Refresh watchdog timer after command processing:
	RefreshWatchdog
        RET

;-------------------------------------------------------------------------------

CheckBackground:	; Check Background processes
			; For each not zero flag in Background register appropriate action is taken

	MOV	A,Background    	; Register of flags from background processes
	JNZ	CheckBackgroundBody	; Checks whether some of background processes wants to output data, if not => return
	RET

	; Refresh watchdog timer before background processing:
	RefreshWatchdog

  CheckBackgroundBody:
	MOV	A,#'['
	CALL	FT_SendChar     ; Send '[' character to identify message from background process

	; Just for debugging: Send text message with background register
	;MOV	A,#'"'
	;CALL	FT_SendChar     ; Send '"' character to identify text message
	;MOV	A,Background    ; Register of flags from background processes
	;CALL	FT_SendChar     ; Send background register content
	;MOV	A,#'"'
	;CALL	FT_SendChar     ; Send '"' character to identify text message
	;MOV	A,#0dh
	;CALL	FT_SendChar     ; Send CR character to identify end of text message

    CheckBack_Interval:
        JNB	Back_Interval, CheckBack_Period
        CLR	Back_Interval                           ; Clear Interval flag
        CALL	BackServ_Interval               	; Service of Interval call

    CheckBack_Period:
        JNB	Back_Period, CheckBack_Trigger
        CLR	Back_Period    				; Clear Period flag
        CALL	BackServ_Period               		; Service of Period call

    CheckBack_Trigger:
        JNB	Back_Trigger, CheckBack_Counter
        CLR	Back_Trigger                            ; Clear trigger flag
        CALL	BackServ_Trigger                        ; Service of Trigger call

    CheckBack_Counter:
        JNB	Back_Counter, CheckBack_Scope
        CLR	Back_Counter                            ; Clear counter flag
        CALL	BackServ_Counter                        ; Service of Counter call

    CheckBack_Scope:
        JNB	Back_Scope, CheckBack_ReadMatrix
        CLR	Back_Scope                         	; Clear scope flag
        CALL	BackServ_Scope                       	; Service of Counter call

    CheckBack_ReadMatrix:
        JNB	Back_ReadMatrix, CheckBack_Command1
	CLR 	Back_ReadMatrix
	CALL	BackServ_ReadMatrix

    CheckBack_Command1:
        JNB	Back_ToDoCom1, CheckBack_Command2
        CLR	Back_ToDoCom1                         	; Clear ToDoCom1 flag
        MOV     DPTR,#Str_Command2			; Send OK message
        CALL    FT_SendString
        MOV	A,TODO_Command1
        JZ	CheckBack_Command2
        CALL	ProcessCommand                       	; Call processing of TODO_Command1

    CheckBack_Command2:
        JNB	Back_ToDoCom2, CheckBackFinished
        CLR	Back_ToDoCom2                         	; Clear ToDoCom2 flag
        MOV     DPTR,#Str_Command2			; Send message
        CALL    FT_SendString
        MOV	A,TODO_Command2
        JZ	CheckBackFinished
        CALL	ProcessCommand                       	; Call processing of TODO_Command2

    CheckBackFinished:
	MOV	A,#']'
	CALL	FT_SendChar   				; Send ']' character to identify end of background messages

  ChkBackReturn:
	; Refresh watchdog timer after background processing:
	RefreshWatchdog
  	RET

;-------------------------------------------------------------------------------

ProcessTODO:		; Calls procedures given by TODO mask stored in A
			; Called from various interrupts.
			; Changes just PSW and ACC

;  	JNZ	TDStartScope
;  	RET

  TDStartScope:
  ;---------------
  	JNB	TODO_StartScope, TDStartScopeEnd
  	; Skipping this event if previous was not processed yet (Scope is not finished)
  	JB	ScopeTgrDisabled, TDStartScopeEnd
;  	JB	Back_Trigger, TDStartScopeEnd

  	PUSH	ACC
	MOV	A,DELAY_TODOWHEN			; Should be delay inserted?
	JNB	TODO_StartScope,TDStartScopeMake
	CALL	MakeDelayNotPushA			; Delay
    TDStartScopeMake:
  	ScopeStart
    	POP	ACC
    TDStartScopeEnd:

  TDCloseShutter:
  ;---------------
	JNB 	TODO_CloseShutter, TDCloseShutterEnd
	PUSH	ACC
	MOV	A,DELAY_TODOWHEN			; Should be delay inserted?
	JNB	TODO_CloseShutter,TDCloseShutterMake
	CALL	MakeDelayNotPushA			; Delay
    TDCloseShutterMake:
	POP	ACC
	call	CloseShutterInt
	SETB	Back_ReadMatrix				; Everytime when shutter is closed in interrupt the data transfer is initiated
    TDCloseShutterEnd:

  TDOpenShutter:
  ;---------------
	JNB	TODO_OpenShutter, TDOpenShutterEnd
	PUSH	ACC
	MOV	A,DELAY_TODOWHEN			; Should be delay inserted?
	JNB	TODO_OpenShutter,TDOpenShutterMake
	CALL	MakeDelayNotPushA			; Delay
    TDOpenShutterMake:
	POP	ACC
	JB      TODO_StartInterval,TDOpenShutterNoInt   ; If also interval has to be started then it is not measured:
	CALL	IntervalMeasure
    TDOpenShutterNoInt:
	OpenShutter
    TDOpenShutterEnd:

  TDStartInterval:
  ;---------------
	JNB	TODO_StartInterval, TDStartIntervalEnd
	PUSH	ACC
	MOV	A,DELAY_TODOWHEN			; Should be delay inserted?
	JNB	TODO_StartInterval,TDStartIntervalMake
	CALL	MakeDelayNotPushA			; Delay
    TDStartIntervalMake:
	POP	ACC
	CALL	IntervalRestart
    TDStartIntervalEnd:

  TDStartTrigger:
  ;---------------
	JNB	TODO_StartTrigger, TDStartTriggerEnd
	PUSH	ACC
	MOV	A,DELAY_TODOWHEN			; Should be delay inserted?
	JNB	TODO_StartTrigger,TDStartTriggerMake
	CALL	MakeDelayNotPushA			; Delay
    TDStartTriggerMake:
	POP	ACC
	CALL	TriggerRestart
    TDStartTriggerEnd:

;  TDstartCounter:
  ;---------------
;	JNB	TODO_StartCounter,TDstartCounterEnd
;	PUSH	ACC
;	MOV	A,DELAY_TODOWHEN			; Should be delay inserted?
;	JNB	TODO_StartCounter,TDStartCounterMake
;	CALL	MakeDelayNotPushA			; Delay
;    TDStartCounterMake:
;	POP	ACC
;	CALL	CounterRestart
;    TDstartCounterEnd:

  TDMakeCommand1:
  ;---------------
	JNB	TODO_MakeCommand1, TDMakeCommand1End
	SETB	Back_ToDoCom1 				; TODO command is postponed to be executed in the main loop
    TDMakeCommand1End:

  TDMakeCommand2:
  ;---------------
	JNB 	TODO_MakeCommand2, TDMakeCommand2End
	SETB	Back_ToDoCom2                        	; TODO command is postponed to be executed in the main loop
    TDMakeCommand2End:

  TDReturn:
  	RET

;===============================================================================
; Background flags processes services:
;===============================================================================

BackServ_Interval: 	; Service of Interval process

  	MOV	DPTR,#Str_IntervalFinished
  	CALL	FT_SendString
  	RET
;-------------------------------------------------------------------------------

BackServ_Period:       	; Service of Period process

  	MOV	DPTR,#Str_PeriodFinished
  	CALL	FT_SendString
	RET

;-------------------------------------------------------------------------------

BackServ_Trigger:       ; Service of Trigger process

  	MOV	DPTR,#Str_TriggerFinished
  	CALL	FT_SendString
  	MOV	A,TRIGGER_COINC
  	JB	COINC_CoincEvent,BackServTriggerCoinc
  	RET

  BackServTriggerCoinc:
  	JB 	COINC_InValidCoinc,BackServTriggerInvCoinc
    BackServTriggerValCoinc:
  	MOV	DPTR,#Str_CoincidenceEventValid
  	CALL	FT_SendString
	RET
    BackServTriggerInvCoinc:
  	MOV	DPTR,#Str_CoincidenceEventInValid
  	CALL	FT_SendString
	RET

;-------------------------------------------------------------------------------

BackServ_Counter:       ; Service of Counter process

  	MOV	DPTR,#Str_CounterFinished
  	CALL	FT_SendString
	RET

;-------------------------------------------------------------------------------

BackServ_Scope:		; Service of Scope process
              		; Transmits scope data
			; Clears Back_Scope flag.

  	MOV	A,#'$'
  	CALL	FT_SendChar
  	CALL	ScopeSendData
  	MOV	DPTR,#Str_OK
  	CALL	FT_SendString
  	CLR	ScopeTgrDisabled		; Scope action finished
	RET

;-------------------------------------------------------------------------------

BackServ_ReadMatrix:    ; Service of any process which will set the Back_ReadMatrix flag
              		; Transmits matrix data

	call 	CloseShutterInt                 ; Close the shutter and interval (just to be sure as it is probably already stopped)

  ; If Scope is measuring wait for end of measurement:
  WaitForScope:
	MOV	A,ADCused			; is ADC free?
	JNZ	WaitForScope

  ; If Coincidence mode is set and a not valid coincidence is detected then just reset the matrix
  ReadoutOrResetMatrix:
  	MOV	A,TRIGGER_COINC
  	JNB	COINC_CoincEvent,ReadOutAndSendMatrix           ; Is it coincidence event?
  	JNB	COINC_InValidCoinc,ReadOutAndSendMatrix         ; Is it valid coincidence?

  ; Bad coincidence detected => Matrix has to be reseted
  ; Waiting for a next trigger is started:
  ResetMatrixNotifyAndContinue:
  	MOV	A,#'%'
	CALL 	FT_SendChar                     ; Send command prefix
	CALL 	IntervalElapsedTime		; Live time is notified
	MOV	DPTR,#Str_OK 			; Send OK string
	CALL	FT_SendString
  	MOV	A,#'e'
	CALL 	FT_SendChar                     ; Send command prefix
  	CALL	ComServ_EraseMatrix		; Erase matrix
	MOV	A,#150
	CALL	WaitMicroseconds		; Wait a little bit to avoid immediate trigger caused by EraseMatrix
	MOV	DPTR,#Str_OK 			; Send OK string
	CALL	FT_SendString
	CALL	CoincFinalNotification		; Make final notification if desired, clear coincidence flags

	; Reopen the shutter and wait for next trigger
	OpenShutterInt
	TriggerStart
	RET
	; Continue wait for coincidence:
	;JMP	FrameDone                	; Make the same action which is done after matrix readout if coincidence is valid.

  ; Send data via USB:
  ReadOutAndSendMatrix:
	MOV	A,#'C'
	CALL 	FT_SendChar                     ; Send prefix
	CALL	ReadMatrixAndTime   		; Read a data matrix
	MOV	DPTR,#Str_OK 			; Send OK string
	CALL	FT_SendString
	CALL	CoincFinalNotification		; Make final notification if desired, clear flags

  ; Count of taken frames:
  FrameCountTest:
	MOV	R0,#FRAME_CURR    		; Address of curent counter value is in R0
	MOV	B,#4            		; 4 bytes of counter
  FrameCounterIncrement:
	INC	@R0             		; Increment one byte of counter
	MOV	A,@R0
	JNZ	FrameDone  			; If value of the byte is not zero after increment => Overflow doesn't occured
        INC	R0              		; Next counter byte has to be inctremented as well
        DJNZ	B,FrameCounterIncrement     	; Do it 4 times at maximum

  FrameCounterOverflow:                     	; Frame counter overflow occured
  	IntervalStop
  	PeriodStop
  	CounterStop
  	TriggerStop 				; Stop all processes
  	MOV	DPTR,#Str_FramesFinished
  	CALL	FT_SendString
  	JMP FrameCounterRestart			; Reload frame counter and end

  FrameDone:
	; TODO actions:
  	MOV	A,FRAME_TODO
  	;CALL	SendTODOdescription		; Sends todo cdescription
	CALL	ProcessTODO			; Make tasks given by TODO
	RET


  ; If coincidence mode is set and notification is desired then generate it:
  CoincFinalNotification:
  	MOV	A,TRIGGER_COINC
  	JNB	COINC_CoincEvent,CoincFinalNotifDone 	; If COINC_CoincEvent is not set then coincidence mode is not used
  	JNB	COINC_Notify,CoincFinalNotifDone 	; If COINC_Notify is not set then notification is switched off
	JNB	COINC_NotifyValidCoinc, CoincFinalNotifyMake	; If all coincidenes have to be notified then do notify
	JB      COINC_InValidCoinc, CoincResetEventFlags        ; If it is invalid coincidence => skip the notification
    CoincFinalNotifyMake:
	MOV 	C,COINC_NotifyInitLevel                         ; Trigger out initial level is restored (no change should be seen on output)
	CPL	C
	MOV	TriggerOut,C					; Trigger out is set
	JNB	COINC_NotifyByPulse, CoincResetEventFlags
	CPL	C
	MOV	TriggerOut,C					; Trigger out is reset
    	CLR	C
    CoincResetEventFlags:
    	CLR	COINC_CoincEvent
    	CLR	COINC_InValidCoinc
    	MOV	TRIGGER_COINC,A				; Write reseted flags back to register
    CoincFinalNotifDone:
	RET

;===============================================================================
; Command table:
;===============================================================================
;   XX   - Command
;   XXXX - Service address
; Last command in the table has to be 0

Com_Table:
	;General commands
	;-----------------------
        DB '_'
        DW CompServ_CPUReset           		; CPU Reset, reinitialization
        DB 'i'
        DW ComServ_Info                 	; Version information
        DB '?'
        DW ComServ_Help                 	; List of all supported commands

	; Trigger out:
	;-----------------------
	DB '+'
	DW ComServ_TriggerOutHigh		; Sets trigger out line to high level
	DB '-'
	DW ComServ_TriggerOutLow 		; Sets trigger out line to low level
	DB '*'
	DW ComServ_TriggerOutCpl		; Inverts leel of trigger out line

	; Medipix
	;-----------------------
        DB 'r'
        DW ComServ_Reset                	; Medipix Reset
        DB 'R'
        DW ComServ_ResetToDefault       	; Medipix Reset, default values of CmosIn
        DB 'S'
        DW ComServ_SetMatrixStream           	; Set matrix. Values are received from USB
        DB 'E'
        DW ComServ_SetMatrixDefault         	; Matrix is set to default values FF
        DB '^'
        DW ComServ_SetMatrixDefaultNotify     	; Matrix is set to default values FF
        DB '$'
        DW ComServ_SetMatrixTestPattern       	; Matrix is set to default values AA
        DB 'e'
        DW ComServ_ReadMatrixClear          	; Erase matrix (MPX command)
        DB 'm'
        DW ComServ_ReadMatrix           	; Read matrix and send it to USB
        DB 'f'
        DW ComServ_TestFSR              	; FSR test
        DB 's'
        DW ComServ_TestFSRCyclic		; Cyclic FSR test
        DB 'd'
        DW ComServ_SetDACs              	; Set all DACS. Values received from USB
        DB 'o'
        DW ComServ_OpenShutter          	; Open the shutter
        DB 'c'
        DW ComServ_CloseShutter         	; Close the shutter
        DB 'C'
        DW ComServ_CloseShutterAndSendData 	; Closes the shutter and sends data to USB
        DB '0'
        DW ComServ_SelectFSR0           	; Selects FSR0
        DB '1'
        DW ComServ_SelectFSR1           	; Selets FSR1
        DB 'p'
        DW ComServ_PolarityNegative            	; Sets Negative polarity
        DB 'P'
        DW ComServ_PolarityPositive            	; Sets Positive polarity
        DB 'n'
        DW ComServ_SendCMOSSettings         	; Sends CMOS setings to USB
        DB 'O'
        DW ComServ_TestPulse			; Generates given number of test pulses
        DB '#'
        DW ComServ_MedipixCount 		; Sends number of detected Medipix chips
        DB '|'
        DW ComServ_ChipboardID                  ; Returns chipboard ID
        DB '3'
        DW ComServ_SetTPXClock                  ; Set clock frequency for Timepix

	; Power sources
	;-----------------------
        DB 'x'
        DW ComServ_DCswitchON           	; Switch Medipix power ON
        DB 'X'
        DW ComServ_DCswitchOFF          	; Switch Medipix power OFF
        DB 'b'
        DW ComServ_SetBias 			; Sets bias value (8 bit) is received from USB
        DB 'B'
        DW ComServ_ResetBias 			; Sets bias to 0.

	; ADCs (Monitor, Scope)
	;-----------------------
        DB 'M' 					; Performs single shot monitor measurement
        DW MonitorSingleShotMeasurement
        DB 'a'
        DW ScopeSingleShotMeasurement		; Performs single shot scope measurement
        DB 'A'
        DW ComServ_ScopeConfigure 		; Configures the scope (channels, samples, timing)
        DB '$'
        DW ComServ_ScopeSendData		; Sends last scope data (length and data)

	; DACs
	;-----------------------
	DB '>'
	DW ComServ_SetDAC			; Sets n-th internal DAC value

	; Interval timer:
	;-----------------------
        DB 'y'
        DW ComServ_IntervalReset 		; Sets zeroes to interval timer and starts it
        DB 'H'
        DW IntervalRestart                      ; Reloads CURR from TIME and starts the interval timer
        DB 'Y'
        DW ComServ_IntervalStop			; Stops the interval timer
        DB 'Z'
        DW ComServ_IntervalSet 			; Sets interval length and ToDo config
        DB 'z'
        DW ComServ_IntervalState 		; Sends current interval state (on/off, todo, all-time, curr-time)
        DB '%'
        DW ComServ_IntervalElapsedTime          ; Sends elapsed interval time
        DB 'g'
        DW ComServ_IntervalExposition		; Starts single interval exposition
        DB ';'
        DW ComServ_IntervalExpositionSet
        DB 'h'
        DW ComServ_IntervalMultiExposition 	; Starts multiple interval exposition
        DB 'G'
        DW ComServ_AbortExposition 		; Aborts interval exposition

	; Period timer:
	;-----------------------
        DB 'u'
        DW ComServ_PeriodStart
        DB 'U'
        DW ComServ_PeriodStop
        DB 'V'
        DW ComServ_PeriodSet
        DB 'v'
        DW ComServ_PeriodState
        DB '&'
        DW ComServ_PeriodicalExposition

	; Counter
	;-----------------------
        DB 'j'
        DW ComServ_CounterStart
        DB 'J'
        DW ComServ_CounterStop
        DB 'K'
        DW ComServ_CounterSet
;        DB 'k'
;        DW ComServ_CounterState

	; External trigger:
	;-----------------------
        DB 'l'
        DW ComServ_TriggerStart
        DB 'L'
        DW ComServ_TriggerStop
        DB 'Q'
        DW ComServ_TriggerSet
        DB 'q'
        DW ComServ_TriggerState
        DB '<'
        DW ComServ_TriggerCoincSet

	; Delay
	;-----------------------
        DB '/'
        DW ComServ_SetDelay

	; Frame counter
	;-----------------------
	DB '@'
	DW ComServ_FrameCounterStart
	DB '('
	DW ComServ_FrameCounterSet
	DB ')'
	DW ComServ_FrameCounterState

	; Common, ToDo commands
	;-----------------------
        DB 'W'
        DW ComServ_SetTODO
        DB 'w'
        DW ComServ_GetTODO
        DB '~'
        DW ComServ_MakeTODO

	; Some specialities:
	;-----------------------
	DB ','
	DW ComServ_WatchdogTest
	DB ' '
	DW ComServ_Synchonization
	DB '.'
	DW ComServ_OscilatingShutter
	DB '='
	DW ComServ_ResetMpxStartTriggerStartInterval

	; Flash firmware:
	;-----------------------
	DB 'F'
	DW ComServ_Flash

	; Tester:
	;-----------------------
	DB ':'
	DW ComServ_LVDSTest
	DB 'I'
	DW ComServ_CMOSTest
	DB 27h
	DW ComServ_I2CTest

	; End of the table
	;-----------------------
        DB 0

;===============================================================================
; Text data
;===============================================================================

; System messages:
; ------------------

Str_Online:
        DB '!Unit is on line. ',0
Str_ReadySpc:
	DB ' '
Str_Ready:
	DB 'Ready',0dh,0
Str_OK:
        DB 'OK',0dh,0
Str_MedipixCnt:
	DB 'Medipix chips: ',0
Str_Medipix20Cnt:
	DB 'Medipix 2.1 chips: ',0
Str_MedipixMXRCnt:
	DB 'Medipix MXR chips: ',0
Str_TimepixCnt:
	DB 'Medipix TPXXX chips: ',0
Str_Info:
        DB 'Medipix USB readout interface.',0ah,'Firmware version:'
Str_Version:
	DB ('0'+VERSION_MAJOR)
	DB '.'
	DB ('0'+VERSION_MINOR)
	DB '.'
	DB ('0'+VERSION_SUBMINOR)
        DB ('a'+CERN_CABLE)
        DB 0dh,0
Str_Help:
        DB 'Known commands: ',0
Str_SyncString:
	DB '1234567890qwertyuiopasdfghjklzxcvbnm',0

; Errors:
; --------
; Each error has to begin with '!'

Str_Error:
        DB '!Error',0dh,0
Watchdog_Alert:
        DB '!Watchdog alert',0dh,0
Str_UnknownComBeg:
        DB '!Unknown command: "',0
Str_UnknownComEnd:
        DB '"',0dh,0
Str_MedipixNotConnected:
        DB '!Medipix chipboard is not connected',0dh,0
Str_MedipiNotEnableOut:
        DB '!Bad response from Medipix - No Enable Out',0dh,0
Str_ErrorFSR:
	DB '!FSR Failed',0dh,0


; Various messages:
; ------------------

Str_FramesFinished:
	DB '"Frames finished"',0dh,0
Str_IntervalFinished:
	DB '"Interval finished"',0dh,0
Str_PeriodFinished:
	DB '"Period finished"',0dh,0
Str_CounterFinished:
	DB '"Counter overflowed"',0dh,0
Str_TriggerFinished:
	DB '"Trigger detected"',0dh,0
Str_CoincidenceEventValid:
	DB '"Coincidence valid"',0dh,0
Str_CoincidenceEventInValid:
	DB '"Coincidence invalid"',0dh,0
Str_Command1:
	DB '"Command1"',0dh,0
Str_Command2:
	DB '"Command2"',0dh,0
Str_Scope:
	DB '"Scope"',0dh,0

Str_TODO:
	DB 'TODO actions: ',0
Str_TODO_CloseShutter:
	DB 'Close shutter ',0
Str_TODO_OpenShutter:
	DB 'Open shutter ',0
Str_TODO_StartInterval:
	DB 'Start interval ',0
Str_TODO_StartScope:
	DB 'Start scope ',0
Str_TODO_StartTrigger:
	DB 'Start trigger ',0
Str_TODO_StartCounter:
	DB 'Start counter ',0
Str_TODO_Command1:
	DB 'Command1 ',0
Str_TODO_Command2:
	DB 'Command2 ',0

;===============================================================================
; Stream length for different Medipixes:
;===============================================================================
; Number of bytes to be received from Medipix as a response to ReadMatrix:
; Or number of bytes to be transmited during WriteMatrix
; The most significant byte is first!

StreamLength_Medipix21:
  Mpx21_Single:
  	DB 	001h,0c0h,001h     	; 114689
  Mpx21_Double:
  	DB 	003h,080h,001h     	; 229377
  Mpx21_Triple:
  	DB 	005h,040h,001h     	; 344065
  Mpx21_Quad:
  	DB 	007h,000h,001h     	; 458753

StreamLength_MedipixMXR:
  MpxMXR_Single:
  	DB 	001h,0c0h,021h       	; 114721
  MpxMXR_Double:
  	DB 	003h,080h,042h     	; 229442
  MpxMXR_Triple:
  	DB 	005h,040h,063h     	; 344163
  MpxMXR_Quad:
  	DB 	007h,000h,084h     	; 458884


;===============================================================================
; Medipix support:
;===============================================================================

CompServ_CPUReset:		;CPU reset
	MOV	DPTR,#Str_OK
	CALL	FT_SendString
	JMP	MAIN
;_______________________________________________________________________________

                        	; General reset of Medipix2
ComServ_Reset:          	; Resets only Medipix2 chip. The CMOS inputs are in the previous state

        CLR     M_Reset
        SET_CmosIn
        SETB    M_Reset
        SET_CmosIn
        SETB    M_EnableIn
        RET
;_______________________________________________________________________________
                                ; General reset of Medipix2
ComServ_ResetToDefault:         ; Resets the Medipix2 chip. The CMOS inputs are in default state:
				; Reset=1, PulseAdress=0,EnablePulse=0, Polarity=1, SpareFSR=0, M1=0, M0=0,

        CLR     M_Reset
        SET_CmosIn
        MOV     M_CmosIn,#089H          ; Set the Medipix input byte to default values: Reset=1, PulseAdress=0, EnablePulse=0, Polarity=1, SpareFSR=0, M1=0, M0=0, Shutter=1
        SET_CmosIn
        SETB    M_EnableIn
	RET
;_______________________________________________________________________________
                                ; Erase the Medipix2 matrix
ComServ_SetMatrixDefault:   	; Erase matrix sending 0FFH to all pixels

	MOV	R2,#0FFH		; Data to be written

  ; Alternative entry point - data from R2 are written to MPX:
  SetMatrixR2:

	MOV     SPICON,#SPI_CONFIG_MPX	; SPI is enabled in Master Mode (I2C is enabled), CPOL=0, CPHA=1, frequency Fosc/N
        CLR	CPHA                    ; Write data on leading edge (prepare on trailing edge)

        CLR     M_M0                    ; Set the control bits to Set Matrix configuration
        SETB    M_M1
        SETB    M_Shutter
        SET_CmosIn                      ; Put control bits to output register
        CLR     M_EnableIn              ; Start the operation

	JB      M_EnableOut,SetMatrixStartBytes ; Is Enable out in correct state?
	MOV	MPX_CHIPS,#00h
	MOV	DPTR,#Str_MedipiNotEnableOut	; Load pointer to Error message to DPTR
	SETB	C                               ; CY flag indicates an Error
	JMP	SetMatrixDefEnd

  SetMatrixStartBytes:
	MOV	DPTR,#0
	MOV	DPP,#0 			        ; Initialize byte counter (24 bit DPTR)

        MOV     A,R2				; Data to be sent to Medipix

  SetMatrixDefByte:
        MOV     SPIDAT,A                	; Write data from ACC to SPI => This initiates transmission

	; To avoid deadlock let count bytes:           	Cycles:
	INC	DPTR				;		3

	MOV	A,DPL                           ;               1
	JNZ	SetMatrixByteDefContinue	;	        2

	; Just refresh watchdog whenever 256 bytes are transmitted
	RefreshWatchdog

	MOV	R0,DPP                          ;               1
	CJNE	R0,#8,SetMatrixByteDefContinue	;               4       => 11 cycles => even at max SPI speed transmission of one byte
						;					will take 16 cycles => OK
    SetMatrixNoMpxDetected:
        ; If we are here then Medipix chipboard is probably not connected:
	MOV	MPX_CHIPS,#00h
	MOV	DPTR,#Str_MedipixNotConnected	; Load pointer to Error message to DPTR
	SETB	C                               ; CY flag indicates an Error
	JMP	SetMatrixDefEnd

    SetMatrixByteDefContinue:
        MOV     A,R2				; Data to be sent to Medipix
        JNB     ISPI,$                  	; Wait until whole byte is transmited by SPI
        CLR     ISPI                    	; Clear ISPI

	SPI_WAIT				; If low communication speed is set then certain wait time is inserted here

        JB      M_EnableOut,SetMatrixDefByte    ; End of transmission?

	; Mask has been successfully loaded
	; Let us determine number of Medipix chips:
	MOV	A,DPP				; ACC = 01 (Single), 03 (Double), 05 (Triple), 07 (Quad)
	INC	A 				; ACC = 02 (Single), 04 (Double), 06 (Triple), 08 (Quad)
	RR	A				; ACC = 01 (Single), 02 (Double), 03 (Triple), 04 (Quad) => v ACC je pocet cipu
	ANL	A,#07h
	MOV	MPX_CHIPS,A

	; Let us determine type of chip:
	MOV	A,MPX_ISMXR
	ANL	A,#80h                          ; Reset number of MXR chips, conserve just Timepix flag
	MOV	MPX_ISMXR,A

	MOV	A,DPL				; ACC = 01 (Mpx21), 21/42/63/84 (MpxMXR)
	ANL	A,#0F0h 			; ACC = 00 (Mpx21), 20/40/60/80 (MpxMXR)
	SWAP	A 				; ACC = 00 (Mpx21), 02/04/06/08 (MpxMXR)
	RR	A 				; ACC = 00 (Mpx21), 01/02/03/04 (MpxMXR)

	ORL	A,MPX_ISMXR			; ADD Timepix fleg if it was set
	MOV	MPX_ISMXR,A			; Zero for Mpx21, Nonzero for MXR (number of MXR chips)

	; Divide it by 16384 - shift right 8 x and then 6 x, then divide by 7 = divide by 114688
	;MOV	A,DPH
	;SWAP	A
	;RR	A
	;RR	A
	;ANL	A,#03h
	;MOV	DPL,A
	;MOV	A,DPP
	;RL	A
	;RL	A
	;ORL	A,DPL
	;MOV	B,#7
	;DIV	AB
	;MOV	MPX_CHIPS,A
	CLR	C

   SetMatrixDefEnd:
        SETB    M_EnableIn              ; End of operation
	MOV	DPP,#0			; As DPTR overflowed DPP can be nonzero => reinitialize it to zero
        RET

ComServ_SetMatrixTestPattern:
	MOV	R2,#0AAH		; Data to be written
  	CALL 	SetMatrixR2		; Jump to SetMatrix function
  	RET

;_______________________________________________________________________________

ComServ_SetMatrixDefaultNotify:         ; Sets default Matrix in Medipix
					; Outputs notification containing number of detected chips
	CALL	ComServ_SetMatrixDefault
	;JC	SetMatrixDefError

  NotifyNumberOfChips:                  ; Outputs notification with number of chips connected
	MOV	DPTR,#Str_MedipixCnt
	CALL	FT_SendString
	MOV	A,#'0'
	ADD	A,MPX_CHIPS
	CALL	FT_SendChar
	RET

  SetMatrixDefError:
	CALL	FT_SendString
	RET

;_______________________________________________________________________________

ComServ_MedipixCount:
	MOV	A,MPX_CHIPS
	CALL	FT_SendChar
	RET

;_______________________________________________________________________________
                        	; Erase the Medipix2 matrix
ComServ_EraseMatrix:    	; Erase matrix using Medipix2 commands

	MOV	A,MPX_ISMXR
	JNZ	DummyReadMatrix         ; MXR doesn't support erase command

	MOV     SPICON,#SPI_CONFIG_MPX	; SPI is enabled in Master Mode (I2C is enabled), CPOL=0, CPHA=1, frequency Fosc/N

        CLR     CPHA                    ; Write data on leading edge (prepare on trailing edge)
        SETB    M_M0                    ; Set control bits to Reset Matrix configuration
        SETB    M_M1
        SETB    M_Shutter
        SET_CmosIn
        CLR     M_EnableIn              ; Start the operation

	MOV	DPTR,#0
	MOV	DPP,#0 			; Initialize byte counter (24 bit DPTR)

        MOV     A,#0FFH			; This data has to be written to Medipix

  EraseMatrixByte:
        MOV     SPIDAT,A                	; Write data from ACC to SPI => This initiates transmission

	; To avoid deadlock let count bytes:    	Cycles:
	INC	DPTR				;		3
	MOV	R0,DPP                          ;               1
	CJNE	R0,#1,EraseMatrixByteContinue	;	        4       => 8 cycles => 	even at max SPI speed transmission of one byte
						;					will take 16 cycles => OK
	; If we are here then Medipix chipboard is probably not connected:
	MOV	MPX_CHIPS,#00h
	MOV	DPTR,#Str_MedipixNotConnected	; Load pointer to Error message to DPTR
	SETB	C                               ; CY flag indicates an Error
	JMP	EraseMatrixEnd

    EraseMatrixByteContinue:
        JNB     ISPI,$                  ; Wait until whole byte is transmited by SPI
        CLR     ISPI                    ; Clear ISPI
	SPI_WAIT			; If low communication speed is set then certain wait time is inserted here
        JB      M_EnableOut,EraseMatrixByte

	; If we are here then at least one Medipix chip is present
	; We can check number of Medipix chips.

	CLR	C

  EraseMatrixEnd:
        SETB    M_EnableIn              ; End of operation
	MOV	DPP,#0			; As DPTR overflowed DPP can be nonzero => reinitialize it to zero
        RET

; Dymmy reading of a matrix (no data are sent to USB) - just to erase matrix
DummyReadMatrix:
	MOV     SPICON,#SPI_CONFIG_MPX	; SPI is enabled in Master Mode (I2C is enabled), CPOL=0, CPHA=1, frequency Fosc/N
        SETB    CPHA                    ; Read data on trailing edge (prepare on leading edge)

        CLR     M_M0                    ; Set control bits to Read Matrix configuration
        CLR     M_M1
        SETB    M_Shutter		; Shutter is closed
        SET_CmosIn
        CLR     M_EnableIn              ; Start the operation

        MOV     A,#0FFH
  DRM_1:
        MOV     SPIDAT,A                ; Transmit one byte (transmitted data are dummy - just to initiate clocks)
        JNB     ISPI,$                  ; Wait until whole byte is received by SPI
        CLR     ISPI
	SPI_WAIT			; If low communication speed is set then certain wait time is inserted here
        JB      M_EnableOut,DRM_1       ; End of transmission?

        SETB    M_EnableIn              ; End of operation
        RET

;_______________________________________________________________________________
                        	; Set the matrix via USB
ComServ_SetMatrixStream:

        ; We will count number of transmitted bytes:
	MOV	DPTR,#0
	MOV	DPP,#0 			; Initialize byte counter (24 bit DPTR)

	; Configure SPI interface
	MOV     SPICON,#SPI_CONFIG_MPX	; SPI is enabled in Master Mode (I2C is enabled), CPOL=0, CPHA=1, frequency Fosc/N
        CLR     CPHA                    ; Write data on leading edge (prepare on trailing edge)

	; Configure Medipix interface
        CLR     M_M0                    ; Set the control bits to Set Matrix configuration
        SETB    M_M1
        SETB    M_Shutter
        SET_CmosIn
        CLR     M_EnableIn              ; Start the operation

        MOV     SBUF,#0FFH			; Dummy byte is not taken from USB
  SetMatrixLoop:
        ; MOV     FT_port,A               ; Set FT_port as input
        ; JB      FT_RxF,$                ; Wait until FTDI is ready to read
        ; CLR     FT_RD                   ; Make read data pulse
	;CLR	TI
	;MOV	SBUF, #41
	;JNB	TI,$

	; Just refresh watchdog whenever 256 bytes are transmitted
	RefreshWatchdog

	JNB	RI,SetMatrixLoop              ; test if it is a reception
       	CLR	RI			       ; clear reception flag for next reception

	MOV SPIDAT, SBUF

 	;MOV	A, #'A'
	;CALL FT_SendChar

	; To avoid deadlock let count bytes:       Cycles:
	INC	DPTR				;   	3
	MOV	R0,DPL                          ;       1
	CJNE	R0,#0,SetMatrixContinue		;	4 => 8 cycles => even at max SPI speed transmission of one byte will take 16 cycles => OK

	; Just refresh watchdog whenever 256 bytes are transmitted
	RefreshWatchdog

	MOV	R0,DPP
	CJNE	R0,#8,SetMatrixContinue		; Too many bytes written => Error

	; If we are here then to many bytes are transmitted => error
	MOV	DPTR,#Str_MedipixNotConnected	; Load pointer to Error message to DPTR
	SETB	C                               ; CY flag indicates an Error
	JMP	SetMatrixEnd

  SetMatrixContinue:

 	;MOV	A, #'B'
	;CALL FT_SendChar

        JNB     ISPI,$                  ; Wait until whole byte is sent/received by SPI

	;MOV	A, #'C'
	;CALL FT_SendChar

        CLR     ISPI                    ; Clear ISPI
	SPI_WAIT			; If low communication speed is set then certain wait time is inserted here
        JB      M_EnableOut,SetMatrixLoop

        CLR	C                       ; No error

  SetMatrixEnd:
	;MOV	A, #'X'
	;CALL FT_SendChar
        SETB    M_EnableIn              ; Stop the operation
	MOV	DPP,#0			; As DPTR overflowed DPP can be nonzero => reinitialize it to zero
        RET
;_______________________________________________________________________________
                        	; Alternative entry point of ReadMatrix
                        	; Sends elapsed interval time first				
ComServ_ReadMatrix:
	NOP				; To have possibility to set breakpoint here

ReadMatrixAndTime:
	call	CloseShutterInt		; Closes shutter and stops the interval if it is not stopped yet
	;; TADY SE PREDTIM POSILAL CAS
	;CALL 	IntervalElapsedTime	; Sends 6 bytes with elapsed time

				; Entry point of ReadMatrix
				; Sends data read from MPX chip
ReadMatrix:
        ; We will count number of transmitted bytes:
	CALL    Wait10MicroSeconds
	MOV	DPTR,#0
	MOV	DPP,#0 			; Initialize byte counter (24 bit DPTR)

	; Configure SPI interface
	MOV     SPICON,#SPI_CONFIG_MPX	; SPI is enabled in Master Mode (I2C is enabled), CPOL=0, CPHA=1, frequency Fosc/N

	; Configure Medipix interface
        CLR     M_M0                    ; Set control bits to Read Matrix mode
        CLR     M_M1
        SETB    M_Shutter		; 1 = Shutter is closed
        SET_CmosIn

        CLR     M_EnableIn              ; Start the operation

        MOV     A,#0FFH
        MOV     SPIDAT,A                ; Transmit dymmy byte
        JMP     ReadMatrixContinue	; Go to loop entry point

  ReadMatrixLoop:
        MOV     SPIDAT,A                ; Transmit one byte (transmitted data are dummy - just to initiate clocks)
        
	;; TADY SE DRIVE CEKALO, NEZ BYLO FTDI READY
	;JB      FT_TxE,$                ; Wait util FTDI is ready to write
        ;CLR     FT_WR                   ; Make a write pulyse
        ;SETB    FT_WR 			; Prepare a write pulse
	
	;; CEKA SE AZ BUDE UART READY
	JNB	TI,$

  ReadMatrixLoopEntry:
	; To avoid deadlock let count bytes:    	Cycles:
	INC	DPTR				;		3
	MOV	A,DPL                           ;               1
	JNZ	ReadMatrixContinue		;	        2       => 6 cycles => even at max SPI speed transmission of one byte will take 16 cycles => OK
	JMP 	WaitingForPes

   WaitingForPes:

	; Just refresh watchdog whenever 256 bytes are received
	RefreshWatchdog

	JNB	RI,WaitingForPes

	;MOV	A, #0DH
	;CALL FT_SendChar

	CLR 	RI

	MOV	R0,DPP
	CJNE	R0,#8,ReadMatrixContinue	; Too many bytes => Error

	; If we are here then to many bytes are received => error
	MOV	DPTR,#Str_MedipixNotConnected	; Load pointer to Error message to DPTR
	SETB	C                               ; CY flag indicates an Error

  ReadMatrixContinue:
        JNB     ISPI,$                  ; Wait until whole byte is received by SPI
        ;MOV     FT_port,SPIDAT          ; Send received data from SPIDAD to FTDI port (SPI is cleared)

	;; TADY SE POSILAJI TA SKUTECNA DATA ZE SENSORU
	;;;; UART ;;;;
	CLR 	TI			; write uart
	MOV 	SBUF,SPIDAT

       	SPI_WAIT			; If low communication speed is set then certain wait time is inserted here
        JB      M_EnableOut,ReadMatrixLoop     ; End of transmision?

	;; TADY SE DRIVE POSILALA DATA DO FTDI
        ;JB      FT_TxE,$                ; Wait util FTDI is ready to write
        ;CLR     FT_WR                   ; Make a write pulse
        ;SETB    FT_WR
	
	;; TADY SE CEKA, NEZ JSOU DATA (ZNAK) POSLANA
	JNB	TI,$			; Wait until Uart is written

        CLR	C                       ; No error

  ReadMatrixEnd:
        SETB    M_EnableIn              ; End of operation
	MOV	DPP,#0			; As DPTR overflowed DPP can be nonzero => reinitialize it to zero
        RET
;_______________________________________________________________________________

ComServ_ReadMatrixClear:
	NOP				; To have possibility to set breakpoint here

ReadMatrixClearAndTime:
	call	CloseShutterInt		; Closes shutter and stops the interval if it is not stopped yet
	;; TADY SE PREDTIM POSILAL CAS
	;CALL 	IntervalElapsedTime	; Sends 6 bytes with elapsed time

				; Entry point of ReadMatrix
				; Sends data read from MPX chip
ReadMatrixClear:
        ; We will count number of transmitted bytes:
	CALL    Wait10MicroSeconds
	MOV	DPTR,#0
	MOV	DPP,#0 			; Initialize byte counter (24 bit DPTR)

	; Configure SPI interface
	MOV     SPICON,#SPI_CONFIG_MPX	; SPI is enabled in Master Mode (I2C is enabled), CPOL=0, CPHA=1, frequency Fosc/N

	; Configure Medipix interface
        CLR     M_M0                    ; Set control bits to Read Matrix mode
        CLR     M_M1
        SETB    M_Shutter		; 1 = Shutter is closed
        SET_CmosIn

        CLR     M_EnableIn              ; Start the operation

        MOV     A,#0FFH
        MOV     SPIDAT,A                ; Transmit dymmy byte
        JMP     ReadMatrixClearContinue	; Go to loop entry point

  ReadMatrixClearLoop:
        MOV     SPIDAT,A                ; Transmit one byte (transmitted data are dummy - just to initiate clocks)
        
	;; TADY SE DRIVE CEKALO, NEZ BYLO FTDI READY
	;JB      FT_TxE,$                ; Wait util FTDI is ready to write
        ;CLR     FT_WR                   ; Make a write pulyse
        ;SETB    FT_WR 			; Prepare a write pulse
	
	;; CEKA SE AZ BUDE UART READY
	;; ... NECEKA, V TETO METODE NECHCEME POSILAT DATA NA SERIOVOU LINKU
	;; JNB	TI,$

  ReadMatrixClearLoopEntry:
	; To avoid deadlock let count bytes:    	Cycles:
	INC	DPTR				;		3
	MOV	A,DPL                           ;               1
	JNZ	ReadMatrixClearContinue		;	        2       => 6 cycles => even at max SPI speed transmission of one byte will take 16 cycles => OK
	JMP 	WaitingForPesClear

   WaitingForPesClear:

	; Just refresh watchdog whenever 256 bytes are received
	RefreshWatchdog
	
	;; tady se cekalo az xmega posle potvrzovaci znak
	;; JNB	RI,WaitingForPesClear

	;MOV	A, #0DH
	;CALL FT_SendChar

	;; CLR 	RI
	MOV	R0,DPP
	CJNE	R0,#8,ReadMatrixClearContinue	; Too many bytes => Error

	; If we are here then to many bytes are received => error
	MOV	DPTR,#Str_MedipixNotConnected	; Load pointer to Error message to DPTR
	SETB	C                               ; CY flag indicates an Error

  ReadMatrixClearContinue:
        JNB     ISPI,$                  ; Wait until whole byte is received by SPI
        ;MOV     FT_port,SPIDAT          ; Send received data from SPIDAD to FTDI port (SPI is cleared)

	;; TADY SE POSILAJI TA SKUTECNA DATA ZE SENSORU
	;;;; UART ;;;;
	;; CLR 	TI			; write uart
	;; MOV 	SBUF,SPIDAT

       	SPI_WAIT			; If low communication speed is set then certain wait time is inserted here
        JB      M_EnableOut,ReadMatrixClearLoop     ; End of transmision?

	;; TADY SE DRIVE POSILALA DATA DO FTDI
        ;JB      FT_TxE,$                ; Wait util FTDI is ready to write
        ;CLR     FT_WR                   ; Make a write pulse
        ;SETB    FT_WR
	
	;; TADY SE CEKA, NEZ JSOU DATA (ZNAK) POSLANA
	;; ... V TETO METODE NEPOSILAME DATA PO SERIOVE LINCE
	;;JNB	TI,$			; Wait until Uart is written

        CLR	C                       ; No error

  ReadMatrixClearEnd:
        SETB    M_EnableIn              ; End of operation
	MOV	DPP,#0			; As DPTR overflowed DPP can be nonzero => reinitialize it to zero
        RET
;_______________________________________________________________________________

                            	; Test Fast Shift Registers by microcontroller
ComServ_TestFSR:


  	; Make MPX reset to copy MXR ID to shift register:
        CLR     M_Reset
        SET_CmosIn
        SETB    M_Reset
        SET_CmosIn

  	; Configure SPI interface:
	MOV     SPICON,#SPI_CONFIG_MPX	; SPI is enabled in Master Mode (I2C is enabled), CPOL=0, CPHA=1, frequency Fosc/N
        CLR     CPHA                    ; Write data on leading edge (prepare on trailing edge)

  	; Configure Medipix interface:
        SETB    M_M0                    ; Set the control bits to Set DACs configuration
        CLR     M_M1
        SETB    M_Shutter
        SET_CmosIn                      ; Write control bits to CMOS control register
        CLR     M_EnableIn              ; Start the operation

  ; Send 1 dummy byte:
        MOV     SPIDAT,#0FFH            ; Move dummy data to SPI interface
        JNB     ISPI,$                  ; Wait until whole byte is sent/received by SPI
	SPI_WAIT			; If low communication speed is set then certain wait time is inserted here

  ; Now send max 256 bytes of FFH => enough for 8 chips
  	MOV	B,#0
  	MOV	R0,#0                   ; RO is used as counter of transmitted bytes
  TestFSRSendByte:
        MOV     SPIDAT,#0FFH            ; Move data to SPI interface
        JNB     ISPI,$                  ; Wait until whole byte is sent/received by SPI
        CLR     ISPI                    ; Clear ISPI
	SPI_WAIT			; If low communication speed is set then certain wait time is inserted here

        MOV	A,SPIDAT		; Received data byte is in A
  	CALL	FT_SendChar		; Just for debugging
  	INC	R0

        JNB     M_EnableOut,TestFSREnableOut     ; End of transmision?

        DJNZ    B,TestFSRSendByte	;

  TestFSRfailed:
        SETB    M_EnableIn              ; End of operation
	MOV	DPTR,#Str_ErrorFSR	; Enable out was not received
	SETB	C			; Error
        RET

  TestFSREnableOut:                     ; Enable out was received
        SETB    M_EnableIn              ; End of operation
  	MOV	A,R0
  	MOV	B,#32
  	DIV	AB                      ; Divide number of bytes by 32 => in A is number of chips
  	MOV	MPX_CHIPS,A		; Initialize MPX_CHIPS register

  	MOV	dptr,#Str_MedipixCnt
  	CALL	FT_SendString
  	MOV	A,MPX_CHIPS
  	ADD	A,#'0'			; Convert number of chips to ASCII
  	CALL	FT_SendChar
  	MOV	A,#0dh
  	CALL	FT_SendChar		; Send terminating 'CR'

  	MOV	A,MPX_CHIPS		; Initialize MPX_CHIPS register
  	JZ	TestFSRfailed           ; If zero chips then error

  TestFSRdoneOK:
  	CLR	C
  	RET				; FSR test has beenleted successfully


;_______________________________________________________________________________
				; Cyclic FSR test
				; FSR_Test is repeated till next byte is receiveed from FTDI

ComServ_TestFSRCyclic:

	CALL	ComServ_TestFSR		; Make single FSR test
        JNC	TestFSRCycl_NoErr
        ; Error occured => print it
        CALL	FT_SendString

  TestFSRCycl_NoErr:
  	; Reset watchdog
  	RefreshWatchdog
  	; Wait a little:
  	CALL 	WaitQuarter
  	; Reset watchdog
  	RefreshWatchdog

        ; Is there some stop command in FTDI?
        JB      FT_RxF,ComServ_TestFSRCyclic                ; Repeat until FTDI is ready to read
        CLR     FT_RD                   ; Make FTDI read pulse
        SETB    FT_RD                   ; End of FTDI read pulse

  	CLR	C
  	RET				; FSR test has been completed successfully

;_______________________________________________________________________________
				; Get ID of the chipboard, transmits ID stored in X memory
				; IDs are copied to X-memory by CALL GetChipboardTypeAndIDSilent
				; during reset sequence
ComServ_ChipboardID:
	MOV	B,MPX_CHIPS 		; Number of chips is in B
	MOV	dptr,#CHIPBOARD_ID	; Pointer to memory buffer with ID(s)

  TransmitSingleID:
  	MOVX	A,@dptr			; first byte of ID to A
  	CALL	FT_SendChar		; Send it
  	INC	dptr
  	MOVX	A,@dptr			; second byte of ID to A
  	CALL	FT_SendChar		; Send it
  	INC	dptr
  	MOVX	A,@dptr			; third byte of ID to A
  	CALL	FT_SendChar		; Send it
  	INC	dptr
  	MOVX	A,@dptr			; forth byte of ID to A
  	CALL	FT_SendChar		; Send it
  	INC	dptr
  	DJNZ	B,TransmitSingleID	; Repeat it for all chips
  	RET


; Get ID of the chipboard, number of chips and IsMXR flag, chip IDs are copied to X memory.
; Medipix FSR is reseted to value 0x88

GetChipboardTypeAndIDSilent:
  ; Make MPX reset to copy MXR ID to shift register:
        CLR     M_Reset
        SET_CmosIn
        SETB    M_Reset
        SET_CmosIn

  ; Clear the MXR flag and a number of chips:
	MOV	MPX_ISMXR,#0
	MOV 	MPX_CHIPS,#0

  ; Configure SPI interface:
	MOV     SPICON,#SPI_CONFIG_MPX	; SPI is enabled in Master Mode (I2C is enabled), CPOL=0, CPHA=1, frequency Fosc/N
        CLR     CPHA                    ; Write data on leading edge (prepare on trailing edge)

  ; Configure Medipix interface:
        SETB    M_M0                    ; Set the control bits to Set DACs configuration
        CLR     M_M1
        SETB    M_Shutter
        SET_CmosIn                      ; Write control bits to CMOS control register
        CLR     M_EnableIn              ; Start the operation

	MOV	B,#0			; Counter of bytes (max 256 => 8 chips)
	MOV	R0,#5			; Counter of ID position
	MOV	R1,#4			; Counter of ID bytes
	MOV	dptr,#CHIPBOARD_ID	; Pointer to memory buffer for ID

  ; Start with one dummy byte:
  	; Put any data to SPI:
        MOV     SPIDAT,#0FFH            ; Move default data to SPI interface
        JNB     ISPI,$                  ; Wait until whole byte is sent/received by SPI
        CLR     ISPI                    ; Clear ISPI

	SPI_WAIT			; If low communication speed is set then certain wait time is inserted here

  ; Continue with FSR bits:
  SetDACdefbyteS:
  	; Put default data to SPI:
        MOV     SPIDAT,#0FFH            ; Move default data to SPI interface
        JNB     ISPI,$                  ; Wait until whole byte is sent/received by SPI
        CLR     ISPI                    ; Clear ISPI

        MOV     A,SPIDAT                ; Get received data from SPIDAT (SPI is cleared)

  	;CALL	FT_SendChar		; Just for debugging

    ; Is it ID byte?
	DJNZ	R0,SetDACdefnextbyteS	; Increment ID position counter, if not zero jump to continuation

        ; Now we are at the ID position:
        ; Get byte received from SPI and put it to Xmemory:
        MOVX	@dptr,A			; Save byte to buffer
        INC	DPTR

  	;CALL	FT_SendChar		; Just for debugging

	JZ	GotonextIDbyteS		; zero ID is invalid
	INC	A                       ; if ID is 0xFF it overflows to 0x00
	JZ	GotonextIDbyteSdecA     ; 0xFF ID is also invalid

	INC	MPX_ISMXR               ; valid ID => it is MXR (or Timepix)

      GotonextIDbyteSdecA:
	DEC	A			; Recover original value of ACC

    GotonextIDbyteS:
	DJNZ	R1,OtherIDbytefollowsS  ; R1 says how many bytes the ID takes
	MOV	R0,#28 			; next ID will come after 28 = 32 - 4 - 4 + 4 bytes
	MOV	R1,#4 			; next ID will take 4 bytes
	INC	MPX_CHIPS		; At least one chip is connected

	; Timepix test: Last ID byte (in A) contains 0x10 bit (if it is not MXR then it is definitely not Timepix)
	JNB	ACC.4,SetDACdefnextbyteS; Timepix bit is not set => not a Timepix => continue
	MOV	A,MPX_ISMXR
	JZ	SetDACdefnextbyteS      ; To be Timepix it has to be MXR
	SETB	ACC.7
	MOV	MPX_ISMXR,A             ; Timepix => MPX_ISMXR = 0x8X
	JMP	SetDACdefnextbyteS

    OtherIDbytefollowsS:
	MOV	R0,#1			; DJNZ will reach zero next time

        ; Prepare for next byte:
    SetDACdefnextbyteS:
        JNB     M_EnableOut,DACdefEndS	; M_EnableOut=0 is signal of end
        DJNZ	B,SetDACdefbyteS	; Repeat at maximum 256 times (8 chips)

  DACdefEndS:
        SETB    M_EnableIn              ; End of operation

	; Determine number of chips
	MOV	A,B
	CPL	A
	INC	A
	INC	A
	MOV	B,#32
	DIV	AB
	MOV	MPX_CHIPS,A
	RET


NotifyChipboardType:
	MOV	A,MPX_ISMXR
	JZ	NCT_NotMXRchipboard

	JNB	ACC.7,NCT_MXRChipboard
	MOV	dptr,#Str_TimepixCnt
	JMP	NCT_SendType

  NCT_MXRChipboard:
	MOV	dptr,#Str_MedipixMXRCnt
	JMP	NCT_SendType
  NCT_NotMXRchipboard:
  	MOV	dptr,#Str_Medipix20Cnt

  NCT_SendType:
  	CALL	FT_SendString
  	MOV	A,MPX_CHIPS
  	ADD	A,#'0'			; Convert number of chips to ASCII
  	CALL	FT_SendChar
  	MOV	A,#' '
  	CALL	FT_SendChar		; Send terminating Space
  	RET

;_______________________________________________________________________________
                                ; Set DACs of Medipix2
ComServ_SetDACs:                ; Set the Medipix DACs - dummy byte is sent by PC!!

  ; Configure SPI interface:
	MOV     SPICON,#SPI_CONFIG_MPX	; SPI is enabled in Master Mode (I2C is enabled), CPOL=0, CPHA=1, frequency Fosc/N
        CLR     CPHA                    ; Write data on leading edge (prepare on trailing edge)

  ; Configure Medipix interface:
        SETB    M_M0                    ; Set the control bits to Set DACs configuration
        CLR     M_M1
        SETB    M_Shutter
        SET_CmosIn

  ; Start the operation:
        CLR     M_EnableIn              ; Start the operation

        ;MOV     A,#0FFH
        ;MOV     FT_port,A               ; Set FT_port as input

	MOV 	DPTR,#0			; Counter of stream bytes
  CS080:
        ;JB      FT_RxF,$                ; Wait until FTDI is ready to read
        ;CLR     FT_RD                   ; Make FTDI read pulse
        ;MOV     SPIDAT,FT_port          ; Move data from FTDI to SPI interface
        ;SETB    FT_RD                   ; End of FTDI read pulse

	; Just refresh watchdog whenever 256 bytes are transmitted
	RefreshWatchdog

	JNB	RI, CS080
	CLR	RI
	MOV 	SPIDAT, SBUF
        JNB     ISPI,$                  ; Wait until whole byte is sent/received by SPI
        CLR     ISPI                    ; Clear ISPI
	SPI_WAIT			; If low communication speed is set then certain wait time is inserted here
        INC	DPTR			; Incerement byte counter
        JB      M_EnableOut,CS080	; Repeat until M_EnableOut=0 which is signal of end

        SETB    M_EnableIn              ; End of operation

	CALL    FT_ClearFIFO		; Flush all other characters remaining in the FTDI FIFO

	MOV	A,DPL
	CALL	FT_SendChar
	MOV	A,DPH
	CALL	FT_SendChar

        RET

;_______________________________________________________________________________
                                ; Open the Shutter
ComServ_OpenShutter:

	OpenShutterInt			; Open shutter and start interval measurement
        RET
;_______________________________________________________________________________
				; Close Shutter
ComServ_CloseShutter:

	call	CloseShutterInt		; Closes shutter, stops interval measurement
        RET
;_______________________________________________________________________________
                                ; Close Shutter and send data
ComServ_CloseShutterAndSendData:

	call	CloseShutterInt		; Closes shutter, stops interval measurement
        JMP     ReadMatrixAndTime	; Reads and transmits matrix

;_______________________________________________________________________________
                                ; Generates given number of test pulses
ComServ_TestPulse:		; Receives two bytes: count of pulses
				; Receives one byte:  half period in microseconds

	CALL	FT_GetChar
	CPL	A
	MOV	DPL,A
	CALL	FT_GetChar
	CPL	A
	MOV	DPH,A			; DPTR contains desired number of counts

	CALL	FT_GetChar
	DEC	A			; half period is decremented to correct overhead
        MOV	B,A			; B contains half test pulse periode in microseconds

	SETB	M_EnablePulse
        CLR     M_Shutter
        SETB	M_M0
        SETB	M_M1
        SET_CmosIn			; Opens the shutter and enables test pulse
        MOV	A,#1
	CALL	Wait10MicroSeconds

  GenerateSinglePulse:
	SETB	M_PulseAdress           ;                                         (2 cycles)
        SET_CmosIn			; Rising edge (depending on DAC setings)  (5 cycles)
        MOV	A,B                     ;                                         (1 cycle   => 8 extra cycles)
	CALL	WaitMicroseconds

	RefreshWatchdog			; Refresh watchdog to prevent reset       (8 extra cycles => 16)
	NOP
	NOP
	NOP
	NOP				; Just wait				  (4 extra cycles => 20) => 1 us

	CLR	M_PulseAdress
        SET_CmosIn			; Falling edge (depending on DAC setings)
        MOV	A,B                     ;                                         (8 extra cycles)
	CALL	WaitMicroseconds

	NOP
	NOP
	NOP
	NOP				; Just wait				  (4 extra cycles => 12)

	INC	DPTR			; 3
	MOV	A,DPH                   ; 1
	ORL	A,DPL                   ; 1
	JNZ 	GenerateSinglePulse     ; 3                                       (8 extra cycles => 20) => 1us

        MOV	A,#1
	CALL	Wait10MicroSeconds
        SETB	M_Shutter
        CLR	M_M0
        CLR	M_M1
	CLR	M_EnablePulse
        SET_CmosIn			; Closes the shutter and disables test pulse

	CLR     C
        JMP     ReadMatrixAndTime	; Reads and transmits matrix
;_______________________________________________________________________________
				; Select first of two FSR registers
ComServ_SelectFSR0:

        CLR     M_SpareFsr              ; Select FSR0
        SET_CmosIn
        RET
;_______________________________________________________________________________
				; Select second of two FSR registers
ComServ_SelectFSR1:
        SETB    M_SpareFsr              ; Select FSR1
        SET_CmosIn
        RET
;_______________________________________________________________________________
				; Set pixel to collect negative charges (electrons)
ComServ_PolarityNegative:

        CLR     M_Polarity              ; Negative polarity - negative charges collecting
        SET_CmosIn
        RET
;_______________________________________________________________________________
				; Set pixel to collect positive charges (holes)
ComServ_PolarityPositive:
        SETB    M_Polarity              ; Positive polarity - positive charges collecting
        SET_CmosIn
        RET
;_______________________________________________________________________________
				; Send the Medipix inputs settings to PC
ComServ_SendCMOSSettings:
        MOV     A,M_CmosIn              ; Send the Medipix CMOS inputs settings
        CALL    FT_SendChar
        RET
;_______________________________________________________________________________
				; Sends list of known commands
ComServ_Help:
        MOV     DPTR,#Str_Help          ; Send the help text "Known commands: "
        CALL    FT_SendString
        MOV	dptr,#Com_Table
  HelpSingleCommand:
        CLR	A
        MOVC	A,@A+dptr
        JZ	HelpFinished
        CALL	FT_SendChar		; Sends command character
        INC	DPTR                    ; Skip the command
        INC	DPTR
        INC	DPTR		        ; Skip address of service => dptr points to the next command
        JMP	HelpSingleCommand
  HelpFinished:
  	MOV	A,#0dh
  	CALL	FT_SendChar		; Sends terminating 'CR' character
        RET
;_______________________________________________________________________________
				; Name of the device
ComServ_Info:
        MOV     DPTR,#Str_Info          ; Send the device name info
        CALL    FT_SendString
        RET
;_______________________________________________________________________________
				; Switch the DC-DC converter for Medipix power supply ON
ComServ_DCswitchON:
        CLR     DCShutdown
        RET
;_______________________________________________________________________________
				; Switch the DC-DC converter for Medipix power supply OFF
ComServ_DCswitchOFF:
        SETB    DCShutdown
        RET

;_______________________________________________________________________________
				; Sets clock for Timepix
ComServ_SetTPXClock:
        CALL    FT_GetChar              ; Wait for value FREQ and get it to A
        ; PLL   FREQ
        ; 10 .. 00 .. 10MHz
        ; 00 .. 01 .. 20MHz
        ; 01 .. 10 .. 40MHz
	; 11 .. 11 .. 80MHz
	ANL     A,#03h

   CSSTC_0:
	JNZ     CSSTC_1
	SETB    Tmpx_FREQ1
	CLR	Tmpx_FREQ0
	RET
   CSSTC_1:
   	DEC	ACC
	JNZ     CSSTC_2
	CLR	Tmpx_FREQ1
	CLR	Tmpx_FREQ0
	RET
   CSSTC_2:
   	DEC	ACC
	JNZ     CSSTC_3
	CLR	Tmpx_FREQ1
	SETB	Tmpx_FREQ0
	RET
   CSSTC_3:
	SETB	Tmpx_FREQ1
	SETB	Tmpx_FREQ0
	RET
;_______________________________________________________________________________
				; Receives byte from USB and sets it to HV voltage source
ComServ_SetBias:
        CALL    FT_GetChar              ; Wait for value
SetBias:				; Sets bias voltage to value A
        ; Configure SPI interface:
	MOV     SPICON,#SPI_CONFIG_BIAS ; SPI is enabled in Master Mode (I2C is enabled), CPOL=0, CPHA=1, frequency Fosc/16

        CLR     P3.7			; Enable HV source
        MOV     SPIDAT,A                ; Send value
        JNB     ISPI,$                  ; Wait until whole byte is received by SPI
        CLR     ISPI
        SETB    P3.7                    ; Disable HV source

        ; Configure SPI interface to maximum speed:
	MOV     SPICON,#SPI_CONFIG_CLOCK; SPI is enabled in Master Mode (I2C is enabled), CPOL=0, CPHA=1, frequency Fosc/2
        RET
;_______________________________________________________________________________
				; Sets HV voltage to initial state
ComServ_ResetBias:
        CLR	A
        JMP	SetBias
;_______________________________________________________________________________

ComServ_SetDAC:			; Receives three bytes from USB
				; 1. DAC number (0,1)
				; 2.-3. is DAC code (12 bits) LSB byte first
	CALL	FT_GetChar
	ANL	A,#01h		; Just 0 or 1 is allowed
	MOV	B,A

	CALL	FT_GetChar
	MOV	DPL,A
	CALL	FT_GetChar
	ANL	A,#0Fh		; Just 4 bits in upper byte
	MOV	DPH,A

	MOV	A,B

  SetDAC:			; Alternative entry point: Sets DPTR to ADC of number given by A
	JNZ	SetDAC1

  SetDAC0:
  	MOV	DAC0H,DPH
  	MOV	DAC0L,DPL
  	RET

  SetDAC1:
  	MOV	DAC1H,DPH
  	MOV	DAC1L,DPL
  	RET
;_______________________________________________________________________________

ComServ_WatchdogTest:
	JMP	$
	RET
;_______________________________________________________________________________

ComServ_Synchonization:

        MOV     DPTR,#Str_SyncString    ; Send the synchonisation string
        CALL    FT_SendString
	RET
;_______________________________________________________________________________

ComServ_OscilatingShutter:
			; Will make oscilations on shutter signal until
			; another USB command appears

; Helper macros
;---------------

; One pulse on Shutter pin (latch has to be transparent)
ShutterPinTick	MACRO
  	CPL	P0.7	; P0.7 is connected to IN8 of the latch, Q8 of the latch is shutter
  	CPL	P0.7	; => if latch is transparent then P0.7 controls shutter directly
	ENDM

; 2 pulses on Shutter pin (latch has to be transparent)
ShutterPinTick2	MACRO
	ShutterPinTick
	ShutterPinTick
	ENDM

; 4 pulses on Shutter pin (latch has to be transparent)
ShutterPinTick4	MACRO
	ShutterPinTick2
	ShutterPinTick2
	ENDM

; 8 pulses on Shutter pin (latch has to be transparent)
ShutterPinTick8	MACRO
	ShutterPinTick4
	ShutterPinTick4
	ENDM

; 16 pulses on Shutter pin (latch has to be transparent)
ShutterPinTick16	MACRO
	ShutterPinTick8
	ShutterPinTick8
	ENDM

; 32 pulses on Shutter pin (latch has to be transparent)
ShutterPinTick32	MACRO
	ShutterPinTick16
	ShutterPinTick16
	ENDM

; 64 pulses on Shutter pin (latch has to be transparent)
ShutterPinTick64	MACRO
	ShutterPinTick32
	ShutterPinTick32
	ENDM

; 128 pulses on Shutter pin (latch has to be transparent)
ShutterPinTick128	MACRO
	ShutterPinTick64
	ShutterPinTick64
	ENDM

; 256 pulses on Shutter pin (latch has to be transparent)
ShutterPinTick256	MACRO
	ShutterPinTick128
	ShutterPinTick128
	ENDM

; Service code is here:
;-----------------------------

  ShutOscBurst:
        CLR	EA				; Disable all interrrupts
        MOV     M_port,M_CmosIn 		; Send Medipix CMOS inputs configuration to latch
	; Latch has to be transparent now:
        SETB    LatchEnable     		; Enable Medipix CMOS input latch
	MOV	B,#0				; Make alays burst of 256 pulses in one burst
  ShutOscTick:
       	ShutterPinTick256
       	ShutterPinTick256                       ; Make 512 pulses on shutter signal  => each pulse takes 4 cycles = 200ns i.e. 5MHz
	DJNZ	B,ShutOscNextTick		; Takes 4 cycles + 4 cycles of LJMP => two pulses is skipped

  ; Burst finished => close the latch
        CLR     LatchEnable     		; Close Medipix CMOS input latch - last M_port content is stored in latch
        RefreshWatchdogBase                     ; Refresh watchdog timer
        SETB	EA				; Reenable interrrupts

  ; Check USB interface:
        JB      FT_RxF,ShutOscNextBurst   	; Check if FTDI is ready to read - if not make another burst
        SETB	P0.7                            ; Just Ensure that shutter is closed
	RET

  ShutOscNextTick:
  	JMP 	ShutOscTick
  ShutOscNextBurst:
  	JMP	ShutOscBurst

;_______________________________________________________________________________
; Command used for coincidence measurements. Does followind actions:
;	- stops a trigger by macro TriggerStop
;       - EraseMpx (zeroes matrix) - shutter is closed by this command
;       - TODO command2 is set to this command: "=" = ComServ_ResetMpxStartTriggerStartInterval
;       - Configures interval timer to perform TODO command2
;	- Trigger is configured to perform CloseShutter
;	- Opens a shutter
;       - Restarts interval by call of IntervalRestart
;       - Starts trigger

ComServ_ResetMpxStartTriggerStartInterval:
	TriggerStop
	CALL	ComServ_EraseMatrix
	MOV	A,#250
	CALL	WaitMicroseconds		; Wait a little bit to avoid immediate trigger caused by EraseMatrix

	MOV	TODO_Command2,#'=' 		; TODO command2 is this function i.e. ComServ_ResetMpxStartTriggerStartInterval

	MOV	A,INTERVAL_TODO
	CLR	TODO_StartInterval
	CLR	TODO_StartTrigger
	CLR	TODO_CloseShutter
	SETB	TODO_MakeCommand2
	MOV	INTERVAL_TODO,A       		; When Interval is finished then TODO Command2 is executed

	MOV	A,TRIGGER_TODO
	CLR	TODO_StartInterval
	CLR	TODO_StartTrigger
	;SETB	TODO_StartScope
	SETB	TODO_CloseShutter
	;SETB	TODO_MakeCommand2
	MOV	TRIGGER_TODO,A			; When trigger is detected then shutter is closed and data transmitted

	CALL	IntervalRestart 		; Interval is restarted => ComServ_ResetMpxStartTriggerStartInterval will be called after
	OpenShutter				; Shutter is opened
	TriggerStart				; Waiting for trigger is started
	RET


;_______________________________________________________________________________
; Jump to bootloader:

ComServ_Flash:
	CLR	C
	CALL 	FT_GetChar
	SUBB	A,#'l'
	JNZ	ComServ_Flash_Invalid
	CALL 	FT_GetChar
	SUBB	A,#'a'
	JNZ	ComServ_Flash_Invalid
	CALL 	FT_GetChar
	SUBB	A,#'s'
	JNZ	ComServ_Flash_Invalid
	CALL 	FT_GetChar
	SUBB	A,#'h'
	JNZ	ComServ_Flash_Invalid

 	MOV     EADRH,#0
     	MOV     EADRL,#0
     	MOV     ECON,#FLASH_ERASE           	; Erase the checksum page
     	JMP	0E000h				; Jump to bootloader

  ComServ_Flash_Invalid:
  	RET

;_______________________________________________________________________________
; Test of LVDS lines: Requires three parameters:
;	1. Lower nible of SPICON reg.   : 0   0   0   0   CPOL   CPHA  SPR1  SPR2
;          CPOL - Clock polarity bit (1 = clock idles high, 0 = clock idles low)
;          CPHA - Clock phase bit    (1 = leading SCLOCK edge transmits data, 0 = trailing SCLOCK edge transmits data)
;	2. Enable_In pin state 1=High, 0=Low
;       3. Byte to be sent via SPI
; Returns two bytes:
;	1. Byte received from SPI
;	2. State of Enable_Out input pin
; Command ':'

ComServ_LVDSTest:
	CLR	C
	CALL 	FT_GetChar
	ANL	A,#0Fh		; Ensure that higher nible is zero
	ORL	A,#30h          ; SPI is enabled in Master Mode

	MOV     SPICON,A	; Configure SPI

	CALL 	FT_GetChar
	MOV	C, ACC.0
	MOV	M_EnableIn,C	; Set Enable In to desired state

	CALL 	FT_GetChar      ; Get data to be sent
        MOV     SPIDAT,A        ; Transmit the data
        JNB     ISPI,$          ; Wait until whole byte is received by SPI
        CLR     ISPI

	MOV	A,SPIDAT	; Get received data
	CALL	FT_SendChar 	; Send the data via USB

	CLR	A
	MOV	C, M_EnableOut
	MOV	ACC.0,C       	; Get state of Enable Out pin
	CALL	FT_SendChar     ; Send it via USB

        MOV     SPICON,#SPI_CONFIG_MPX	; Restore SPI configuration
	CLR	C
	RET

;_______________________________________________________________________________
; Test of CMOS lines:
;	One input parameter - byte = CMOSIn
; Command 'I'

ComServ_CMOSTest:
	CLR	C
	MOV	B,M_CmosIn
	CALL 	FT_GetChar
	MOV	M_CmosIn,A
	SET_CmosIn
	MOV	M_CmosIn,B
	CLR	C
	RET

;_______________________________________________________________________________
; Test of I2C lines:
;	One input parameter  - "X 0 0 0  0 0 SDA SCL"   or   "X 0 0 0  0 0 Tmpx_FREQ1 Tmpx_FREQ0" if X is High then data will be just read (not written i.e. SDA and SCL bits are ignored)
;	One output parameter - "0 0 0 0  0 0 SDA SCL"   or   "0 0 0 0  0 0 Tmpx_FREQ1 Tmpx_FREQ0"
; Command '\''

ComServ_I2CTest:
	CLR	C
	CALL 	FT_GetChar

	JB	ACC.7, I2CTest_JustRead

	MOV	C,ACC.0		;
	MOV	SCL_I2C, C 	; Set SCL (Tmpx_FREQ0)

	MOV	C,ACC.1		;
	MOV	SDA_I2C, C 	; Set SDA (Tmpx_FREQ1)

  I2CTest_JustRead:
	CLR	A
	MOV	C, SCL_I2C
	MOV	ACC.0,C		; Get SCL_I2C (Tmpx_FREQ0)
	MOV	C, SDA_I2C
	MOV	ACC.1,C		; Get SDA_I2C (Tmpx_FREQ1)

	CALL	FT_SendChar

	CLR	C
	RET

;===============================================================================
; Interval timer
;===============================================================================

IntervalDefault:        ; Clears all registers and sets default Interval of 0.5 sec

	CALL	IntervalClear
	MOV	INTERVAL_TIME+5,#0FFh
	MOV	INTERVAL_TIME+4,#0FFh
	MOV	INTERVAL_TIME+3,#0FFh
	MOV	INTERVAL_TIME+2,#067h
	MOV	INTERVAL_TIME+1,#069h
	MOV	INTERVAL_TIME+0,#07Fh

	CLR	A
	SETB	TODO_CloseShutter               ; Default TODO action is to close the shutter
	MOV	INTERVAL_TODO,A
	RET

IntervalClear:  	; Erase all registers of Interval timer (12 bytes from address INTERVAL_TIME)

	PUSH	00
	PUSH	B
	PUSH	ACC

	IntervalStop
	MOV	R0,#INTERVAL_TIME
	MOV	B,#16
	CLR	A
    ClearTimer_:
        MOV	@R0,A
        INC	R0
    	DJNZ	B,ClearTimer_

	POP	ACC
	POP	B
	POP	00
	RET

IntervalMeasure:	; Starts interval timer
			; INTERVAL_CURR register is cleared
			; T0 registers are cleared
			; INTERVAL_MODE is cleared
			; T0 is started => Timer is counting clocks - interupt can occur in 162 days :-)

	IntervalStop                            ; Stop the timer
	PUSH	ACC
	CLR	A
	MOV	TL0, A
	MOV	TH0, A 				; T0 is cleared
	MOV	INTERVAL_CURR, A
	MOV	INTERVAL_CURR+1, A
	MOV	INTERVAL_CURR+2, A
	MOV	INTERVAL_CURR+3, A		; INTERVAL_CURR register is cleared
	MOV	INTERVAL_MODE,#0		; INTERVAL_MODE is 0 => we are measuring
	POP	ACC
	IntervalStart
	RET

IntervalRestart:	; Restarts interval timer:
			; Sets INTERVAL_CURR register to preselected INTERVAL_TIME
			; INTERVAL_MODE is set to 1 (=> normal overflow handling)
			; Sets T0 registers and starts the timer

	IntervalStop                            ; Stop the timer
	MOV	TL0, INTERVAL_TIME
	MOV	TH0, INTERVAL_TIME+1 		; Lower two bytes are copied directly to T0
	MOV	INTERVAL_CURR,INTERVAL_TIME+2
	MOV	INTERVAL_CURR+1,INTERVAL_TIME+3
	MOV	INTERVAL_CURR+2,INTERVAL_TIME+4
	MOV	INTERVAL_CURR+3,INTERVAL_TIME+5 ; Upper four bytes are copied to INTERVAL_CURR register

	MOV	INTERVAL_MODE,#1		; INTERVAL_MODE is 1 => we are generating an interrupt
	IntervalStart				; Start the timer again
	RET

;_______________________________________________________________________________

ComServ_IntervalExpositionSet:
                        ; Combination of ComServ_IntervalSet which takes interval parameters from USB
                        ; and ComServ_IntervalExposition which starts IntervalExposition

   	call 	ComServ_IntervalSet

ComServ_IntervalExposition:
			; Just alias for IntervalExposition
IntervalExposition:
			; Refils INTERVAL_CURR and T0 by content of INTERVAL_TIME
			; Opens shutter and starts Interval timer


	PeriodStop
	IntervalStop
	TriggerStop
	CounterStop				; Stop all processes

	CALL	FrameCounterRestart		; Restarts frame counter

	CLR	A
	SETB	TODO_CloseShutter
	MOV	INTERVAL_TODO,A       		; When Interval is finished then shutter will be closed and data transmitted

	CLR	A
	MOV	FRAME_TODO,A       		; When data are transfered no action will be taken

	CALL	IntervalRestart
	OpenShutter
        RET


;_______________________________________________________________________________

ComServ_IntervalMultiExposition:

	PeriodStop
	IntervalStop
	TriggerStop
	CounterStop				; Stop all processes

	CALL	FrameCounterRestart		; Restart frame counter

	CLR	A
	SETB	TODO_CloseShutter
	MOV	INTERVAL_TODO,A       		; When Interval is finished then shutter will be closed and data transmitted

	CLR	A
	SETB	TODO_StartInterval
	SETB	TODO_OpenShutter
	MOV	FRAME_TODO,A    		; When frame data are transfered the shutter wiil be opened and interval restarted

	CALL	IntervalRestart
	OpenShutter
        RET
;_______________________________________________________________________________

ComServ_AbortExposition:; Stops Current interval,
			; Closes shutter if it is opened, sends data (only if shutter was opened)

	call	CloseShutterInt 		; Close the shutter and stops interval
  	PeriodStop
  	CounterStop
  	TriggerStop
  	CALL FrameCounterRestart
  	RET
;_______________________________________________________________________________

ComServ_IntervalSet:    ; Receives:
			; 6 bytes from USB (LSB byte first) and stores them in INTERVAL_TIME register
			; 7th byte is TODO mask

	MOV	R0,#INTERVAL_TIME
	MOV 	B,#6
  OneIntervalByte:
	CALL 	FT_GetChar
	CPL	A
	MOV	@R0,A
	INC	R0
	DJNZ	B,OneIntervalByte

	CALL 	FT_GetChar
	MOV	INTERVAL_TODO,A
        RET
;_______________________________________________________________________________

ComServ_IntervalReset:
	CALL	IntervalMeasure
        RET
;_______________________________________________________________________________

ComServ_IntervalStop:
	IntervalStop
	RET
;_______________________________________________________________________________

ComServ_IntervalState:	; Sends current status of Interfval Timer to FTDI
			;  1.    byte = run state: bit 0 = Is running (TR0), bit 1 = Is intrrupt ensbled (ET0)
			;  2.    byte = INTERVAL_TODO mask
			;  3.-8. byte = INTERVAL_TIME (LSB byte first)
			;  9.-14.byte = INTERVAL_CURR (LSB byte first)

        CLR 	A
        MOV 	C,TR0
        MOV	ACC.0,C
        MOV 	C,ET0
        MOV	ACC.1,C
        CALL    FT_SendChar             ; Send flags (1. bit = Running, 2. bit = Interrupt Enable)
        CLR	C

	MOV	A, INTERVAL_TODO
        CALL    FT_SendChar             ; Send INTERVAL_TODO mask

     	MOV	B,#6
     	MOV 	R0,#INTERVAL_TIME
  OneIntervalByteSend:
	MOV	A,@R0
	CPL	A
	CALL 	FT_SendChar
	INC	R0
	DJNZ	B,OneIntervalByteSend

	MOV     DPL,TL0
	MOV	DPH,TH0
	MOV	A,DPL
	CPL	A
	CALL 	FT_SendChar
	MOV	A,DPH
	CPL	A
	CALL 	FT_SendChar
	MOV	A,INTERVAL_CURR
	CPL	A
	CALL 	FT_SendChar
	MOV	A,INTERVAL_CURR+1
	CPL	A
	CALL 	FT_SendChar
	MOV	A,INTERVAL_CURR+2
	CPL	A
	CALL 	FT_SendChar
	MOV	A,INTERVAL_CURR+3
	CPL	A
	CALL 	FT_SendChar
        RET

;_______________________________________________________________________________

ComServ_IntervalElapsedTime:
IntervalElapsedTime:	; Computes elapsed interval time:
			; - if INTERVAL_MODE is 0 => interval has been measured, current value is transmitted without any change
			; - if INTERVAL_MODE is 1 => interval generates an interrupt, current value is subtracted from INTERVAL_TIME and transmitted
			; Anyway it will transmit 6 bytes with elapsed time information (LSB first).

	MOV 	A,INTERVAL_MODE
	JNZ	ITS_InterruptMode

  ITA_MeasureMode:
	MOV     DPL,TL0
	MOV	DPH,TH0
	MOV	A,DPL
	CALL 	FT_SendChar
	MOV	A,DPH
	CALL 	FT_SendChar
	MOV	A,INTERVAL_CURR
	CALL 	FT_SendChar
	MOV	A,INTERVAL_CURR+1
	CALL 	FT_SendChar
	MOV	A,INTERVAL_CURR+2
	CALL 	FT_SendChar
	MOV	A,INTERVAL_CURR+3
	CALL 	FT_SendChar
        RET

  ITS_InterruptMode:
	MOV     DPL,TL0
	MOV	DPH,TH0

	CLR	C
	MOV	A,DPL
	SUBB    A,INTERVAL_TIME
	CALL 	FT_SendChar
	MOV	A,DPH
	SUBB    A,INTERVAL_TIME+1
	CALL 	FT_SendChar
	MOV	A,INTERVAL_CURR
	SUBB    A,INTERVAL_TIME+2
	CALL 	FT_SendChar
	MOV	A,INTERVAL_CURR+1
	SUBB    A,INTERVAL_TIME+3
	CALL 	FT_SendChar
	MOV	A,INTERVAL_CURR+2
	SUBB    A,INTERVAL_TIME+4
	CALL 	FT_SendChar
	MOV	A,INTERVAL_CURR+3
	SUBB    A,INTERVAL_TIME+5
	CALL 	FT_SendChar
        RET

;===============================================================================
; Counter
;===============================================================================

CounterDefault:        	; Clears all registers and sets default Vector and Service

	CALL	CounterClear
	MOV	COUNTER_COUNT+5,#0FFh
	MOV	COUNTER_COUNT+4,#0FFh
	MOV	COUNTER_COUNT+3,#0FFh
	MOV	COUNTER_COUNT+2,#0FFh
	MOV	COUNTER_COUNT+1,#0FFh
	MOV	COUNTER_COUNT+0,#0FEh

	CLR	A
	SETB	TODO_StartCounter
	SETB	TODO_CloseShutter
	MOV	COUNTER_TODO,A
	RET

CounterClear:		; Clears all registers used by Counter

	PUSH	00
	PUSH	B
	PUSH	ACC

	CounterStop
	MOV	R0,#COUNTER_COUNT
	MOV	B,#15
	CLR	A
    ClearCounter_:
        MOV	@R0,A
        INC	R0
    	DJNZ	B,ClearCounter_

	POP	ACC
	POP	B
	POP	00
	RET

ComServ_CounterStart:	; Alias for CounterRestart

;CounterRestart:		; Restarts counter:
			; Sets COUNTER_CURR register to preselected COUNTER_COUNT
			; Sets T1 registers and starts the timer

;	CounterStop                            	; Stop the timer
;	MOV	TL1, COUNTER_COUNT
;	MOV	TH1, COUNTER_COUNT+1 		; Lower two bytes are copied directly to T1
;	MOV	COUNTER_CURR,COUNTER_COUNT+2
;	MOV	COUNTER_CURR+1,COUNTER_COUNT+3
;	MOV	COUNTER_CURR+2,COUNTER_COUNT+4
;	MOV	COUNTER_CURR+3,COUNTER_COUNT+5 	; Upper four bytes are copied to COUNTER_CURR register
;	CounterStart				; Start the timer again
;	RET

;_______________________________________________________________________________

ComServ_CounterSet:     ; Receives:
			; 6 bytes from USB (LSB byte first) and stores them in COUNTER_COUNT register
			; 7th byte is TODO mask

	MOV	R0,#COUNTER_COUNT
	MOV 	B,#6
  OneCounterByte:
	CALL 	FT_GetChar
	CPL	A
	MOV	@R0,A
	INC	R0
	DJNZ	B,OneCounterByte

	CALL 	FT_GetChar
	MOV	COUNTER_TODO,A

	CLR	C		; CY=0 means no error
        RET

;_______________________________________________________________________________

ComServ_CounterStop:
	CounterStop
	RET
;_______________________________________________________________________________

;ComServ_CounterState:	; Sends current status of Counter to FTDI
			;  1.    byte = run state: bit 0 = Is running (TR1), bit 1 = Is intrrupt ensbled (ET1)
			;  2.    byte = COUNTER_TODO mask
			;  3.-8. byte = COUNTER_COUNT (LSB byte first)
			;  9.-14.byte = COUNTER_CURR (LSB byte first)

;        CLR 	A
;        MOV 	C,TR1
;        MOV	ACC.0,C
;        MOV 	C,ET1
;        MOV	ACC.1,C
;        CALL    FT_SendChar             ; Send flags (1. bit = Running, 2. bit = Interrupt Enable)
;        CLR	C;;;

;	MOV	A, COUNTER_TODO
;        CALL    FT_SendChar             ; Send COUNTER_TODO mask;

;     	MOV	B,#6
;    	MOV 	R0,#COUNTER_COUNT
;  OneCounterByteSend:
;	MOV	A,@R0
;	CPL	A
;	CALL 	FT_SendChar;
;	INC	R0
;	DJNZ	B,OneCounterByteSend

;	MOV     DPL,TL1
;	MOV	DPH,TH1
;	MOV	A,DPL
;	CPL	A
;	CALL 	FT_SendChar
;	MOV	A,DPH
;	CPL	A
;	CALL 	FT_SendChar
;	MOV	A,COUNTER_CURR
;	CPL	A
;	CALL 	FT_SendChar
;	MOV	A,COUNTER_CURR+1
;	CPL	A
;	CALL 	FT_SendChar
;	MOV	A,COUNTER_CURR+2
;	CPL	A
;	CALL 	FT_SendChar
;	MOV	A,COUNTER_CURR+3
;	CPL	A
;	CALL 	FT_SendChar
;        RET

;===============================================================================
; Period
;===============================================================================

PeriodDefault:		; Clears all and sets default Period time to 1 sec

	CALL 	PeriodClear
	MOV	PERIOD_TIME+5,#0FFh
	MOV	PERIOD_TIME+4,#0FFh
	MOV	PERIOD_TIME+3,#0FFh
	MOV	PERIOD_TIME+3,#0feh
	MOV	PERIOD_TIME+2,#0ceh
	MOV	PERIOD_TIME+1,#0d2h
	MOV	PERIOD_TIME+0,#0ffh

	CLR	A
	SETB	TODO_StartInterval 		; The Interval will be launched each period
	SETB	TODO_OpenShutter		; The shutter will be opened at the begining of each period
	MOV	PERIOD_TODO,A
	RET

PeriodClear:		; Clears all registers used by Period

	PUSH	00
	PUSH	B
	PUSH	ACC

	PeriodStop
	MOV	R0,#PERIOD_TIME
	MOV	B,#15
	CLR	A
    ClearPeriod_:
        MOV	@R0,A
        INC	R0
    	DJNZ	B,ClearPeriod_

	POP	ACC
	POP	B
	POP	00
	RET


ComServ_PeriodStart:	; Just alias for PeriodRestart

PeriodRestart:		; Restarts period timer:
			; Sets PERIOD_CURR register to preselected PERIOD_TIME
			; Sets T2 registers and starts the timer

	PeriodStop                            	; Stop the timer
	MOV	TL2, PERIOD_TIME
	MOV	TH2, PERIOD_TIME+1 		; Lower two bytes are copied directly to T0
	MOV	PERIOD_CURR,PERIOD_TIME+2
	MOV	PERIOD_CURR+1,PERIOD_TIME+3
	MOV	PERIOD_CURR+2,PERIOD_TIME+4
	MOV	PERIOD_CURR+3,PERIOD_TIME+5 	; Upper four bytes are copied to PERIOD_CURR register
	PeriodStart				; Start the timer again
	RET

;_______________________________________________________________________________

ComServ_PeriodSet:	; Receives:
			; - 6 bytes from USB (LSB byte first) and stores them in PERIOD_COUNT register
			; - 7th byte is TODO mask

	MOV	R0,#PERIOD_TIME
	MOV 	B,#6
  OnePeriodByte:
	CALL 	FT_GetChar
	CPL	A
	MOV	@R0,A
	INC	R0
	DJNZ	B,OnePeriodByte

	CALL 	FT_GetChar
	MOV	PERIOD_TODO,A
        RET

;_______________________________________________________________________________

ComServ_PeriodStop:
	PeriodStop
	RET
;_______________________________________________________________________________

ComServ_PeriodState:	; Sends current status of Period to FTDI
			;  1.    byte = run state: bit 0 = Is running (TR2), bit 1 = Is intrrupt ensbled (ET2)
			;  2.    byte = PERIOD_TODO mask
			;  3.-8. byte = PERIOD_COUNT (LSB byte first)
			;  9.-14.byte = PERIOD_CURR (LSB byte first)

        CLR 	A
        MOV 	C,TR2
        MOV	ACC.0,C
        MOV 	C,ET2
        MOV	ACC.1,C
        CALL    FT_SendChar             ; Send flags (1. bit = Running, 2. bit = Interrupt Enable)
        CLR	C

	MOV	A, PERIOD_TODO
        CALL    FT_SendChar             ; Send PERIOD_TODO mask

     	MOV	B,#6
     	MOV 	R0,#PERIOD_TIME
  OnePeriodByteSend:
	MOV	A,@R0
	CPL	A
	CALL 	FT_SendChar
	INC	R0
	DJNZ	B,OnePeriodByteSend

	MOV     DPL,TL2
	MOV	DPH,TH2
	MOV	A,DPL
	CPL	A
	CALL 	FT_SendChar
	MOV	A,DPH
	CPL	A
	CALL 	FT_SendChar
	MOV	A,PERIOD_CURR
	CPL	A
	CALL 	FT_SendChar
	MOV	A,PERIOD_CURR+1
	CPL	A
	CALL 	FT_SendChar
	MOV	A,PERIOD_CURR+2
	CPL	A
	CALL 	FT_SendChar
	MOV	A,PERIOD_CURR+3
	CPL	A
	CALL 	FT_SendChar
        RET

ComServ_PeriodicalExposition:
			; Refils PERIOD_CURR and T0 by content of PERIOD_TIME
			; Starts period timer
	PeriodStop
	IntervalStop
	TriggerStop
	CounterStop				; Stop all processes

	CALL	FrameCounterRestart		; Restart frame counter

	CLR	A
	SETB	TODO_StartInterval 		; The Interval will be launched each period
	SETB	TODO_OpenShutter		; The shutter will be opened at the begining of each period
	MOV	PERIOD_TODO,A

	CLR	A
	SETB	TODO_CloseShutter		; The shutter will be closed at the end of each interval
	MOV	INTERVAL_TODO,A

	CALL	PeriodRestart			; Restart period
	CALL	IntervalRestart			; Restart the interval
	OpenShutter				; Opens the shutter
        RET

;===============================================================================
; Trigger
;===============================================================================

TriggerDefault:        	; Clears all registers and sets default Vector and Service

	CALL	TriggerClear

	; Default TODO mask:
	CLR	A
	SETB	TODO_StartScope
	SETB	TODO_CloseShutter
	MOV	TRIGGER_TODO,A

	; Default COINC flags:
	CLR	A
;	SETB	COINC_TestLevel		; This level of P3.0 (RxD) means coincidence (data have to be transmitted), oposite level means noncoincidence (data are scratched)
	SETB	COINC_Notify            ; The notification on P3.1 (TxD) should be generated
	SETB	COINC_NotifyInitLevel   ; This is default level of P3.1 (TxD)
        SETB	COINC_NotifyValidCoinc	; If TRUE just coincidence will be notified, if FALSE each trigger generates notification
	SETB	COINC_NotifyByPulse     ; If TRUE then two transitions (forming pulse) are generated on P3.1. If FALSE then just one
	MOV	TRIGGER_COINC,A
	MOV	TRIGGER_COINCDELAY,#0	; Default is Maximum delay time

	; Default pin configuration for coincidence mode:
	JNB	COINC_Test,ReturnTriggerDefault		; If not coincidence mode then leave pins as they are now
	SETB	TriggerIn				; TriggerIn is configured as input
	JNB	COINC_Notify,ReturnTriggerDefault 	; If no notification is desired then leave TriggerOut without any change
	MOV 	C,COINC_NotifyInitLevel			; Initial TriggerOut state is in Carry flag
	MOV	TriggerOut,C 				; Carry flag is copied to TriggerOut
	CLR	C					; Reset carry flag to avoid transmission of error message

  ReturnTriggerDefault:
	RET


TriggerClear:		; Clears all registers used by Trigger

	PUSH	00
	PUSH	B
	PUSH	ACC

	TriggerStop
	CLR	IgnoreNextTrg
	MOV	R0,#TRIGGER_COUNT
	MOV	B,#9
	CLR	A
    ClearTrigger_:
        MOV	@R0,A
        INC	R0
    	DJNZ	B,ClearTrigger_

	POP	ACC
	POP	B
	POP	00
	RET

ComServ_TriggerStart:	; Just Alias for TriggerRestart

TriggerRestart:		; Restarts Trigger
			; Clears trigger counter and enables interrupt

	CLR	A
	MOV	TRIGGER_COUNT+0,A
	MOV	TRIGGER_COUNT+1,A
	MOV	TRIGGER_COUNT+2,A
	MOV	TRIGGER_COUNT+3,A
	TriggerStart
	RET
;_______________________________________________________________________________

ComServ_TriggerSet:	; Receives:
			; 1 bytes from USB with Trigger TODO mask

	CALL 	FT_GetChar
	MOV	TRIGGER_TODO,A
        RET

;_______________________________________________________________________________

ComServ_TriggerCoincSet:; Receives:
			; 1 byte from USB with TRIGGER_COINC mask
			; 1 byte from USB with TRIGGER_COINCDELAY

	CALL 	FT_GetChar
	MOV	TRIGGER_COINC,A
	CALL 	FT_GetChar
	MOV	TRIGGER_COINCDELAY,A

	MOV	A,TRIGGER_COINC
	JNB	COINC_Test,ReturnTriggerCoincSet
	SETB	TriggerIn
	JNB	COINC_Notify,ReturnTriggerCoincSet
	MOV 	C,COINC_NotifyInitLevel
	MOV	TriggerOut,C
	CLR	C

  ReturnTriggerCoincSet:
        RET

;_______________________________________________________________________________


ComServ_TriggerStop:
	TriggerStop
	RET
;_______________________________________________________________________________

ComServ_TriggerState:	; Sends current status of Trigger to FTDI
			;  1.    byte = run state: bit 0 = Is enabled (EX0)
			;  2.    byte = TODO mask
			;  3.    byte = Port state:
			;                    P3.7 = BIAS CS 	-RD
			;                    P3.6 = SCL_I2C 	-WR
			;                    P3.5 = COUNTER 	T1(CONVST)
			;                    P3.4 = SDA_I2C 	T0/PWMC/PWM0/EXTCLK
			;                    P3.3 = DATA_OUT    INT1/MISO/PWM1
			;                    P3.2 = TRIGGER	INT0
			;                    P3.1 = TxD		TxD
			;                    P3.0 = RxD 	RxD
			;  4.-7. byte = COUNT (LSB byte first)

        CLR 	A
        MOV 	C,EX0
        MOV	ACC.0,C
        CALL    FT_SendChar             ; Send flags (1. bit = Interrupt Enable)
        CLR	C

	MOV	A, TRIGGER_TODO
        CALL    FT_SendChar             ; Send TRIGGER_TODO mask

	MOV	A,P3
        CALL    FT_SendChar             ; Send P3 state

	MOV	A,TRIGGER_COUNT
	CPL	A
	CALL 	FT_SendChar
	MOV	A,TRIGGER_COUNT+1
	CPL	A
	CALL 	FT_SendChar
	MOV	A,TRIGGER_COUNT+2
	CPL	A
	CALL 	FT_SendChar
	MOV	A,TRIGGER_COUNT+3
	CPL	A
	CALL 	FT_SendChar
        RET

;===============================================================================
; Frame counter
;===============================================================================

FrameCounterDefault:        	; Sets default number of frames (100) to be taken

	MOV	FRAME_COUNT+0,#09bh
	MOV	FRAME_COUNT+1,#0ffh
	MOV	FRAME_COUNT+2,#0ffh
	MOV	FRAME_COUNT+3,#0ffh
	MOV	FRAME_TODO,#0
	CALL	FrameCounterRestart
	RET

FrameCounterClear:		; Clears all registers used by Trigger

	PUSH	00
	PUSH	B
	PUSH	ACC

	MOV	R0,#FRAME_COUNT
	MOV	B,#9
	CLR	A
    ClearFrameCounter_:
        MOV	@R0,A
        INC	R0
    	DJNZ	B,ClearFrameCounter_

	POP	ACC
	POP	B
	POP	00
	RET

;_______________________________________________________________________________

ComServ_FrameCounterSet:
			; Receives from USB
			; 4 bytes with Frame count (LSB first)
			; 1 bytes with Frame TODO mask

	CALL 	FT_GetChar
	CPL	A
	MOV	FRAME_COUNT+0,A

	CALL 	FT_GetChar
	CPL	A
	MOV	FRAME_COUNT+1,A

	CALL 	FT_GetChar
	CPL	A
	MOV	FRAME_COUNT+2,A

	CALL 	FT_GetChar
	CPL	A
	MOV	FRAME_COUNT+3,A

	CALL 	FT_GetChar
	MOV	FRAME_TODO,A
;_______________________________________________________________________________

ComServ_FrameCounterStart:
			; Just Alias for FrameCounterRestart

FrameCounterRestart:	; Restarts Frame counter
			; Reloads FRAME_CURR with FRAME_COUNT

	CLR	EA
	MOV	FRAME_CURR+0,FRAME_COUNT+0
	MOV	FRAME_CURR+1,FRAME_COUNT+1
	MOV	FRAME_CURR+2,FRAME_COUNT+2
	MOV	FRAME_CURR+3,FRAME_COUNT+3
	SETB	EA
	RET
;_______________________________________________________________________________

ComServ_FrameCounterState:
			; Sends current status of Frame counter to FTDI
			;  1.    byte = TODO mask
			;  2.-5. byte = COUNT (LSB byte first)
			;  6.-9. byte = CURR (LSB byte first)

	MOV	A, FRAME_TODO
        CALL    FT_SendChar             ; Send FRAME_TODO mask

	MOV	A,FRAME_COUNT
	CPL	A
	CALL 	FT_SendChar

	MOV	A,FRAME_COUNT+1
	CPL	A
	CALL 	FT_SendChar

	MOV	A,FRAME_COUNT+2
	CPL	A
	CALL 	FT_SendChar

	MOV	A,FRAME_COUNT+3
	CPL	A
	CALL 	FT_SendChar

	MOV	A,FRAME_CURR
	CPL	A
	CALL 	FT_SendChar

	MOV	A,FRAME_CURR+1
	CPL	A
	CALL 	FT_SendChar

	MOV	A,FRAME_CURR+2
	CPL	A
	CALL 	FT_SendChar

	MOV	A,FRAME_CURR+3
	CPL	A
	CALL 	FT_SendChar

        RET

;===============================================================================
; DELAY
;===============================================================================

; Sets default values of delay => no delay
DelayDefault:
	CLR	A
	MOV	DELAY_TODOWHEN,A
	MOV	DELAY_HOWMUCH,A
	MOV	DELAY_HOWMUCH+1,A
	RET

; Receives configuration of delay from FTDI
ComServ_SetDelay:
			; 2 bytes with count (LSB first)
			; 1 bytes with TODO mask
	CALL 	FT_GetChar
	CPL	A
	MOV	DELAY_HOWMUCH+0,A

	CALL 	FT_GetChar
	CPL	A
	MOV	DELAY_HOWMUCH+1,A

	CALL 	FT_GetChar
	MOV	DELAY_TODOWHEN,A
	RET

; Calibrated delay loop
; Protects all registers
MakeDelay:
	PUSH	ACC
	PUSH	DPP
	PUSH	DPH
	PUSH	DPL

	MOV	DPL,DELAY_HOWMUCH
	MOV	DPH,DELAY_HOWMUCH+1
	MOV	DPP,#0

  InnerDelayLoop:
  	INC	DPTR            ;3
  	MOV	A,DPL   	;1
  	ORL	A,DPH           ;1
  	JNZ	InnerDelayLoop  ;3 	=> 8 cycles at 20MHz = 8 x 50 ns = 400 ns period

  	POP	DPL
  	POP	DPH
  	POP	DPP
  	POP	ACC
  	RET

; Calibrated delay loop
; Protects all registers except of ACC
MakeDelayNotPushA:
	PUSH	DPP
	PUSH	DPH
	PUSH	DPL

	MOV	DPL,DELAY_HOWMUCH
	MOV	DPH,DELAY_HOWMUCH+1
	MOV	DPP,#0

  InnerDelayLoop1:
  	INC	DPTR            ;3
  	MOV	A,DPL   	;1
  	ORL	A,DPH           ;1
  	JNZ	InnerDelayLoop1 ;3 	=> 8 cycles at 20MHz = 8 x 50 ns = 400 ns period

  	POP	DPL
  	POP	DPH
  	POP	DPP
  	RET
;===============================================================================
; TODO
;===============================================================================

ComServ_SetTODO:	; Receives two TODO commands

	CALL	FT_GetChar
	MOV	TODO_Command1,A
	CALL	FT_GetChar
	MOV	TODO_Command2,A
	RET

ComServ_GetTODO:	; Transmits two TODO commands

	MOV	A,TODO_Command1
	CALL	FT_SendChar
	MOV	A,TODO_Command2
	CALL	FT_SendChar
	RET

ComServ_MakeTODO:	; Performs ToDO actions given by mask received from USB

	CALL 	FT_GetChar
	JMP	ProcessTODO

;===============================================================================
; Scope
;===============================================================================

ComServ_TriggerOutHigh:
	SETB	TriggerOut
	RET

ComServ_TriggerOutLow:
	CLR	TriggerOut
	RET

ComServ_TriggerOutCpl:
	CPL	TriggerOut
	RET

;===============================================================================
; Scope
;===============================================================================

ScopeSetDefaults:		; Set default values of all registers
				; initializes XRAM buffer
				; initializes ADC

	MOV	SCOPE_SAMPLES,#Low(SCOPE_MAXCOUNT-1)
	MOV	SCOPE_SAMPLES+1,#High(SCOPE_MAXCOUNT-1)
	MOV	SCOPE_TIMING, #0BCh
	MOV	SCOPE_CHANNELS, #SCOPE_DEFCHANMSK

	JMP	ScopeSetup

;-------------------------------------------------------------------------------

ScopeSetup:			; Configures XRAM memory buffer for DMA ADC measurement
				; Configures ADC to perform DMA measurement

	PUSH	00
	PUSH	01
	PUSH	02
	PUSH	B
	PUSH	DPL
	PUSH	DPH
	PUSH	ACC
	PUSH	PSW

        CLR	EA            	; disable interrupts

	CLR	C
	MOV	A,#Low(SCOPE_MAXCOUNT-1)
	SUBB	A,SCOPE_SAMPLES
	MOV	A,#High(SCOPE_MAXCOUNT-1)
	SUBB	A,SCOPE_SAMPLES+1
	JNC	CheckScopeChannels
	MOV	SCOPE_SAMPLES,#Low(SCOPE_MAXCOUNT-1)
	MOV	SCOPE_SAMPLES+1,#High(SCOPE_MAXCOUNT-1)
	CLR	C

  CheckScopeChannels:
	MOV	A,SCOPE_CHANNELS
	JNZ	SetupScopeScanBits	; No channel is selected => select default (channel 1)
	MOV	A,SCOPE_DEFCHANMSK	; Set default channel
	MOV	SCOPE_CHANNELS,A        ; Put it to register

  ; Scan bits in SCOPE_CHANNELS and fill SCOPE_CHANBUFF by indexes of selected bits, also SCOPE_CHANCNT will be determined
  SetupScopeScanBits:
	MOV   	SCOPE_CHANCNT,#0	; Initialize SCOPE_CHANCNT with zero
	MOV	R0,#SCOPE_CHANBUFF	; Address of channel buffer is in R0
	MOV	B,#0			; BIt counter
  SetupScopeCheckChan:
	RRC	A 			; Last bit is in CY
	JNC	SetupScopeNext 		; This channel shuld not be measured
	MOV 	@R0,B			; In B is order of the channel => set it to current buffer position
	INC	SCOPE_CHANCNT		; Number of channels incremented
	INC	R0 			; Go to next buffer position
	CLR	C			; Clear CY to don't overflow back to A
  SetupScopeNext:
  	INC	B			; Increment bit counter
  	JNZ	SetupScopeCheckChan

  ; Channel numbers have to be in upper nibble => have to swap bytes in SCOPE_CHANBUFF
  SetupScopeSwap:
	MOV	B,SCOPE_CHANCNT
	MOV	R0,#SCOPE_CHANBUFF
  SetupScopeChanSwap:
  	MOV	A,@R0
  	SWAP	A
  	MOV	@R0,A
  	INC	R0
  	DJNZ	B,SetupScopeChanSwap

  ; Prepare XRAM memory buffer for DMA conversion:
  SetupScopeXRAM:
	MOV	R1,SCOPE_SAMPLES
	MOV	R2,SCOPE_SAMPLES+1

  	MOV	DPTR,#SCOPE_BUFF
  	MOV	DPP,#0

    SSCopySerie:
	MOV	B,SCOPE_CHANCNT
  	MOV	R0,#SCOPE_CHANBUFF

    SSCopyByte:
	MOV	A,@R0
	MOVX	@dptr,A		; Configure channel
	INC	dptr
	CLR	A
	MOVX	@dptr,A		; Space for data
	INC	dptr

	CLR	C
	MOV	A,R1
	SUBB	A,#1
	MOV	R1,A
	MOV	A,R2
	SUBB	A,#0
	MOV	R2,A
	JC      SSFinished	; Check if it is right number of samples

    SSGoToNext:
	INC	R0          	; Next channel
	DJNZ	B,SSCopyByte	; Continue if it is not last channel
	JMP	SSCopySerie	; For the last channel rotate back to the first one

    SSFinished:
	MOV	A,@R0
	MOVX	@dptr,A
	INC	dptr		; Repeat last channel
	CLR	A
	MOVX	@dptr,A		; Space
	INC	dptr
	MOV	A,#0F0h
	MOVX	@dptr,A 	; Stop condition
	INC	dptr

	POP	PSW
	POP	ACC
	POP	DPH
	POP	DPL
	POP	B
	POP	02
	POP	01
	POP	00

	JMP	ScopeSetupADC	; Memory is prepared => can setup the ADC:

;-------------------------------------------------------------------------------

ScopeSetupADC:			; Setup ADC for DMA continuous conversion to
				; scope external memory buffer
				; Timing is defined by SCOPE_TIMING register

        CLR	EA            		; disable interrupts
	MOV	ADCCON1,#00h    	; ADC must be powered down during DMA configuring
        MOV     DMAL,#LOW(SCOPE_BUFF)   ; start address for DMA operation
        MOV     DMAH,#HIGH(SCOPE_BUFF)  ; (must write DMA registers in this
        MOV     DMAP,#0         	;  order:  DMAL, DMAH, DMAP)
        MOV     ADCCON1,SCOPE_TIMING   	; speed is given by /2 4 acq clocks => (16+4)*2=40 cycles => at 20MHz 2 us / conversion
        SETB    EA	            	; reenable all interrupts
        RET

;-------------------------------------------------------------------------------

ScopeSingleShotMeasurement:	; Starts DMA ADC measurement
				; Assumes the Scope memory buffer and ADC properly configured
				; When measurement finishes interrupt procedure will be called

	MOV	A,ADCused              	; Have to wait for ADC
	JNZ	ScopeSingleShotMeasurement

        ScopeStart
        RET

;-------------------------------------------------------------------------------

ComServ_ScopeSendData:
ScopeSendData: 			; Sends ADC data from Scope external memory buffer to FTDI
                         	; Doesn't make any measurement!
                         	; This procedure is called from background service BackServ_Scope

        MOV     A,SCOPE_SAMPLES		; Sends the number of counts (lower byte)
        CALL    FT_SendChar
        MOV     A,SCOPE_SAMPLES+1	; Sends the number of counts (upper byte)
        CALL    FT_SendChar

	MOV	R1,SCOPE_SAMPLES
	MOV	R2,SCOPE_SAMPLES+1

  	MOV	DPTR,#SCOPE_BUFF
  	MOV	DPP,#0

    SSdCopyByte:
	MOVX	A,@dptr
        CALL    FT_SendChar
	INC	dptr
	MOVX	A,@dptr
        CALL    FT_SendChar
	INC	dptr

	CLR	C
	MOV	A,R1
	SUBB	A,#1
	MOV	R1,A
	MOV	A,R2
	SUBB	A,#0
	MOV	R2,A
	JNC     SSdCopyByte	; Check if it is right number of samples

	CLR 	C		; Cy=0 indicates no Error
        RET

;_______________________________________________________________________________

ComServ_ScopeConfigure:		; Configures:
				;  - channels to measure (1 byte mask)
				;  - number of samples  (2 bytes, lower first)
				;  - timing (1 byte)
	CALL 	FT_GetChar
	MOV 	R0,A
	CALL 	FT_GetChar
	MOV 	R1,A
	CALL 	FT_GetChar
	MOV 	R2,A
	CALL 	FT_GetChar

	ANL	A,#00Fh
	RL	A
	RL	A
	ORL	A,#080h	 		; Power up the ADC

        CLR	EA
	MOV 	SCOPE_TIMING,A
	MOV	SCOPE_CHANNELS,R0
	MOV	SCOPE_SAMPLES,R1
	MOV	SCOPE_SAMPLES+1,R2

	CALL	ScopeSetup
	SETB	EA
	RET

;===============================================================================
; Voltage Monitor
;===============================================================================

MonitorSetup:			; Setups XRAM memory buffer for Monitor DMA ADC measurement
				; Does not change ADC configuration
        MOV     R0,#00h
        MOV     DPTR,#MONITOR_BUFF  	; set DPTR to DMASTART address
	MOV	DPP,#0			; DPP can be nonzero then external XRAM would be addressed

  SmXRAMChannel:
  	MOV     A,R0			; set up x-mem with init value
        MOVX    @DPTR,A
        INC     DPTR
        ADD     A,#10h
        MOV     R0,A

        CLR     A               	; clear second byte
        MOVX    @DPTR,A
        INC     DPTR
        CJNE    R0,#0D0h,SmXRAMChannel

        MOV     A,#0C0h        		; repeat last channel for valid stop condition
        MOVX    @DPTR,A
        INC     DPTR
        CLR     A               	; clear second byte
        MOVX    @DPTR,A
        INC     DPTR

        MOV     A,#0F0h         	; DMA stop condition
        MOVX    @DPTR,A
        RET

;-------------------------------------------------------------------------------

MonitorSingleShotMeasurement:	; ADC is assumed tobe configured for Scope:
				;  - All Interrupts are disabled!
				;  - ADC is configured to make DMA conversion to Monitor XRAM
				;  - DMA Conversion to Monitor XRAM is started
				;  - Waits for end of conversion
				;  - SetupScopeADC is called to retrieve original configuration
				;  - ADC Interrupt is reenabled

	MOV	A,ADCused              	; Have to wait for ADC
	JNZ	MonitorSingleShotMeasurement
	SETB	ADCused_Monitor		; Monitor uses ADC
	SETB	ScopeTgrDisabled		; Just to prevent trigger of the scope

        CLR	EA	            	; disable interrupts
	MOV	ADCCON1,#00h    	; ADC must be powered down during DMA configuring
        MOV     DMAL,#LOW(MONITOR_BUFF) ; start address for DMA operation
        MOV     DMAH,#HIGH(MONITOR_BUFF); (must write DMA registers in this
        MOV     DMAP,#0         	;  order:  DMAL, DMAH, DMAP)
        MOV     ADCCON2,#040h   	; enable DMA mode
        MOV     ADCCON1,#09Ch   	; speed CoreClock/4, 4 acq clocks => (16+4)*4=80 cycles => at 20MHz 4us/conversion => 4*13 = 52us all
        SETB	EADC			; Enable ADC interrupt
        SETB	EA	            	; reenable interrupts

        SETB    CCONV           	; start continuous ADC conversions using DMA
        JB	ADCused_Monitor,$	; wait for end of conversion
        CLR	ScopeTgrDisabled		; Trigger can start the scope now

        MOV     B,#13			; Send 13 measured channels of MONITR_BUFF in XRAM
        MOV     DPTR,#MONITOR_BUFF
  MSsMSendChannel:
        MOVX    A,@DPTR
        CALL	FT_SendChar
        INC     DPTR
	MOVX    A,@DPTR
        CALL	FT_SendChar
        INC     DPTR
        DJNZ    B,MSsMSendChannel

	CALL	ScopeSetupADC		; Retrieve ADC configuration for scope
	SETB	EADC                    ; Enable ADC interrupt
	SETB	EA			; Enable all interrupts
	RET

;===============================================================================
; For debugging:
;===============================================================================

SendTODOdescription:	; Will send ASCII description od TODO mask in ACC in format of background string

	PUSH	B
	PUSH	DPH
	PUSH	DPL

	MOV	B,A
	MOV	A,#'"'
	CALL 	FT_SendChar

	MOV	DPTR,#Str_TODO
	CALL	FT_SendString

	MOV 	A,B

	JNB	TODO_CloseShutter, No_TODO_CloseShutter
	MOV	dptr,#Str_TODO_CloseShutter
	CALL	FT_SendString

  No_TODO_CloseShutter:
	JNB	TODO_OpenShutter, No_TODO_OpenShutter
	MOV	dptr,#Str_TODO_OpenShutter
	CALL	FT_SendString

  No_TODO_OpenShutter:
	JNB 	TODO_StartInterval, No_TODO_StartInterval
	MOV	dptr,#Str_TODO_StartInterval
	CALL	FT_SendString

  No_TODO_StartInterval:
  	JNB	TODO_StartScope, No_TODO_StartScope
	MOV	dptr,#Str_TODO_StartScope
	CALL	FT_SendString

  No_TODO_StartScope:
  	JNB	TODO_StartTrigger, No_TODO_StartTrigger
	MOV	dptr,#Str_TODO_StartTrigger
	CALL	FT_SendString

  No_TODO_StartTrigger:
  	JNB	TODO_StartCounter, No_TODO_StartCounter
	MOV	dptr,#Str_TODO_StartCounter
	CALL	FT_SendString

  No_TODO_StartCounter:
  	JNB	TODO_MakeCommand1, No_TODO_MakeCommand1
	MOV	dptr,#Str_TODO_Command1
	CALL	FT_SendString

  No_TODO_MakeCommand1:
  	JNB	TODO_MakeCommand2, No_TODO_MakeCommand2
	MOV	dptr,#Str_TODO_Command2
	CALL	FT_SendString

  No_TODO_MakeCommand2:
	MOV	A,#'"'
	CALL	FT_SendChar
	MOV	A,#0dh
	CALL	FT_SendChar

	MOV	A,B
	POP	DPL
	POP	DPH
	POP	B
	RET

;===============================================================================
; Subroutines, Utilities
;===============================================================================

WaitMicrosecond:
		; Waits 1 microsecond by NOP
		; Expets call by LCALL

        ; CALL		        	; 4/3 cycles (Use LCALL to be precise)
	NOP                     	; 1

	NOP
	NOP
	NOP
	NOP
	NOP

	NOP
	NOP
	NOP
	NOP
	NOP

	NOP
	RET				; 4 cycles


WaitMicroseconds:
                ; Waits roughly A microseconds (use LCALL to be precise)
                ; Expects: 20MHz clock => 50 ns/cycle
                ; 1 us = 20 cycles;
                                	; cycles:
        ;CALL command of caller 	; 4/3 (LCALL, ACALL)    4
        push 	b       		; 2                     6
        push	acc             	; 2                     8
        MOV	B,A			; 1                     9
        SJMP	OneMicLoopEntry		; 3                     12

  OneMicrosecnd:
  	NOP                     	; 1
  	NOP                     	; 2
  	NOP                     	; 3
  	NOP				; 4
  	NOP				; 5

	NOP				; 6
	NOP				; 7
	NOP				; 8
	NOP				; 9
	NOP				; 10

	NOP				; 11
	NOP				; 12
	NOP				; 13
	NOP				; 14
	NOP				; 15

	NOP				; 16
    OneMicLoopEntry:
	DJNZ	B,OneMicrosecnd 	; 20 cycles = 1 us

   WaitMicrosEnd:
        pop	acc             	; 2			14
        pop	b               	; 2                     16
        ret                     	; 4                     20


Wait10MicroSeconds:
		; Waits roughly A tens of microseconds
	push 	b
	push 	acc
	push	dpl
	mov	dpl,a
    TenMicroSeconds_:
        mov b,#50
      TenMicroInner_:
        djnz b,TenMicroInner_  		; Waits 0.01 of milisecond
	djnz dpl,TenMicroSeconds_

	POP	DPL
	POP	ACC
	POP	B
	RET

Wait:           ; Waits roughly A milliseconds

        push b
        push dpl
        push dph

        mov  dph,a

  Milisecond:
        mov dpl,#100

    TenMicroSeconds:
        mov b,#50
      TenMicroInner:
        djnz b,TenMicroInner   		; Waits 0.01 of milisecond
        djnz dpl,TenMicroSeconds	; Waits 1 milisecond

        djnz dph,Milisecond

        pop dph
        pop dpl
        pop b
        ret


WaitTenth:    	; Waits tenth of second
	push acc
	mov a,#100
	call Wait
	pop acc
	ret

WaitQuarter:    ; Waits quarter of second
	push acc
	mov a,#250
	call Wait
	pop acc
	ret

WaitHalf:	; Waits half of second
	call WaitQuarter
	jmp  WaitQuarter

WaitSecond:     ; Waits one second
	call WaitHalf
	jmp  WaitHalf

;===============================================================================
; FTDI support:
;===============================================================================

; Symbols:
FT_port   EQU   P0      ; Port P0 is set as I/O gate
FT_RD     EQU   P2.0    ; P3.7 drives reading from FTDI (active low)
FT_WR     EQU   P2.1    ; P3.6 drives writing to FTDI (active high)
FT_TxE    EQU   P2.2    ; P2.1 monitor the write buffer (low -> data can be writed)
FT_RxF    EQU   P2.3    ; P2.0 monitor the read buffer (low -> data can be readed)

;_______________________________________________________________________________
                                                                   ; FT_SendChar

FT_SendChar:    ; sends byte contained in A to FTDI

        ;MOV     FT_port,A       ; Send data from accumulator to FTDI port
        ;JB      FT_TxE,$        ; Wait util FTDI is ready to write	
        ;CLR     FT_WR           ; Make a write pulse
        ;SETB    FT_WR
		
	;;;; UART ;;;;
	CLR 	TI
	MOV 	SBUF,A
	JNB	TI,$
        RET
;_______________________________________________________________________________
                                                                    ; FT_GetChar

FT_GetChar:     ; waits for a byte to be received from FTDI
                ; places this character into A.

        ;MOV     FT_port,#0FFh   ; Set FT_port as input
        ;JB      FT_RxF,$        ; Wait until FTDI is ready to read
        ;CLR     FT_RD
        ;MOV     A,FT_port       ; Get data from FTDI port to accumulator
        ;SETB    FT_RD           ; Make read data pulse	
	
	RefreshWatchdog

	;;;; UART ;;;;
	JNB	RI, FT_GetChar             ; test if it is a reception
       	CLR	RI			       ; clear reception flag for next reception
	MOV	A,SBUF         
	RET
;_______________________________________________________________________________
                                                                    ; FT_GetChar

FT_CheckChar:   ; gets a byte from FTDI places this character into A.
		; if FTDI is not ready returns CY=1

        JB     RI,FT_TakeChar  ; Check if FTDI is ready to be read (if ready then jump)
        SETB	C
        RET


  FT_TakeChar:
       	CLR RI			       ; clear reception flag for next reception
	MOV A,SBUF  
        CLR	C
        RET

;_______________________________________________________________________________

FT_ClearFIFO:   ; gets all characters in FTDI FIFO

        MOV     FT_port,#0FFh   	; Set FT_port as input
        JB      FT_RxF,ftClearEnd  	; Check if FTDI is ready to read
  ftClrNext:
	CLR     FT_RD
        SETB    FT_RD           	; Make read data pulse
        JNB 	FT_RxF,ftClrNext	; Check if FTDI is ready to read

  ftClearEnd:
	RET
;_______________________________________________________________________________
                                                                 ; FT_SendString

FT_SendString:  ; sends zero terminated ASCII string stored in program memory to FTDI
                ; starting at location DPTR

        PUSH    ACC

  FT_010:
        CLR     A
        MOVC    A,@A+DPTR
        JZ	FT_020          ; test terminating zero
        ;MOV     FT_port,A       ; Send data from accumulator to FTDI port

	;;;; UART ;;;;
	;;;; Posílání textového infa ;;;;
	CLR 	TI		; write uart
	MOV 	SBUF,A		
	JNB	TI,$		; Wait until UART is written

        INC     DPTR
        ;JB      FT_TxE,$        ; Wait util FTDI is ready to write
        ;CLR     FT_WR           ; Make a write pulse
        ;SETB    FT_WR
        
	JMP     FT_010          ; Go to next character

  FT_020:
        POP     ACC
        RET
;_______________________________________________________________________________

END

