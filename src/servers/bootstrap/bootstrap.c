#include <handover/handover.h>
#include <handover/utils.h>
#include <munix-debug/debug.h>
#include <unistd.h>

static MuRes handover_find_file(HandoverPayload *handover, HandoverRecord *rec, unused cstr name) 
{
    handover_foreach_record(handover, *rec) 
    {
        if (rec->tag == HANDOVER_FILE)
        {
            cstr filename = (cstr) handover + rec->file.name;
            if (strcmp(name, filename) == 0)
            {
                return MU_RES_OK;
            }
        }
    }

    return MU_RES_BAD_ARG;
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

    debug_info("{}", (char const *)rc.start)

    loop;
    unreachable();
}