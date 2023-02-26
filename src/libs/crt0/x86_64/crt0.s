.text
.globl _start
.extern _entry

_start:
    call _entry
    ud2