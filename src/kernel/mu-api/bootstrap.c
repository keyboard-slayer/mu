#include <string.h>

#include "api.h"
#include "bootstrap.h"

void bootstrap_register_server(MuCap port, cstr name)
{
    MuCap bootstrap;
    MuMsg reg = mu_msg(BOOTSTRAP_REGISTER_SERVER, port, strdup(name));

    if (mu_bootstrap_port(&bootstrap) != MU_RES_OK)
    {
        debug_warn("Failed to get bootstrap port");
        return;
    }

    if (mu_ipc(&bootstrap, &reg, MU_IPC_SEND) != MU_RES_OK)
    {
        debug_warn("Failed to register server");
        return;
    }
}