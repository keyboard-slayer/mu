#include <misc/lock.h>
#include <misc/macro.h>

#include "asm.h"
#include "bochs.h"

static Spinlock lock = {0};

static void bochs_putc(unused Output *self, char c)
{
    asm_out8(PORT_E9, c);
}

static void bochs_release(Output *self)
{
    memset(self, 0, sizeof(Output));
    spinlock_release(&lock);
}

Output bochs_out_acquire(void)
{
    spinlock_acquire(&lock);
    return (Output){
        .putc = bochs_putc,
        .puts = generic_puts,
        .release = bochs_release,
    };
}

void bochs_set_breakpoint()
{
    Output out = bochs_out_acquire();
    out.puts(&out, "\n[ * ] Breakpoint hit !\n", 25);
    out.release(&out);

    asm_out16(BOCHS_COMMAND_REG, BOCHS_ENABLE_DEVICE);
    asm_out16(BOCHS_COMMAND_REG, BOCHS_RETURN_PROMPT);
}