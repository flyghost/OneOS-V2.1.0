    AREA |.text|, CODE, READONLY, ALIGN=2
    THUMB
    REQUIRE8
    PRESERVE8
	EXPORT push_common_registers
	EXPORT pop_common_registers
		
push_common_registers    PROC
    PUSH    {R4-R11}
    BX LR
    ENDP
    
pop_common_registers    PROC
    ADD  SP, #0x20
    BX LR
    ENDP
    
	ALIGN   4
    END
