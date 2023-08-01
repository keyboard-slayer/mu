#include <misc/fb.h>
#include <mu-api/api.h>
#include <mu-mem/heap.h>
#include <munix-debug/debug.h>
#include <pico-misc/macro.h>
#include <string.h>

#include "../bootstrap/bootstrap.h"

noreturn int mu_main(MuArgs args)
{
    MuCap bootstrap_port = {._raw = args.arg1};
    Framebuffer *fb = (Framebuffer *)args.arg2;

    MuCap port;
    cstr server_name = strdup("engineering.cute.hello");

    debug_info("Hello, world! {a}", server_name);

    Alloc heap = heap_acquire();
    MuMsg *ipc_msg = unwrap(heap.malloc(&heap, sizeof(MuMsg)));
    heap.release(&heap);

    memcpy(ipc_msg, &mu_msg(BOOTSTRAP_REGISTER_SERVER, bootstrap_port, server_name, 23), sizeof(MuMsg));

    if (mu_create_port(&port, MU_PORT_SEND | MU_PORT_RECV))
    {
        panic("Couldn't create port");
    }

    if (mu_ipc(&bootstrap_port, ipc_msg, MU_MSG_SEND | MU_MSG_BLOCK) != MU_RES_OK)
    {
        panic("Couldn't register service");
    }

    debug_info("Message to bootstrap sent");

    for (size_t x = 0; x < fb->width; x++)
    {
        for (size_t y = 0; y < fb->height; y++)
        {
            size_t raw_position = x + y * fb->width;

            fb->pixels[raw_position].blue = x ^ y;
            fb->pixels[raw_position].red = (y * 2) ^ (x * 2);
            fb->pixels[raw_position].green = (y * 4) ^ (x * 4);
        }
    }

    loop;
    unreachable();
}