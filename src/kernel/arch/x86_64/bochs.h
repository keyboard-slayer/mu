#pragma once

// Documentation: https://bochs.sourceforge.io/doc/docbook/development/debugger-advanced.html

#include <traits/output.h>

#define PORT_E9 (0xe9)

#define BOCHS_COMMAND_REG (0x8a00)

#define BOCHS_ENABLE_DEVICE (0x8a00)
#define BOCHS_RETURN_PROMPT (0x8ae0)

void bochs_set_breakpoint(void);
Output bochs_out_acquire(void);