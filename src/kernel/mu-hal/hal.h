#pragma once

#include <handover/handover.h>
#include <mu-api/api.h>
#include <pico-traits/writer.h>

typedef struct _HalRegs HalRegs;

void hal_init(void);

/* --- CPU ------------------------------------------------------------------ */

#define HAL_CPU_MAX_LEN (255)

typedef struct _HalCpu HalCpu;

HalCpu *hal_cpu_begin(void);

HalCpu *hal_cpu_end(void);

HalCpu *hal_cpu_get(usize id);

HalCpu *hal_cpu_self(void);

usize hal_cpu_len(void);

void hal_cpu_cli(void);

void hal_cpu_debug(void);

void hal_cpu_halt(void);

void hal_cpu_relax(void);

void hal_cpu_sti(void);

void hal_cpu_stop(void);

void hal_cpu_goto(void (*fn)(void));

/* --- Context -------------------------------------------------------------- */

typedef struct _HalCtx HalCtx;

MuRes hal_ctx_create(HalCtx *self, uintptr_t ip, uintptr_t sp, MuArgs args);

void hal_ctx_destroy(HalCtx *self);

void hal_ctx_save(HalCtx *self, HalRegs *regs);

void hal_ctx_restore(HalCtx *self, HalRegs *regs);

/* --- Space ---------------------------------------------------------------- */

typedef struct _HalSpace HalSpace;

MuRes hal_space_create(HalSpace **self);

void hal_space_destroy(HalSpace *self);

void hal_space_apply(HalSpace *self);

MuRes hal_space_map(HalSpace *self, uintptr_t virt, uintptr_t phys, usize len, MuMapFlags flags);

MuRes hal_space_unmap(HalSpace *self, uintptr_t virt, usize len);

MuRes hal_space_virt2phys(HalSpace *self, uintptr_t virt, uintptr_t *phys);

HalSpace *hal_space_kernel(void);

/* --- Mmap ----------------------------------------------------------------- */

typedef struct
{
    uintptr_t phys;
    uintptr_t virt;
} HalAddr;

HalAddr hal_mmap_kaddr(void);

uintptr_t hal_mmap_lower_to_upper(uintptr_t phys);

uintptr_t hal_mmap_upper_to_lower(uintptr_t virt);

/* --- Handover ------------------------------------------------------------- */

HandoverPayload *hal_get_handover(void);

void hal_parse_handover(void);

usize hal_get_handover_size(void);

/* --- Misc ----------------------------------------------------------------- */

Writer hal_acquire_serial(void);
