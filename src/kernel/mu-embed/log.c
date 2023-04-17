#include <mu-embed/log.h>
#include <mu-hal/hal.h>

Writer embed_acquire_writer(void)
{
    return hal_acquire_serial();
}