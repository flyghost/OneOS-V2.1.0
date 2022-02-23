
/* Highest address of the user mode stack */
_estack = 0x20008000;    /* end of RAM */
/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size = 0x200;      /* required amount of heap  */        
_Min_Stack_Size = 0x200; /* required amount of stack */

/* Use contiguous memory regions for simple. */
MEMORY
{
    FLASH  (rx): ORIGIN = 0x00000000, LENGTH = 256K
    RAM0   (rw): ORIGIN = 0x20000000, LENGTH = 0x10
    RAM    (rx): ORIGIN = 0x20000010, LENGTH = 0x7ff0 /* 0x7ff0 sram */
}

ENTRY(Reset_Handler)

SECTIONS
{
    .vectors :
    {
        KEEP(*(vtor_table))
    } >FLASH

    .text :
    {
        *(.text)
        *(.text*)
        *(.glue_7)
        *(.glue_7t)
        *(.eh_fream)
        
        /* section information for shell shell */
        . = ALIGN(4);
        __fsymtab_start = .;
        KEEP(*(FSymTab))
        __fsymtab_end = .;

        /* section information for shell atest */
        . = ALIGN(4);
        __atest_tc_table_start = .;
        KEEP(*(AtestTcTab))
        __atest_tc_table_end = .;

        /* section information for initial. */
        . = ALIGN(4);
        __os_init_start = .;
        KEEP(*(SORT(.init_call*)))
        __os_init_end = .;

 
         /* section information for OneOS AT Parser */
         . = ALIGN(4);
         __at_cmd_tab_start = .;
         KEEP(*(at_cmd_tab))
         __at_cmd_tab_end = .;
 
        /* section information for OneOS drivers */
        . = ALIGN(4);
        __driver_table_start = .;
        KEEP(*(driver_table))
        __driver_table_end = .;
        
        /* section information for OneOS devices */
        . = ALIGN(4);
        __device_table_start = .;
        KEEP(*(device_table))
        __device_table_end = .;
        
        /* section information for modules */
        . = ALIGN(4);
        __osmsymtab_start = .;
        KEEP(*(MSymTab))
        __osmsymtab_end = .;
        
        /* section information for irq hook */
        . = ALIGN(4);
        __os_irq_hook_start = .;
        KEEP(*(os_irq_hook))
        __os_irq_hook_end = .;
        
        KEEP(*(.init))
        KEEP(*(.fini))
    } >FLASH

    .text1 0x700 :
    {
        *(.ARM.__at_0x600)
    } >FLASH
    
    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
    } >FLASH

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } >FLASH

    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } >FLASH
    __exidx_end = .;

    .reserved_ram :
    {
        * (reserved_ram)
    } > RAM0

    .preinit_array :
    {
        /* preinit data */
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP(*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);
    } >FLASH

    .init_array :
    {
        /* init data */
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP(*(SORT(.init_array.*)))
		KEEP(*(.init_array))
		PROVIDE_HIDDEN (__init_array_end = .);
    } >FLASH

    .fini_array :
    {
        /* finit data */
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP(*(SORT(.fini_array.*)))
		KEEP(*(.fini_array))
		PROVIDE_HIDDEN (__fini_array_end = .);
    } >FLASH

    __etext = LOADADDR(.data);
    .data :
    {
        . = ALIGN(4);
        __data_start__ = .;
        *(vtable)
        *(.data)
        *(.data*)
        . = ALIGN(4);
        __data_end__ = .;
    } >RAM AT> FLASH

    .bss :
    {
        . = ALIGN(4);
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    } >RAM


    /* User_heap_stack section, used to check that there is enough RAM left */
    ._user_heap_stack :
    {
      . = ALIGN(8);
      PROVIDE ( end = . );
      PROVIDE ( _end = . );
      . = . + _Min_Heap_Size;
      . = . + _Min_Stack_Size;
      . = ALIGN(8);
    } >RAM
    __StackTop = .;
    PROVIDE(_stack = __StackTop);

    ASSERT(__StackTop < _estack, "region RAM overflowed with stack")
}

