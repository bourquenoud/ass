;Memory address declaration
.constant       porta   0x05
.constant       tx_reg  0x19
.constant       status  0x83
.constant       var     0x50 ;Variable address to store the current data

0x00:    GOTO    main 

0x10:  ; Place following instructions at 0x10 
main:
press:  BTFSS   porta,b0
        GOTO    press
relea:  BTFSC   porta,b0
        GOTO    relea

print:  MOVLW   ' '    ;Start from z
        MOVWF   var

loop:   DECF    var,F
        MOVF    var,W
        MOVWF   tx_reg
        SUBLW   'a'
        BTFSS   status,b2
        GOTO    loop
        GOTO    main
