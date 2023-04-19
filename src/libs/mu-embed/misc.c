#include <mu-api/api.h>

void embed_abort(void)
{
    mu_exit(1);
    for (;;)
        ;
}