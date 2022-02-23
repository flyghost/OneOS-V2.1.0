#ifndef __ARCH_HW_H__
#define __ARCH_HW_H__

#include <oneos_config.h>
#include <os_types.h>

#ifdef __cplusplus
extern "C" {
#endif

    /*
     * Some macros define
     */
#ifndef HWREG32
#define HWREG32(x)          (*((volatile os_uint32_t *)(x)))
#endif
#ifndef HWREG16
#define HWREG16(x)          (*((volatile os_uint16_t *)(x)))
#endif
#ifndef HWREG8
#define HWREG8(x)           (*((volatile os_uint8_t *)(x)))
#endif

#ifndef OS_CPU_CACHE_LINE_SZ
#define OS_CPU_CACHE_LINE_SZ    32
#endif

    enum OS_HW_CACHE_OPS
    {
        OS_HW_CACHE_FLUSH      = 0x01,
        OS_HW_CACHE_INVALIDATE = 0x02,
    };

    /*
     * CPU interfaces
     */
    void os_hw_cpu_icache_enable(void);
    void os_hw_cpu_icache_disable(void);
    os_base_t os_hw_cpu_icache_status(void);
    void os_hw_cpu_icache_ops(int ops, void* addr, int size);

    void os_hw_cpu_dcache_enable(void);
    void os_hw_cpu_dcache_disable(void);
    os_base_t os_hw_cpu_dcache_status(void);
    void os_hw_cpu_dcache_ops(int ops, void* addr, int size);

    void os_hw_cpu_reset(void);
    void os_hw_cpu_shutdown(void);

    /*
     * Interrupt handler definition
     */
    typedef void (*os_isr_handler_t)(int vector, void *param);

    struct os_irq_desc
    {
        os_isr_handler_t handler;
        void            *param;

#ifdef OS_USING_INTERRUPT_INFO
        char             name[OS_NAME_MAX];
        os_uint32_t      counter;
#endif
    };

    /*
     * Interrupt interfaces
     */
    void os_hw_interrupt_init(void);
    void os_hw_interrupt_mask(int vector);
    void os_hw_interrupt_umask(int vector);
    os_isr_handler_t os_hw_interrupt_install(int              vector,
            os_isr_handler_t handler,
            void            *param,
            const char      *name);


    os_base_t os_hw_interrupt_disable(void);
    void os_hw_interrupt_enable(os_base_t level);

    /*
     * Context interfaces
     */

    void os_hw_context_switch(os_ubase_t from, os_ubase_t to);
    void os_hw_context_switch_to(os_ubase_t to);
    void os_hw_context_switch_interrupt(os_ubase_t from, os_ubase_t to);

    //void os_hw_console_output(const char *str);

    void os_hw_backtrace(os_uint32_t *fp, os_ubase_t thread_entry);
    void os_hw_show_memory(os_uint32_t addr, os_size_t size);

    /*
     * Exception interfaces
     */
    void os_hw_exception_install(os_err_t (*exception_handle)(void *context));

#ifdef __cplusplus
}
#endif

#endif

