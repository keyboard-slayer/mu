#include <handover/handover.h>
#include <handover/utils.h>
#include <mu-api/api.h>
#include <mu-mem/heap.h>
#include <munix-debug/debug.h>
#include <munix-loader/elf.h>
#include <string.h>
#include <tiny-json/json.h>

#include "bootstrap.h"

static MuRes handover_find_file(HandoverPayload handover[static 1], HandoverRecord *rec, const char name[static 1])
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

static MaybeRcVec init_servers(json_t rc)
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

        if (path.type != JSON_STRING || args.type != JSON_ARRAY || args.array.len > 6)
        {
            return None(MaybeRcVec);
        }

        json_t arg;
        usize idx = 0;
        MuArg mu_args[6];

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
                    mu_args[idx] = (uintptr_t)strdup(arg.string);

                    if (mu_map(vspace, (MuCap){(uintptr_t)mu_args[idx]}, 0, (uintptr_t)mu_args[idx], strlen(arg.string), MU_MEM_USER | MU_MEM_READ) != MU_RES_OK)
                    {
                        return None(MaybeRcVec);
                    }

                    idx++;
                    break;
                case JSON_NULL:
                    mu_args[idx++] = 0;
                    break;
            }
        }

        RcEntry rc_entry = {
            .vspace = vspace,
            .path = str(path.string),
            .args = {(MuArg){0}, mu_args[0], mu_args[1], mu_args[2], mu_args[3], mu_args[4]},
        };

        vec_push(&rc_vec, rc_entry);
    }

    return Some(MaybeRcVec, rc_vec);
}

noreturn int mu_main(MuArgs args)
{
    HandoverPayload *payload = (HandoverPayload *)args.arg1;
    HandoverRecord rc;

    if (payload->magic != 0xB00B1E5)
    {
        panic("Invalid handover magic");
    }

    if (handover_find_file(payload, &rc, "/etc/rc.json") != MU_RES_OK)
    {
        panic("Couldn't find /etc/rc.json file");
    }

    json_reader_t reader = json_init((cstr)rc.start, rc.size);

    cleanup(json_free) json_t entries = json_parse(&reader);
    RcVec servers = unwrap(init_servers(entries));
    RcEntry server;
    HandoverRecord server_binary;

    vec_foreach(&servers, server)
    {
        if (handover_find_file(payload, &server_binary, (cstr)server.path.buf) != MU_RES_OK)
        {
            panic("Couldn't find server binary");
        }

        ElfReturn task = unwrap(elf_parse((cstr)server.path.buf, server_binary.start, .vspace = server.vspace));
        mu_start(task.task, task.entry, USER_STACK_BASE, server.args);
    }

    vec_free(&servers);
    loop;
    unreachable();
}