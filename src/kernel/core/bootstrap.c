#include <abstract/arch.h>

int _start()
{
    Output serial = abstract_serial_acquire();
    serial.puts(&serial, "Hello, World !", 14);
    serial.release(&serial);

    for (;;)
        ;
}