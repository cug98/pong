

;The following is needed to subtract 
;0x20 from I/O addresses
#define __SFR_OFFSET 0
#define TEMP R17
#define TEMP2 R25
#define TEMP3 R19
#define TEMP_LOW R30
#define TEMP_HIGH R31

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
  push TEMP
  push R24
  ldi TEMP, (1<<DDB1) | (1<<DDB2) | (1<<DDB5)
  out DDRB,TEMP

  ; einzelne bits auslesen und seriell auf die datenleitung schreiben
  ; verundung der register
 
  clr TEMP
  start_loop:
    //shift von R24 nach jedem Schleifenaufruf

    ;überspringe befehl wenn bit gelöscht ist
    sbrc R24,7
    sbi PORTB,PB2

    ;überspringe wenn bit gesetzt ist
    sbrs R24,7
    cbi PORTB,PB2
    
    call clock_short
    
    lsl R24
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
  push R24
  push R25
  push TEMP ;save carry
  push TEMP2 ;count divisions
  push TEMP_LOW
  push TEMP_HIGH

  mov TEMP_LOW,R24
  mov TEMP_HIGH,R25

  clr TEMP;save carry
  clr TEMP2;count divisions by two
  clr R24; send 0-bytes

  sbiw TEMP_LOW,1 ;substract 1

  ;divide by 16 (by 2 four times)
  divide_loop:
    lsr TEMP_HIGH
    ror TEMP_LOW
    inc TEMP2
    brcc dangling_carry
    rjmp round_up
  ;save carry if necessary
  dangling_carry:
    ldi TEMP, 1
    cpi TEMP2, 4
    brne divide_loop

  ;add one if carry is set
  round_up:
  cpi TEMP, 1
  brne send_loop
  adiw TEMP_LOW,1 

  ;send 0-byte while TEMP_HIGH != 0 || TEMP_LOW != 0
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