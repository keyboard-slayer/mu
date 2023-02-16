#include "ipc.h"

IpcBuffer *ipc_buffer_get(void)
{
    return (IpcBuffer *)IPC_STRUCT_POS;
}