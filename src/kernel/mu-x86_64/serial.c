#include <mu-hal/hal.h>
#include <mu-misc/lock.h>
#include <pico-misc/macro.h>
#include <pico-traits/writer.h>

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

static void serial_putc(unused Writer *self, char c)
{
    while ((asm_in8(SERIAL_PORT + COM_REGS_LINE_STATUS) & 0x20) == 0)
        ;

    serial_write(COM_REGS_DATA, c);
}

void serial_release(Writer *self)
{
    spinlock_release(&lock);
    *self = (Writer){0};
}

Writer hal_acquire_serial(void)
{
    spinlock_acquire(&lock);

    if (!init)
    {
        serial_init();
    }

    return (Writer){
        .putc = serial_putc,
        .release = serial_release,
        .puts = generic_puts,
    };
}