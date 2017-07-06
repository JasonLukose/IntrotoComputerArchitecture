    .ORIG x3000
    AND R2, R2, #0
    AND R0, R0, #0

    LEA R0, KBSR   ; read KBSR first
    LDW R0, R0, #0 ; address of KBSR
    LDW R3, R0, #0 ;
 
    ADD R2, R2, #1  
    STW R2, R0, #0 ; write a 1 to kbsr 
    LDW R3, R0, #0 ; read it back

    LEA R0, DSR    ; read DSR first
    LDW R0, R0, #0 ; address of DSR
    LDW R4, R0, #0 ;
 
    ADD R2, R2, #1  
    STW R2, R0, #0 ; write a 1 to DSR 
    LDW R4, R0, #0 ; read it back


    LEA R0, DDR    ; write DDR 
    LDW R0, R0, #0 ; address of DSR
 
    ADD R2, R2, #1  
    STW R2, R0, #0 ; write a 1 to DSR 

    LEA R0, KBDR    ; read KBDR 
    LDW R0, R0, #0 ; address of KBDR
 
    LDW R5, R0, #0 ;  read a 1 to KBDR 

    HALT

KBSR .FILL 0xFE00
KBDR .FILL 0xFE02
DSR  .FILL 0xFE04
DDR  .FILL 0xFE06
    .END
