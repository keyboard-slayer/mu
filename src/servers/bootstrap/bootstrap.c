#include <const.h>
#include <handover/handover.h>
#include <handover/utils.h>
#include <misc/fb.h>
#include <mu-api/api.h>
#include <mu-embed/alloc.h>
#include <mu-mem/heap.h>
#include <munix-debug/debug.h>
#include <munix-loader/elf.h>
#include <string.h>
#include <sys/mman.h>
#include <tiny-json/json.h>

#include "bootstrap.h"
#include "sys/mman.h"

HandoverPayload *handover = NULL;
static bool framebuffer_assigned = false;

static MuRes handover_find_file(HandoverRecord *rec, const char name[static 1])
{
    handover_foreach_record(handover, *rec)
    {
        if (rec->tag == HANDOVER_FILE)
        {
            cstr filename = (cstr)handover + rec->file.name;
            if (strcmp(name, filename) == 0)
            {
                return MU_RES_OK;
            }
        }
    }

    return MU_RES_BAD_ARG;
}

static MuRes handover_find_framebuffer(HandoverRecord *fb)
{
    if (framebuffer_assigned)
    {
        return MU_RES_BAD_ARG;
    }

    handover_foreach_record(handover, *fb)
    {
        if (fb->tag == HANDOVER_FB)
        {
            framebuffer_assigned = true;
            return MU_RES_OK;
        }
    }

    return MU_RES_BAD_ARG;
}

static MaybeRcVec init_servers(json_t rc, MuCap port)
{
    RcVec rc_vec;
    MuCap vspace;

    vec_init(&rc_vec, heap_acquire);

    if (rc.type != JSON_OBJECT || mu_create_vspace(&vspace) != MU_RES_OK)
    {
        return None(MaybeRcVec);
    }

    json_t servers = json_get(rc, "servers");
    if (servers.type != JSON_ARRAY)
    {
        return None(MaybeRcVec);
    }

    json_t server;
    json_arr_foreach(&servers, server)
    {
        if (server.type != JSON_OBJECT)
        {
            return None(MaybeRcVec);
        }

        json_t path = json_get(server, "path");
        json_t args = json_get(server, "args");
        json_t req = json_get(server, "requires");

        if (path.type != JSON_STRING)
        {
            return None(MaybeRcVec);
        }

        MuArg mu_args[6] = {0};
        usize idx = 0;

        if (req.type == JSON_OBJECT)
        {
            json_t fb = json_get(req, "framebuffer");

            if (fb.type == JSON_BOOL && fb.number)
            {
                HandoverRecord fb_rec;
                uintptr_t fb_ptr;

                if (handover_find_framebuffer(&fb_rec) != MU_RES_OK)
                {
                    debug_warn("Failed to find framebuffer\n");
                    return None(MaybeRcVec);
                }

                if (mu_map(vspace, (MuCap){align_down(fb_rec.start, PAGE_SIZE)},
                           &fb_ptr, 0,
                           align_up(fb_rec.size, PAGE_SIZE), MU_MEM_READ | MU_MEM_WRITE) != MU_RES_OK)
                {
                    debug_warn("Failed to map framebuffer");
                    return None(MaybeRcVec);
                }

                Alloc heap = heap_acquire();
                Framebuffer *fb = unwrap(heap.malloc(&heap, sizeof(Framebuffer)));
                heap.release(&heap);

                fb->pixels = (FramebufferPixel *)fb_ptr;
                fb->width = fb_rec.fb.width;
                fb->height = fb_rec.fb.height;
                fb->pitch = fb_rec.fb.pitch;
                fb->format = fb_rec.fb.format;

                uintptr_t struct_ptr;

                if (mu_map(vspace, (MuCap){._raw = align_down((uintptr_t)fb, PAGE_SIZE)},
                           &struct_ptr, 0, align_up(sizeof(Framebuffer), PAGE_SIZE), MU_MEM_READ) != MU_RES_OK)
                {
                    debug_warn("Failed to map framebuffer structure\n");
                    return None(MaybeRcVec);
                }

                mu_args[idx++] = (MuArg){(uintptr_t)struct_ptr};
            }
        }

        if (args.type == JSON_ARRAY && args.array.len <= 6)
        {
            json_t arg;
            json_arr_foreach(&args, arg)
            {
                switch (arg.type)
                {
                    case JSON_OBJECT:
                        [[fallthrough]];
                    case JSON_ARRAY:
                        [[fallthrough]];
                    case JSON_ERROR:
                        [[fallthrough]];
                    case JSON_KEY:
                        return None(MaybeRcVec);
                    case JSON_NUMBER:
                        [[fallthrough]];
                    case JSON_BOOL:
                        mu_args[idx++] = arg.number;
                        break;
                    case JSON_STRING:
                        // mu_args[idx] = (uintptr_t)strdup(arg.string);

                        // if (mu_map(vspace, (MuCap){(uintptr_t)mu_args[idx]}, (uintptr_t)mu_args[idx], 0, strlen(arg.string), MU_MEM_USER | MU_MEM_READ) != MU_RES_OK)
                        // {
                        //     return None(MaybeRcVec);
                        // }

                        idx++;
                        break;
                    case JSON_NULL:
                        mu_args[idx++] = 0;
                        break;
                }
            }
        }

        RcEntry rc_entry = {
            .vspace = vspace,
            .path = str(path.string),
            .args = {(MuArg)port._raw, mu_args[0], mu_args[1], mu_args[2], mu_args[3], mu_args[4]},
        };

        vec_push(&rc_vec, rc_entry);
    }

    return Some(MaybeRcVec, rc_vec);
}

noreturn int mu_main(MuArgs args)
{
    handover = (HandoverPayload *)args.arg1;
    HandoverRecord rc;
    MuCap port;
    MuCap self_cap;
    MuTask *self;

    MuMsg *ipc_msg = mmap(NULL, sizeof(MuMsg), PROT_WRITE | PROT_READ, MAP_ANON | MAP_SHARED, -1, 0);
    if (ipc_msg == NULL)
    {
        panic("Couldn't allocate IPC buffer");
    }

    if (handover->magic != 0xB00B1E5)
    {
        panic("Invalid handover magic");
    }

    if (handover_find_file(&rc, "/etc/rc.json") != MU_RES_OK)
    {
        panic("Couldn't find /etc/rc.json file");
    }

    if (mu_create_port(&port, MU_PORT_SEND | MU_PORT_RECV) != MU_RES_OK)
    {
        panic("Couldn't create port");
    }

    json_reader_t reader = json_init((cstr)rc.start, rc.size);

    cleanup(json_free) json_t entries = json_parse(&reader);
    RcVec servers = unwrap(init_servers(entries, port));
    RcEntry server;
    HandoverRecord server_binary;

    vec_foreach(&servers, server)
    {
        if (handover_find_file(&server_binary, (cstr)server.path.buf) != MU_RES_OK)
        {
            panic("Couldn't find server binary");
        }

        ElfReturn task = unwrap(elf_parse((cstr)server.path.buf, server_binary.start, .vspace = server.vspace));
        mu_start(task.task, task.entry, USER_STACK_TOP, server.args);
    }

    vec_free(&servers);

    if (mu_self(&self_cap) != MU_RES_OK)
    {
        panic("Couldn't get bootstrap process informations");
    }

    self = (MuTask *)self_cap._raw;

    loop
    {
        if (mu_ipc(&port, ipc_msg, MU_MSG_RECV | MU_MSG_BLOCK) != MU_RES_OK)
        {
            panic("Couldn't receive message");
        }

        debug_info("Message address is: {a}", (uintptr_t)ipc_msg);

        switch (ipc_msg->label)
        {
            case BOOTSTRAP_REGISTER_SERVER:
            {
                uintptr_t server_name_ptr;

                if (mu_map(self->space, (MuCap){align_down(ipc_msg->args.arg1, PAGE_SIZE)}, &server_name_ptr, 0, align_up(ipc_msg->args.arg2, PAGE_SIZE), MU_MEM_READ) != MU_RES_OK)
                {
                    panic("Couldn't map server binary");
                }

                debug_info("Got registration for server {}", (cstr)server_name_ptr);

                break;
            }

            default:
            {
                debug_warn("Unknown message label: {d}", ipc_msg->label);
            }
        }
    }

    unreachable();
}