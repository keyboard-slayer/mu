#include <mu-hal/hal.h>

void embed_abort(void)
{
    hal_cpu_stop();
}