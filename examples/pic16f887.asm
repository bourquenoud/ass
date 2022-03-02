0x0:    GOTO    main

0x10: ;;Comment
main:
_press: BTFSS   0x05,b0
        GOTO    _press
relea:  BTFSC   0x05,b0
        GOTO    relea

print:  MOVLW   ' '    ;Start from z
        MOVWF   0x50

loop:   DECF    0x50,F
        MOVF    0x50,W
        MOVWF   0x19
        SUBLW   'a'
        BTFSS   0x83,b2
        GOTO    loop
        GOTO    main