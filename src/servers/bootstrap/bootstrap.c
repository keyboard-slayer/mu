#include <elf/elf.h>
#include <mu-api/api.h>
#include <mu-api/bootstrap.h>
#include <mu-mem/heap.h>
#include <tiny-json/json.h>

#include "rc.h"

static Module *mods = nullptr;
static usize len = 0;

static MaybeModule search_module(cstr name)
{
    for (usize i = 0; i < len; i++)
    {
        if (memcmp(mods[i].name, name, strlen(name)) == 0)
        {
            return Some(MaybeModule, mods[i]);
        }
    }

    return None(MaybeModule);
}

static MaybeRCVec init_servers(void)
{
    MuCap vspace;
    json_t item;
    json_t arg;
    MuArg mu_args[6];
    usize idx;
    RCVec rc_vec;

    vec_init(&rc_vec, heap_acquire);
    Module rc = unwrap(search_module("/etc/rc.json"));
    json_reader_t r = json_init((cstr)rc.ptr, rc.len);
    cleanup(json_free) json_t obj = json_parse(&r);

    if (obj.type != JSON_ARRAY)
    {
        return None(MaybeRCVec);
    }

    json_arr_foreach(&obj, item)
    {
        if (mu_create_vspace(&vspace) != MU_RES_OK || obj.array.buf[i].type != JSON_OBJECT)
        {
            return None(MaybeRCVec);
        }

        json_t json_entry = obj.array.buf[i];

        json_t path = json_get(json_entry, "path");
        json_t args = json_get(json_entry, "args");

        if (path.type != JSON_STRING || args.type != JSON_ARRAY || args.array.len > 6)
        {
            return None(MaybeRCVec);
        }

        memset(mu_args, 0, sizeof(mu_args));
        idx = 0;

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
                    return None(MaybeRCVec);
                case JSON_NUMBER:
                    [[fallthrough]];
                case JSON_BOOL:
                    mu_args[idx++] = arg.number;
                    break;
                case JSON_STRING:
                    mu_args[idx] = (uintptr_t)strdup(arg.string);
                    if (mu_map(vspace, (MuCap){(uintptr_t)mu_args[idx]}, 0, (uintptr_t)mu_args[idx], strlen(arg.string), MU_MEM_USER | MU_MEM_READ) != MU_RES_OK)
                    {
                        return None(MaybeRCVec);
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
            .args = {mu_args[0], mu_args[1], mu_args[2], mu_args[3], mu_args[4], mu_args[5]},
        };

        vec_push(&rc_vec, rc_entry);
    }

    return Some(MaybeRCVec, rc_vec);
}

int mu_main(MuArgs args)
{
    MuCap info;
    mods = (Module *)args.arg1;
    len = args.arg2;

    assert(len, "No modules received");
    assert(mu_self(&info) == MU_RES_OK, "Failed to get self info");

    auto servers = unwrap(init_servers());

    RcEntry entry;

    vec_foreach(&servers, entry)
    {
        Module mod = unwrap(search_module((cstr)entry.path.buf));
        unwrap(elf_parse((cstr)entry.path.buf, mod.ptr, entry.vspace._raw, entry.args));
    }

    return 0;
}