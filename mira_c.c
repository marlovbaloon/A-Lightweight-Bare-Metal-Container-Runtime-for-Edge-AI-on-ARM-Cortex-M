/**
 * @file mira_c.c
 * @brief Implementation of MIRA-C Kernel-less Container Runtime
 */

#include "mira_c.h"
#include <stdio.h>

static mira_container_t *active_container = 0;

int mira_runtime_init(void) {
    active_container = 0;
    printf("[MIRA-C Kernel] Runtime initialized successfully.\n");
    return 0;
}

void mira_mpu_isolate_container(const mira_container_t *container) {
#ifdef __arm__
    /* ARM Cortex-M MPU Configuration */
    MPU_CTRL = 0;
    MPU_RNR = 1;
    MPU_RBAR = 0x20000000UL | MPU_REGION_ENABLE | MPU_REGION_VALID;
    MPU_RASR = MPU_RASR_AP_NO_ACCESS | (14UL << 1) | MPU_REGION_ENABLE;

    MPU_RNR = 2;
    MPU_RBAR = (container->header.ram_base) | MPU_REGION_ENABLE | MPU_REGION_VALID;
    MPU_RASR = MPU_RASR_AP_FULL_ACCESS | (15UL << 1) | MPU_REGION_ENABLE;

    SCB_SHCSR |= (1UL << 16);
    MPU_CTRL = (1UL << 2) | (1UL << 0);
#else
    /* Host Simulation Mode (x86_64 Codespaces) */
    (void)container;
#endif
}

int mira_container_spawn(mira_container_t *container, const uint8_t *image_flash_addr) {
    const mira_container_header_t *hdr = (const mira_container_header_t *)image_flash_addr;

    if (hdr->magic != 0x4D495241) { /* "MIRA" */
        return -1; 
    }

    container->header = *hdr;
    container->id = 1;
    container->state = CONTAINER_RUNNING;
    container->fault_count = 0;
    active_container = container;

    mira_mpu_isolate_container(container);

#ifdef __arm__
    uint32_t top_of_stack = container->header.ram_base + container->header.ram_size;
    __asm volatile ("msr psp, %0" : : "r" (top_of_stack));

    uint32_t control_reg;
    __asm volatile ("mrs %0, control" : "=r" (control_reg));
    control_reg |= 0x03; 
    __asm volatile ("msr control, %0" : : "r" (control_reg));
    __asm volatile ("isb");

    void (*entry_func)(void) = (void (*)(void))container->header.entry_point;
    entry_func();
#else
    /* Host Simulation Mode: Safe Function Pointer Execution */
    void (*entry_func)(void) = (void (*)(void))container->header.entry_point;
    if (entry_func != NULL) {
        entry_func();
    } else {
        return -2; /* Invalid Entry Point */
    }
#endif

    return 0;
}
void MemManage_Handler(void) {
#ifdef __arm__
    __asm volatile ("msr control, %0" : : "r" (0x00));
    __asm volatile ("isb");
    if (active_container != 0) {
        active_container->state = CONTAINER_FAULTED;
        active_container->fault_count++;
    }
    while (1) { /* Hardware trap on real Cortex-M */ }
#else
    /* Host Simulation Mode: Non-blocking trap for 10,000-iteration stress testing */
    if (active_container != 0) {
        active_container->state = CONTAINER_FAULTED;
        active_container->fault_count++;
    }
#endif
}