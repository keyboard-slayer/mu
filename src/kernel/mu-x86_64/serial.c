#include <mu-base/std.h>
#include <mu-misc/lock.h>

#include "asm.h"
#include "serial.h"

static bool init = false;
static Spinlock lock = 0;

static void serial_write(int reg, u8 value)
{
    asm_out8(SERIAL_PORT + reg, value);
}

static void serial_init(void)
{
    const u8 div_low = COM_BAUD_DIV & 0xff;
    const u8 div_high = COM_BAUD_DIV >> 8;

    serial_write(COM_REGS_LINE_CONTROL, 0x80);
    serial_write(COM_REGS_INTERRUPT, 0x00);
    serial_write(COM_REGS_BAUD_RATE_LOW, div_low);
    serial_write(COM_REGS_BAUD_RATE_HIGH, div_high);
    serial_write(COM_REGS_LINE_CONTROL, 0x03);
    serial_write(COM_REGS_FIFO_CONTROLLER, 0xc7);
    serial_write(COM_REGS_MODEM_CONTROL, 0x0b);

    init = true;
}

void hal_serial_acquire(void)
{
    spinlock_acquire(&lock);
}

void hal_serial_release(void)
{
    spinlock_release(&lock);
}

void hal_serial_write(char const *str, usize len)
{
    if (!init)
    {
        serial_init();
    }

    for (usize i = 0; i < len; i++)
    {
        serial_write(COM_REGS_DATA, str[i]);
    }
}
