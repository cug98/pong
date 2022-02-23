

;The following is needed to subtract 
;0x20 from I/O addresses
#define __SFR_OFFSET 0
#define TEMP R17
#define TEMP2 R25
#define TEMP3 R19
#define TEMP4 R26

#include "avr/io.h"

; Arduino MEGA2560 PIN51 -- DATA  -- PB2
; Arduino MEGA2560 PIN52 -- CLOCK -- PB1

; STRIPE_com: implements the protocol/timing to communicate with the LED-Stripe
; Parameter: R24
; Values:
;   0 - 255: data

;takt
clock_short:
  sbi PORTB,PB1
  cbi PORTB,PB1
  ret

.global STRIPE_com
STRIPE_com: ; am besten register r24 vom stack poppen und kopieren
  ;setze externen clock und datenleitung und buzzer auf output
  ldi TEMP4, (1<<DDB1) | (1<<DDB2) | (1<<DDB5)
  out DDRB,TEMP4

  ; einzelne bits auslesen und seriell auf die datenleitung schreiben
  ; verundung der register
 

    //TODO: use loop with an AND to shorten function?
    ;überspringe befehl wenn bit gelöscht ist
    sbrc R24,7
    sbi PORTB,PB2

    ;überspringe wenn bit gesetzt ist
    sbrs R24,7
    cbi PORTB,PB2

    call clock_short

    ;überspringe befehl wenn bit gelöscht ist
    sbrc R24,6
    sbi PORTB,PB2


    ;überspringe wenn bit gesetzt ist
    sbrs R24,6
    cbi PORTB,PB2
    call clock_short

    ;überspringe befehl wenn bit gelöscht ist
    sbrc R24,5
    sbi PORTB,PB2


    ;überspringe wenn bit gesetzt ist
    sbrs R24,5
    cbi PORTB,PB2
    call clock_short

    ;überspringe befehl wenn bit gelöscht ist
    sbrc R24,4
    sbi PORTB,PB2


    ;überspringe wenn bit gesetzt ist
    sbrs R24,4
    cbi PORTB,PB2
    call clock_short

    ;überspringe befehl wenn bit gelöscht ist
    sbrc R24,3
    sbi PORTB,PB2

    ;überspringe wenn bit gesetzt ist
    sbrs R24,3
    cbi PORTB,PB2
    call clock_short

    ;überspringe befehl wenn bit gelöscht ist
    sbrc R24,2
    sbi PORTB,PB2

    ;überspringe wenn bit gesetzt ist
    sbrs R24,2
    cbi PORTB,PB2
    call clock_short

    ;überspringe befehl wenn bit gelöscht ist
    sbrc R24,1
    sbi PORTB,PB2


    ;überspringe wenn bit gesetzt ist
    sbrs R24,1
    cbi PORTB,PB2
    call clock_short

    ;überspringe befehl wenn bit gelöscht ist
    sbrc R24,0
    sbi PORTB,PB2


    ;überspringe wenn bit gesetzt ist
    sbrs R24,0
    cbi PORTB,PB2
    call clock_short

 

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
  mov TEMP, R24 ;R24
  mov TEMP2,R25
  push R24
  push R25

  ;set temp3 auf max
  ser TEMP3

  ;start frame
  ldi R24, 0 
  call STRIPE_com
  call STRIPE_com
  call STRIPE_com
  call STRIPE_com



  msb_and_lsb_loop:
  ;cpi TEMP2,0 ; wenn msb nicht leer ist zähle temp3 also 0xFF herunter
  ;brne temp3_loop
  cpi TEMP,0 ; wenn msb leer ist zähle temp herunter
  brne temp_loop
  rjmp done;done


  temp3_loop:
    subi TEMP2,1
    ser TEMP3
    inner_loop:
    cpi TEMP3,0
    breq msb_and_lsb_loop

    ;LED FRAME
    ldi R24, 0xE0  //brightness null 
    call STRIPE_com
    ldi R24, 0 
    call STRIPE_com
    call STRIPE_com
    call STRIPE_com

    subi TEMP3,1

    rjmp inner_loop

  temp_loop:
    cpi TEMP,0
    breq done

    ;LED FRAME
    ldi R24, 0xE5;0xE0  //brightness null 
    call STRIPE_com
    ldi R24,0 ;0 ;0x0A  
    call STRIPE_com
    call STRIPE_com
    call STRIPE_com

    subi TEMP,1; schleife verkleinern

    rjmp temp_loop

  
  done:
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
    call END_frame
    ret


 ;END_frame: implements the variable bytes for the End-Frame
 ;Parameter R25, R24 (MSB, LSB)
.global END_frame
END_frame:
  //TODO: add correct end_frame
  push R24
  mov TEMP,2;R24
  mov TEMP2,R25

  ldi R24,0

  ser TEMP3

  msb_and_lsb_loop1:
  cpi TEMP2,0 ; wenn msb nicht leer ist zähle temp3 also 0xFF herunter
  brne temp3_loop1
  cpi TEMP,0 ; wenn msb leer ist zähle temp herunter
  brne temp_loop1
  rjmp done1


  temp3_loop1:
    subi TEMP2,1
    ser TEMP3
    inner_loop1:
    cpi TEMP3,0
    breq msb_and_lsb_loop1

    ;END FRAME

    call STRIPE_com

    subi TEMP3,1

    rjmp inner_loop1

  temp_loop1:
    cpi TEMP,0
    breq done1

    ;END FRAME
    
    call STRIPE_com

    subi TEMP,1; schleife verkleinern

    rjmp temp_loop1

  done1:
  pop R24
  ret
 

 
.global blue_LED
blue_LED:
  push R24
  //LED-ON
  //Start-Frame
  ldi R24, 0
  call STRIPE_com
  call STRIPE_com
  call STRIPE_com
  call STRIPE_com
  //LED-Frame
  ldi R24, 0xE5 //Brigtness 5
  call STRIPE_com
  
  ldi R24, 0x0A //Blue
  call STRIPE_com
  
  ldi R24, 0x0 //Green
  call STRIPE_com
  call STRIPE_com //Red
  
  //END-Frame
  call STRIPE_com 
  pop R24
ret


.global red_LED
red_LED:
  push R24
  //LED-ON
  //Start-Frame
  ldi R24, 0
  call STRIPE_com
  call STRIPE_com
  call STRIPE_com
  call STRIPE_com
  //LED-Frame
  ldi R24, 0xE5 //Brigtness 5
  call STRIPE_com
  
  ldi R24, 0x00 //Blue
  call STRIPE_com
  
  ldi R24, 0x0 //Green
  call STRIPE_com

  ldi R24, 0x0A //Red
  call STRIPE_com

  //END-Frame
  call STRIPE_com 
  pop R24
ret