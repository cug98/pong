/*
* Unterstützende Assembler-Funktionen für 1D-Pong
* 
* Abgabe von Übungsgruppe 1, Gruppe 9:
* Maximilian Filzer, Matrikelnummer: 1196756
* Christian Galda, Matrikelnummer: 1224466
*/

;The following is needed to subtract 
;0x20 from I/O addresses
#define __SFR_OFFSET 0
#define TEMP R17
#define TEMP2 R19
#define TEMP_LOW R30
#define TEMP_HIGH R31

#include "avr/io.h"

; Arduino MEGA2560 PIN51 -- DATA  -- PB2
; Arduino MEGA2560 PIN52 -- CLOCK -- PB1


;takt
clock_short:
  sbi PORTB,PB1
  cbi PORTB,PB1
ret

; STRIPE_com: implements the protocol/timing to communicate with the LED-Stripe
; Parameter: R24
; Values:
;   0 - 255: data
.global STRIPE_com
STRIPE_com:
  ;setze externen clock und datenleitung und buzzer auf output
  push TEMP
  push R24

  ;setze Ports für Switches und Buzzer auf input
  ldi TEMP, (1<<DDB1) | (1<<DDB2) | (1<<DDB5)
  out DDRB,TEMP
 
  clr TEMP
  start_loop:
    ;überspringe befehl wenn bit gelöscht ist
    sbrc R24,7
    sbi PORTB,PB2

    ;überspringe wenn bit gesetzt ist
    sbrs R24,7
    cbi PORTB,PB2
    
    call clock_short
    
    ;shift von R24, um nächstes Bit zu überprüfen
    lsl R24

    ;falls alle bits durch, beende
    inc TEMP
    cpi TEMP, 8
    brne start_loop

  pop R24
  pop TEMP
ret

; STRIPE_show: sets a single pixel to the specified color (and switches off all others before it)
; Parameter: r25-r16
; Values:
;   R25, R24 (MSB, LSB)   0 - 2^16: index of pixel to set
;   R22, R20, R18 (R,G,B) 0 - 255:  RGB colors
;   R16                   0-31:     Brigthness
.global STRIPE_show
STRIPE_show:
  ;register saven
  push TEMP
  push TEMP2
  push R24
  push R25
 
  mov TEMP, R24 
  mov TEMP2,R25


  ;start frame
  ldi R24, 0 
  call STRIPE_com
  call STRIPE_com
  call STRIPE_com
  call STRIPE_com


  m_loop:
    msb_and_lsb_loop:
    ;überprüfe Zähler
    cpi TEMP,0
    brne temp_loop
    cpi TEMP2,0
    brne temp_loop
    rjmp done


  temp_loop:

    ;LED FRAME für ausgeschaltete LEDs
    ldi R24, 0xE0  //Helligkeit auf 0
    call STRIPE_com
    ldi R24,0 ;Alle 3 Farben aus
    call STRIPE_com
    call STRIPE_com
    call STRIPE_com

    subi TEMP,1; schleife verkleinern

    rjmp m_loop

  
  done:
    ;angeschaltete LED
    mov R24,R16 
    ori R24, 0xE0 ; 224 für die drei high bits im led frame
    call STRIPE_com
    mov R24,R18  ;blue
    call STRIPE_com
    mov R24,R20  ;green
    call STRIPE_com
    mov R24,R22  ;red
    call STRIPE_com
    

    pop R24
    pop R25
    pop TEMP2
    pop TEMP
    call END_frame
  ret


;END_frame: implements the variable bytes for the End-Frame
;Parameter R25, R24 (MSB, LSB)
.global END_frame
END_frame:
  push R24
  push R25
  push TEMP ;save carry
  push TEMP2 ;zähle Divisionen
  push TEMP_LOW
  push TEMP_HIGH

  mov TEMP_LOW,R24
  mov TEMP_HIGH,R25

  clr TEMP;save carry
  clr TEMP2;zähle Divisionen durch 2
  clr R24; sende 0-bytes

  sbiw TEMP_LOW,1 ;substract 1

  ;dividiere durch 16 (durch 2 vier mal)
  divide_loop:
    lsr TEMP_HIGH
    ror TEMP_LOW
    inc TEMP2
    brcc dangling_carry
    rjmp round_up

  ;speichere carry falls Rest
  dangling_carry:
    ldi TEMP, 1
    cpi TEMP2, 4
    brne divide_loop

  ;addiere 1 falls Rest
  round_up:
  cpi TEMP, 1
  brne send_loop
  adiw TEMP_LOW,1 

  ;sende 0-byte solange TEMP_HIGH != 0 || TEMP_LOW != 0
  send_loop:
    call STRIPE_com
    sbiw TEMP_LOW,1

    cpi TEMP_HIGH, 0
    brne send_loop
    cpi TEMP_LOW, 0
    brne send_loop
    
  pop TEMP_HIGH
  pop TEMP_LOW
  pop TEMP2
  pop TEMP
  pop R25
  pop R24
ret
