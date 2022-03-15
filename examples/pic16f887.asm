;Memory address declaration
.constant       porta,  0x05
.constant       tx_reg, 0x19
.constant       status, 0x83
.constant       var,    0x50 ;Variable address to store the current data

0x0:    GOTO    main 

0x10: ;;Comment
main:
press: BTFSS   porta,b0
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