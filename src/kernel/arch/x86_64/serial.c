#include <abstract/arch.h>
#include <macro.h>
#include <stdbool.h>
#include <stdint.h>

#include "asm.h"
#include "serial.h"

static bool init = false;

static void serial_write(int reg, uint8_t value)
{
    asm_out8(SERIAL_PORT + reg, value);
}

static uint8_t serial_read(int reg)
{
    return asm_in8(SERIAL_PORT + reg);
}

static void serial_init(void)
{
    const uint8_t div_low = COM_BAUD_DIV & 0xff;
    const uint8_t div_high = COM_BAUD_DIV >> 8;

    serial_write(COM_REGS_LINE_CONTROL, 0x80);
    serial_write(COM_REGS_INTERRUPT, 0x00);
    serial_write(COM_REGS_BAUD_RATE_LOW, div_low);
    serial_write(COM_REGS_BAUD_RATE_HIGH, div_high);
    serial_write(COM_REGS_LINE_CONTROL, 0x03);
    serial_write(COM_REGS_FIFO_CONTROLLER, 0xc7);
    serial_write(COM_REGS_MODEM_CONTROL, 0x0b);

    init = true;
}

static void serial_putc(unused Output *self, char c)
{
    if (!init)
    {
        serial_init();
    }

    while ((serial_read(COM_REGS_LINE_STATUS) & 0x20) == 0)
        ;

    serial_write(COM_REGS_DATA, c);
}

static void release_serial(Output *self)
{
    *self = (Output){0};
}

Output abstract_serial_acquire(void)
{
    return (Output){
        .putc = serial_putc,
        .puts = generic_puts,
        .release = release_serial,
    };
}