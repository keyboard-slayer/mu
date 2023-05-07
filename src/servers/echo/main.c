#include <mu-api/bootstrap.h>
#include <mu-base/std.h>

#include "mu-mem/heap.h"

int mu_main(unused MuArgs args)
{
    MuCap port;
    // MuMsg msg;

    mu_create_port(&port, MU_IPC_RECV | MU_IPC_SEND);
    bootstrap_register_server(port, "engineering.cute.echo");

    // for (;;)
    // {
    //     mu_ipc(&port, &msg, MU_IPC_RECV | MU_IPC_BLOCK);
    //     debug_info("Echo: {}", (cstr)msg.args.arg1);

    //     if (msg.args.arg2)
    //     {
    //         auto heap = heap_acquire();
    //         heap.free(&heap, (void *)msg.args.arg1, strlen((cstr)msg.args.arg1) + 1);
    //         heap_release(&heap);
    //     }
    // }

    return 0;
}