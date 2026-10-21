/**
 * @file mira_c.h
 * @brief MIRA-C: Micro Isolation Runtime Architecture for ARM Cortex-M
 * @author Arsira "Marlov" Langphang
 * 
 * Formal Mathematical Mapping:
 * - Access Function: A(m, a) mapping privilege state m and address a to {0, 1}
 * - Latency Model: T_MIRA = tau_mpu_write + tau_stack_switch + tau_priv_drop = O(1) ~ microseconds
 * - Invariant: Hardware-Enforced Containment via ARM MPU & MemManage Fault Handler
 */

#ifndef MIRA_C_H
#define MIRA_C_H

#include <stdint.h>
#include <stdbool.h>

/* ========================================================================
 * 1. HARDWARE REGISTER MAPPINGS (ARMv7-M / ARMv8-M MPU & Core System)
 * ======================================================================== */
#define MPU_BASE            (0xE000ED90UL)
#define MPU_CTRL            (*(volatile uint32_t *)(MPU_BASE + 0x00))
#define MPU_RNR             (*(volatile uint32_t *)(MPU_BASE + 0x04))
#define MPU_RBAR            (*(volatile uint32_t *)(MPU_BASE + 0x08))
#define MPU_RASR            (*(volatile uint32_t *)(MPU_BASE + 0x0C))

#define SCB_SHCSR           (*(volatile uint32_t *)(0xE000ED24UL))

/* MPU Region Attribute Masks */
#define MPU_REGION_ENABLE       (1UL << 0)
#define MPU_RASR_AP_NO_ACCESS   (0UL << 24)  /* Priv: None, Unpriv: None */
#define MPU_RASR_AP_FULL_ACCESS (3UL << 24)  /* Priv: R/W,  Unpriv: R/W  */
#define MPU_RASR_AP_RO_UNPRIV   (2UL << 24)  /* Priv: R/W,  Unpriv: R    */
#define MPU_REGION_VALID        (1UL << 4)

/* ========================================================================
 * 2. CONTAINER DATA STRUCTURES & ZERO-COPY FLASH IMAGE HEADER
 * ======================================================================== */
typedef enum {
    CONTAINER_STOPPED = 0,
    CONTAINER_RUNNING,
    CONTAINER_FAULTED
} container_state_t;

/**
 * @brief Zero-Copy Flash Image Header 
 * Represents I_flash mapped directly to Flash memory without OverlayFS.
 */
typedef struct {
    uint32_t magic;          /* Magic Number: 0x4D495241 ("MIRA") */
    uintptr_t entry_point;    /* Container Application Entry Point Address */
    uint32_t ram_base;       /* Isolated RAM Base Address (Omega_app) */
    uint32_t ram_size;       /* Allocated RAM Size (Power of 2 aligned) */
    uint32_t cpu_quota_cycles;/* Deterministic CPU Cycle Quota */
} __attribute__((packed)) mira_container_header_t;

typedef struct {
    uint8_t id;
    container_state_t state;
    mira_container_header_t header;
    uint32_t sp_entry;       /* Isolated Process Stack Pointer (PSP) */
    uint32_t fault_count;    /* Security violation telemetry */
} mira_container_t;

/* ========================================================================
 * 3. RUNTIME API PROTOTYPES
 * ======================================================================== */
int mira_runtime_init(void);
int mira_container_spawn(mira_container_t *container, const uint8_t *image_flash_addr);
void mira_mpu_isolate_container(const mira_container_t *container);
void MemManage_Handler(void);
#endif /* MIRA_C_H */